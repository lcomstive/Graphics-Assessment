#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/ShadowMap.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>

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
	specs.Attachments =
	{
		{
			TextureFormat::Depth16,	 // Format
			TexturePixelType::Float, // Data Type
			{ MAX_LIGHTS * LightMaxViews }	// 2D Texture Array Depth Layers
		}
	};
	m_Pass.Pass = new Framebuffer(specs);
	m_Pass.ResizeWithScreen = false;

	m_Pass.Shader = ResourceManager::LoadNamed<Shader>("Shaders/ShadowMap",
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Misc/ShadowMap.vert",
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
RenderTexture* ShadowMapPass::GetTexture() { return m_Pass.Pass->GetDepthAttachment(); }

void ShadowMapPass::SetBorder()
{
	// Set texture clamping to border and assign border colour of white.
	// If sampling texture outside of texture coordinates (outside of shadow map),
	//   a value of white represents no shadow. Therefore any texture sampling outside
	//   of the shadow map implies no shadows
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_Pass.Pass->GetDepthAttachment()->GetID());
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColour);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void SortDrawQueue(vector<MeshRenderer*>& renderers, DrawSortType sortType, vec3 viewPos)
{
	if (sortType == DrawSortType::None)
		return;

	sort(renderers.begin(), renderers.end(), [=](MeshRenderer* a, MeshRenderer* b)
		{
			float distanceA = glm::distance(viewPos, a->GetTransform()->GetGlobalPosition());
			float distanceB = glm::distance(viewPos, b->GetTransform()->GetGlobalPosition());
			return sortType == DrawSortType::BackToFront ? (distanceA > distanceB) : (distanceB > distanceA);
		});
}

void ShadowMapPass::DrawCallback(Framebuffer* previous)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);
	glViewport(0, 0, m_Resolution.x, m_Resolution.y);

	SetBorder();

	Scene* currentScene = Application::GetService<SceneService>()->CurrentScene();

	unsigned int shadowMapArrayIndex = 0;
	RenderTexture* outputTexture = m_Pass.Pass->GetDepthAttachment();

	Scene* scene = Application::GetService<SceneService>()->CurrentScene();
	vector<MeshRenderer*> shadowReceivers = scene->Root().GetComponentsInChildren<MeshRenderer>();

	Shader* shader = ResourceManager::Get<Shader>(m_Pass.Shader);

	Camera* mainCam = Camera::GetMainCamera();

	vector<Light*> lights = currentScene->Root().GetComponentsInChildren<Light>();
	for(unsigned int lightIndex = 0; lightIndex < (unsigned int)std::min((int)lights.size(), MAX_LIGHTS); lightIndex++)
	{
		Light* light = lights[lightIndex];
		if (!light->CastShadows)
			continue;
		for (unsigned int i = 0; i < LightMaxViews; i++)
		{
			light->ShadowData.ShadowMapArrayIndex[i] = (light->Type == LightType::Point || i == 0) ? shadowMapArrayIndex : -1;
			if (light->Type != LightType::Point && i > 0)
				continue; // Only point lights (at this time) have multiple views

			light->FillShadowData();
			light->FillShader(0, shader);
			shader->Set("lightSpaceIndex", (int)i);

			glFramebufferTextureLayer(
				GL_FRAMEBUFFER,			// Target
				GL_DEPTH_ATTACHMENT,	// Attachment
				outputTexture->GetID(),	// Texture
				0,						// Mipmap Level
				shadowMapArrayIndex		// Layer
			);

			SortDrawQueue(shadowReceivers, DrawSortType::BackToFront, light->GetTransform()->GetGlobalPosition());
			for (MeshRenderer*& renderer : shadowReceivers)
			{
				renderer->GetTransform()->FillShader(shader);
				for (MeshRenderer::MeshInfo& mesh : renderer->Meshes)
				{
					if (mesh.Material.CanCastShadows)
						ResourceManager::Get<Mesh>(mesh.Mesh)->Draw();
				}
			}

			shadowMapArrayIndex++;
		}
	}

	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
}