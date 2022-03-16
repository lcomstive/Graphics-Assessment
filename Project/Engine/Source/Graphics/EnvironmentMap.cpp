#include <Engine/Utilities.hpp>
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

const string CubemapCacheExtension	  = ".Cubemap.hdr";
const string IrradianceCacheExtension = ".Irradiance.hdr";

EnvironmentMap::EnvironmentMap(EnvironmentMapArgs args, ivec2 resolution) :
	m_Args(args),
	m_Framebuffer(nullptr),
	m_BRDFTexture(nullptr),
	m_Resolution(resolution),
	m_CubemapTexture(nullptr),
	m_PreFilterTexture(nullptr),
	m_IrradianceTexture(nullptr),
	m_Texture(InvalidResourceID),
	m_ReflectionCubemapTexture(nullptr)
{
	if (m_Args.Background.empty())
	{
		Log::Warning("Cannot create environment map - background has empty path");
		return;
	}
	if (!filesystem::exists(m_Args.Background))
	{
		Log::Warning("Cannot create environment map - background path cannot be found");
		return;
	}

	// Texture
	TextureArgs textureArgs;
	textureArgs.HDR = true;
	textureArgs.FlipVertically = true;
	textureArgs.Wrap = GL_CLAMP_TO_EDGE;
	m_Texture = ResourceManager::LoadNamed<Texture>(m_Args.Background, m_Args.Background, textureArgs);

	if (!m_Args.Reflection.empty() && filesystem::exists(m_Args.Reflection))
		m_ReflectionTexture = ResourceManager::LoadNamed<Texture>(m_Args.Reflection, m_Args.Reflection, textureArgs);

	// Shaders
	m_EquiToCubeShader = ResourceManager::LoadNamed<Shader>("Shaders/EquiToCube", ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Misc/Cubemap.vert",
			Application::AssetDir + "Shaders/Misc/EquiRectToCubemap.frag"
		});
	m_IrradianceShader = ResourceManager::LoadNamed<Shader>("Shaders/EnvironmentIrradiance", ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Misc/Cubemap.vert",
			Application::AssetDir + "Shaders/Misc/IrradianceConvolution.frag"
		});
	m_PreFilterShader = ResourceManager::LoadNamed<Shader>("Shaders/EnvironmentPreFilter", ShaderStageInfo
		{
			Application::AssetDir + "Shaders/Misc/Cubemap.vert",
			Application::AssetDir + "Shaders/Misc/PreFilterCubemap.frag"
		});
	m_BRDFShader = ResourceManager::LoadNamed<Shader>("Shaders/EnvironmentBRDF", ShaderStageInfo
		{
			Application::AssetDir + "Shaders/TextureQuad.vert",
			Application::AssetDir + "Shaders/Misc/BRDF.frag"
		});
}

EnvironmentMap::~EnvironmentMap()
{
	if (m_Framebuffer)				delete m_Framebuffer;
	if (m_IrradianceTexture)		delete m_IrradianceTexture;
	if (m_ReflectionCubemapTexture) delete m_ReflectionCubemapTexture;
	if (m_PreFilterTexture)			delete m_PreFilterTexture;

	ResourceManager::Unload(m_Texture);
}

RenderTexture* EnvironmentMap::GetBRDF()
{
	if (!m_Framebuffer)
		Generate();
	return m_BRDFTexture;
}

RenderTexture* EnvironmentMap::GetCubemap()
{
	if (!m_Framebuffer)
		Generate();
	return m_CubemapTexture;
}

RenderTexture* EnvironmentMap::GetIrradianceMap()
{
	if (!m_Framebuffer)
		Generate();
	return m_IrradianceTexture;
}

RenderTexture* EnvironmentMap::GetReflectionCubemap()
{
	if (!m_Framebuffer)
		Generate();
	return m_ReflectionCubemapTexture ? m_ReflectionCubemapTexture : m_CubemapTexture;
}

RenderTexture* EnvironmentMap::GetPrefilteredCubemap()
{
	if (!m_Framebuffer)
		Generate();
	return m_PreFilterTexture ? m_PreFilterTexture : nullptr;
}

ResourceID EnvironmentMap::GetTexture() { return m_Texture; }

