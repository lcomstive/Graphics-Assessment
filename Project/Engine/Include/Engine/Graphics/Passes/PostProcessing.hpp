#pragma once
#include <Engine/Api.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics
{
	class FullscreenEffectPass
	{
		RenderPipelinePass m_Pass;
		Shader* m_Shader = nullptr;

		void DrawCallback(Framebuffer* previous);

	protected:
		/// <summary>
		/// Useful for things like setting shader variables prior to drawing
		/// </summary>
		ENGINE_API virtual void OnDraw(Shader* shader) { }

	public:
		ENGINE_API FullscreenEffectPass(std::string fragmentShaderPath);
		ENGINE_API ~FullscreenEffectPass();

		ENGINE_API RenderPipelinePass& GetPipelinePass();
	};

	enum class Tonemapper { None = 0, Aces, Reinhard };

	class TonemappingPass : public FullscreenEffectPass
	{
	public:
		float Gamma = 2.2f;
		float Exposure = 1.0f;
		Tonemapper Tonemapper = Tonemapper::None;
		
		ENGINE_API TonemappingPass();

	protected:
		ENGINE_API virtual void OnDraw(Shader* shader) override;
	};
}