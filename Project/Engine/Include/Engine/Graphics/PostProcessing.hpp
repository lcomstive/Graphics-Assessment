#pragma once
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics
{
	class FullscreenEffectPass
	{
		RenderPipelinePass m_Pass;

	protected:
		/// <summary>
		/// Useful for things like setting shader variables prior to drawing
		/// </summary>
		virtual void OnDraw(Shader* shader) { }

	public:
		FullscreenEffectPass(std::string fragmentShaderPath);
		~FullscreenEffectPass();

		RenderPipelinePass& GetPipelinePass();
	};

	enum class Tonemapper { None = 0, Aces, Reinhard, Exposure };

	class TonemappingPass : public FullscreenEffectPass
	{
	public:
		TonemappingPass();

		float Exposure = 1.0f;
		Tonemapper Tonemapper = Tonemapper::None;

	protected:
		virtual void OnDraw(Shader* shader) override;
	};
}