void EnvironmentMap::Generate()
{
	if (m_Framebuffer || // Already generated
		m_Texture == InvalidResourceID // No texture found/loaded
		)
		return; 
	
	FramebufferSpec specs;
	specs.Attachments =
	{
		{ TextureFormat::Cubemap, TexturePixelType::Float },
		{ TextureFormat::RenderBuffer }
	};
	specs.Resolution = m_Resolution;
	m_Framebuffer = new Framebuffer(specs);
	m_Framebuffer->Bind();

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

	Shader* shader = ResourceManager::Get<Shader>(m_EquiToCubeShader);
	Mesh* cubeMesh = ResourceManager::Get<Mesh>(Mesh::Cube());

	RenderTextureArgs args;
	args.GenerateMipmaps = true;
	args.Format = TextureFormat::Cubemap;
	args.PixelType = TexturePixelType::Float;
	args.Resolution = m_Resolution;

	// Environment Cubemap //
	string cubemapPath = m_Args.Background + CubemapCacheExtension;
	if (!std::filesystem::exists(cubemapPath))
	{
		shader->Bind();
		shader->Set("equirectangularMap", 0);
		shader->Set("projectionMatrix", projection);

		Texture* inputTexture = ResourceManager::Get<Texture>(m_Texture);
		inputTexture->Bind(0);

		m_Framebuffer->Bind();

		for (unsigned int i = 0; i < 6; i++)
		{
			shader->Set("viewMatrix", views[i]);

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				m_Framebuffer->GetColourAttachment(i)->GetID(),
				0 // Mipmap level
			);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cubeMesh->Draw();
		}
		m_CubemapTexture = m_Framebuffer->GetColourAttachment();
		m_CubemapTexture->SaveHDR(cubemapPath);
	}
	else
		m_CubemapTexture = RenderTexture::LoadFromHDR(cubemapPath, args);

	// Reflection Cubemap //
	cubemapPath = m_Args.Reflection + CubemapCacheExtension;
	bool fileExists = filesystem::exists(cubemapPath);
	if (!fileExists && m_ReflectionTexture != InvalidResourceID)
	{
		args.GenerateMipmaps = true;
		m_ReflectionCubemapTexture = new RenderTexture(args);

		shader->Bind();
		shader->Set("equirectangularMap", 0);
		shader->Set("projectionMatrix", projection);

		Texture* inputTexture = ResourceManager::Get<Texture>(m_ReflectionTexture);
		inputTexture->Bind(0);

		m_Framebuffer->Bind();
		for (unsigned int i = 0; i < 6; i++)
		{
			shader->Set("viewMatrix", views[i]);

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				m_ReflectionCubemapTexture->GetID(),
				0 // Mipmap level
			);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cubeMesh->Draw();
		}

		m_ReflectionCubemapTexture->SaveHDR(cubemapPath);
	}
	else if(fileExists)
		m_ReflectionCubemapTexture = RenderTexture::LoadFromHDR(cubemapPath, args);

	// Irradiance Map Generation //
	const ivec2 irradianceResolution = { 128, 128 };

	args.GenerateMipmaps = false;
	args.Resolution = irradianceResolution;
	cubemapPath = m_Args.Background + IrradianceCacheExtension;
	if (!std::filesystem::exists(cubemapPath))
	{
		m_IrradianceTexture = new RenderTexture(args);

		shader = ResourceManager::Get<Shader>(m_IrradianceShader);
		shader->Bind();
		shader->Set("projectionMatrix", projection);
		shader->Set("environmentMap", 6);
		GetReflectionCubemap()->Bind(6);

		glViewport(0, 0, irradianceResolution.x, irradianceResolution.y);
		for (unsigned int i = 0; i < 6; i++)
		{
			shader->Set("viewMatrix", views[i]);

			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				m_IrradianceTexture->GetID(),
				0 // Mipmap level
			);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cubeMesh->Draw();
		}

		m_IrradianceTexture->SaveHDR(cubemapPath);
	}
	else
		m_IrradianceTexture = RenderTexture::LoadFromHDR(cubemapPath, args);

	// Pre-filter environment map
	args.GenerateMipmaps = true;
	args.MinFilter = GL_LINEAR_MIPMAP_LINEAR;
	args.Resolution = { 1024, 1024 }; // TODO: Expose this or test in different environments
	
	m_PreFilterTexture = new RenderTexture(args);
	shader = ResourceManager::Get<Shader>(m_PreFilterShader);

	shader->Bind();
	shader->Set("projectionMatrix", projection);
	shader->Set("environmentMap", 6);
	GetReflectionCubemap()->Bind(6);

	const unsigned int MaxMipLevels = 5;
	for (unsigned int mip = 0; mip < MaxMipLevels; mip++)
	{
		vec2 resolution =
		{
			args.Resolution.x * pow(0.5f, mip),
			args.Resolution.y * pow(0.5f, mip)
		};

		glViewport(0, 0, (GLsizei)resolution.x, (GLsizei)resolution.y);

		shader->Set("resolution", resolution);
		shader->Set("roughness", (float)mip / (float)(MaxMipLevels - 1));
		for (int i = 0; i < 6; i++)
		{
			shader->Set("viewMatrix", views[i]);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				m_PreFilterTexture->GetID(),
				mip
			);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cubeMesh->Draw();
		}
	}
	shader->Unbind();

	args.MinFilter = GL_LINEAR;
	args.GenerateMipmaps = false;
	args.Resolution = { 512, 512 };
	args.Format = TextureFormat::RG16F;
	args.PixelType = TexturePixelType::Float;
	m_BRDFTexture = new RenderTexture(args);

	shader = ResourceManager::Get<Shader>(m_BRDFShader);
	shader->Bind();

	glViewport(0, 0, args.Resolution.x, args.Resolution.y);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		m_BRDFTexture->GetID(),
		0
	);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ResourceManager::Get<Mesh>(Mesh::Quad())->Draw();
	shader->Unbind();

	m_Framebuffer->Unbind();
}