#pragma once
#include <glm/glm.hpp>
#include <Engine/Physics/Shapes.hpp>

namespace Engine::Components { struct Collider; }

namespace Engine::Physics
{
#pragma region Collision Tests
	bool TestSphereBoxCollider(Sphere& a, OBB& b);
	bool TestSphereBoxCollider(Sphere& a, AABB& b);
	bool TestSpherePlaneCollider(Sphere& a, Plane& b);
	bool TestSphereSphereCollider(Sphere& a, Sphere& b);

	bool TestBoxBoxCollider(OBB& a, OBB& b);
	bool TestBoxBoxCollider(AABB& a, OBB& b);
	bool TestBoxBoxCollider(AABB& a, AABB& b);
	bool TestBoxPlaneCollider(OBB& a, Plane& b);
	
	bool TestPlanePlaneCollider(Plane& a, Plane& b);
#pragma endregion

#pragma region Collision Manifolds
	struct CollisionManifold
	{
		bool IsColliding = false;
		glm::vec3 Normal = { 0, 0, 1 };
		float PenetrationDepth = FLT_MAX;
		std::vector<glm::vec3> Contacts = {};
	};

	CollisionManifold FindCollisionFeatures(OBB& a, OBB& b);
	CollisionManifold FindCollisionFeatures(OBB& a, Sphere& b);
	CollisionManifold FindCollisionFeatures(Sphere& a, Sphere& b);
	CollisionManifold FindCollisionFeatures(Engine::Components::Collider* a, Engine::Components::Collider* b);
#pragma endregion
}
