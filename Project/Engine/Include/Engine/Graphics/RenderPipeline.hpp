#pragma once
#include <vector>
#include <functional>
#include <Engine/Api.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/Components/Camera.hpp>

#define MAX_LIGHTS 32

namespace Engine::Graphics
{
	// Forward declarations
	class Shader;
	class Framebuffer;
	class ShadowMapPass;

	struct ENGINE_API RenderPipelinePass
	{
		ResourceID Shader = InvalidResourceID;
		Framebuffer* Pass = nullptr;
		std::function<void(Framebuffer* previous)> DrawCallback = nullptr;
		bool ResizeWithScreen = true;
	};

	class ENGINE_API RenderPipeline
	{
		ShadowMapPass* m_ShadowPass = nullptr;

	protected:
		Shader* m_CurrentShader = nullptr;
		Framebuffer* m_PreviousPass = nullptr;
		std::vector<RenderPipelinePass> m_RenderPasses;

	public:
		RenderPipeline();
		virtual ~RenderPipeline();

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
		ShadowMapPass* GetShadowMapPass();
	};
}