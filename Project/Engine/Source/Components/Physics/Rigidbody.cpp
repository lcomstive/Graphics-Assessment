#include <algorithm>
#include <Engine/Scene.hpp>
#include <Engine/Utilities.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Physics/PhysicsSystem.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Physics;
using namespace Engine::Components;

Rigidbody::Rigidbody() :
	m_Force(),
	m_CoR(0.5f),
	m_Mass(1.0f),
	m_Drag(0.1f),
	m_Velocity(),
	m_Torque(0.0f),
	IsTrigger(false),
	UseGravity(true),
	m_Friction(0.6f),
	m_Sleeping(false),
	m_IsStatic(false),
	m_AngularVelocity()
{ }

PhysicsSystem& Rigidbody::GetSystem() { return GetGameObject()->GetScene()->GetPhysics(); }
float Rigidbody::GetMass() { return m_IsStatic ? 0.0f : m_Mass; }
void  Rigidbody::SetMass(float mass) { m_Mass = std::clamp(mass, 0.0f, FLT_MAX); }
float Rigidbody::GetRestitution() { return m_CoR; }
void  Rigidbody::SetRestitution(float value) { m_CoR = value; }
float Rigidbody::GetFriction() { return m_Friction; }
void  Rigidbody::SetFriction(float value) { m_Friction = value; }
bool Rigidbody::IsStatic() { return m_IsStatic; }
void Rigidbody::SetStatic(bool isStatic) { m_IsStatic = isStatic; }
vec3 Rigidbody::GetVelocity() { return m_Velocity; }

float Rigidbody::InverseMass() { return (m_IsStatic || m_Mass <= 0.0f) ? 0.0f : (1.0f / m_Mass); }
float Rigidbody::KineticEnergy() { return m_Mass * Magnitude(m_Velocity) * 0.5f; }
float Rigidbody::PotentialEnergy() { return m_Mass * dot(GetSystem().GetGravity(), GetTransform()->Position); }

const mat4 DefaultTensor = inverse(mat4(0.0f));
mat4& Rigidbody::InverseTensor()
{
	Collider* collider = GetGameObject()->GetComponent<Collider>(true);
	return collider ? collider->InverseTensor() : (mat4&)DefaultTensor;
}

