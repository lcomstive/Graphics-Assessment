#include <Engine/Utilities.hpp> // Magnitude
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Components/Physics/Constraints/DistanceJoint.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Graphics;
using namespace Engine::Components;

float DistanceJoint::GetLength() { return m_Length; }
void DistanceJoint::SetLength(float length) { m_Length = std::max(length, 0.1f); }

void DistanceJoint::Initialise(Particle* a, Particle* b, float length)
{
	m_Body1 = a;
	m_Body2 = b;
	m_Length = length;
}

void DistanceJoint::SolveConstraints(float timestep)
{
	Transform* tA = m_Body1->GetTransform();
	Transform* tB = m_Body2->GetTransform();
	vec3 delta = tB->Position - tA->Position;
	float distance = Magnitude(delta);
	float correction = (distance - m_Length) / distance;

	if(!m_Body1->IsStatic()) tA->Position += delta * (m_Body2->IsStatic() ? 1.0f : 0.5f) * correction;
	if(!m_Body2->IsStatic()) tB->Position -= delta * (m_Body1->IsStatic() ? 1.0f : 0.5f) * correction;

	// Try to prevent tunnelling
	m_Body1->SolveConstraints(timestep);
	m_Body2->SolveConstraints(timestep);
}

void DistanceJoint::DrawGizmos()
{
#ifndef NDEBUG
	// Draw joint between two points
	Gizmos::SetColour(0, 0, 1, 1);
	Gizmos::DrawLine(m_Body1->GetTransform()->Position, m_Body2->GetTransform()->Position);
#endif
}