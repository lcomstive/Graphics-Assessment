#include <imgui.h>
#include "Demo.hpp"
#include <Engine/Utilities.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/PostProcessing.hpp>
#include <Engine/Services/ExternalService.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Components/Physics/BoxCollider.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>
#include <Engine/Components/OrbitCameraController.hpp>

#pragma warning(disable : 4244)

#define DRAW_GRID 1

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Physics;
using namespace Engine::Services;
using namespace Engine::Graphics;
using namespace Engine::Components;

#if DRAW_GRID
// Grid
ResourceID GridMeshID = InvalidResourceID;
const int GridSize = 250;
#endif

vector<GameObject*> CreatedObjects;

// Main Model
const string ModelPath = "Models/Cerberus/Cerberus_LP.fbx";
const string NormalMapPath = "Models/Cerberus/Textures/Cerberus_N.tga";
const string MetalnessMapPath = "Models/Cerberus/Textures/Cerberus_M.tga";
const string RoughnessMapPath = "Models/Cerberus/Textures/Cerberus_R.tga";

ResourceID MainModel;
Texture *NormalMap, *MetalnessMap, *RoughnessMap;

// Floor Texture
const string FloorTexturePath = "Textures/Rock/Rock_044_BaseColor.jpg";
const string FloorNormalMapPath = "Textures/Rock/Rock_044_Normal.jpg";
const string FloorRoughnessMapPath = "Textures/Rock/Rock_044_Roughness.jpg";
const string FloorHeightMapPath = "Textures/Rock/Rock_044_Height.jpg";
Material FloorMaterial;

// Rusted Iron Texture
const string RustedIronTexturePath = "Textures/Rusted Iron/rustediron2_basecolor.png";
const string RustedIronNormalMapPath = "Textures/Rusted Iron/rustediron2_normal.png";
const string RustedIronRoughnessMapPath = "Textures/Rusted Iron/rustediron2_roughness.png";
const string RustedIronMetallicMapPath = "Textures/Rusted Iron/rustediron2_metallic.png";
Material RustedIronMaterial;

// Post Processing
TonemappingPass* PP_Tonemapping;

// Lights
struct LightData
{
	Light* Component;
	float Speed;
	float Distance;
	float Offset;
	Transform* Transform;
	bool SwapDirection = false;
};
vector<LightData> Lights;

#if _WIN32
const string ExternalServicePath = "Services/Test Service.dll";
#else
const string ExternalServicePath = "Services/Test Service.so";
#endif

void SetMaterialTextures(Model::MeshData& meshData)
{
	for(Material& mat : meshData.Materials)
	{
		mat.NormalMap = NormalMap;
		mat.MetalnessMap = MetalnessMap;
		mat.RoughnessMap = RoughnessMap;
	}

	for (Model::MeshData& child : meshData.Children)
		SetMaterialTextures(child);
}

void Demo::OnStart()
{
	string appDir = Application::AssetDir;
	// Set ImGui Font
	ImGui::GetIO().Fonts->AddFontFromFileTTF((appDir + "Fonts/Source Sans Pro/SourceSansPro-Regular.ttf").c_str(), 16.0f);

	// Main Model
	MainModel = ResourceManager::Load<Model>(appDir + ModelPath);
	NormalMap = new Texture(appDir + NormalMapPath);
	MetalnessMap = new Texture(appDir + MetalnessMapPath);
	RoughnessMap = new Texture(appDir + RoughnessMapPath);
	SetMaterialTextures(ResourceManager::Get<Model>(MainModel)->GetRootMeshData());

	// Floor
	FloorMaterial.AlbedoMap = new Texture(appDir + FloorTexturePath);
	FloorMaterial.NormalMap = new Texture(appDir + FloorNormalMapPath);
	FloorMaterial.RoughnessMap = new Texture(appDir + FloorRoughnessMapPath);

	// Rusted Iron Material
	RustedIronMaterial.AlbedoMap = new Texture(appDir + RustedIronTexturePath);
	RustedIronMaterial.NormalMap = new Texture(appDir + RustedIronNormalMapPath);
	RustedIronMaterial.RoughnessMap = new Texture(appDir + RustedIronRoughnessMapPath);
	RustedIronMaterial.MetalnessMap = new Texture(appDir + RustedIronMetallicMapPath);

	// Tonemapping
	PP_Tonemapping = new TonemappingPass();
	// Renderer::GetPipeline()->AddPass(PP_Tonemapping->GetPipelinePass());

	// Create scene
	ResetScene();

	// Load external service
	Application::GetService<ExternalServices>()->Add(appDir + ExternalServicePath);
}

