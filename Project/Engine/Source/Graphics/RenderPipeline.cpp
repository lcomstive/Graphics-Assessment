#include <algorithm>
#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Services;
using namespace Engine::Graphics;
using namespace Engine::Components;

void RenderPipeline::Draw(Camera& camera)
{
	Scene* scene = Application::GetService<SceneService>()->CurrentScene();

	m_PreviousPass = nullptr;
	for(unsigned int i = 0; i < (unsigned int)m_RenderPasses.size(); i++)
	{
		RenderPipelinePass& info = m_RenderPasses[i];

		// Setup
		info.Pass->Bind();

		m_CurrentShader = info.Shader;
		if (m_CurrentShader)
		{
			m_CurrentShader->Bind();

			m_CurrentShader->Set("resolution", vec2{ Renderer::GetResolution().x, Renderer::GetResolution().y });
			m_CurrentShader->Set("time", Renderer::GetTime());
			m_CurrentShader->Set("deltaTime", Renderer::GetDeltaTime());

			camera.FillShader(m_CurrentShader);

			auto lights = scene->Root().GetComponentsInChildren<Light>();
			int lightCount = std::min((int32_t)lights.size(), MAX_LIGHTS);
			m_CurrentShader->Set("lightCount", lightCount);
			for (int i = 0; i < lightCount; i++)
			{
				m_CurrentShader->Set("lights[" + to_string(i) + "].Colour", lights[i]->Colour);
				m_CurrentShader->Set("lights[" + to_string(i) + "].Radius", lights[i]->Radius);
				m_CurrentShader->Set("lights[" + to_string(i) + "].Intensity", lights[i]->Intensity);
				m_CurrentShader->Set("lights[" + to_string(i) + "].Position", lights[i]->GetTransform()->Position);
			}
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
	if (passInfo.Pass)
		m_RenderPasses.emplace_back(passInfo);
}

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