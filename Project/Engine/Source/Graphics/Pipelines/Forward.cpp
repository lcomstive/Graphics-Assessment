#include <Engine/Application.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Graphics/Pipelines/Forward.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;
using namespace Engine::Graphics::Pipelines;

ForwardRenderPipeline::ForwardRenderPipeline() 
{
	FramebufferSpec framebufferSpecs = { Renderer::GetResolution() };
	framebufferSpecs.Attachments =
	{
		{ TextureFormat::RGBA16F, TexturePixelType::Float },
		TextureFormat::Depth
	};
	
	Shader* shader = new Shader(ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Forward/Mesh.vert",
			Application::AssetDir + "Shaders/Forward/Mesh.frag"
		});
	m_ForwardPass = new Framebuffer(framebufferSpecs);
	RenderPipelinePass pass = { shader, m_ForwardPass };
	pass.DrawCallback = bind(&ForwardRenderPipeline::ForwardPass, this, ::placeholders::_1);

	AddPass(pass);

	Log::Info("Forward Renderer initialized");
}

ForwardRenderPipeline::~ForwardRenderPipeline()
{
	Framebuffer* forwardPass = m_RenderPasses[0].Pass;
	RemovePass(forwardPass);
	delete forwardPass;
}

void ForwardRenderPipeline::ForwardPass(Framebuffer* previous)
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