#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Renderer.hpp>
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
	specs.Attachments = { TextureFormat::RGBA8 };
	m_Pass.Pass = new RenderPass(specs);

	m_Pass.Shader = new Shader({
		Application::AssetDir + FullscreenVertexShader,
		Application::AssetDir + fragmentShaderPath
	});
	m_Pass.DrawCallback = [=](RenderPass* previous)
	{
		previous->GetColourAttachment()->Bind();
		m_Pass.Shader->Set("inputTexture", 0);
		OnDraw(m_Pass.Shader);

		Mesh::Quad()->Draw();
	};
}

FullscreenEffectPass::~FullscreenEffectPass()
{
	delete m_Pass.Pass;
	delete m_Pass.Shader;
}

RenderPipelinePass& FullscreenEffectPass::GetPipelinePass() { return m_Pass; }
#pragma endregion

#pragma region Tonemapping Pass
const string TonemappingShader = "Shaders/PostProcessing/Tonemapping.frag";
TonemappingPass::TonemappingPass() : FullscreenEffectPass(TonemappingShader) { }

void TonemappingPass::OnDraw(Shader* shader)
{
	shader->Set("exposure", Exposure);
	shader->Set("tonemapper", (int)Tonemapper);
}
#pragma endregion
