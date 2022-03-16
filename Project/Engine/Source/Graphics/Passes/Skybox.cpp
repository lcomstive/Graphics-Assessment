#include <Engine/Application.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Graphics/Passes/Skybox.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

Skybox::Skybox() : EnvironmentMap(nullptr), AmbientLightStrength(1.0f)
{
	m_ShaderID = ResourceManager::LoadNamed<Shader>("Shaders/Skybox", ShaderStageInfo
	{
		Application::AssetDir + "Shaders/Misc/Skybox.vert",
		Application::AssetDir + "Shaders/Misc/Skybox.frag"
	});

	m_Shader = ResourceManager::Get<Shader>(m_ShaderID);
}

Skybox::~Skybox() {}

void Skybox::Draw()
{
	if (!EnvironmentMap)
		return;

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	m_Shader->Bind();

	FillShaderData(m_Shader);

	Camera* camera = Camera::GetMainCamera();
	if (camera)
		camera->FillShader(m_Shader);

	ResourceManager::Get<Mesh>(Mesh::Cube())->Draw();

	m_Shader->Unbind();

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
}

void Skybox::FillShaderData(Shader* shader)
{
	shader->Set("environment.AmbientLightStrength", AmbientLightStrength);

	// Environment Map
	shader->Set("environment.EnvironmentMap", 6);
	shader->Set("environment.IrradianceMap", 7);
	shader->Set("environment.ReflectionMap", 8);
	shader->Set("environment.PrefilterMap", 9);
	shader->Set("environment.BRDFMap", 10);

	// Environment Map
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP,
		EnvironmentMap && EnvironmentMap->GetCubemap() ?
		EnvironmentMap->GetCubemap()->GetID() : 0
	);

	// Irradiance Map
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_CUBE_MAP,
		EnvironmentMap && EnvironmentMap->GetIrradianceMap() ?
		EnvironmentMap->GetIrradianceMap()->GetID() : 0
	);

	// Reflection Map
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP,
		EnvironmentMap && EnvironmentMap->GetReflectionCubemap() ?
		EnvironmentMap->GetReflectionCubemap()->GetID() : 0
	);
	
	// Prefilter Map
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_CUBE_MAP,
		EnvironmentMap && EnvironmentMap->GetPrefilteredCubemap() ?
		EnvironmentMap->GetPrefilteredCubemap()->GetID() : 0
	);
	
	// BRDF Map
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D,
		EnvironmentMap && EnvironmentMap->GetBRDF() ?
		EnvironmentMap->GetBRDF()->GetID() : 0
	);
}