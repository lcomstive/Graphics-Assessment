#pragma once
#include <Engine/Components/Camera.hpp>
#include <Engine/Components/Component.hpp>

namespace Engine::Components
{
	class OrbitCameraController : public Component
	{
		Camera* m_Camera = nullptr;

		void Added() override;
		void Update(float deltaTime) override;

	public:
		float MoveSpeed = 5.0f;
		float RotateSpeed = 5.0f;
		float SprintSpeed = 15.0f;
	};
}