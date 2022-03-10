#pragma once
#include <vector>
#include <functional>
#include <Engine/Api.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/Components/Camera.hpp>

#define MAX_LIGHTS 100

namespace Engine::Graphics
{
	class Shader; // Forward declaration
	class Framebuffer;

	struct ENGINE_API RenderPipelinePass
	{
		ResourceID Shader = InvalidResourceID;
		Framebuffer* Pass = nullptr;
		std::function<void(Framebuffer* previous)> DrawCallback = nullptr;
		bool ResizeWithScreen = true;
	};

	class ENGINE_API RenderPipeline
	{
	protected:
		Shader* m_CurrentShader = nullptr;
		Framebuffer* m_PreviousPass = nullptr;
		std::vector<RenderPipelinePass> m_RenderPasses;

	public:
		virtual ~RenderPipeline() { }

		void Draw(Engine::Components::Camera& camera);

		/// <returns>The texture of the last render pass' output</returns>
		RenderTexture* GetOutputAttachment(unsigned int index = 0);

		Framebuffer* GetPassAt(unsigned int index);
		RenderPipelinePass& GetRenderPassAt(unsigned int index);
		std::vector<RenderPipelinePass>& GetAllRenderPasses();

		virtual void RemovePass(Framebuffer* pass);
		virtual void AddPass(RenderPipelinePass& passInfo);
		
		virtual void OnResized(glm::ivec2 resolution);

		Shader* CurrentShader();
		Framebuffer* GetPreviousPass();
	};
}