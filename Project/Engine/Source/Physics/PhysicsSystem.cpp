#include <chrono>
#include <functional>
#include <condition_variable>
#include <Engine/Application.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Physics/PhysicsSystem.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>

using namespace std;
using namespace glm;
using namespace std::chrono;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

const vec3 InitialGravity = { 0, -9.81f, 0 };

PhysicsSystem::PhysicsSystem(milliseconds fixedTimestep) :
	m_Thread(),
	m_Substeps(5),
	m_CollidersMutex(),
	m_LastTimestep(-1ms),
	m_ImpulseIteration(1),
	m_Broadphase(nullptr),
	m_Gravity(InitialGravity),
	m_FixedTimestep(fixedTimestep),
	m_ThreadState((int)(m_PhysicsState = PhysicsPlayState::Stopped))
{
	SetBroadphase<BasicBroadphase>();
}

PhysicsSystem::~PhysicsSystem()
{
	if (m_Broadphase)
		delete m_Broadphase;
}

void PhysicsSystem::SetTimestep(milliseconds timestep)
{
	lock_guard guard(m_VariableMutex);
	m_FixedTimestep = timestep;
}

void PhysicsSystem::SetGravity(vec3 gravity)
{
	lock_guard guard(m_VariableMutex);
	m_Gravity = gravity;
}

glm::vec3 PhysicsSystem::GetGravity() { return m_Gravity; }
std::chrono::milliseconds PhysicsSystem::LastTimestep() { return m_LastTimestep; }
milliseconds PhysicsSystem::Timestep() { return m_FixedTimestep; }

void PhysicsSystem::Start()
{
	m_ThreadState.store((int)(m_PhysicsState = PhysicsPlayState::Playing));
	m_Thread = thread(&PhysicsSystem::PhysicsLoop, this);
}

void PhysicsSystem::Stop()
{
	m_ThreadState.store((int)(m_PhysicsState = PhysicsPlayState::Stopped));
	m_PauseConditional.notify_one();

	if (m_Thread.joinable())
		m_Thread.join();
}

void PhysicsSystem::Resume()
{
	lock_guard lock(m_PhysicsStateMutex);
	m_ThreadState.store((int)(m_PhysicsState = PhysicsPlayState::Playing));
	m_PauseConditional.notify_one();
}

void PhysicsSystem::Pause()
{
	lock_guard lock(m_PhysicsStateMutex);
	m_ThreadState.store((int)(m_PhysicsState = PhysicsPlayState::Paused));
	m_PauseConditional.notify_one();
}

void PhysicsSystem::TogglePause()
{
	if (m_PhysicsState == PhysicsPlayState::Playing)
		Pause();
	else
		Resume();
}

PhysicsPlayState PhysicsSystem::GetState() { return m_PhysicsState; }

void PhysicsSystem::AddCollider(Collider* collider)
{
	lock_guard guard(m_CollidersMutex);
	m_Colliders.emplace_back(collider);

	if (m_Broadphase)
		m_Broadphase->Insert(collider);
}

void PhysicsSystem::RemoveCollider(Collider* collider)
{
	const auto& it = find(m_Colliders.begin(), m_Colliders.end(), collider);
	if (it != m_Colliders.end())
	{
		lock_guard guard(m_CollidersMutex);
		m_Colliders.erase(it);
	}

	if (m_Broadphase)
		m_Broadphase->Remove(collider);
}

void PhysicsSystem::AddPhysicsComponent(PhysicsComponent* rb)
{
	lock_guard guard(m_VariableMutex);
	m_Components.emplace_back(rb);
}

void PhysicsSystem::RemovePhysicsComponent(PhysicsComponent* rb)
{
	const auto& it = find(m_Components.begin(), m_Components.end(), rb);
	if (it != m_Components.end())
	{
		lock_guard guard(m_VariableMutex);
		m_Components.erase(it);
	}
}

void PhysicsSystem::PhysicsLoop()
{
	Log::Assert(m_Broadphase, "A broadphase is required! Use PhysicsSystem::SetBroadphase to set one");

	PhysicsPlayState currentState;
	while ((currentState = (PhysicsPlayState)m_ThreadState.load()) != PhysicsPlayState::Stopped)
	{
		if (currentState == PhysicsPlayState::Paused)
		{
			unique_lock lock(m_PhysicsStateMutex);
			m_PauseConditional.wait(lock, [&] { return m_PhysicsState != PhysicsPlayState::Paused; });
		}

		time_point timeStart = high_resolution_clock::now();
		float fixedTimestep = m_FixedTimestep.count() / 1000.0f;

		// Check for collisions
		m_Collisions = m_Broadphase->GetPotentialCollisions();
		NarrowPhase(); // Test potential collisions & generate manifolds

		for (PhysicsComponent* component : m_Components)
			component->ApplyWorldForces(fixedTimestep);

		ApplyImpulse();
		PositionalCorrect();

		for (PhysicsComponent* component : m_Components)
			component->FixedUpdate(fixedTimestep);

		// Apply forces
		for (PhysicsComponent* component : m_Components)
			component->ApplyForces(fixedTimestep);

		// Solve constraints
		for (PhysicsComponent* component : m_Components)
			component->SolveConstraints(fixedTimestep);

		m_LastTimestep = duration_cast<milliseconds>(high_resolution_clock::now() - timeStart);
		milliseconds remainingTime = m_FixedTimestep - m_LastTimestep;

		if (remainingTime.count() > 0)
			this_thread::sleep_for(remainingTime);
	}
}

