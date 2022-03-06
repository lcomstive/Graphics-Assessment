#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Pipelines/Deferred.hpp>

#include <Engine/Components/Light.hpp>
#include <Engine/Components/Transform.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;
using namespace Engine::Graphics::Pipelines;

DeferredRenderPipeline::DeferredRenderPipeline() : m_ForwardPass(nullptr)
{
	FramebufferSpec framebuffer = { Renderer::GetResolution()};

	// Mesh Pass //
	framebuffer.Attachments =
	{
		TextureFormat::RGBA16F,	// Position + Roughness
		TextureFormat::RGBA16F,	// Normal + Metalness
		TextureFormat::RGB8,	// Albedo
		TextureFormat::Depth
	};

	m_MeshPass = new RenderPass(framebuffer);

	Shader* meshShader = new Shader(ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Deferred/Mesh.vert",
			Application::AssetDir + "Shaders/Deferred/Mesh.frag"
		});

	RenderPipelinePass pass = { meshShader , m_MeshPass };
	pass.DrawCallback = std::bind(&DeferredRenderPipeline::MeshPass, this, ::placeholders::_1);

	AddPass(pass);

	// Lighting Pass //
	framebuffer.Attachments =
	{
		TextureFormat::RGBA8,
		TextureFormat::Depth
	};
	m_LightingPass = new RenderPass(framebuffer);
	pass.Pass = m_LightingPass;

	pass.DrawCallback = bind(&DeferredRenderPipeline::LightingPass, this, ::placeholders::_1);

	pass.Shader = new Shader(
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Deferred/Lighting.vert",
			Application::AssetDir + "Shaders/Deferred/Lighting.frag",
		});
	AddPass(pass);
	
	// Forward/Transparent Pass //
	framebuffer.Attachments =
	{
		TextureFormat::RGBA8,
		TextureFormat::Depth
	};
	framebuffer.SwapchainTarget = true;
	m_ForwardPass = new RenderPass(framebuffer);
	pass.Pass = m_ForwardPass;

	pass.DrawCallback = bind(&DeferredRenderPipeline::ForwardPass, this, ::placeholders::_1);

	pass.Shader = new Shader(
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Forward/Mesh.vert",
			Application::AssetDir + "Shaders/Forward/Mesh.frag",
		});
	AddPass(pass);
}

DeferredRenderPipeline::~DeferredRenderPipeline()
{
	RemovePass(m_MeshPass);
	RemovePass(m_ForwardPass);
	RemovePass(m_LightingPass);

	delete m_MeshPass;
	delete m_ForwardPass;
	delete m_LightingPass;
}

void DeferredRenderPipeline::MeshPass(RenderPass* previous)
{
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawArgs args;
	args.ClearQueue = false;
	args.RenderTransparent = false;
	Renderer::Draw(args);
}

const unsigned int MaxLights = 16;
void DeferredRenderPipeline::LightingPass(RenderPass* previous)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Shader* shader = CurrentShader();

	// FILL G-BUFFER MAPS //
	// Position
	m_MeshPass->GetColourAttachment()->Bind();
	shader->Set("inputPositionRoughness", 0);

	// Normals
	m_MeshPass->GetColourAttachment(1)->Bind(1);
	shader->Set("inputNormalMetalness", 1);

	// Albedo
	m_MeshPass->GetColourAttachment(2)->Bind(2);
	shader->Set("inputAlbedo", 2);

	// Depth
	m_MeshPass->GetDepthAttachment()->Bind(3);
	shader->Set("inputDepth", 3);

	// FILL LIGHT DATA //
	auto lights = Renderer::GetApp()->CurrentScene()->Root().GetComponentsInChildren<Light>();

	unsigned int lightIndex = 0;
	unsigned int lightCount = std::min((unsigned int)lights.size(), MaxLights);
	shader->Set("lightCount", (int)lightCount);
	for (Light* light : lights)
	{
		shader->Set("lights[" + to_string(lightIndex) + "].Colour", light->Colour);
		shader->Set("lights[" + to_string(lightIndex) + "].Radius", light->Radius);
		shader->Set("lights[" + to_string(lightIndex) + "].Position", light->GetTransform()->Position);

		if (++lightIndex >= MaxLights)
			break;
	}

	// DRAW FULLSCREEN QUAD //
	Mesh::Quad()->Draw();
}

void DeferredRenderPipeline::ForwardPass(RenderPass* previous)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_MeshPass->GetFramebuffer()->BlitTo(m_ForwardPass->GetFramebuffer(), GL_DEPTH_BUFFER_BIT);
	m_LightingPass->GetFramebuffer()->BlitTo(m_ForwardPass->GetFramebuffer(), GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawArgs args;
	args.RenderOpaque = false;
	args.DrawSorting = DrawSortType::BackToFront;
	Renderer::Draw(args);
}