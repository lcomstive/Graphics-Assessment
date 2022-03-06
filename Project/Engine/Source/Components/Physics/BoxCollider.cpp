#include <glm/gtx/euler_angles.hpp>
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

void BoxCollider::Added()
{
	Collider::Added();

	Transform* transform = GetTransform();
	m_Bounds.Position = transform->GetGlobalPosition() + Offset;
	m_Bounds.Extents = m_Extents * (m_PreviousScale = transform->GetGlobalScale());

	vec3 rotation = transform->GetGlobalRotation();
	m_Bounds.Orientation = eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

	CalculateInverseTensor();
}

void BoxCollider::DrawGizmos()
{
#ifndef NDEBUG
	Transform* transform = GetTransform();
	Gizmos::Colour = { 0, 1, 0, 1 };
	Gizmos::DrawWireCube(
		m_Bounds.Position,
		m_Bounds.Extents,
		m_Bounds.Orientation);

	/*
	Gizmos::Colour = { 1, 0, 1, 0.75f };
	vector<vec3>& vertices = m_Bounds.GetVertices();
	for (vec3& vertex : vertices)
		Gizmos::DrawSphere(vertex, 0.05f);

	Gizmos::Colour = { 1, 1, 0, 0.75f };
	vector<Line>& edges = m_Bounds.GetEdges();
	for (Line& edge : edges)
		Gizmos::DrawLine(edge.Start, edge.End);
	*/
#endif
}

OBB& BoxCollider::GetOBB() { return m_Bounds; }

glm::vec3& BoxCollider::GetExtents() { return m_Extents; }
void BoxCollider::SetExtents(glm::vec3 value)
{
	m_Extents = value;
	CalculateInverseTensor();
}

void BoxCollider::FixedUpdate(float timestep)
{
	Transform* transform = GetTransform();
	m_Bounds.Position = transform->GetGlobalPosition() + Offset;

	vec3 rotation = transform->GetGlobalRotation();
	m_Bounds.Orientation = eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

	vec3 scale = transform->GetGlobalScale();
	if (m_PreviousScale != scale)
	{
		m_PreviousScale = scale;
		m_Bounds.Extents = m_Extents * scale;
		CalculateInverseTensor();
	}
}

bool BoxCollider::LineTest(Physics::Line& line) { return GetOBB().LineTest(line); }
bool BoxCollider::Raycast(Ray& ray, RaycastHit* outResult) { return GetOBB().Raycast(ray, outResult); }

OBB& BoxCollider::GetBounds() { return m_Bounds; }
mat4& BoxCollider::InverseTensor() { return m_InverseTensor; }
bool BoxCollider::IsPointInside(glm::vec3& point) const { return m_Bounds.IsPointInside(point); }

bool BoxCollider::CheckCollision(Collider* other) { return other->CheckCollision(this); }

bool BoxCollider::CheckCollision(BoxCollider* other)
{
	return TestBoxBoxCollider(
		m_Bounds,
		other->m_Bounds
	);
}

bool BoxCollider::CheckCollision(SphereCollider* other)
{
	return TestSphereBoxCollider(
		other->GetSphere(),
		m_Bounds
	);
}

bool BoxCollider::CheckCollision(PlaneCollider* other)
{
	return TestBoxPlaneCollider(
		m_Bounds,
		other->GetPlane()
	);
}

void BoxCollider::CalculateInverseTensor()
{
	Rigidbody* rb = GetRigidbody();
	float mass = 0.0f;
	if (!rb || (mass = rb->GetMass()) == 0.0f)
	{
		m_InverseTensor = mat4(0.0f);
		return;
	}

	vec3 size = m_Bounds.Extents * 2.0f;
	float fraction = (1.0f / 12.0f);

	float x2 = size.x * size.x;
	float y2 = size.y * size.y;
	float z2 = size.z * size.z;

	m_InverseTensor = inverse(mat4(
		(y2 + z2) * mass * fraction, 0, 0, 0,
		0, (x2 + z2) * mass * fraction, 0, 0,
		0, 0, (x2 + y2) * mass * fraction, 0,
		0, 0, 0, 1.0f
	));
}
