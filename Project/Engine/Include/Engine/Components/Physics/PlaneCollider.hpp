#pragma once
#include <Engine/Components/Physics/Collider.hpp>

namespace Engine::Components
{
	struct PlaneCollider : public Collider
	{
		PlaneCollider(glm::vec3 normal = { 1, 0, 0 }, float distance = 0.0f);

		float& GetDistance();
		void SetDistance(float value);

		glm::vec3& GetNormal();
		void SetNormal(glm::vec3 value);

		Physics::Plane& GetPlane();

		void DrawGizmos() override;

		Physics::OBB& GetBounds() override;
		bool LineTest(Physics::Line& line) override;
		bool IsPointInside(glm::vec3& point) const override;
		bool Raycast(Physics::Ray& ray, Physics::RaycastHit* outResult) override;

		bool CheckCollision(Collider* other) override;
		bool CheckCollision(BoxCollider* other) override;
		bool CheckCollision(PlaneCollider* other) override;
		bool CheckCollision(SphereCollider* other) override;

	private:
		Physics::OBB m_Bounds;
		Physics::Plane m_Plane;
	};
}