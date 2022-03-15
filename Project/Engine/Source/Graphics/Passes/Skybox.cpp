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

Skybox::Skybox() : EnvironmentMap(nullptr)
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
	m_Shader->Set("environmentMap", 0);
	EnvironmentMap->GetCubemap()->Bind(0);

	Camera* camera = Camera::GetMainCamera();
	if (camera)
		camera->FillShader(m_Shader);

	ResourceManager::Get<Mesh>(Mesh::Cube())->Draw();

	m_Shader->Unbind();

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
}