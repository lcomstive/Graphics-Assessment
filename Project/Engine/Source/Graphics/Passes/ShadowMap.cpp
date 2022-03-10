#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/ShadowMap.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Services;
using namespace Engine::Components;

ShadowMapPass::ShadowMapPass(ivec2 resolution)
{
	FramebufferSpec specs;
	specs.Resolution = m_Resolution = resolution;
	specs.Attachments = { { TextureFormat::Depth, TexturePixelType::Float } };
	m_Pass.Pass = new Framebuffer(specs);

	m_Pass.Shader = ResourceManager::LoadNamed<Shader>("Shaders/ShadowMap",
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Misc/BasicMesh.vert",
			Application::AssetDir + "Shaders/Misc/Empty.frag"
		});
	m_Pass.DrawCallback = bind(&ShadowMapPass::DrawCallback, this, ::placeholders::_1);
}

ShadowMapPass::~ShadowMapPass()
{
	Renderer::GetPipeline()->RemovePass(m_Pass.Pass);
	if (m_Pass.Pass)
		delete m_Pass.Pass;
}

void ShadowMapPass::SetResolution(ivec2 resolution)
{
	m_Resolution = resolution;
	if (m_Pass.Pass)
		m_Pass.Pass->SetResolution(resolution);
}

ivec2& ShadowMapPass::GetResolution() { return m_Resolution; }
RenderPipelinePass& ShadowMapPass::GetPipelinePass() { return m_Pass; }

void ShadowMapPass::DrawCallback(Framebuffer* previous)
{
	glViewport(0, 0, m_Resolution.x, m_Resolution.y);
	glClear(GL_DEPTH_BUFFER_BIT);

	Scene* currentScene = Application::GetService<SceneService>()->CurrentScene();

	// Directional Lights
	vector<DirectionalLight*> lights = currentScene->Root().GetComponentsInChildren<DirectionalLight>();
	for (DirectionalLight* light : lights)
	{
		mat4 lightProjection = ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 1000.0f);
		mat4 viewMatrix = lookAt(-light->GetTransform()->Rotation,
									vec3(0),
									vec3(0, 1, 0)
								);

		mat4 lightView = lightProjection * viewMatrix;
	}
}