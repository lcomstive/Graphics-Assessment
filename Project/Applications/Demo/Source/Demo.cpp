#include <imgui.h>
#include "Demo.hpp"
#include <Engine/Utilities.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/PostProcessing.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Components/Physics/BoxCollider.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>
#include <Engine/Components/OrbitCameraController.hpp>

#pragma warning(disable : 4244)

#define DRAW_GRID			1

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

Demo::Demo(ApplicationArgs args) : Application(args) { }

#if DRAW_GRID
// Grid
Mesh* gridMesh = nullptr;
const int GridSize = 250;
#endif

vector<GameObject*> CreatedObjects;

// Main Model
const string ModelPath = "Models/Cerberus/Cerberus_LP.fbx";
const string NormalMapPath = "Models/Cerberus/Textures/Cerberus_N.tga";
const string MetalnessMapPath = "Models/Cerberus/Textures/Cerberus_M.tga";
const string RoughnessMapPath = "Models/Cerberus/Textures/Cerberus_R.tga";

Model* MainModel;
Texture *NormalMap, *MetalnessMap, *RoughnessMap;

// Floor Texture
const string FloorTexturePath = "Textures/Rock/Rock_044_BaseColor.jpg";
const string FloorNormalMapPath = "Textures/Rock/Rock_044_Normal.jpg";
const string FloorRoughnessMapPath = "Textures/Rock/Rock_044_Roughness.jpg";
const string FloorHeightMapPath = "Textures/Rock/Rock_044_Height.jpg";
Material FloorMaterial = {};

// Rusted Iron Texture
const string RustedIronTexturePath = "Textures/Rusted Iron/rustediron2_basecolor.png";
const string RustedIronNormalMapPath = "Textures/Rusted Iron/rustediron2_normal.png";
const string RustedIronRoughnessMapPath = "Textures/Rusted Iron/rustediron2_roughness.png";
const string RustedIronMetallicMapPath = "Textures/Rusted Iron/rustediron2_metallic.png";
Material RustedIronMaterial = {};

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
	// Set ImGui Font
	ImGui::GetIO().Fonts->AddFontFromFileTTF((AssetDir + "Fonts/Source Sans Pro/SourceSansPro-Regular.ttf").c_str(), 16.0f);

	// Main Model
	MainModel = new Model(AssetDir + ModelPath);
	NormalMap = new Texture(AssetDir + NormalMapPath);
	MetalnessMap = new Texture(AssetDir + MetalnessMapPath);
	RoughnessMap = new Texture(AssetDir + RoughnessMapPath);
	SetMaterialTextures(MainModel->GetRootMeshData());

	// Floor
	FloorMaterial.AlbedoMap = new Texture(AssetDir + FloorTexturePath);
	FloorMaterial.NormalMap = new Texture(AssetDir + FloorNormalMapPath);
	FloorMaterial.RoughnessMap = new Texture(AssetDir + FloorRoughnessMapPath);

	// Rusted Iron Material
	RustedIronMaterial.AlbedoMap = new Texture(AssetDir + RustedIronTexturePath);
	RustedIronMaterial.NormalMap = new Texture(AssetDir + RustedIronNormalMapPath);
	RustedIronMaterial.RoughnessMap = new Texture(AssetDir + RustedIronRoughnessMapPath);
	RustedIronMaterial.MetalnessMap = new Texture(AssetDir + RustedIronMetallicMapPath);


	PP_Tonemapping = new TonemappingPass();
	Renderer::GetPipeline()->AddPass(PP_Tonemapping->GetPipelinePass());

	ResetScene();
}

void Demo::OnShutdown()
{
	delete PP_Tonemapping;

#if DRAW_GRID
	delete gridMesh;
#endif
}

void Demo::ResetScene()
{
	// Clean up any pre-existing resources
	for (GameObject* go : CreatedObjects)
		delete go;
	CreatedObjects.clear();

	CurrentScene()->Clear();

	// Main Camera //
	GameObject* cameraObj = new GameObject(CurrentScene(), "Main Camera");
	cameraObj->AddComponent<OrbitCameraController>();
	cameraObj->GetTransform()->Position = { 0, 0, 20 };
	cameraObj->GetTransform()->Rotation = { 0, radians(-90.0f), 0 }; // From euler angles
	CreatedObjects.emplace_back(cameraObj);

	// Cerberus //
	GameObject* cerberus = MainModel->CreateEntity(&CurrentScene()->Root());
	cerberus->GetTransform()->Scale = vec3(0.1f);
	cerberus->GetTransform()->Position = { 0, 1, 5 };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(90.0f) };
	CreatedObjects.emplace_back(cerberus);

	cerberus = MainModel->CreateEntity(&CurrentScene()->Root());
	cerberus->GetTransform()->Scale = vec3(0.075f);
	cerberus->GetTransform()->Position = { -2.5f, 1, -2 };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(-60.0f) };
	CreatedObjects.emplace_back(cerberus);

	cerberus = MainModel->CreateEntity(&CurrentScene()->Root());
	cerberus->GetTransform()->Scale = vec3(0.05f);
	cerberus->GetTransform()->Position = { -5, 1, -2.5f };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(160.0f) };
	CreatedObjects.emplace_back(cerberus);

	// Floor //
	GameObject* floor = new GameObject(CurrentScene(), "Floor");
	floor->AddComponent<MeshRenderer>()->Meshes = {{ Mesh::Quad(), FloorMaterial }};
	floor->GetTransform()->Position = { 0, -5, 0 };
	floor->GetTransform()->Scale = { 10, 10, 1 };
	floor->GetTransform()->Rotation = { radians(90.0f), 0, 0 };

	const int LightCount = 30;
	for (int i = 0; i < LightCount; i++)
	{
		vec4 lightColour =
		{
			Random(0.5f, 1.0f),
			Random(0.5f, 1.0f),
			Random(0.5f, 1.0f),
			1.0
		};
		LightData data;
		GameObject* light = new GameObject(CurrentScene(), "Light");
		light->GetTransform()->Scale = vec3(0.25f);
		light->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Sphere(), Material { lightColour }} };

		data.Component = light->AddComponent<Light>();
		data.Component->Colour = lightColour;
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
			GameObject* sphere = new GameObject(CurrentScene(), "Sphere (" + to_string(i) + ", " + to_string(j) + ")");
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

void Demo::OnUpdate()
{
	if (Input::IsKeyPressed(GLFW_KEY_F11))
		ToggleFullscreen();

	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
		Exit();

	if (Input::IsKeyPressed(GLFW_KEY_SPACE))
		CurrentScene()->GetPhysics().TogglePause();

	vec3 lightMovement = vec3(0.0f);
	const float LightMoveSpeed = 2.5f;
	float deltaTime = Renderer::GetDeltaTime();
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
	PhysicsSystem& physicsSystem = CurrentScene()->GetPhysics();
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

		if (CurrentScene()->GetPhysics().GetState() == PhysicsPlayState::Paused)
			ImGui::Text("PHYSICS PAUSED");

		ImGui::End();
	}
}

void Demo::OnDrawGizmos()
{
#if DRAW_GRID
	// Draw grid as gizmo
	if (!gridMesh)
		gridMesh = Mesh::Grid(GridSize);

	Gizmos::Colour = { 1, 1, 1, 0.2f };
	Gizmos::Draw(gridMesh, vec3(-GridSize, 0.0f, -GridSize), vec3(GridSize * 2.0f));

#endif
}
