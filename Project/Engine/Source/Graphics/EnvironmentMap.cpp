#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Graphics/EnvironmentMap.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;

EnvironmentMap::EnvironmentMap(string filepath, ivec2 resolution) : m_Framebuffer(nullptr), m_Resolution(resolution)
{
	// Texture
	TextureArgs args;
	args.HDR = true;
	args.FlipVertically = true;
	args.Wrap = GL_CLAMP_TO_EDGE;
	m_Texture = ResourceManager::LoadNamed<Texture>(filepath, filepath, args);

	// Shaders
	m_EquiToCubeShader = ResourceManager::LoadNamed<Shader>("Shaders/EquiToCube", ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Misc/Cubemap.vert",
			Application::AssetDir + "Shaders/Misc/EquiRectToCubemap.frag"
		});
}

EnvironmentMap::~EnvironmentMap()
{
	if(m_Framebuffer)
		delete m_Framebuffer;
	ResourceManager::Unload(m_Texture);
}

RenderTexture* EnvironmentMap::GetCubemap()
{
	if (!m_Framebuffer)
		Generate();
	return m_Framebuffer->GetColourAttachment();
}

ResourceID EnvironmentMap::GetTexture() { return m_Texture; }

void EnvironmentMap::Generate()
{
	if (m_Framebuffer)
		return; // Already generated
	
	FramebufferSpec specs;
	specs.Attachments =
	{
		{ TextureFormat::Cubemap },
		{ TextureFormat::RenderBuffer }
	};
	specs.Resolution = m_Resolution;
	m_Framebuffer = new Framebuffer(specs);

	mat4 projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
	mat4 views[] =
	{
		lookAt(vec3(0), vec3( 1,  0,  0), vec3(0, -1,  0)),
		lookAt(vec3(0), vec3(-1,  0,  0), vec3(0, -1,  0)),
		lookAt(vec3(0), vec3( 0,  1,  0), vec3(0,  0,  1)),
		lookAt(vec3(0), vec3( 0, -1,  0), vec3(0,  0, -1)),
		lookAt(vec3(0), vec3( 0,  0,  1), vec3(0, -1,  0)),
		lookAt(vec3(0), vec3( 0,  0, -1), vec3(0, -1,  0))
	};

	Shader* equiShader = ResourceManager::Get<Shader>(m_EquiToCubeShader);
	equiShader->Bind();
	equiShader->Set("equirectangularMap", 0);
	equiShader->Set("projectionMatrix", projection);

	Texture* inputTexture = ResourceManager::Get<Texture>(m_Texture);
	inputTexture->Bind(0);

	m_Framebuffer->Bind();
	Mesh* cubeMesh = ResourceManager::Get<Mesh>(Mesh::Cube());
	for (unsigned int i = 0; i < 6; i++)
	{
		equiShader->Set("viewMatrix", views[i]);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			m_Framebuffer->GetColourAttachment()->GetID(),
			0 // Mipmap level
		);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cubeMesh->Draw();
	}
	m_Framebuffer->Unbind();
}