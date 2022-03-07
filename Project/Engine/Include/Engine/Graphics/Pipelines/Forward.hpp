#include <Engine/Application.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics::Pipelines
{
	class ForwardRenderPipeline : public RenderPipeline
	{
		Framebuffer* m_ForwardPass = nullptr;

		void ForwardPass(Framebuffer* previous);
		
	public:
		ForwardRenderPipeline();
		~ForwardRenderPipeline();
	};
}
