#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/Skybox.hpp>
#include <Engine/Graphics/Passes/ShadowMap.hpp>
#include <Engine/Graphics/Pipelines/Deferred.hpp>

#include <Engine/Components/Light.hpp>
#include <Engine/Components/Transform.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;
using namespace Engine::Graphics::Pipelines;

DeferredRenderPipeline::DeferredRenderPipeline()
{
	FramebufferSpec framebufferSpecs = { Renderer::GetResolution() };

	// Mesh Pass //
	framebufferSpecs.Attachments =
	{
		{ TextureFormat::RGBA16F, TexturePixelType::Float },	// Position + Roughness
		{ TextureFormat::RGBA16F, TexturePixelType::Float },	// Normal + Metalness
		TextureFormat::RGB8,	// Albedo
		TextureFormat::Depth
	};

	m_MeshPass = new Framebuffer(framebufferSpecs);

	ShaderStageInfo meshShaderStages =
	{
		Application::AssetDir + "Shaders/Deferred/Mesh.vert",
		Application::AssetDir + "Shaders/Deferred/Mesh.frag"
	};

	if (Renderer::SupportsTessellation())
	{
		meshShaderStages.TessellationControl = Application::AssetDir + "Shaders/Tessellation/Control.tess";
		meshShaderStages.TessellationEvaluate = Application::AssetDir + "Shaders/Tessellation/Evaluate.tess";
	}
	
	m_MeshShader = ResourceManager::LoadNamed<Shader>("Shaders/Deferred/Mesh", meshShaderStages);

	RenderPipelinePass pass = { "Deferred -> Mesh", m_MeshShader, m_MeshPass};
	pass.DrawCallback = std::bind(&DeferredRenderPipeline::MeshPass, this, ::placeholders::_1);
	AddPass(pass);

	// Lighting Pass //
	framebufferSpecs.Attachments =
	{
		{ TextureFormat::RGBA16F, TexturePixelType::Float },
		TextureFormat::Depth
	};
	m_LightingPass = new Framebuffer(framebufferSpecs);
	pass.Name = "Deferred -> Lighting";
	pass.Pass = m_LightingPass;
	pass.DrawCallback = bind(&DeferredRenderPipeline::LightingPass, this, ::placeholders::_1);

	pass.Shader = m_LightingShader = ResourceManager::LoadNamed<Shader>("Shaders/Deferred/Lighting",
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Deferred/Lighting.vert",
			Application::AssetDir + "Shaders/Deferred/Lighting.frag",
		});
	AddPass(pass);

	// Forward/Transparent Pass //
	framebufferSpecs.Attachments =
	{
		{ TextureFormat::RGBA16F, TexturePixelType::Float },
		TextureFormat::Depth
	};
	m_ForwardPass = new Framebuffer(framebufferSpecs);
	pass.Pass = m_ForwardPass;
	pass.DrawCallback = bind(&DeferredRenderPipeline::ForwardPass, this, ::placeholders::_1);

	m_ForwardShader = ResourceManager::LoadNamed<Shader>("Shaders/Forward",
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Forward/Mesh.vert",
			Application::AssetDir + "Shaders/Forward/Mesh.frag",
		});
	pass.Shader = m_ForwardShader;
	pass.Name = "Deferred -> Forward";
	AddPass(pass);
}

DeferredRenderPipeline::~DeferredRenderPipeline()
{
	delete m_MeshPass;
	delete m_ForwardPass;
	delete m_LightingPass;
}

Framebuffer* DeferredRenderPipeline::GetMainMeshPass() { return m_MeshPass; }

void DeferredRenderPipeline::MeshPass(Framebuffer* previous)
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

void DeferredRenderPipeline::LightingPass(Framebuffer* previous)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// FILL G-BUFFER MAPS //
	// Position
	m_MeshPass->GetColourAttachment()->Bind();
	m_CurrentShader->Set("inputPositionRoughness", 0);
	m_CurrentShader->Set("inputPositionRoughnessMS", 0);

	// Normals
	m_MeshPass->GetColourAttachment(1)->Bind(1);
	m_CurrentShader->Set("inputNormalMetalness", 1);
	m_CurrentShader->Set("inputNormalMetalnessMS", 1);

	// Albedo
	m_MeshPass->GetColourAttachment(2)->Bind(2);
	m_CurrentShader->Set("inputAlbedo", 2);
	m_CurrentShader->Set("inputAlbedoMS", 2);

	// Depth
	m_MeshPass->GetDepthAttachment()->Bind(3);
	m_CurrentShader->Set("inputDepth", 3);
	m_CurrentShader->Set("inputDepthMS", 3);

	// Environment Map
	Skybox* skybox = Renderer::GetPipeline()->GetSkybox();
	if (skybox)
		skybox->FillShaderData(m_CurrentShader);

	// FILL LIGHT DATA //
	Scene* scene = Application::GetService<Services::SceneService>()->CurrentScene();
	auto lights = scene->Root().GetComponentsInChildren<Light>();
	int lightCount = std::min((int32_t)lights.size(), MAX_LIGHTS);
	m_CurrentShader->Set("lightCount", lightCount);
	for (int i = 0; i < lightCount; i++)
		lights[i]->FillShader(i, m_CurrentShader);

	ShadowMapPass* shadowMap = Renderer::GetPipeline()->GetShadowMapPass();
	if (shadowMap && shadowMap->GetPipelinePass().Pass)
	{
		shadowMap->GetTexture()->Bind(5);
		m_CurrentShader->Set("shadowMap", 5);
	}

	// DRAW FULLSCREEN QUAD //
	ResourceManager::Get<Mesh>(Mesh::Quad())->Draw();

	// Unbind textures
	for (int i = 0; i < 4; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	}

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	glActiveTexture(GL_TEXTURE0);
}

void DeferredRenderPipeline::ForwardPass(Framebuffer* previous)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_MeshPass->BlitTo(m_ForwardPass, GL_DEPTH_BUFFER_BIT);
	m_LightingPass->BlitTo(m_ForwardPass, GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Environment Map
	Skybox* skybox = Renderer::GetPipeline()->GetSkybox();
	if (skybox)
		skybox->FillShaderData(m_CurrentShader);

	// Shadow Map
	ShadowMapPass* shadowMap = Renderer::GetPipeline()->GetShadowMapPass();
	if (shadowMap && shadowMap->GetPipelinePass().Pass)
	{
		shadowMap->GetTexture()->Bind(5);
		m_CurrentShader->Set("shadowMap", 5);
	}

	DrawArgs args;
	args.RenderOpaque = false;
	args.DrawSorting = DrawSortType::BackToFront;
	Renderer::Draw(args);

	Renderer::GetPipeline()->GetSkybox()->Draw();
}