void Rigidbody::ApplyForce(glm::vec3 force, ForceMode mode)
{
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

void Rigidbody::ApplyForce(glm::vec3 force, glm::vec3 position, ForceMode mode, bool globalForcePosition)
{
	ApplyForce(force, mode);
	AddRotationalImpulse(position, force, globalForcePosition);
}

void Rigidbody::AddRotationalImpulse(vec3 point, vec3 impulse, bool globalPoint)
{
	vec3 CoM = globalPoint ? GetTransform()->GetGlobalPosition() : GetTransform()->Position; // Center of Mass
	vec3 torque = cross(point - CoM, impulse);
	m_AngularVelocity += vec3(InverseTensor() * vec4(torque, 1.0f));
}

void Rigidbody::ApplyWorldForces(float timestep)
{
	// ApplyForce(GetSystem().GetGravity() * m_Mass);
}

void Rigidbody::CheckSleeping()
{
	m_Sleeping = CanSleep && MagnitudeSqr(m_Velocity) < InverseMass() &&
		MagnitudeSqr(m_AngularVelocity) < 0.005f;
}

void Rigidbody::FixedUpdate(float timestep)
{
	if(m_IsStatic)
		return;
	
	Transform* transform = GetTransform();

	const float damping = 0.95f;
#if 0 // Implicit-Euler Integration
	ApplyForce(GetSystem().GetGravity() * m_Mass);

	// Linear motion
	vec3 acceleration = m_Force * InverseMass();
	m_Velocity = (acceleration * timestep) + m_Velocity * damping;

	// Angular motion
	vec3 angAccel = vec3(vec4(m_Torque, 1.0f) * InverseTensor());
	m_AngularVelocity += (angAccel * timestep) * damping;

	m_Force = vec3(0.0f);
	CheckSleeping();
	if (m_Sleeping && CanSleep)
		return;

	transform->Position += m_Velocity * timestep;
	transform->Rotation += m_AngularVelocity * timestep;
#else // Verlet Integration
	vec3 dragForce = 0.5f * m_Drag * (m_Velocity * abs(m_Velocity));
	vec3 dragAcc = dragForce / m_Mass;
	vec3 acceleration = GetSystem().GetGravity() - dragAcc;

	transform->Position += m_Velocity * timestep + m_Force * (timestep * timestep * 0.5f);
	m_Velocity += (m_Force + acceleration) * timestep * 0.5f;
	m_Force = acceleration;

	// Angular motion
	vec3 angAccel = vec3(vec4(m_Torque, 1.0f) * InverseTensor());
	m_AngularVelocity += (angAccel * timestep) * damping;
	transform->Rotation += m_AngularVelocity * timestep;

	CheckSleeping();
#endif
}

// Linear impulse, assumes neither object is static
void Rigidbody::ApplyImpulse(Rigidbody* other, CollisionManifold manifold, int contactIndex)
{
	float invMass1 = InverseMass();
	float invMass2 = other->InverseMass();
	float invMassSum = invMass1 + invMass2;
	if (BasicallyZero(invMassSum))
		return;

	// Contact points relative to center of mass
	vec3 r1 = manifold.Contacts[contactIndex] - GetTransform()->Position;
	vec3 r2 = manifold.Contacts[contactIndex] - other->GetTransform()->Position;

	// Store inverse tensors
	const mat4& i1 = InverseTensor();
	const mat4& i2 = other->InverseTensor();

	vec3 relativeVel = (other->m_Velocity + cross(other->m_AngularVelocity, r2)) -
						(m_Velocity + cross(m_AngularVelocity, r1));
	vec3 relativeNorm = normalize(manifold.Normal);

	if (dot(relativeVel, relativeNorm) > 0)
		return; // Moving away from each other? Ignore

	float e = fminf(m_CoR, other->m_CoR);
	float numerator = -(1.0f + e) * dot(relativeVel, relativeNorm);
	float d1 = invMassSum;
	vec3 d2 = cross(vec3(i1 * vec4(cross(r1, relativeNorm), 1.0f)), r1);
	vec3 d3 = cross(vec3(i2 * vec4(cross(r2, relativeNorm), 1.0f)), r2);
	float denominator = d1 + dot(relativeNorm, d2 + d3);
	float j = (denominator == 0.0f) ? 0.0f : numerator / denominator; // Magnitude of impulse to apply
	if (!manifold.Contacts.empty() && j != 0.0f)
		j /= (float)manifold.Contacts.size();

	vec3 impulse = relativeNorm * j;
	m_Velocity		  -= impulse * invMass1;
	other->m_Velocity += impulse * invMass2;
	m_AngularVelocity		 -= vec3(i1 * vec4(cross(r1, impulse), 1.0f));
	other->m_AngularVelocity += vec3(i2 * vec4(cross(r2, impulse), 1.0f));

	// Call collision event
	Collider* thisCollider = GetGameObject()->GetComponent<Collider>(true);
	Collider* otherCollider = other->GetGameObject()->GetComponent<Collider>(true);
	if( thisCollider->m_CollisionEvent) thisCollider->m_CollisionEvent(otherCollider, other);
	if(otherCollider->m_CollisionEvent)	otherCollider->m_CollisionEvent(thisCollider, this);

	// Tangent to normal, used to apply friction
	vec3 t = relativeVel - (relativeNorm * dot(relativeVel, relativeNorm));
	if (BasicallyZero(MagnitudeSqr(t)))
		return;
	t = normalize(t);

	// Get magnitude of friction to apply
	numerator = -dot(relativeVel, t);
	d1 = invMassSum;
	d2 = cross(vec3(i1 * vec4(cross(r1, t), 1.0f)), r1);
	d3 = cross(vec3(i2 * vec4(cross(r2, t), 1.0f)), r2);
	denominator = d1 + dot(t, d2 + d3);

	float jt = denominator == 0.0f ? 0.0f : numerator / denominator;
	if (!manifold.Contacts.empty() && jt != 0.0f)
		jt /= (float)manifold.Contacts.size();
	if (BasicallyZero(jt))
		return;

	float friction = sqrtf(m_Friction * other->m_Friction);
	jt = std::clamp(jt, -j * friction, j * friction);  // Coulumb's Law
	
	vec3 tangentImpulse = t * jt;
	m_Velocity		  -= tangentImpulse * invMass1;
	other->m_Velocity += tangentImpulse * invMass2;
	m_AngularVelocity		 -= vec3(i1 * vec4(cross(r1, tangentImpulse), 1.0f));
	other->m_AngularVelocity += vec3(i2 * vec4(cross(r2, tangentImpulse), 1.0f));
}

void Rigidbody::ApplyImpulse(Collider* other, CollisionManifold manifold, int contactIndex)
{
	// `other` does not have a rigidbody, or the rigidbody is static
		
	float invMass1 = InverseMass();
	if (BasicallyZero(invMass1))
		return;

	// Contact points relative to center of mass
	vec3 r1 = manifold.Contacts[contactIndex] - GetTransform()->GetGlobalPosition();
	vec3 r2 = manifold.Contacts[contactIndex] - other->GetTransform()->GetGlobalPosition();

	// Store inverse tensors
	mat4& i1 = InverseTensor();
	mat4& i2 = other->InverseTensor();

	vec3 relativeVel = -(m_Velocity + cross(m_AngularVelocity, r1));
	vec3 relativeNorm = normalize(manifold.Normal);

	if (dot(relativeVel, relativeNorm) > 0)
		return; // Moving away from each other? Ignore

	float e = m_CoR;

	float numerator = -(1.0f + e) * dot(relativeVel, relativeNorm);
	float d1 = invMass1;
	vec3 d2 = cross(vec3(i1 * vec4(cross(r1, relativeNorm), 1.0f)), r1);
	vec3 d3 = cross(vec3(i2 * vec4(cross(r2, relativeNorm), 1.0f)), r2);
	float denominator = d1 + dot(relativeNorm, d2 + d3);
	float j = (denominator == 0.0f) ? 0.0f : numerator / denominator; // Magnitude of impulse to apply
	if (!manifold.Contacts.empty() && j != 0.0f)
		j /= (float)manifold.Contacts.size();

	vec3 impulse = relativeNorm * j;
	m_Velocity -= impulse * invMass1;
	m_AngularVelocity -= vec3(i1 * vec4(cross(r1, impulse), 1.0f));

	// Call collision event
	Collider* thisCollider = GetGameObject()->GetComponent<Collider>(true);
	if(thisCollider->m_CollisionEvent)
		thisCollider->m_CollisionEvent(other, nullptr);

	// Tangent to normal, used to apply friction
	vec3 t = relativeVel - (relativeNorm * dot(relativeVel, relativeNorm));
	if (BasicallyZero(MagnitudeSqr(t)))
		return;
	t = normalize(t);

	// Get magnitude of friction to apply
	numerator = -dot(relativeVel, t);
	d1 = invMass1;
	d2 = cross(vec3(i1 * vec4(cross(r1, t), 1.0f)), r1);
	d3 = cross(vec3(i2 * vec4(cross(r2, t), 1.0f)), r2);
	denominator = d1 + dot(t, d2 + d3);
	if (denominator == 0.0f)
		return;

	float jt = numerator / denominator;
	if (!manifold.Contacts.empty() && jt != 0.0f)
		jt /= (float)manifold.Contacts.size();
	if (BasicallyZero(jt))
		return;

	float friction = sqrtf(m_Friction);
	jt = std::clamp(jt, -j * friction, j * friction);  // Coulumb's Law

	vec3 tangentImpulse = t * jt;
	m_Velocity -= tangentImpulse * invMass1;
	m_AngularVelocity -= vec3(i1 * vec4(cross(r1, tangentImpulse), 1.0f));
}
