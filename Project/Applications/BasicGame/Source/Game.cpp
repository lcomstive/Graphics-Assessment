#include "Game.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include "Components/MouseLook.hpp"
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/Skybox.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>
#include <Engine/Graphics/EnvironmentMap.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>
#include <Engine/Components/OrbitCameraController.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Services;
using namespace Engine::Graphics;
using namespace Engine::Components;

#pragma region Environment Maps
#define ENVIRONMENT_MAP(dirName) { "Textures/Environment Maps/" dirName "/Environment.jpg", "Textures/Environment Maps/" dirName "/Reflection.hdr" }

const ivec2 EnvironmentMapRes = { 2048, 2048 };
vector<EnvironmentMapArgs> EnvironmentMaps =
{
	ENVIRONMENT_MAP("None"),
	ENVIRONMENT_MAP("Ice Lake"),
	ENVIRONMENT_MAP("Lobby"),
	ENVIRONMENT_MAP("Paper Mill"),
	ENVIRONMENT_MAP("Monument Valley")
};
unsigned int EnvironmentMapIndex = 0;
vector<EnvironmentMap*> EnvironmentMapData;

void SetEnvironmentMap(unsigned int index, bool force = false);
#pragma endregion

const float GridAlpha = 0.25f;
const int GridSize = 100;
const string FontPath = "Fonts/Source Sans Pro/SourceSansPro-Regular.ttf";

void Game::OnStart()
{
	// Set ImGui Font
	ImGui::GetIO().Fonts->AddFontFromFileTTF((Application::AssetDir + FontPath).c_str(), 16.0f);

	// Load environment map
	SetEnvironmentMap(EnvironmentMapIndex, true);

	ResetScene();
}

void Game::OnShutdown()
{

}

void Game::ResetScene()
{
	// Load scene
	SceneService* sceneService = Application::GetService<SceneService>();
	m_Scene = sceneService->CurrentScene();

	if (!m_Scene) // No scene active, load one in
		m_Scene = sceneService->NewScene("Game Scene");

	// Remove previously spawned objects
	for (GameObject* go : m_GameObjects)
		delete go;

#pragma region Player
	m_Player = new GameObject(m_Scene, "Player");
	MouseLook* mouseLook = m_Player->AddComponent<MouseLook>();
	mouseLook->Axis = MouseLook::LookAxis::Horizontal;
	mouseLook->Sensitivity = 5.0f;
	m_Player->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Cube(), Material { { 1, 0, 0, 1} } } };
	m_Player->GetTransform()->Position = { 0, 2, 0 };
	m_Player->GetTransform()->Scale = { 1, 2, 1 };

	// Player camera
	GameObject* cameraObj = new GameObject(m_Player, "Player Camera");
	cameraObj->AddComponent<Camera>();
	cameraObj->GetTransform()->Position = { 2, 2, 0 };

	GameObject* whereTheCameraWouldBe = new GameObject(m_Player, "CamSpot");
	whereTheCameraWouldBe->AddComponent<MeshRenderer>()->Meshes = { {Mesh::Cube(), Material {{0, 1, 0, 1}}} };
	whereTheCameraWouldBe->GetTransform()->Position = { 1, 2, 0 };
	whereTheCameraWouldBe->GetTransform()->Scale = vec3(0.1f);

	mouseLook = whereTheCameraWouldBe->AddComponent<MouseLook>();
	mouseLook->Axis = MouseLook::LookAxis::Vertical;
	mouseLook->Invert = true;

	cameraObj->AddComponent<OrbitCameraController>();
#pragma endregion

#pragma region World
	// Light
	GameObject* lightObj = new GameObject(m_Scene, "Light");
	m_Light = lightObj->AddComponent<Light>();

	m_Light->Type = LightType::Directional;
	m_Light->Intensity = 1.1f;

	lightObj->GetTransform()->Rotation = { radians(-60.0f), radians(25.0f), 0 };

	// Floor
	GameObject* floor = new GameObject(m_Scene, "Floor");
	Material floorMat;
	floorMat.Roughness = 1.0f;
	floorMat.Metalness = 0.2f;
	floor->GetTransform()->Scale = { 30, 1, 30 };
	floor->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Cube(), floorMat } };
