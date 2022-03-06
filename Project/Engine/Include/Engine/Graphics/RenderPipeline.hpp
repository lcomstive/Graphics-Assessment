#pragma once
#include <vector>
#include <functional>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/RenderPass.hpp>

namespace Engine { class Application; }

namespace Engine::Graphics
{
	struct RenderPipelinePass
	{
		Shader* Shader;
		RenderPass* Pass;
		std::function<void(RenderPass* previous)> DrawCallback;
		bool ResizeWithScreen = true;
	};

	class RenderPipeline
	{
	protected:
		Shader* m_CurrentShader = nullptr;
		RenderPass* m_PreviousPass = nullptr;
		std::vector<RenderPipelinePass> m_RenderPasses;

	public:
		void Draw(Engine::Components::Camera& camera);

		/// <returns>The texture of the last render pass' output</returns>
		RenderTexture* GetOutputAttachment(unsigned int index = 0);

		virtual void RemovePass(RenderPass* pass);
		virtual void AddPass(RenderPipelinePass& passInfo);
		
		virtual void OnResized(glm::ivec2 resolution);

		Shader* CurrentShader();
		RenderPass* GetPreviousPass();
	};
}