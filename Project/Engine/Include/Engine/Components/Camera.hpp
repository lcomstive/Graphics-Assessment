#pragma once
#include <glm/glm.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

namespace Engine::Components
{
	struct Camera : public Component
	{
		Camera();

		float FieldOfView = 60.0f;

		float ClipNear = 0.1f;
		float ClipFar = 1000.0f;

		bool Orthographic = false;
		float OrthoSize = 5.0f;

		Graphics::RenderTexture* RenderTarget = nullptr;

		glm::mat4 GetViewMatrix();
		glm::vec3 GetUpDirection();
		glm::vec3 GetRightDirection();
		glm::vec3 GetForwardDirection();
		glm::mat4 GetProjectionMatrix();
		
		void FillShader(Graphics::Shader* shader);

		void SetMainCamera();
		static Camera* GetMainCamera();

	protected:
		virtual void Removed() override;
		virtual void Update(float deltaTime) override;

	private:
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;
		glm::vec3 m_GlobalPosition, m_Forward, m_Right, m_Up;

		static Camera* s_MainCamera;
	};
}