/**
*
* THIS CLASS IS BROKEN AND UNUSED.
*
* LEFT IN PROJECT FOR FUTURE REPAIRS.
*
**/

#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Physics/Broadphase/BroadphaseOctree.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

BroadphaseOctree::BroadphaseOctree(unsigned int maxDepth, float boundsWidth) : m_MaxDepth(maxDepth)
{
	m_Root.Bounds.Extents = vec3(boundsWidth / 2.0f);
}

void BroadphaseOctree::Insert(Collider* collider)
{
	m_Root.Insert(collider);
	m_Dirty = true;
}

void BroadphaseOctree::Remove(Collider* collider)
{
	m_Root.Insert(collider);
	m_Dirty = true;
}

void BroadphaseOctree::SetMaxDepth(unsigned int maxDepth)
{
	m_MaxDepth = maxDepth;
	m_Dirty = true;
}

void DrawOctreeNode(OctreeNode& node)
{
	if (!node.Children)
	{
		Gizmos::Colour = { 0, 0, 1, 1 };
		Gizmos::DrawWireCube(node.Bounds.Position, node.Bounds.Extents);
		return;
	}

	for (int i = 0; i < 8; i++)
		DrawOctreeNode(node.Children[i]);
}

void BroadphaseOctree::DrawGizmos() { DrawOctreeNode(m_Root); }

Collider* GetCollider(OctreeNode& node, vec3& point)
{
	if (!node.Children)
	{
		for (uint32_t i = 0; i < (uint32_t)node.Colliders.size(); i++)
		{
			if (node.Colliders[i]->GetBounds().IsPointInside(point))
				return node.Colliders[i];
		}
	}
	else
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			Collider* collider = GetCollider(node.Children[i], point);
			if (collider)
				return collider;
		}
	}
	return nullptr;
}

Collider* BroadphaseOctree::GetCollider(vec3& point) { return ::GetCollider(m_Root, point); }

void BroadPhase(OctreeNode& node, vector<CollisionFrame>& collisions)
{
	if (node.Children)
	{
		for(int i = 0; i < 8; i++)
			BroadPhase(node.Children[i], collisions);
		return;
	}

	// Leaf node
	for (uint32_t i = 0, size = (uint32_t)node.Colliders.size(); i < size; i++)
	{
		Rigidbody* aRb = node.Colliders[i]->GetRigidbody();
		OBB& bounds = node.Colliders[i]->GetBounds();
		for (uint32_t j = i; j < size; j++)
		{
			OBB& bounds2 = node.Colliders[j]->GetBounds();
			Rigidbody* bRb = node.Colliders[j]->GetRigidbody();
			if (((aRb && !aRb->IsStatic()) || (bRb && !bRb->IsStatic())) &&
				TestBoxBoxCollider(bounds, bounds2))
				collisions.emplace_back(CollisionFrame
					{
						node.Colliders[i],
						aRb,
						node.Colliders[j],
						bRb
					});
		}
	}
}

std::vector<CollisionFrame>& BroadphaseOctree::GetPotentialCollisions()
{
	if (m_Dirty)
		m_Root.Split(m_MaxDepth);
	m_Dirty = false;
	m_Collisions.clear();
	BroadPhase(m_Root, m_Collisions);
	return m_Collisions;
}

void Query(OctreeNode& node, Sphere& bounds, vector<Collider*>& results)
{
	if (!TestSphereBoxCollider(bounds, (AABB&)node.Bounds))
		return;
	if (node.Children)
	{
		for (int i = 0; i < 8; i++)
			Query(node.Children[i], bounds, results);
	}
	else
	{
		for (uint32_t i = 0; i < (uint32_t)node.Colliders.size(); i++)
		{
			OBB& childBounds = node.Colliders[i]->GetBounds();
			if (TestSphereBoxCollider(bounds, childBounds))
				results.emplace_back(node.Colliders[i]);
		}
	}
}

vector<Collider*> BroadphaseOctree::Query(Sphere& bounds) const
{
	vector<Collider*> results;
	::Query((OctreeNode&)m_Root, bounds, results);
	return results;
}


void Query(OctreeNode& node, AABB& bounds, vector<Collider*>& results)
{
	if (!TestBoxBoxCollider(bounds, (AABB&)node.Bounds))
		return;
	if (node.Children)
	{
		for (int i = 0; i < 8; i++)
			Query(node.Children[i], bounds, results);
	}
	else
	{
		for (uint32_t i = 0; i < (uint32_t)node.Colliders.size(); i++)
		{
			OBB& childBounds = node.Colliders[i]->GetBounds();
			if (TestBoxBoxCollider(bounds, childBounds))
				results.emplace_back(node.Colliders[i]);
		}
	}
}

vector<Collider*> BroadphaseOctree::Query(AABB& bounds) const
{
	vector<Collider*> results;
	::Query((OctreeNode&)m_Root, bounds, results);
	return results;
}

Collider* FindClosest(std::vector<Collider*>& set, Ray& ray, RaycastHit* outResult)
{
	if (set.size() == 0)
		return nullptr;

	Collider* closest = nullptr;
	float closestDistance = 99999;
	for (uint32_t i = 0; i < (uint32_t)set.size(); i++)
	{
		RaycastHit hit;
		if (!set[i]->Raycast(ray, &hit) ||
			hit.Distance >= closestDistance)
			continue;
		closest = set[i];
		closestDistance = hit.Distance;

		if (outResult)
			*outResult = hit;
	}
	return closest;
}

Collider* Raycast(OctreeNode& node, Ray& ray, Collider* ignoreCollider, RaycastHit* outResult)
{
	if (outResult)
		*outResult = {}; // Set to initial values

	RaycastHit hit;
	if (!node.Bounds.Raycast(ray, &hit))
		return nullptr;
	if (!node.Children) // Leaf node, check colliders
		return FindClosest(node.Colliders, ray, outResult);

	vector<Collider*> results;
	for (int i = 0; i < 8; i++)
	{
		Collider* result = Raycast(node.Children[i], ray, ignoreCollider, outResult);
		if (result && result != ignoreCollider)
			results.emplace_back(result);
	}

	// Find closest result
	return FindClosest(results, ray, outResult);
}

Collider* BroadphaseOctree::Raycast(Ray ray, Collider* ignoreCollider, RaycastHit* outResult) const
{ return ::Raycast((OctreeNode&)m_Root, ray, ignoreCollider, outResult); }

bool LineTest(OctreeNode& node, Line& line, Collider* ignoreCollider)
{
	if (!node.Bounds.LineTest(line))
		return false;
	if (node.Children) // Leaf node, check colliders
	{
		for (int i = 0; i < 8; i++)
			if (LineTest(node.Children[i], line, ignoreCollider))
				return true;
		return false;
	}

	for (uint32_t i = 0; i < (uint32_t)node.Colliders.size(); i++)
	{
		if (node.Colliders[i] != ignoreCollider &&
			node.Colliders[i]->LineTest(line))
			return true;
	}
	return false;
}

bool BroadphaseOctree::LineTest(Line& line, Collider* ignoreCollider) { return ::LineTest(m_Root, line, ignoreCollider); }
