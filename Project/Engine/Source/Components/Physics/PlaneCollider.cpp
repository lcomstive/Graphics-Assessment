#include <Engine/Utilities.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <Engine/Physics/Shapes.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Components/Physics/BoxCollider.hpp>
#include <Engine/Components/Physics/PlaneCollider.hpp>
#include <Engine/Components/Physics/SphereCollider.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

const vec3 Extents = vec3(10000, 10000, 10000);

PlaneCollider::PlaneCollider(glm::vec3 normal, float distance)
{
	SetNormal(normal);
	SetDistance(distance);
}

void PlaneCollider::DrawGizmos()
{
#ifndef NDEBUG
	Gizmos::Colour = { 0, 1, 0, 1 };
	Gizmos::DrawWireQuad(
		m_Plane.Normal * m_Plane.Distance,
		{ 100.0f, 100.0f },
		RotationFromDirection(m_Plane.Normal)
	);
#endif
}

OBB& PlaneCollider::GetBounds() { return m_Bounds; }
Plane& PlaneCollider::GetPlane() { return m_Plane; }

float& PlaneCollider::GetDistance() { return m_Plane.Distance; }
void PlaneCollider::SetDistance(float value)
{
	m_Plane.Distance = value;
	m_Bounds.Position = m_Plane.Normal * m_Plane.Distance;
}

glm::vec3& PlaneCollider::GetNormal() { return m_Plane.Normal; }
void PlaneCollider::SetNormal(glm::vec3 value)
{
	m_Bounds.Position = m_Plane.Normal * m_Plane.Distance;
	m_Bounds.Extents = m_Plane.Normal * Extents;
	m_Bounds.Orientation = lookAt(vec3(0), m_Plane.Normal, { 0, 1, 0 });
}

bool PlaneCollider::LineTest(Line& line) { return m_Plane.LineTest(line); }
bool PlaneCollider::Raycast(Ray& ray, RaycastHit* outResult) { return m_Plane.Raycast(ray, outResult); }
bool PlaneCollider::IsPointInside(glm::vec3& point) const { return m_Plane.CheckPoint(point) == PlaneIntersection::Intersecting; }

bool PlaneCollider::CheckCollision(Collider* other) { return other->CheckCollision(this); }

bool PlaneCollider::CheckCollision(BoxCollider* other)
{
	return TestBoxPlaneCollider(
		other->GetOBB(),
		m_Plane
	);
}

bool PlaneCollider::CheckCollision(SphereCollider* other)
{
	return TestSpherePlaneCollider(
		other->GetSphere(),
		m_Plane
	);
}

bool PlaneCollider::CheckCollision(PlaneCollider* other)
{
	return TestPlanePlaneCollider(
		m_Plane,
		other->m_Plane
	);
}