void Demo::OnShutdown()
{
	delete PP_Tonemapping;

#if DRAW_GRID
	ResourceManager::Unload(GridMeshID);
#endif
}

void Demo::ResetScene()
{
	// Clean up any pre-existing resources
	for (GameObject* go : CreatedObjects)
		delete go;
	CreatedObjects.clear();

	SceneService* sceneService = Application::GetService<SceneService>();
	Scene* scene = sceneService->CurrentScene();
	if (!scene)
		scene = sceneService->NewScene("Demo Scene");
	else
		scene->Clear();

	// Main Camera //
	GameObject* cameraObj = new GameObject(scene, "Main Camera");
	cameraObj->AddComponent<OrbitCameraController>();
	cameraObj->GetTransform()->Position = { 0, 0, 20 };
	cameraObj->GetTransform()->Rotation = { 0, radians(-90.0f), 0 }; // From euler angles
	CreatedObjects.emplace_back(cameraObj);

	// Cerberus //
	Model* model = ResourceManager::Get<Model>(MainModel);
	GameObject* cerberus = model->CreateEntity(&scene->Root());
	cerberus->GetTransform()->Scale = vec3(0.1f);
	cerberus->GetTransform()->Position = { 0, 1, 5 };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(90.0f) };
	CreatedObjects.emplace_back(cerberus);

	cerberus = model->CreateEntity(&scene->Root());
	cerberus->GetTransform()->Scale = vec3(0.075f);
	cerberus->GetTransform()->Position = { -2.5f, 1, -2 };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(-60.0f) };
	CreatedObjects.emplace_back(cerberus);

	cerberus = model->CreateEntity(&scene->Root());
	cerberus->GetTransform()->Scale = vec3(0.05f);
	cerberus->GetTransform()->Position = { -5, 1, -2.5f };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(160.0f) };
	CreatedObjects.emplace_back(cerberus);

	// Floor //
	GameObject* floor = new GameObject(scene, "Floor");
	floor->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Cube(), FloorMaterial } };
	floor->GetTransform()->Position = { 0, -7.5f, 0 };
	floor->GetTransform()->Scale = { 10, 1, 10 };

	Material lightMaterial;

	const int LightCount = 4;
	for (int i = 0; i < LightCount; i++)
	{
		lightMaterial.Albedo =
		{
			Random(0.5f, 1.0f),
			Random(0.5f, 1.0f),
			Random(0.5f, 1.0f),
			1.0
		};
		LightData data;
		GameObject* light = new GameObject(scene, "Light");
		light->GetTransform()->Scale = vec3(0.25f);
		light->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Sphere(), lightMaterial } };

		data.Component = light->AddComponent<Light>();
		data.Component->Colour = lightMaterial.Albedo;
		data.Transform = light->GetTransform();
		data.Transform->Position.y = Random(-10.0f, 10.0f);
		data.Component->Radius = 20.0f;
		data.Speed = Random(2.5f, 10.0f);
		data.Distance = Random(5.0f, 15.0f);
		data.SwapDirection = Random(0, 100) > 50;
		data.Offset = radians(Random(0, 360));

		Lights.emplace_back(data);

		CreatedObjects.emplace_back(light);
	}

	const int Rows = 5;
	const float RowSpacing = 2.5f;
	for (int i = 0; i < Rows; i++)
	{
		for (int j = 0; j < Rows; j++)
		{
			GameObject* sphere = new GameObject(scene, "Sphere (" + to_string(i) + ", " + to_string(j) + ")");
			sphere->AddComponent<MeshRenderer>()->Meshes = { { j % 2 == 0 ? Mesh::Sphere() : Mesh::Cube(), RustedIronMaterial}};
			sphere->GetTransform()->Position =
			{
				(j - (Rows / 2.0f)) * RowSpacing,
				(i - (Rows / 2.0f)) * RowSpacing,
				0.0f
			};
			CreatedObjects.emplace_back(sphere);
		}
	}
}

