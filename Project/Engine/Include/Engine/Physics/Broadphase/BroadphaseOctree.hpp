/**
*
* THIS CLASS IS BROKEN AND UNUSED.
* 
* LEFT IN PROJECT FOR FUTURE REPAIRS.
*
**/

#pragma once
#include <Engine/Physics/Octree.hpp>
#include <Engine/Physics/Broadphase/Broadphase.hpp>

namespace Engine::Physics
{
	struct BroadphaseOctree : public Broadphase
	{
		BroadphaseOctree(unsigned int maxDepth = 5, float boundsWidth = 1000);

		void DrawGizmos() override;

		void SetMaxDepth(unsigned int maxDepth);

		void Insert(Components::Collider* collider) override;
		void Remove(Components::Collider* collider) override;		

		virtual Components::Collider* GetCollider(glm::vec3& point) override;
		virtual std::vector<CollisionFrame>& GetPotentialCollisions() override;

		std::vector<Components::Collider*> Query(AABB& bounds) const override;
		std::vector<Components::Collider*> Query(Sphere& bounds) const override;
		bool LineTest(Line& line, Components::Collider* ignoreCollider) override;
		Components::Collider* Raycast(Ray ray, Components::Collider* ignoreCollider, RaycastHit* outResult = nullptr) const override;

	private:
		bool m_Dirty = false;
		unsigned int m_MaxDepth;

		OctreeNode m_Root;
		std::vector<CollisionFrame> m_Collisions;
	};
}