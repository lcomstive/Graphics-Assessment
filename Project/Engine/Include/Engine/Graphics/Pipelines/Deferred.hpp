#include <Engine/Application.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics::Pipelines
{
	class DeferredRenderPipeline : public RenderPipeline
	{
		RenderPass *m_MeshPass, *m_LightingPass, *m_ForwardPass;

		void MeshPass(RenderPass* previous);
		void ForwardPass(RenderPass* previous);
		void LightingPass(RenderPass* previous);

	public:
		DeferredRenderPipeline();
		~DeferredRenderPipeline();
	};
}