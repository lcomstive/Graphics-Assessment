#include <Engine/Utilities.hpp> // Magnitude
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Components/Physics/Constraints/Spring.hpp>

using namespace glm;
using namespace Engine::Graphics;
using namespace Engine::Components;

SPRING_PARTICLE_TYPE* Spring::GetPoint1() { return m_Point1; }
SPRING_PARTICLE_TYPE* Spring::GetPoint2() { return m_Point2; }

void Spring::SetBodies(SPRING_PARTICLE_TYPE* a, SPRING_PARTICLE_TYPE* b)
{
	m_Point1 = a;
	m_Point2 = b;
}

void Spring::SetConstants(float stiffness, float dampening)
{
	m_Stiffness = std::max(stiffness, 0.0f);
	m_Dampening = std::clamp(dampening, 0.0f, 1.0f);
}

float Spring::GetRestingLength() { return m_RestingLength; }
void  Spring::SetRestingLength(float length) { m_RestingLength = length; }

void Spring::ApplyForces(float timestep)
{
	vec3 point1 = m_Point1->GetTransform()->GetGlobalPosition();
	vec3 point2 = m_Point2->GetTransform()->GetGlobalPosition();

	float length = distance(point1, point2);
	vec3 direction = normalize(point1 - point2);

	vec3 relVel = m_Point2->GetVelocity() - m_Point1->GetVelocity();

#if SPRING_USE_RIGIDBODIES
	relVel *= -1.0f; // Honestly not sure why
#endif

	vec3 force = direction * m_Stiffness * (m_RestingLength - length) - m_Dampening * relVel;

	// Cap the force to prevent numerical instability
	const float MaxForce = 1000.0f; // Newtons
	float forceMagnitude = Magnitude(force);
	if (forceMagnitude > MaxForce)
		force *= MaxForce / forceMagnitude;

	m_Point1->ApplyForce( force * timestep, ForceMode::Impulse);
	m_Point2->ApplyForce(-force * timestep, ForceMode::Impulse);
}

void Spring::DrawGizmos()
{
#ifndef NDEBUG
	Gizmos::Colour = { 0, 1, 0, 1 };
	Gizmos::DrawLine(m_Point1->GetTransform()->GetGlobalPosition(), m_Point2->GetTransform()->GetGlobalPosition());
#endif
}