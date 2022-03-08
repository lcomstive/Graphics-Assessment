#pragma once
#include <Engine/Api.hpp>
#include <Engine/Components/Physics/Collider.hpp>

namespace Engine::Components
{
	struct SphereCollider : public Collider
	{
		glm::vec3 Offset = { 0, 0, 0 }; // Relative to attached GameObject's transform
		
		ENGINE_API float& GetRadius();
		ENGINE_API void SetRadius(float radius);

		ENGINE_API Physics::Sphere& GetSphere();

		ENGINE_API void DrawGizmos() override;

		ENGINE_API glm::mat4& InverseTensor() override;
		ENGINE_API Physics::OBB& GetBounds() override;
		ENGINE_API bool LineTest(Physics::Line& line) override;
		ENGINE_API bool IsPointInside(glm::vec3& point) const override;
		ENGINE_API bool Raycast(Physics::Ray& ray, Physics::RaycastHit* outResult) override;

		ENGINE_API bool CheckCollision(Collider* other) override;
		ENGINE_API bool CheckCollision(BoxCollider* other) override;
		ENGINE_API bool CheckCollision(PlaneCollider* other) override;
		ENGINE_API bool CheckCollision(SphereCollider* other) override;

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