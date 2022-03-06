#pragma once
#include <Engine/Physics/Shapes.hpp>
#include <Engine/Components/Physics/Collider.hpp>

namespace Engine::Physics
{
	struct CollisionFrame
	{
		Components::Collider* A;
		Components::Rigidbody* ARigidbody;

		Components::Collider* B;
		Components::Rigidbody* BRigidbody = nullptr;

		CollisionManifold Result;
	};

	struct Broadphase
	{
		virtual void Insert(Components::Collider* collider) = 0;
		virtual void Remove(Components::Collider* collider) = 0;

		virtual void Update() {}
		virtual void DrawGizmos() {}

		virtual std::vector<CollisionFrame>& GetPotentialCollisions() = 0;

		virtual Components::Collider* GetCollider(glm::vec3& point) { return nullptr; }

		virtual bool LineTest(Line& line, Components::Collider* ignoreCollider = nullptr) = 0;
			
		Components::Collider* Raycast(Ray ray, RaycastHit* outResult = nullptr) { return Raycast(ray, nullptr, outResult); }
		virtual Components::Collider* Raycast(Ray ray, Components::Collider* ignoreCollider, RaycastHit* outResult = nullptr) const = 0;

		virtual std::vector<Components::Collider*> Query(Sphere& bounds) const = 0;
		virtual std::vector<Components::Collider*> Query(AABB& bounds) const = 0;
	};

	struct BasicBroadphase : public Broadphase
	{		
		void Insert(Components::Collider* collider) override;
		void Remove(Components::Collider* collider) override;

		Components::Collider* GetCollider(glm::vec3& point) override;
		std::vector<CollisionFrame>& GetPotentialCollisions() override;

		std::vector<Components::Collider*> Query(Sphere& bounds) const override;
		std::vector<Components::Collider*> Query(AABB& bounds) const override;
		bool LineTest(Line& line, Components::Collider* ignoreCollider) override;
		Components::Collider* Raycast(Ray ray, Components::Collider* ignoreCollider, RaycastHit* outResult = nullptr) const override;

	private:
		std::vector<CollisionFrame> m_Collisions;
		std::vector<Components::Collider*> m_Colliders;
	};
}
