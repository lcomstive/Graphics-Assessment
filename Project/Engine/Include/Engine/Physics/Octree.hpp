#pragma once
#include <vector>
#include <Engine/Api.hpp>
#include <Engine/Physics/Shapes.hpp>
#include <Engine/Components/Physics/Collider.hpp>

namespace Engine::Physics
{
	struct ENGINE_API OctreeNode
	{
		AABB Bounds;
		OctreeNode* Children;
		std::vector<Components::Collider*> Colliders;

		OctreeNode();
		~OctreeNode();

		void Split(int depth);

		void Insert(Components::Collider* collider);
		void Remove(Components::Collider* collider);
		void Update(Components::Collider* collider);
	};
}