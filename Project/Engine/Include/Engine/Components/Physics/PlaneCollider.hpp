#pragma once
#include <Engine/Api.hpp>
#include <Engine/Components/Physics/Collider.hpp>

namespace Engine::Components
{
	struct PlaneCollider : public Collider
	{
		ENGINE_API PlaneCollider(glm::vec3 normal = { 1, 0, 0 }, float distance = 0.0f);

		ENGINE_API float& GetDistance();
		ENGINE_API void SetDistance(float value);

		ENGINE_API glm::vec3& GetNormal();
		ENGINE_API void SetNormal(glm::vec3 value);

		ENGINE_API Physics::Plane& GetPlane();

		ENGINE_API void DrawGizmos() override;

		ENGINE_API Physics::OBB& GetBounds() override;
		ENGINE_API bool LineTest(Physics::Line& line) override;
		ENGINE_API bool IsPointInside(glm::vec3& point) const override;
		ENGINE_API bool Raycast(Physics::Ray& ray, Physics::RaycastHit* outResult) override;

		ENGINE_API bool CheckCollision(Collider* other) override;
		ENGINE_API bool CheckCollision(BoxCollider* other) override;
		ENGINE_API bool CheckCollision(PlaneCollider* other) override;
		ENGINE_API bool CheckCollision(SphereCollider* other) override;

	private:
		Physics::OBB m_Bounds;
		Physics::Plane m_Plane;
	};
}