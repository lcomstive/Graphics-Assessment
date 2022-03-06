#include <glm/glm.hpp>
#include <Engine/Physics/Octree.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Physics;
using namespace Engine::Components;

OctreeNode::OctreeNode() : Children(nullptr) {}

OctreeNode::~OctreeNode()
{
	if (Children)
		delete[] Children;
}

void OctreeNode::Split(int depth)
{
	if (depth-- <= 0)
		return;

	if (!Children)
	{
		// Create children
		Children = new OctreeNode[8];

		// Position
		vec3 p = Bounds.Position;
		// Half extents
		vec3 e = Bounds.Extents / 2.0f;

		Children[0].Bounds = { p + vec3(-e.x,  e.y, -e.z), e };
		Children[1].Bounds = { p + vec3( e.x,  e.y, -e.z), e };
		Children[2].Bounds = { p + vec3(-e.x,  e.y,  e.z), e };
		Children[3].Bounds = { p + vec3( e.x,  e.y,  e.z), e };
		Children[4].Bounds = { p + vec3(-e.x, -e.y, -e.z), e };
		Children[5].Bounds = { p + vec3( e.x, -e.y, -e.z), e };
		Children[6].Bounds = { p + vec3(-e.x, -e.y,  e.z), e };
		Children[7].Bounds = { p + vec3( e.x, -e.y,  e.z), e };
	}

	if (!Children || Colliders.empty())
		return;

	for (uint32_t i = 0; i < 8; i++)
	{
		for (uint32_t j = 0; j < (uint32_t)Colliders.size(); j++)
		{
			OBB& bounds = Colliders[j]->GetBounds();
			if (TestBoxBoxCollider(Children[i].Bounds, bounds))
				Children[i].Colliders.emplace_back(Colliders[j]);
		}
	}

	Colliders.clear();

	for (int i = 0; i < 8; i++)
		Children[i].Split(depth);
}

void OctreeNode::Insert(Collider* collider)
{
	OBB& bounds = collider->GetBounds();
	if (!TestBoxBoxCollider(Bounds, bounds))
		return;
	if (!Children)
		Colliders.emplace_back(collider);
	else
		for (int i = 0; i < 8; i++)
			Children[i].Insert(collider);
}

void OctreeNode::Remove(Collider* collider)
{
	if (Children)
	{
		for (int i = 0; i < 8; i++)
			Children[i].Remove(collider);
		return;
	}

	const auto& it = find(Colliders.begin(), Colliders.end(), collider);
	if (it != Colliders.end())
		Colliders.erase(it);
}

void OctreeNode::Update(Collider* collider)
{
	Remove(collider);
	Insert(collider);
}