#include "Inspector.hpp"
#include "ModelViewer.hpp"
#include <misc/cpp/imgui_stdlib.h>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/Skybox.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>
#include <Engine/Graphics/EnvironmentMap.hpp>
#include <Engine/Components/OrbitCameraController.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace MV;
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

void ModelViewer::OnStart()
{
	// Set ImGui Font
	ImGui::GetIO().Fonts->AddFontFromFileTTF((Application::AssetDir + FontPath).c_str(), 16.0f);

	// Generate GridSize x GridSize grid
	m_GridMeshID = Mesh::Grid((unsigned int)GridSize);

	// Generate line, for showing axes on grid
	m_LineID = ResourceManager::Load<Mesh>(vector<Mesh::Vertex>
	{
		{ vec3(0) }, { vec3(0, 1, 0) } // Vertices
	}, vector<unsigned int>(), Mesh::DrawMode::Lines);

	// Load environment map
	SetEnvironmentMap(EnvironmentMapIndex, true);

	// Load scene
	SceneService* sceneService = Application::GetService<SceneService>();
	m_Scene = sceneService->CurrentScene();

	if (!m_Scene) // No scene active, load one in
		m_Scene = sceneService->NewScene("Model Viewer");

	// Load default model
	LoadModel(DefaultModelPath);

	// Create basic orbit camera
	GameObject* cameraObj = new GameObject(m_Scene, "Camera");
	cameraObj->AddComponent<OrbitCameraController>();

	GameObject* lightObj = new GameObject(m_Scene, "Light");
	m_Light = lightObj->AddComponent<Light>();

	m_Light->Type = LightType::Directional;
	m_Light->Intensity = 0.75f;

	lightObj->GetTransform()->Rotation = { radians(60.0f), radians(25.0f), 0 };
}

void ModelViewer::OnShutdown()
{
	if (m_GameObject)
		delete m_GameObject;
	if (m_Light)
		delete m_Light->GetGameObject();

	ResourceManager::Unload(m_LineID);
	ResourceManager::Unload(m_ModelID);
	ResourceManager::Unload(m_GridMeshID);
}

void ModelViewer::LoadModel(std::string path)
{
	if (path == m_CurrentModelPath)
		return; // No change

	m_CurrentModelPath = m_NewModelPath = path;

	if (m_GameObject)
	{
		delete m_GameObject;
		m_GameObject = nullptr;
	}

	// Unload previously loaded model data
	if (m_ModelID != InvalidResourceID)
		ResourceManager::Unload(m_ModelID);

	m_ModelID = ResourceManager::Load<Model>(Application::AssetDir + string(m_CurrentModelPath));

	Model* model = ResourceManager::Get<Model>(m_ModelID);

	if (!model || model->GetMeshes().empty())
		return;

	// Create GameObject.
	// Model->CreateEntity creates hierarchy based off model hierarchy,
	//	adding MeshRenderer components as required
	m_GameObject = model->CreateEntity(&m_Scene->Root());

	m_GameObject->GetTransform()->Scale = vec3(0.1f);
}

void ModelViewer::OnUpdate(float deltaTime)
{
	if (Input::IsKeyPressed(GLFW_KEY_F11))
		Application::ToggleFullscreen();
}

void ModelViewer::OnDraw()
{
	if (ImGui::Begin("Model"))
	{
		bool loadModel = ImGui::InputText("Path", &m_NewModelPath, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if(ImGui::Button("->") || loadModel)
			LoadModel(m_NewModelPath);

		ImGui::Checkbox("Show Grid", &m_DrawGrid);

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
		ImGui::SliderFloat3("Rotation", &rotation[0], 0.0f, 360.0f, "%.2f");
		m_Light->GetTransform()->Rotation = radians(rotation);
	}
	ImGui::End();

	DrawHierarchy(m_Scene);
}

void ModelViewer::OnDrawGizmos()
{
	if (!m_DrawGrid)
		return;

	Gizmos::SetLineWidth(1.0f);
	Gizmos::SetColour(1, 1, 1, 0.25f);
	Gizmos::Draw(m_GridMeshID, vec3(-GridSize, 0, -GridSize), vec3(GridSize * 2.0f));

	Gizmos::SetLineWidth(4.0f);

	// Draw Y Axis
	Gizmos::SetColour(0, 1, 0, GridAlpha);
	Gizmos::Draw(m_LineID);

	// Draw X Axis
	Gizmos::SetColour(1, 0, 0, GridAlpha);
	Gizmos::Draw(m_LineID, vec3(0), vec3(1), vec3(radians(90.0f), 0, 0));

	// Draw Z Axis
	Gizmos::SetColour(0, 0, 1, GridAlpha);
	Gizmos::Draw(m_LineID, vec3(0), vec3(1), vec3(0, 0, radians(90.0f)));

	Gizmos::SetLineWidth(1.0f);
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
