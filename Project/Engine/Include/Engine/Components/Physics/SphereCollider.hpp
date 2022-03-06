#pragma once
#include <Engine/Components/Physics/Collider.hpp>

namespace Engine::Components
{
	struct SphereCollider : public Collider
	{
		glm::vec3 Offset = { 0, 0, 0 }; // Relative to attached GameObject's transform
		
		float& GetRadius();
		void SetRadius(float radius);

		Physics::Sphere& GetSphere();

		void DrawGizmos() override;

		glm::mat4& InverseTensor() override;
		Physics::OBB& GetBounds() override;
		bool LineTest(Physics::Line& line) override;
		bool IsPointInside(glm::vec3& point) const override;
		bool Raycast(Physics::Ray& ray, Physics::RaycastHit* outResult) override;

		bool CheckCollision(Collider* other) override;
		bool CheckCollision(BoxCollider* other) override;
		bool CheckCollision(PlaneCollider* other) override;
		bool CheckCollision(SphereCollider* other) override;

	private:
		float m_Radius = 1.0f;
		glm::mat4 m_InverseTensor = glm::mat4(0.0f);

		Physics::OBB m_Bounds;
		Physics::Sphere m_Sphere;

		void CalculateInverseTensor();
		
		void Added() override;
		void FixedUpdate(float timestep) override;
	};
}