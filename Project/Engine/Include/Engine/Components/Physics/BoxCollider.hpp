#pragma once
#include <Engine/Api.hpp>
#include <Engine/Components/Physics/Collider.hpp>

namespace Engine::Components
{
	struct BoxCollider : public Collider
	{
		/// <summary>
		/// Position offset relative to attached object
		/// </summary>
		glm::vec3 Offset = { 0, 0, 0 };
		
		glm::vec3& GetExtents();
		ENGINE_API void SetExtents(glm::vec3 value);

		Physics::OBB& GetOBB();

		ENGINE_API void DrawGizmos() override;

		ENGINE_API Physics::OBB& GetBounds() override;
		ENGINE_API bool LineTest(Physics::Line& line) override;
		ENGINE_API bool IsPointInside(glm::vec3& point) const override;
		ENGINE_API bool Raycast(Physics::Ray& ray, Physics::RaycastHit* outResult) override;
		ENGINE_API glm::mat4& InverseTensor() override;

		ENGINE_API bool CheckCollision(Collider* other) override;
		ENGINE_API bool CheckCollision(BoxCollider* other) override;
		ENGINE_API bool CheckCollision(PlaneCollider* other) override;
		ENGINE_API bool CheckCollision(SphereCollider* other) override;

	private:
		Physics::OBB m_Bounds;
		glm::mat4 m_InverseTensor = glm::mat4(0.0f);
		glm::vec3 m_PreviousScale;
		glm::vec3 m_Extents = { 1, 1, 1 };
		
		void CalculateInverseTensor();

		void Added() override;
		void FixedUpdate(float timestep) override;
	};
}
