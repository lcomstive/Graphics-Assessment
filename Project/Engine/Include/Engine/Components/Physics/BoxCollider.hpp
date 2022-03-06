#pragma once
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
		void SetExtents(glm::vec3 value);

		Physics::OBB& GetOBB();

		void DrawGizmos() override;

		Physics::OBB& GetBounds() override;
		bool LineTest(Physics::Line& line) override;
		bool IsPointInside(glm::vec3& point) const override;
		bool Raycast(Physics::Ray& ray, Physics::RaycastHit* outResult) override;
		glm::mat4& InverseTensor() override;

		bool CheckCollision(Collider* other) override;
		bool CheckCollision(BoxCollider* other) override;
		bool CheckCollision(PlaneCollider* other) override;
		bool CheckCollision(SphereCollider* other) override;

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
