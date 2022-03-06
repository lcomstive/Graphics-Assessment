#include <algorithm>
#include <Engine/Scene.hpp>
#include <Engine/Utilities.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Physics/PhysicsSystem.hpp>
#include <Engine/Components/Physics/Particle.hpp>
#include <Engine/Components/Physics/SphereCollider.hpp>

using namespace glm;
using namespace std;
using namespace Engine::Physics;
using namespace Engine::Components;

Particle::Particle() :
	m_Force(),
	m_Mass(1.0f),
	m_Velocity(),
	m_Radius(0.1f),
	IsTrigger(false),
	UseGravity(true),
	m_Friction(0.95f),
	m_Collider(nullptr),
	m_Restitution(0.7f),
	m_PreviousPosition()
{ }

PhysicsSystem& Particle::GetSystem() { return GetGameObject()->GetScene()->GetPhysics(); }

void Particle::ApplyForce(glm::vec3 force, ForceMode mode)
{
	if (m_IsStatic)
		return;

	switch (mode)
	{
	default:
	case ForceMode::Acceleration:
		m_Force += force;
		break;
	case ForceMode::Impulse:
		m_Velocity += force;
		break;
	}
}

float Particle::GetMass() { return m_Mass; }
void  Particle::SetMass(float mass) { m_Mass = std::clamp(mass, 0.0001f, FLT_MAX); }
float Particle::InverseMass() { return m_Mass == 0.0f ? 0.0f : 1.0f / m_Mass; }
float Particle::GetRestitution() { return m_Restitution; }
void  Particle::SetRestitution(float value) { m_Restitution= value; }
float Particle::GetFriction() { return m_Friction; }
void  Particle::SetFriction(float value) { m_Friction = value; }
vec3  Particle::GetVelocity() { return m_Velocity; }
void  Particle::SetCollisionRadius(float radius) { m_Collider->SetRadius(radius); }
bool  Particle::IsStatic() { return m_IsStatic; }
void  Particle::SetStatic(bool value) { m_IsStatic = value; }

void Particle::Added()
{
	PhysicsComponent::Added();

	m_PreviousPosition = GetTransform()->GetGlobalPosition();
	m_Collider = GetGameObject()->AddComponent<SphereCollider>();

	SetCollisionRadius(m_Radius);
}

void Particle::FixedUpdate(float timestep)
{
	if (m_IsStatic)
		return;

	Transform* transform = GetTransform();
	m_PreviousPosition = transform->GetGlobalPosition();

	vec3 acceleration = m_Force * InverseMass();
	m_Velocity = m_Velocity * m_Friction + (acceleration * timestep);

	transform->Position += m_Velocity * timestep;

	m_Force = vec3(0.0f);
	SolveConstraints(timestep);
}

void Particle::SolveConstraints(float timestep)
{
	if (m_IsStatic)
		return;

	Transform* transform = GetTransform();
	PhysicsSystem& system = GetSystem();

	Line line = { m_PreviousPosition, transform->GetGlobalPosition()};
	if (!system.LineTest(line, m_Collider))
		return; // No collisions will occur

	vec3 direction = normalize(m_Velocity);

	Ray ray;
	ray.Origin = m_PreviousPosition;
	ray.Direction = direction;

	RaycastHit hit;
	Collider* hitCollider = system.Raycast(ray, m_Collider, &hit);
	if(hitCollider)
	{
		transform->Position = hit.Point + hit.Normal * 0.001f;
		if (transform->GetParent())
			transform->Position -= transform->GetParent()->GetGlobalPosition(); // Global -> Local

		vec3 vn = hit.Normal * dot(hit.Normal, m_Velocity);
		vec3 vt = m_Velocity - vn;

		// Record current position to avoid tunnelling
		m_PreviousPosition = transform->GetGlobalPosition();
		m_Velocity = vt - vn * m_Restitution;
	}
}

void Particle::ApplyWorldForces(float timestep)
{
	ApplyForce(GetSystem().GetGravity() * m_Mass);
}