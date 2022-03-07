#include <iostream>
#include <Engine/Log.hpp>
#include <TestService.hpp>
#include <Engine/Input.hpp>
#include <Engine/Application.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Services;
using namespace Engine::Components;

GameObject* go;

void TestService::OnStart()
{
	Log::Info("Test service started!");

	Scene* scene = Application::GetService<SceneService>()->CurrentScene();
	
	go = new GameObject(scene, "Test GO");

	Material mat;
	mat.Albedo = { 1, 0, 1, 1 };
	go->GetTransform()->Scale = vec3(5.0f);
	go->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Cube(), mat } };
}

void TestService::OnShutdown()
{
	Log::Info("Test service shutting down...");
}

void TestService::OnDraw()
{
	if (ImGui::Begin("Test Service Window"))
	{
		ImGui::Text("TESTING");
		ImGui::End();
	}
}

void TestService::OnDrawGizmos()
{
	Gizmos::SetColour(1, 0, 1, 1);
	Gizmos::DrawWireSphere(vec3(0), 5.0f);
}

void TestService::OnUpdate(float dt)
{
	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
		Application::Exit();

	go->GetTransform()->Position.y = cos(Renderer::GetTime());
}