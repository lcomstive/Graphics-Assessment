#pragma once
#include <vector>
#include <functional>
#include <Engine/Components/Camera.hpp>

namespace Engine::Graphics
{
	class Shader; // Forward declaration
	class Framebuffer;

	struct RenderPipelinePass
	{
		Shader* Shader;
		Framebuffer* Pass;
		std::function<void(Framebuffer* previous)> DrawCallback;
		bool ResizeWithScreen = true;
	};

	class RenderPipeline
	{
	protected:
		Shader* m_CurrentShader = nullptr;
		Framebuffer* m_PreviousPass = nullptr;
		std::vector<RenderPipelinePass> m_RenderPasses;

	public:
		void Draw(Engine::Components::Camera& camera);

		/// <returns>The texture of the last render pass' output</returns>
		RenderTexture* GetOutputAttachment(unsigned int index = 0);

		virtual void RemovePass(Framebuffer* pass);
		virtual void AddPass(RenderPipelinePass& passInfo);
		
		virtual void OnResized(glm::ivec2 resolution);

		Shader* CurrentShader();
		Framebuffer* GetPreviousPass();
	};
}