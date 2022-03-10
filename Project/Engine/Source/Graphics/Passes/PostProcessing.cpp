#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Graphics/Passes/PostProcessing.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

const string FullscreenVertexShader = "Shaders/TextureQuad.vert";

#pragma region Fullscreen Effect Pass
FullscreenEffectPass::FullscreenEffectPass(std::string fragmentShaderPath)
{
	FramebufferSpec specs;
	specs.Resolution = Renderer::GetResolution();
	specs.Attachments = { { TextureFormat::RGBA16F, TexturePixelType::Float } };
	m_Pass.Pass = new Framebuffer(specs);

	m_Pass.Shader = ResourceManager::LoadNamed<Shader>("Shaders/Fullscreen/" + fragmentShaderPath,
		ShaderStageInfo
		{
			Application::AssetDir + FullscreenVertexShader,
			Application::AssetDir + fragmentShaderPath
		});
	m_Pass.DrawCallback = bind(&FullscreenEffectPass::DrawCallback, this, ::placeholders::_1);

	m_Shader = ResourceManager::Get<Shader>(m_Pass.Shader);
}

FullscreenEffectPass::~FullscreenEffectPass()
{
	delete m_Pass.Pass;
	ResourceManager::Unload(m_Pass.Shader);
}

void FullscreenEffectPass::DrawCallback(Framebuffer* previous)
{
	previous->GetColourAttachment()->Bind();
	if(m_Shader)
		m_Shader->Set("inputTexture", 0);
	OnDraw(m_Shader);

	// DRAW FULLSCREEN QUAD //
	ResourceManager::Get<Mesh>(Mesh::Quad())->Draw();
}

RenderPipelinePass& FullscreenEffectPass::GetPipelinePass() { return m_Pass; }
#pragma endregion

#pragma region Tonemapping Pass
const string TonemappingShader = "Shaders/PostProcessing/Tonemapping.frag";
TonemappingPass::TonemappingPass() : FullscreenEffectPass(TonemappingShader) { }

void TonemappingPass::OnDraw(Shader* shader)
{
	if (!shader)
		return;

	shader->Set("gamma", Gamma);
	shader->Set("exposure", Exposure);
	shader->Set("tonemapper", (int)Tonemapper);
}
#pragma endregion
