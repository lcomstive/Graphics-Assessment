#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/GizmoService.hpp>

using namespace std;
using namespace Engine::Graphics;
using namespace Engine::Services;

void GizmoService::OnStart()
{
	if (m_Pass.Pass)
		return; // Already initialized

	FramebufferSpec spec;
	spec.Attachments = { { TextureFormat::RGBA16F, TexturePixelType::Float }, TextureFormat::Depth };
	spec.Resolution = Renderer::GetResolution();
	m_Pass.Name = "Gizmos";
	m_Pass.Pass = new Framebuffer(spec);
	m_Pass.Shader = ResourceManager::LoadNamed<Shader>("Shaders/Gizmo",
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Gizmos.vert",
			Application::AssetDir + "Shaders/Gizmos.frag"
		});
	m_Pass.DrawCallback = bind(&GizmoService::DrawCallback, this, ::placeholders::_1);
}

void GizmoService::OnShutdown()
{
	Renderer::GetPipeline()->RemovePass(m_Pass.Pass);
	delete m_Pass.Pass;
}

void GizmoService::OnPipelineChanged(RenderPipeline* pipeline)
{
	if (!m_Pass.Pass)
		OnStart();

	Renderer::GetPipeline()->AddPass(m_Pass);
}

void GizmoService::DrawCallback(Framebuffer* previous)
{
	glClear(GL_COLOR_BUFFER_BIT);

	Framebuffer* meshPass = Renderer::GetPipeline()->GetMainMeshPass();

	// Copy depth buffer to this pass
	if (meshPass && meshPass->HasDepthAttachment())
		meshPass->BlitTo(m_Pass.Pass, GL_DEPTH_BUFFER_BIT);
	// Copy colour buffer to this path
	if (previous && previous->ColourAttachmentCount() > 0)
	{
		// Check for valid format before blitting
		TextureFormat format = previous->GetColourAttachment(0)->GetFormat();
		if(format == TextureFormat::RGB8 ||
			format == TextureFormat::RGBA8 ||
			format == TextureFormat::RGBA16 ||
			format == TextureFormat::RGBA16F)
			previous->BlitTo(m_Pass.Pass, GL_COLOR_BUFFER_BIT);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	Renderer::ClearDrawQueue(); // Incase any leftover draw calls

	auto& services = Application::GetAllServices();
	for (Service* service : services)
		service->OnDrawGizmos();

	Renderer::Draw();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}