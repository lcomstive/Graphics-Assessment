#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Pipelines/Forward.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;
using namespace Engine::Graphics::Pipelines;

ForwardRenderPipeline::ForwardRenderPipeline() 
{
	FramebufferSpec framebuffer = { Renderer::GetResolution() };
	framebuffer.Attachments =
	{
		TextureFormat::RGBA8,
		TextureFormat::Depth
	};
	
	Shader* shader = new Shader(ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Forward/Mesh.vert",
			Application::AssetDir + "Shaders/Forward/Mesh.frag"
		});
	RenderPipelinePass pass = { shader, new RenderPass(framebuffer) };
	pass.DrawCallback = bind(&ForwardRenderPipeline::ForwardPass, this, ::placeholders::_1);

	AddPass(pass);
}

ForwardRenderPipeline::~ForwardRenderPipeline()
{
	RenderPass* forwardPass = m_RenderPasses[0].Pass;
	RemovePass(forwardPass);
	delete forwardPass;
}

void ForwardRenderPipeline::ForwardPass(RenderPass* previous)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawArgs args;
	args.DrawSorting = DrawSortType::BackToFront;
	Renderer::Draw(args);
}