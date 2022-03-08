#include <Engine/Api.hpp>
#include <Engine/Application.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics::Pipelines
{
	class ForwardRenderPipeline : public RenderPipeline
	{
		Framebuffer* m_ForwardPass = nullptr;

		void ForwardPass(Framebuffer* previous);
		
	public:
		ENGINE_API ForwardRenderPipeline();
		ENGINE_API ~ForwardRenderPipeline();
	};
}