void PhysicsSystem::PositionalCorrect()
{
	if (m_Collisions.empty())
		return;

	for (CollisionFrame collision : m_Collisions)
	{
		if (collision.A->IsTrigger || collision.B->IsTrigger)
			continue;

		float totalMass = (collision.ARigidbody ? collision.ARigidbody->InverseMass() : 0.0f) +
			(collision.BRigidbody ? collision.BRigidbody->InverseMass() : 0.0f);
		if (totalMass == 0.0f)
			continue;

		float depth = fmaxf(collision.Result.PenetrationDepth - m_PenetrationSlack, 0.0f);
		vec3 correction = collision.Result.Normal * (depth / totalMass) * m_LinearProjectionPercent;

		for (const auto& contact : collision.Result.Contacts)
		{
			if (collision.ARigidbody)
			{
				// collision.A->GetTransform()->Position -= correction * collision.ARigidbody->InverseMass();
				collision.ARigidbody->ApplyForce(-correction, contact, ForceMode::Impulse, true);
			}
			if (collision.BRigidbody)
			{
				// collision.B->GetTransform()->Position += correction * collision.BRigidbody->InverseMass();
				collision.BRigidbody->ApplyForce( correction, contact, ForceMode::Impulse, true);
			}
		}
	}
}

void PhysicsSystem::NarrowPhase()
{
	for (int i = (int)m_Collisions.size() - 1; i >= 0; i--)
	{
		m_Collisions[i].Result = FindCollisionFeatures(m_Collisions[i].A, m_Collisions[i].B);

		if (!m_Collisions[i].Result.IsColliding)
		{
			m_Collisions.erase(m_Collisions.begin() + i);
			continue;
		}

		if ((!m_Collisions[i].ARigidbody || m_Collisions[i].ARigidbody->IsStatic()) && m_Collisions[i].BRigidbody)
		{
			// A valid non-static rigidbody is expected to be in A

			Collider* temp = m_Collisions[i].A;
			m_Collisions[i].A = m_Collisions[i].B;
			m_Collisions[i].ARigidbody = m_Collisions[i].BRigidbody;
			m_Collisions[i].B = temp;
			m_Collisions[i].BRigidbody = nullptr;
			m_Collisions[i].Result.Normal *= -1.0f;
		}
	}
}

void PhysicsSystem::ApplyImpulse()
{
	// Apply linear impulse resolution
	for (int k = 0; k < m_ImpulseIteration; k++)
	{
		for (CollisionFrame collision : m_Collisions)
		{
			int jSize = (int)collision.Result.Contacts.size();
			for (int j = 0; j < jSize; j++)
			{
				if (collision.A->IsTrigger)
				{
					collision.A->m_CurrentTriggerEntries.emplace_back(collision.B);
					continue;
				}

				if (collision.B->IsTrigger)
				{
					collision.B->m_CurrentTriggerEntries.emplace_back(collision.A);
					continue;
				}

				if (collision.BRigidbody && !collision.BRigidbody->IsStatic())
					collision.ARigidbody->ApplyImpulse(collision.BRigidbody, collision.Result, j);
				else // RB -> Static Collider
					collision.ARigidbody->ApplyImpulse(collision.B, collision.Result, j);
			}
		}
	}

	for (const auto& collider : m_Colliders)
		if (collider->IsTrigger)
			collider->ProcessTriggerEntries();
}

Collider* PhysicsSystem::Raycast(Ray ray, RaycastHit* outResult) { return Raycast(ray, nullptr, outResult); }
vector<Collider*> PhysicsSystem::Query(AABB& bounds) { return m_Broadphase ? m_Broadphase->Query(bounds) : vector<Collider*>(); }
vector<Collider*> PhysicsSystem::Query(Sphere& bounds) { return m_Broadphase ? m_Broadphase->Query(bounds) : vector<Collider*>(); }
bool PhysicsSystem::LineTest(Line line, Engine::Components::Collider* ignoreCollider) { return m_Broadphase ? m_Broadphase->LineTest(line, ignoreCollider) : false; }
Collider* PhysicsSystem::Raycast(Ray ray, Collider* ignoreCollider, RaycastHit* outResult) { return m_Broadphase ? m_Broadphase->Raycast(ray, ignoreCollider, outResult) : nullptr; }

#define DRAW_CONTACTS 1
void PhysicsSystem::DrawGizmos()
{
#if !defined(NDEBUG) && DRAW_CONTACTS
	Gizmos::SetColour(0, 0, 1, 1);
	for (CollisionFrame& collision : m_Collisions)
	{
		for (vec3& contact : collision.Result.Contacts)
			Gizmos::DrawSphere(contact, .05f);
	}
#endif

	if (m_Broadphase)
		m_Broadphase->DrawGizmos();
}
