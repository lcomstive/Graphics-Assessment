#include <Engine/Application.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/ShadowMap.hpp>
#include <Engine/Graphics/Pipelines/Forward.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;
using namespace Engine::Graphics::Pipelines;

#define USE_TESSELLATION 0

ForwardRenderPipeline::ForwardRenderPipeline() 
{
	FramebufferSpec framebufferSpecs = { Renderer::GetResolution() };
	framebufferSpecs.Attachments =
	{
		{ TextureFormat::RGBA16F, TexturePixelType::Float },
		TextureFormat::Depth
	};

	ShaderStageInfo shaderStages =
	{
		Application::AssetDir + "Shaders/Deferred/Mesh.vert",
		Application::AssetDir + "Shaders/Deferred/Mesh.frag"
	};

	if (Renderer::SupportsTessellation())
	{
		shaderStages.TessellationControl = Application::AssetDir + "Shaders/Tessellation/Control.tess";
		shaderStages.TessellationEvaluate = Application::AssetDir + "Shaders/Tessellation/Evaluate.tess";
	}
	m_ForwardShader = ResourceManager::LoadNamed<Shader>("Shaders/Forward", shaderStages);

	m_ForwardPass = new Framebuffer(framebufferSpecs);
	RenderPipelinePass pass = { m_ForwardShader, m_ForwardPass };
	pass.DrawCallback = bind(&ForwardRenderPipeline::ForwardPass, this, ::placeholders::_1);

	AddPass(pass);
}

ForwardRenderPipeline::~ForwardRenderPipeline()
{
	RemovePass(m_ForwardPass);
	delete m_ForwardPass;
}

Framebuffer* ForwardRenderPipeline::GetMainMeshPass() { return m_ForwardPass; }

void ForwardRenderPipeline::ForwardPass(Framebuffer* previous)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	DrawArgs args;
	args.DrawSorting = DrawSortType::BackToFront;
	Renderer::Draw(args);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
}