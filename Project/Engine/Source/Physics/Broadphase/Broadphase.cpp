#include <Engine/Utilities.hpp> // For MagnitudeSqr
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Physics/Broadphase/Broadphase.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Physics;
using namespace Engine::Components;

void BasicBroadphase::Insert(Collider* collider) { m_Colliders.emplace_back(collider); }

void BasicBroadphase::Remove(Collider* collider)
{
	const auto& it = find(m_Colliders.begin(), m_Colliders.end(), collider);
	if (it != m_Colliders.end())
		m_Colliders.erase(it);
}

Collider* BasicBroadphase::GetCollider(vec3& point)
{
	for (Collider* collider : m_Colliders)
		if (collider->IsPointInside(point))
			return collider;
	return nullptr;
}

std::vector<CollisionFrame>& BasicBroadphase::GetPotentialCollisions()
{
	m_Collisions.clear();
	m_Collisions.reserve(m_Colliders.size() * 2); // Assume every object is colliding with something

	auto colliders = m_Colliders; // Make local scoped copy
	for (uint32_t i = 0, size = (uint32_t)colliders.size(); i < size; i++)
	{
		Rigidbody* aRb = colliders[i]->GetRigidbody();
		OBB& bounds = colliders[i]->GetBounds();
		float magnitude = MagnitudeSqr(bounds.Extents);
		for (uint32_t j = i + 1; j < size; j++)
		{
			OBB& bounds2 = colliders[j]->GetBounds();

			float dist = distance(bounds.Position, bounds2.Position);
			if (dist * dist > magnitude)
				continue;

			Rigidbody* bRb = colliders[j]->GetRigidbody();
			if (((aRb && !aRb->IsStatic()) || (bRb && !bRb->IsStatic())) &&
				TestBoxBoxCollider(bounds, bounds2))
				m_Collisions.emplace_back(CollisionFrame
					{
						colliders[i],
						aRb,
						colliders[j],
						bRb
					});
		}
	}

	return m_Collisions;
}

vector<Collider*> BasicBroadphase::Query(Sphere& bounds) const
{
	vector<Collider*> output;
	auto colliders = m_Colliders;

	for (int i = 0; i < colliders.size(); i++)
	{
		if (TestSphereBoxCollider(bounds, colliders[i]->GetBounds()))
			output.emplace_back(colliders[i]);
	}
	return output;
}

vector<Collider*> BasicBroadphase::Query(AABB& bounds) const
{
	vector<Collider*> output;
	auto colliders = m_Colliders;

	for (int i = 0; i < colliders.size(); i++)
	{
		if (TestBoxBoxCollider(bounds, colliders[i]->GetBounds()))
			output.emplace_back(colliders[i]);
	}
	return output;
}

bool BasicBroadphase::LineTest(Line& line, Collider* ignoreCollider)
{
	Collider* closest = nullptr;
	RaycastHit hit = {}, closestHit = {};

	auto colliders = m_Colliders;
	for (int i = 0; i < colliders.size(); i++)
	{
		if (!colliders[i] ||
			colliders[i] == ignoreCollider ||
			!colliders[i]->LineTest(line) ||
			(closest && hit.Distance >= closestHit.Distance))
			continue;
		closest = colliders[i];
		closestHit = hit;
	}

	return closest;
}

Collider* BasicBroadphase::Raycast(Ray ray, Collider* ignoreCollider, RaycastHit* outResult) const
{
	Collider* closest = nullptr;
	RaycastHit hit = {}, closestHit = {};

	auto colliders = m_Colliders;
	for (int i = 0; i < colliders.size(); i++)
	{
		if (colliders[i] == ignoreCollider ||
			!colliders[i]->Raycast(ray, &hit) ||
			(closest && hit.Distance >= closestHit.Distance))
			continue;
		closest = colliders[i];
		closestHit = hit;
	}

	if (outResult)
		*outResult = closestHit;
	return closest;
}