void Demo::OnUpdate(float deltaTime)
{
	Scene* scene = Application::GetService<Services::SceneService>()->CurrentScene();
	
	if (Input::IsKeyPressed(GLFW_KEY_F11))
		Application::ToggleFullscreen();
	
	/*
	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
		Application::Exit();
	*/

	if (Input::IsKeyPressed(GLFW_KEY_SPACE))
		scene->GetPhysics().TogglePause();

	vec3 lightMovement = vec3(0.0f);
	const float LightMoveSpeed = 2.5f;
	if (Input::IsKeyDown(GLFW_KEY_UP))	  lightMovement.y += LightMoveSpeed * deltaTime;
	if (Input::IsKeyDown(GLFW_KEY_DOWN))  lightMovement.y -= LightMoveSpeed * deltaTime;
	if (Input::IsKeyDown(GLFW_KEY_LEFT))  lightMovement.x -= LightMoveSpeed * deltaTime;
	if (Input::IsKeyDown(GLFW_KEY_RIGHT)) lightMovement.x += LightMoveSpeed * deltaTime;

	for (LightData data : Lights)
	{
		float time = Renderer::GetTime() * (data.SwapDirection ? -1.0f : 1.0f) * data.Speed + data.Offset;
		data.Transform->Position.x = cos(radians(time)) * data.Distance;
		data.Transform->Position.z = sin(radians(time)) * data.Distance;
	}
}

void Demo::OnDraw()
{
	Scene* scene = Application::GetService<Services::SceneService>()->CurrentScene();
	
	// Draw GUI
	static bool controlsWindowOpen = true;
	if (ImGui::Begin("Controls", &controlsWindowOpen))
	{
		ImGui::Text("Space:		  Toggle pause physics");
		ImGui::Text("WASD:		  Move camera");
		ImGui::Text("Q/E:		  Move camera up/down");
		ImGui::Text("Right Mouse: Hold and move mouse to look around");
		ImGui::Text("F:			  Applies force at cursor position");
		ImGui::Text("F11:		  Toggle fullscreen");

		ImGui::End();
	}

	const ImVec4 ColourGood = { 1, 1, 1, 1 };
	const ImVec4 ColourBad = { 1, 0, 0, 1 };

	static bool debugWindowOpen = true;
	PhysicsSystem& physicsSystem = scene->GetPhysics();
	if (ImGui::Begin("Debugging", &debugWindowOpen))
	{
		float frameTime = Renderer::GetDeltaTime() * 1000.0f;
		float lastTimeStep = physicsSystem.LastTimestep().count();
		float desiredTimestep = physicsSystem.Timestep().count();

		ImGui::Text("FPS: %f\n", Renderer::GetFPS());
		ImGui::Text("Total Objects: %i", (int)CreatedObjects.size());
		ImGui::TextColored(
			(frameTime < (1000.0f / 30.0f)) ? ColourGood : ColourBad,
			"Render  Frame Time: %.1fms",
			frameTime);
		ImGui::TextColored(
			(lastTimeStep <= desiredTimestep) ? ColourGood : ColourBad,
			"Physics Frame Time: %.1fms / %.1fms",
			lastTimeStep, desiredTimestep
		);
		ImGui::Text("Resolution: (%d, %d)", Renderer::GetResolution().x, Renderer::GetResolution().y);
		ImGui::Text("VSync: %s", Renderer::GetVSync() ? "Enabled" : "Disabled");
		ImGui::Text("Samples: %d", Renderer::GetSamples());

		static string TonemapperNames[] = {"None", "Aces", "Reinhard", "Exposure"};
		if (ImGui::BeginCombo("Tonemapper", TonemapperNames[(int)PP_Tonemapping->Tonemapper].c_str()))
		{
			for (int i = 0; i < 4; i++)
				if (ImGui::Selectable(TonemapperNames[i].c_str(), (int)PP_Tonemapping->Tonemapper == i))
					PP_Tonemapping->Tonemapper = (Tonemapper)i;
			ImGui::EndCombo();
		}

		if (scene->GetPhysics().GetState() == PhysicsPlayState::Paused)
			ImGui::Text("PHYSICS PAUSED");

		if (ImGui::Button("Reload External Service"))
			Application::GetService<ExternalServices>()->Reload(Application::AssetDir + ExternalServicePath);

		ImGui::End();
	}
}

void Demo::OnDrawGizmos()
{
#if DRAW_GRID
	// Draw grid as gizmo
	if (GridMeshID == InvalidResourceID)
		GridMeshID = Mesh::Grid(GridSize);

	Gizmos::SetColour(1, 1, 1, 0.2f);
	Gizmos::Draw(GridMeshID, vec3(-GridSize, 0.0f, -GridSize), vec3(GridSize * 2.0f));
#endif
}
