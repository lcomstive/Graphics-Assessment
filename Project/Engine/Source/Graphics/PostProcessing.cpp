#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Graphics/PostProcessing.hpp>

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

	m_Pass.Shader = new Shader({
		Application::AssetDir + FullscreenVertexShader,
		Application::AssetDir + fragmentShaderPath
	});
	m_Pass.DrawCallback = bind(&FullscreenEffectPass::DrawCallback, this, ::placeholders::_1);
}

FullscreenEffectPass::~FullscreenEffectPass()
{
	delete m_Pass.Pass;
	delete m_Pass.Shader;
}

void FullscreenEffectPass::DrawCallback(Framebuffer* previous)
{
	previous->GetColourAttachment()->Bind();
	m_Pass.Shader->Set("inputTexture", 0);
	OnDraw(m_Pass.Shader);

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
	shader->Set("gamma", Gamma);
	shader->Set("exposure", Exposure);
	shader->Set("tonemapper", (int)Tonemapper);
}
#pragma endregion
