#include <glm/gtx/euler_angles.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Components/Physics/BoxCollider.hpp>
#include <Engine/Components/Physics/PlaneCollider.hpp>
#include <Engine/Components/Physics/SphereCollider.hpp>

using namespace glm;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

void SphereCollider::Added()
{
	Collider::Added();

	SetRadius(m_Radius);
	m_Sphere.Position = m_Bounds.Position = GetTransform()->GetGlobalPosition() + Offset;
}

void SphereCollider::DrawGizmos()
{
#ifndef NDEBUG
	Gizmos::Colour = { 0, 1, 0, 1 };
	Gizmos::DrawWireSphere(m_Sphere.Position, m_Radius);
	// Gizmos::DrawWireCube(m_Bounds.Position, m_Bounds.Extents);
#endif
}

float& SphereCollider::GetRadius() { return m_Radius; }
void SphereCollider::SetRadius(float radius)
{
	m_Bounds.Extents = vec3(radius);
	m_Sphere.Radius = m_Radius = radius;
	CalculateInverseTensor();
}

OBB& SphereCollider::GetBounds() { return m_Bounds; };
Sphere& SphereCollider::GetSphere() { return m_Sphere; }
bool SphereCollider::LineTest(Line& line) { return m_Sphere.LineTest(line); }
bool SphereCollider::IsPointInside(glm::vec3& point) const { return m_Sphere.IsPointInside(point); }
bool SphereCollider::Raycast(Ray& ray, RaycastHit* outResult) { return m_Sphere.Raycast(ray, outResult); }

void SphereCollider::FixedUpdate(float timestep)
{
	Transform* transform = GetTransform();
	m_Sphere.Position = m_Bounds.Position = transform->GetGlobalPosition() + Offset;
}

// https://scienceworld.wolfram.com/physics/MomentofInertiaSphere.html
void SphereCollider::CalculateInverseTensor()
{
	Rigidbody* rb = GetRigidbody();
	float mass = rb ? rb->GetMass() : 0.0f;
	if (!rb || mass == 0.0f)
	{
		m_InverseTensor = mat4(0.0f);
		return;
	}

	float r2 = m_Radius * m_Radius;
	float fraction = (2.0f / 5.0f);
	float i = r2 * mass * fraction;

	m_InverseTensor = inverse(mat4(
		i, 0, 0, 0,
		0, i, 0, 0,
		0, 0, i, 0,
		0, 0, 0, i
	));
}
mat4& SphereCollider::InverseTensor() { return m_InverseTensor; }

bool SphereCollider::CheckCollision(Collider* other) { return other->CheckCollision(this); }

bool SphereCollider::CheckCollision(BoxCollider* other)
{
	return TestSphereBoxCollider(
		m_Sphere,
		other->GetOBB()
	);
}

bool SphereCollider::CheckCollision(SphereCollider* other)
{
	return TestSphereSphereCollider(
		m_Sphere,
		other->m_Sphere
	);
}

bool SphereCollider::CheckCollision(PlaneCollider* other)
{
	return TestSpherePlaneCollider(
		m_Sphere,
		other->GetPlane()
	);
}