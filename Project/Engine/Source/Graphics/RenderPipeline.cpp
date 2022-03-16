#include <algorithm>
#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/Skybox.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>
#include <Engine/Graphics/Passes/ShadowMap.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Services;
using namespace Engine::Graphics;
using namespace Engine::Components;

const ivec2 ShadowMapResolution = { 1024, 1024 };

RenderPipeline::RenderPipeline()
{
	m_Skybox = new Skybox();
	m_ShadowPass = new ShadowMapPass(ShadowMapResolution);

	AddPass(m_ShadowPass->GetPipelinePass());
}

RenderPipeline::~RenderPipeline()
{
	delete m_ShadowPass;
}

void RenderPipeline::Draw(Camera& camera)
{
	Scene* scene = Application::GetService<SceneService>()->CurrentScene();

	m_PreviousPass = nullptr;
	for(unsigned int i = 0; i < (unsigned int)m_RenderPasses.size(); i++)
	{
		RenderPipelinePass& info = m_RenderPasses[i];

		// Setup
		info.Pass->Bind();

		m_CurrentShader = ResourceManager::Get<Shader>(info.Shader);
		if (m_CurrentShader)
		{
			m_CurrentShader->Bind();

			m_CurrentShader->Set("time", Renderer::GetTime());
			m_CurrentShader->Set("deltaTime", Renderer::GetDeltaTime());
			m_CurrentShader->Set("samples", (int)info.Pass->GetSamples());
			m_CurrentShader->Set("resolution", vec2{ Renderer::GetResolution().x, Renderer::GetResolution().y });

			camera.FillShader(m_CurrentShader);
		}

		// Draw calls
		if (info.DrawCallback)
			info.DrawCallback(m_PreviousPass);

		// Finalise
		if (m_CurrentShader)
			m_CurrentShader->Unbind();

		m_CurrentShader = nullptr;
		info.Pass->Unbind();
		m_PreviousPass = info.Pass;
	}

	if (camera.RenderTarget && !m_RenderPasses.empty())
		m_RenderPasses[m_RenderPasses.size() - 1].Pass->CopyAttachmentTo(camera.RenderTarget);
	else if(m_PreviousPass)
		m_PreviousPass->BlitTo(nullptr, GL_COLOR_BUFFER_BIT);
	m_PreviousPass = nullptr;
}

void RenderPipeline::RemovePass(Framebuffer* pass)
{
	for (int i = (int)m_RenderPasses.size() - 1; i >= 0; i--)
	{
		if (m_RenderPasses[i].Pass == pass)
		{
			m_RenderPasses.erase(m_RenderPasses.begin() + i);
			break;
		}
	}
}

void RenderPipeline::AddPass(RenderPipelinePass& passInfo)
{
	if (!passInfo.Pass)
		return;
	m_RenderPasses.emplace_back(passInfo);
}

Framebuffer* RenderPipeline::GetPassAt(unsigned int index)
{
	return index < m_RenderPasses.size() ? m_RenderPasses[index].Pass : nullptr;
}

RenderPipelinePass& RenderPipeline::GetRenderPassAt(unsigned int index)
{
	Log::Assert(index < m_RenderPasses.size(), "Index out of range!");
	return m_RenderPasses[index];
}

std::vector<RenderPipelinePass>& RenderPipeline::GetAllRenderPasses() { return m_RenderPasses; }

RenderTexture* RenderPipeline::GetOutputAttachment(unsigned int index)
{
	if (m_RenderPasses.empty())
		return nullptr;

	Framebuffer* pass = m_RenderPasses[m_RenderPasses.size() - 1].Pass;
	index = std::clamp(index, 0u, pass->ColourAttachmentCount());
	return pass->GetColourAttachment(index);
}

void RenderPipeline::OnResized(ivec2 resolution)
{
	for (RenderPipelinePass& pass : m_RenderPasses)
		if (pass.ResizeWithScreen)
			pass.Pass->SetResolution(resolution);
}

Shader* RenderPipeline::CurrentShader() { return m_CurrentShader; }
Framebuffer* RenderPipeline::GetPreviousPass() { return m_PreviousPass; }
ShadowMapPass* RenderPipeline::GetShadowMapPass() { return m_ShadowPass; }
Skybox* RenderPipeline::GetSkybox() { return m_Skybox; }