#pragma endregion

	Input::ShowMouse(false);
}

void Game::OnUpdate(float deltaTime)
{
	if (Input::IsKeyPressed(GLFW_KEY_F11))
		Application::ToggleFullscreen();

	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
		Application::Exit();

	if (Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		Input::ToggleShowMouse();

		auto& mouseLooks = m_Player->GetComponentsInChildren<MouseLook>();
		for (MouseLook* mLook : mouseLooks)
			mLook->Enabled = !Input::IsMouseShowing();
	}
}

void Game::OnDraw()
{
	ImGui::Begin("Light");
	{
		static const char* LightTypeNames[] = { "Point", "Spot", "Directional" };
		if (ImGui::BeginCombo("Type", LightTypeNames[(int)m_Light->Type]))
		{
			for (int i = 0; i < 3; i++)
				if (ImGui::Selectable(LightTypeNames[i], (int)m_Light->Type == i))
					m_Light->Type = (LightType)i;
			ImGui::EndCombo();
		}

		bool castShadows = m_Light->GetCastShadows();
		ImGui::Checkbox("Cast Shadows", &castShadows);
		m_Light->SetCastShadows(castShadows);

		ImGui::DragFloat("Radius", &m_Light->Radius, 1.0f, 0.1f, 120.0f, "%.1f");
		ImGui::SliderFloat("Distance", &m_Light->Distance, 0.1f, 120.0f, "%.1f");
		ImGui::SliderFloat("Intensity", &m_Light->Intensity, 0.01f, 25.0f, "%.1f");
		ImGui::ColorEdit3("Colour", &m_Light->Colour[0]);
		ImGui::SliderFloat3("Position", &m_Light->GetTransform()->Position[0], -100, 100, "%.2f");

		vec3 rotation = degrees(m_Light->GetTransform()->Rotation);
		ImGui::SliderFloat3("Rotation", &rotation[0], -360.0f, 360.0f, "%.2f");
		m_Light->GetTransform()->Rotation = radians(rotation);

		// Environment Map
		static const char* EnvironmentMapNames[] = { "None", "Ice Lake", "Lobby", "Paper Mill", "Monument Valley" };
		if (ImGui::BeginCombo("Environment", EnvironmentMapNames[EnvironmentMapIndex]))
		{
			for (int i = 0; i < (int)EnvironmentMaps.size(); i++)
				if (ImGui::Selectable(EnvironmentMapNames[i], i == EnvironmentMapIndex))
					SetEnvironmentMap(i);
			ImGui::EndCombo();
		}

		if (Renderer::GetPipeline()->GetSkybox())
			ImGui::DragFloat(
				"Ambient Strength",
				&Renderer::GetPipeline()->GetSkybox()->AmbientLightStrength,
				0.01f,	// Speed
				0.01f,	// Min
				2.50f	// Max
			);
	}
	ImGui::End();
}

void SetEnvironmentMap(unsigned int index, bool force)
{
	if (!force && index == EnvironmentMapIndex)
		return; // No change
	if(EnvironmentMapData.empty())
		EnvironmentMapData.resize(EnvironmentMaps.size());

	EnvironmentMapIndex = index;
	if (!EnvironmentMapData[index])
	{
		EnvironmentMapArgs& args = EnvironmentMaps[index];
		args.Background = Application::AssetDir + args.Background;
		if (!args.Reflection.empty())
			args.Reflection = Application::AssetDir + args.Reflection;
		EnvironmentMapData[index] = new EnvironmentMap(args, EnvironmentMapRes);

		Log::Info("Loading environment map, this may take some time...");
	}

	Renderer::GetPipeline()->GetSkybox()->EnvironmentMap = EnvironmentMapData[index];
}