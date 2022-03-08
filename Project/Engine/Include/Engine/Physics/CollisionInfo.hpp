#pragma once
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/Physics/Shapes.hpp>

namespace Engine::Components { struct Collider; }

namespace Engine::Physics
{
#pragma region Collision Tests
	ENGINE_API bool TestSphereBoxCollider(Sphere& a, OBB& b);
	ENGINE_API bool TestSphereBoxCollider(Sphere& a, AABB& b);
	ENGINE_API bool TestSpherePlaneCollider(Sphere& a, Plane& b);
	ENGINE_API bool TestSphereSphereCollider(Sphere& a, Sphere& b);

	ENGINE_API bool TestBoxBoxCollider(OBB& a, OBB& b);
	ENGINE_API bool TestBoxBoxCollider(AABB& a, OBB& b);
	ENGINE_API bool TestBoxBoxCollider(AABB& a, AABB& b);
	ENGINE_API bool TestBoxPlaneCollider(OBB& a, Plane& b);
	
	ENGINE_API bool TestPlanePlaneCollider(Plane& a, Plane& b);
#pragma endregion

#pragma region Collision Manifolds
	struct ENGINE_API CollisionManifold
	{
		bool IsColliding = false;
		glm::vec3 Normal = { 0, 0, 1 };
		float PenetrationDepth = FLT_MAX;
		std::vector<glm::vec3> Contacts = {};
	};

	ENGINE_API CollisionManifold FindCollisionFeatures(OBB& a, OBB& b);
	ENGINE_API CollisionManifold FindCollisionFeatures(OBB& a, Sphere& b);
	ENGINE_API CollisionManifold FindCollisionFeatures(Sphere& a, Sphere& b);
	ENGINE_API CollisionManifold FindCollisionFeatures(Engine::Components::Collider* a, Engine::Components::Collider* b);
#pragma endregion
}
