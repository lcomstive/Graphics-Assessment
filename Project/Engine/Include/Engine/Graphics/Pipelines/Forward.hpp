#include <Engine/Api.hpp>
#include <Engine/Application.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics::Pipelines
{
	class ForwardRenderPipeline : public RenderPipeline
	{
		ResourceID m_ForwardShader = InvalidResourceID;
		Framebuffer* m_ForwardPass = nullptr;

		void ForwardPass(Framebuffer* previous);
		
	public:
		ENGINE_API ForwardRenderPipeline();
		ENGINE_API ~ForwardRenderPipeline();
	};
}
