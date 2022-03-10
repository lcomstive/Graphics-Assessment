#pragma once
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics
{
	class ShadowMapPass
	{
		glm::ivec2 m_Resolution;
		RenderPipelinePass m_Pass;
		Shader* m_Shader = nullptr;

		void DrawCallback(Framebuffer* previous);

	public:
		ENGINE_API ShadowMapPass(glm::ivec2 resolution = { 1024, 1024 });
		ENGINE_API ~ShadowMapPass();

		ENGINE_API glm::ivec2& GetResolution();
		ENGINE_API void SetResolution(glm::ivec2 resolution);

		ENGINE_API RenderPipelinePass& GetPipelinePass();
	};
}