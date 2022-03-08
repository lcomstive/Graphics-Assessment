#include <Engine/Api.hpp>
#include <Engine/Application.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics::Pipelines
{
	class DeferredRenderPipeline : public RenderPipeline
	{
		Framebuffer *m_MeshPass, *m_LightingPass, *m_ForwardPass;

		void MeshPass(Framebuffer* previous);
		void ForwardPass(Framebuffer* previous);
		void LightingPass(Framebuffer* previous);

	public:
		ENGINE_API DeferredRenderPipeline();
		ENGINE_API ~DeferredRenderPipeline();
	};
}