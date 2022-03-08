#include <iostream>
#include <Engine/Log.hpp>
#include <TestService.hpp>
#include <Engine/Input.hpp>
#include <Engine/Application.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Services;
using namespace Engine::Components;

Light* light = nullptr;

void TestService::OnStart()
{
	Log::Info("Test service started!");

	Scene* scene = Application::GetService<SceneService>()->CurrentScene();

	GameObject* lightGO = new GameObject(scene, "Test Service light");
	light = lightGO->AddComponent<Light>();
	light->Colour = { 1, 1, 1 };
	light->Radius = 100.0f;
	light->Intensity = 10.0f;
}

void TestService::OnShutdown()
{
	if(light)
		delete light->GetGameObject();

	Log::Info("Test service shutting down...");
}

void TestService::OnDraw()
{
	if (!light)
		return;

	if (ImGui::Begin("Test Service Window"))
	{
		ImGui::Text(("Light Radius: " + to_string(light->Radius)).c_str());
		ImGui::Text(("Light Intensity: " + to_string(light->Intensity)).c_str());
		ImGui::Spacing();
		ImGui::Text("Control using arrow keys");
		ImGui::Text("Control Radius:    Left / Right");
		ImGui::Text("Control Intensity: Up   / Down");
		ImGui::End();
	}
}

void TestService::OnDrawGizmos() { }

void TestService::OnUpdate(float dt)
{
	if (!light)
		return;

	if (Input::IsKeyDown(GLFW_KEY_UP))	  light->Intensity += dt * 10.0f;
	if (Input::IsKeyDown(GLFW_KEY_DOWN))  light->Intensity -= dt * 10.0f;
	if (Input::IsKeyDown(GLFW_KEY_LEFT))  light->Radius -= dt * 25.0f;
	if (Input::IsKeyDown(GLFW_KEY_RIGHT)) light->Radius += dt * 25.0f;
}