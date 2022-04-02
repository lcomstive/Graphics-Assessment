#include <Engine/Application.hpp>
#include <Engine/ResourceManager.hpp>
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

ShadowMapPass::ShadowMapPass(ivec2 resolution) : m_ShadowCasters()
{
	FramebufferSpec specs;
	specs.Resolution = m_Resolution = resolution;
	specs.Attachments =
	{
		{
			TextureFormat::Depth16,	 // Format
			TexturePixelType::Float  // Data Type
		}
	};
	specs.Attachments[0].DepthInfo.Force3D = true;
	m_Pass.Pass = new Framebuffer(specs);
	m_Pass.ResizeWithScreen = false;

	m_Pass.Name = "Shadow Mapping";

	m_Pass.Shader = ResourceManager::LoadNamed<Shader>("Shaders/ShadowMap",
		ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Misc/ShadowMap/ShadowMap.vert",
			Application::AssetDir + "Shaders/Misc/ShadowMap/ShadowMap.frag",
			"", // Compute
			Application::AssetDir + "Shaders/Misc/ShadowMap/ShadowMap.geo" // Geometry shader
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
	RenderTexture* attachment = m_Pass.Pass->GetDepthAttachment();
	attachment->Bind();
	glBindTexture(GL_TEXTURE_2D_ARRAY, attachment->GetID());
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColour);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	attachment->Unbind();
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
	glViewport(0, 0, m_Resolution.x, m_Resolution.y);

	RenderTexture* outputTexture = m_Pass.Pass->GetDepthAttachment();
	glFramebufferTexture(
		GL_FRAMEBUFFER,			// Target
		GL_DEPTH_ATTACHMENT,	// Attachment
		outputTexture->GetID(),	// Texture
		0						// Mipmap Level
	);
	glClear(GL_DEPTH_BUFFER_BIT);
	if (m_ShadowCasters.size() == 0)
		return;

	SetBorder();

	Shader* shader = ResourceManager::Get<Shader>(m_Pass.Shader);

	unsigned int lightIndex = 0;
	for (auto& lightPair : m_ShadowCasters)
	{
		FillShadowData(lightPair.first, lightPair.second);

		if(lightPair.first->Type != LightType::Point)
			lightPair.second.ShadowMapArrayIndex = lightIndex;

		lightPair.first->FillShader(lightIndex++, shader);
	}

	shader->Set("lightCount", std::min((int)m_ShadowCasters.size(), MAX_LIGHTS));

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	Scene* scene = Application::GetService<SceneService>()->CurrentScene();
	vector<MeshRenderer*> shadowReceivers = scene->Root().GetComponentsInChildren<MeshRenderer>();
	for (MeshRenderer* renderer : shadowReceivers)
	{
		renderer->GetTransform()->FillShader(shader);
		for (MeshRenderer::MeshInfo& mesh : renderer->Meshes)
		{
			if (mesh.Material.CanCastShadows)
				ResourceManager::Get<Mesh>(mesh.Mesh)->Draw();
		}
	}

	glDisable(GL_CULL_FACE);
}

void ShadowMapPass::FillShadowData(Light* light, LightShadowData& shadowData)
{
	Transform* transform = light->GetTransform();
	Transform* mainCamTransform = Camera::GetMainCamera()->GetTransform();
	switch (light->Type)
	{
	default:
	case LightType::Spot:
	{
		shadowData.ShadowMapArrayIndex = -1; // Reset

		const float SpotNear = 0.1f;

		// Calculate view matrix
		vec3 forward = transform->Forward();
		if (abs(forward.x) < 0.001f) forward.x = 0.001f * sign(forward.x);
		if (abs(forward.y) < 0.001f) forward.y = 0.001f * sign(forward.y);
		if (abs(forward.z) < 0.001f) forward.z = 0.001f * sign(forward.z);

		mat4 viewMatrix = lookAt(
			transform->GetGlobalPosition() - forward,
			transform->GetGlobalPosition(),
			{ 0, 1, 0 }
		);

		light->Radius = std::clamp(light->Radius, 0.01f, 90.0f);
		mat4 projection = perspective(
			radians(light->Radius * 2.0f),
			(float)m_Resolution.x / (float)m_Resolution.y,
			SpotNear,
			light->Distance + SpotNear
		);

		// Combine matrices
		shadowData.LightSpaceMatrix = projection * viewMatrix;
		break;
	}
	case LightType::Directional:
	{
		shadowData.ShadowMapArrayIndex = -1; // Reset

		const float DirNear = 1.0f;
		const float DirFar = 100.0f;

		// Calculate view matrix
		vec3 pos = mainCamTransform->GetGlobalPosition() - transform->Forward() * (DirFar / 2.0f);
		mat4 viewMatrix = lookAt(
			pos,
			pos + transform->Forward(),
			vec3{ 0, 1, 0 }
		);

		mat4 projection = ortho(
			-light->Distance,
			light->Distance,
			-light->Distance,
			light->Distance, 
			DirNear,
			DirFar
		);

		// Combine matrices
		shadowData.LightSpaceMatrix = projection * viewMatrix;
		break;
	}
	case LightType::Point:
		// TODO: Point Lights
		shadowData.ShadowMapArrayIndex = -1; // Reset
		break;
	}
}

void ShadowMapPass::AddShadowCaster(Components::Light* light)
{
	auto& it = m_ShadowCasters.find(light);
	if (it != m_ShadowCasters.end())
		return; // Already in map

	m_ShadowCasters.emplace(make_pair(light, LightShadowData()));

	RenderTexture* depthAttachment = m_Pass.Pass->GetDepthAttachment();
	if (!depthAttachment)
		m_Pass.Pass->GetSpecs().Attachments[0].DepthInfo.Depth++;
	else if(m_ShadowCasters.size() > 0)
	{
		m_Pass.Pass->GetDepthAttachment()->GetArgs().Depth.Depth++;
		depthAttachment->SetDirty();
	}
}

void ShadowMapPass::RemoveShadowCaster(Components::Light* light)
{
	auto& it = m_ShadowCasters.find(light);
	if (it == m_ShadowCasters.end())
		return; // Not in map

	m_ShadowCasters.erase(light);

	RenderTextureArgs& rtArgs = m_Pass.Pass->GetDepthAttachment()->GetArgs();
	rtArgs.Depth.Depth = std::max(rtArgs.Depth.Depth - 1u, 1u); // Minimum value of 1 for valid framebuffer

	RenderTexture* depthAttachment = m_Pass.Pass->GetDepthAttachment();
	unsigned int& depth = depthAttachment ? depthAttachment->GetArgs().Depth.Depth :
							m_Pass.Pass->GetSpecs().Attachments[0].DepthInfo.Depth;
	depth = std::max(depth - 1u, 1u); // Minimum value of 1 for valid framebuffer object

	if(depthAttachment)
		depthAttachment->SetDirty();
}
