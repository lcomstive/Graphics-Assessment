#pragma once
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

namespace Engine::Components
{
	struct Camera : public Component
	{
		/// <summary>
		/// Vertical field of view (doesn't affect orthographic cameras)
		/// </summary>
		float FieldOfView = 60.0f;

		float ClipNear = 0.1f;
		float ClipFar = 1000.0f;

		bool Orthographic = false;
		float OrthoSize = 5.0f;

		Graphics::RenderTexture* RenderTarget = nullptr;

		ENGINE_API glm::mat4 GetViewMatrix();
		ENGINE_API glm::mat4 GetProjectionMatrix();
		
		ENGINE_API void FillShader(Graphics::Shader* shader);

		ENGINE_API void SetMainCamera();
		ENGINE_API static Camera* GetMainCamera();

		/// <summary>
		/// Recalculates view & projection matrices
		/// </summary>
		/// <param name="deltaTime">Unused</param>
		ENGINE_API virtual void Update(float deltaTime) override;

	protected:
		ENGINE_API virtual void Added() override;
		ENGINE_API virtual void Removed() override;

	private:
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	};
}