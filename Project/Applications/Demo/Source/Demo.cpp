#include <imgui.h>
#include "Demo.hpp"
#include <Engine/Utilities.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Graphics/Passes/Skybox.hpp>
#include <Engine/Services/ExternalService.hpp>
#include <Engine/Graphics/Passes/ShadowMap.hpp>
#include <Engine/Graphics/Pipelines/Forward.hpp>
#include <Engine/Graphics/Pipelines/Deferred.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Graphics/Passes/PostProcessing.hpp>
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
using namespace Engine::Graphics::Pipelines;

#if DRAW_GRID
// Grid
ResourceID GridMeshID = InvalidResourceID;
const int GridSize = 250;
#endif

vector<GameObject*> CreatedObjects;

bool DeferredRenderer = false;

// Main Model
#define CERBERUS	0
#define SPONZA		0
#define IBL_TEST	1

#if CERBERUS
ResourceID CerberusModel;
const string CerberusModelPath = "Models/Cerberus/Cerberus_LP.fbx";
const string CerberusNormalMapPath = "Models/Cerberus/Textures/Cerberus_N.tga";
const string CerberusMetalnessMapPath = "Models/Cerberus/Textures/Cerberus_M.tga";
const string CerberusRoughnessMapPath = "Models/Cerberus/Textures/Cerberus_R.tga";
ResourceID CerberusNormalMap, CerberusMetalnessMap, CerberusRoughnessMap;

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

vector<LightData> CerberusLights;
#endif

#if SPONZA
ResourceID SponzaModel;
vector<Light*> SponzaLights;
const string SponzaModelPath = "Models/Sponza/sponza.glb";
#endif

#if IBL_TEST
const vector<string> EnvironmentMaps =
{
	"Textures/Environment Maps/Ice Lake.jpg",
	"Textures/Environment Maps/Lobby.jpg",
	"Textures/Environment Maps/Paper Mill.jpg",
	"Textures/Environment Maps/Road to Monument Valley.jpg",
};

EnvironmentMap* EnvMap;
#endif

// Floor Texture
const string FloorTexturePath = "Textures/Rusted Iron/rustediron2_basecolor.png";
const string FloorNormalMapPath = "Textures/Rusted Iron/rustediron2_normal.png";
const string FloorRoughnessMapPath = "Textures/Rusted Iron/rustediron2_roughness.png";
const string FloorMetalnessMapPath = "Textures/Rusted Iron/rustediron2_metallic.png";
Material FloorMaterial;

// Post Processing
TonemappingPass* PP_Tonemapping;

#if _WIN32
const string ExternalServicePath = "Services/Test Service.dll";
#else
const string ExternalServicePath = "Services/Test Service.so";
#endif

void SetMaterialTextures(
	Model::MeshData& meshData,
	ResourceID normalMap,
	ResourceID metalnessMap = InvalidResourceID,
	ResourceID roughnessMap = InvalidResourceID)
{
	for (Material& mat : meshData.Materials)
	{
		mat.NormalMap = normalMap;
		mat.MetalnessMap = metalnessMap;
		mat.RoughnessMap = roughnessMap;
	}

	for (Model::MeshData& child : meshData.Children)
		SetMaterialTextures(child, normalMap, metalnessMap, roughnessMap);
}

Light* directionalLight = nullptr;
void Demo::OnStart()
{
	Log::SetLogLevel(Log::LogLevel::All);

	const string Title = "Demo App";
	Application::SetTitle(Title);

	string appDir = Application::AssetDir;
	// Set ImGui Font
	ImGui::GetIO().Fonts->AddFontFromFileTTF((appDir + "Fonts/Source Sans Pro/SourceSansPro-Regular.ttf").c_str(), 16.0f);

#if CERBERUS
	CerberusModel = ResourceManager::Load<Model>(appDir + CerberusModelPath);
	CerberusNormalMap = ResourceManager::Load<Texture>(appDir + CerberusNormalMapPath);
	CerberusMetalnessMap = ResourceManager::Load<Texture>(appDir + CerberusMetalnessMapPath);
	CerberusRoughnessMap = ResourceManager::Load<Texture>(appDir + CerberusRoughnessMapPath);

	SetMaterialTextures(
		ResourceManager::Get<Model>(CerberusModel)->GetRootMeshData(),
		CerberusNormalMap,
		CerberusMetalnessMap,
		CerberusRoughnessMap
	);
#endif

#if SPONZA
	SponzaModel = ResourceManager::Load<Model>(appDir + SponzaModelPath);
#endif

#if IBL_TEST
	EnvMap = new EnvironmentMap(Application::AssetDir + EnvironmentMaps[1]);
	Renderer::GetPipeline()->GetSkybox()->EnvironmentMap = EnvMap;
#endif

	// Floor
	FloorMaterial.AlbedoMap = ResourceManager::LoadNamed<Texture>("FloorAlbedo", appDir + FloorTexturePath);
	FloorMaterial.NormalMap = ResourceManager::LoadNamed<Texture>("FloorNormal", appDir + FloorNormalMapPath);
	FloorMaterial.RoughnessMap = ResourceManager::LoadNamed<Texture>("FloorRoughness", appDir + FloorRoughnessMapPath);
	FloorMaterial.MetalnessMap = ResourceManager::LoadNamed<Texture>("FloorMetalness", appDir + FloorMetalnessMapPath);

	// Create scene
	ResetScene();

	// Load external service
	Log::Info("Loading external service...");
	// Application::GetService<ExternalServices>()->Add(appDir + ExternalServicePath);

	Log::Info("Finished demo startup");
}

void Demo::OnShutdown()
{
	delete PP_Tonemapping;

#if DRAW_GRID
	ResourceManager::Unload(GridMeshID);
#endif

	Application::GetService<ExternalServices>()->Remove(Application::AssetDir + ExternalServicePath);

	for (GameObject* go : CreatedObjects)
		delete go;
}

void Demo::OnPipelineChanged(RenderPipeline* pipeline)
{
	// Tonemapping
	if (!PP_Tonemapping)
		PP_Tonemapping = new TonemappingPass();

	pipeline->AddPass(PP_Tonemapping->GetPipelinePass());
	DeferredRenderer = typeid(*pipeline) == typeid(DeferredRenderPipeline);
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
	cameraObj->GetTransform()->Position = { 0, 5, 10 };
	cameraObj->GetTransform()->Rotation = { radians(-30.0f), radians(-90.0f), 0 }; // From euler angles
	CreatedObjects.emplace_back(cameraObj);

	GameObject* directionalLightGO = new GameObject(scene, "Directional Light");
	/*
	directionalLight = directionalLightGO->AddComponent<Light>();
	directionalLight->Type = LightType::Directional;
	directionalLight->Colour = vec3(1);
	directionalLight->Intensity = 0.1f;
	directionalLight->Distance = 100.0f;
	directionalLight->GetTransform()->Rotation = { radians(-60.0f), radians(5.0f), 0};
	directionalLight->SetCastShadows(true);
	*/

	/*
	directionalLightGO = new GameObject(scene, "Spot Light");
	directionalLight = directionalLightGO->AddComponent<Light>();
	directionalLight->Type = LightType::Spot;
	directionalLight->Colour = vec3(1);
	directionalLight->GetTransform()->Rotation = { radians(-90.0f), 0, 0};
	directionalLight->SetCastShadows(true);
	directionalLight->Distance = 10.0f;
	directionalLight->Radius = 60.0f;
	*/

#if IBL_TEST
	Material sphereMaterial;
	sphereMaterial.Roughness = 0;
	sphereMaterial.Metalness = 1;
	sphereMaterial.AlbedoMap = EnvMap->GetTexture();
	GameObject* sphere = new GameObject(scene, "Sphere");
	sphere->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Sphere(), sphereMaterial } };
	sphere->GetTransform()->Position = { 0, 0, 0 };
#endif

#if CERBERUS
	// Cerberus //
	Model* cerberusModel = ResourceManager::Get<Model>(CerberusModel);
	GameObject* cerberus = cerberusModel->CreateEntity(&scene->Root());
	cerberus->GetTransform()->Scale = vec3(0.1f);
	cerberus->GetTransform()->Position = { 0, 1, 5 };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(90.0f) };
	CreatedObjects.emplace_back(cerberus);

	cerberus = cerberusModel->CreateEntity(&scene->Root());
	cerberus->GetTransform()->Scale = vec3(0.075f);
	cerberus->GetTransform()->Position = { -2.5f, 1, -2 };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(-60.0f) };
	CreatedObjects.emplace_back(cerberus);

	cerberus = cerberusModel->CreateEntity(&scene->Root());
	cerberus->GetTransform()->Scale = vec3(0.05f);
	cerberus->GetTransform()->Position = { -5, 1, -2.5f };
	cerberus->GetTransform()->Rotation = { radians(-90.0f), 0, radians(160.0f) };
	CreatedObjects.emplace_back(cerberus);

	// Floor //
	GameObject* floor = new GameObject(scene, "Floor");
	floor->AddComponent<MeshRenderer>()->Meshes = { { Mesh::Cube(), FloorMaterial } };
	floor->GetTransform()->Position = { 0, -5.0f, 0 };
	floor->GetTransform()->Scale = { 10, 1, 10 };

	Material lightMaterial;

	const int LightCount = 0;
	for (int i = 0; i < LightCount; i++)
	{
		lightMaterial.Albedo =
		{
			Random(0.5f, 2.0f),
			Random(0.5f, 2.0f),
			Random(0.5f, 2.0f),
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
		data.Component->Radius = 50.0f;
		data.Speed = Random(2.5f, 10.0f);
		data.Distance = Random(5.0f, 15.0f);
		data.SwapDirection = Random(0, 100) > 50;
		data.Offset = radians(Random(0, 360));

		CerberusLights.emplace_back(data);

		CreatedObjects.emplace_back(light);
	}
#endif

#if SPONZA
	// Sponza //
	Model* sponzaModel = ResourceManager::Get<Model>(SponzaModel);
	GameObject* sponza = sponzaModel->CreateEntity(&scene->Root());
	sponza->GetTransform()->Position = { 0, -5, 0 };
	sponza->GetTransform()->Rotation = { 0, radians(-90.0f), 0 };
	CreatedObjects.emplace_back(sponza);


	const int SponzaLightCount = 4;
	const float SponzaLightSpawnY = -3;
	const float MaxSponzaLightSpawnWidth = 6;
	for (int i = 0; i < SponzaLightCount; i++)
	{
		GameObject* sponzaLightGO = new GameObject(scene, "Sponza Light");
		Light* sponzaLight = sponzaLightGO->AddComponent<Light>();
		sponzaLight->Radius = 10.0f;
		sponzaLight->Intensity = 1.0f;

		sponzaLight->Colour =
		{
			Random(0.5f, 1.0f),
			Random(0.5f, 1.0f),
			Random(0.5f, 1.0f)
		};

		sponzaLightGO->GetTransform()->Position =
		{
			-MaxSponzaLightSpawnWidth + (MaxSponzaLightSpawnWidth / 2.0f) * i,
			SponzaLightSpawnY,
			0
		};

		SponzaLights.emplace_back(sponzaLight);
		CreatedObjects.emplace_back(sponzaLightGO);
	}
#endif
}

float GridAlpha = 0.9f;

void Demo::OnUpdate(float deltaTime)
{
	Scene* scene = Application::GetService<Services::SceneService>()->CurrentScene();

	if (Input::IsKeyPressed(GLFW_KEY_F11))
		Application::ToggleFullscreen();

	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
		Application::Exit();

	if (Input::IsKeyPressed(GLFW_KEY_SPACE))
		scene->GetPhysics().TogglePause();

	vec3 lightMovement = vec3(0.0f);
	const float LightMoveSpeed = 2.5f;
	if (Input::IsKeyDown(GLFW_KEY_UP))	  lightMovement.y += LightMoveSpeed * deltaTime;
	if (Input::IsKeyDown(GLFW_KEY_DOWN))  lightMovement.y -= LightMoveSpeed * deltaTime;
	if (Input::IsKeyDown(GLFW_KEY_LEFT))  lightMovement.x -= LightMoveSpeed * deltaTime;
	if (Input::IsKeyDown(GLFW_KEY_RIGHT)) lightMovement.x += LightMoveSpeed * deltaTime;

	if(directionalLight)
		directionalLight->GetTransform()->Position += lightMovement;

	if (Input::IsKeyDown(GLFW_KEY_MINUS)) GridAlpha -= deltaTime;
	if (Input::IsKeyDown(GLFW_KEY_EQUAL)) GridAlpha += deltaTime;
	GridAlpha = std::clamp(GridAlpha, 0.0f, 1.0f);

	if (Input::IsKeyPressed(GLFW_KEY_Z)) Renderer::ToggleWireframe();
	if (Input::IsKeyPressed(GLFW_KEY_F1)) Application::EnableGizmos(false);
	if (Input::IsKeyPressed(GLFW_KEY_F2)) Application::EnableGizmos(true);

	if (Input::IsKeyPressed(GLFW_KEY_O)) Camera::GetMainCamera()->Orthographic = !Camera::GetMainCamera()->Orthographic;

#if CERBERUS
	for (LightData data : CerberusLights)
	{
		float time = Renderer::GetTime() * (data.SwapDirection ? -1.0f : 1.0f) * data.Speed + data.Offset;
		data.Transform->Position.x = cos(radians(time)) * data.Distance;
		data.Transform->Position.z = sin(radians(time)) * data.Distance;
	}
#endif

#if SPONZA
	for (Light* light : SponzaLights)
	{
		light->GetTransform()->Position.z = sin(Renderer::GetTime() * 0.5f) * 7.5f;
	}
#endif

	/*
	if (Input::IsKeyPressed(GLFW_KEY_TAB))
	{
		if (DeferredRenderer)
			Renderer::SetPipeline<ForwardRenderPipeline>();
		else
			Renderer::SetPipeline<DeferredRenderPipeline>();
	}
	*/
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
		ImGui::Text("F11:		  Toggle fullscreen");
	}
	ImGui::End();

	const ImVec4 ColourGood = { 1, 1, 1, 1 };
	const ImVec4 ColourBad = { 1, 0, 0, 1 };

	auto& totalLights = scene->Root().GetComponentsInChildren<Light>();

	static bool debugWindowOpen = true;
	PhysicsSystem& physicsSystem = scene->GetPhysics();
	if (ImGui::Begin("Debugging", &debugWindowOpen))
	{
		float frameTime = Renderer::GetDeltaTime() * 1000.0f;
		float lastTimeStep = physicsSystem.LastTimestep().count();
		float desiredTimestep = physicsSystem.Timestep().count();

		ImGui::Text("GRID ALPHA: %.2f", GridAlpha);

		ImGui::Text("FPS: %f\n", Renderer::GetFPS());
		ImGui::Text("Total Objects: %i", (int)CreatedObjects.size());
		ImGui::Text("Lights: %i", (int)totalLights.size());
		ImGui::Text("Renderer Type: %s", DeferredRenderer ? "Deferred" : "Forward");
		ImGui::TextColored(
			(frameTime < (1000.0f / 30.0f)) ? ColourGood : ColourBad,
			"Render  Frame Time: %.1fms",
			frameTime);
		/*
		ImGui::TextColored(
			(lastTimeStep <= desiredTimestep) ? ColourGood : ColourBad,
			"Physics Frame Time: %.1fms / %.1fms",
			lastTimeStep, desiredTimestep
		);
		*/
		ImGui::Text("Resolution: (%d, %d)", Renderer::GetResolution().x, Renderer::GetResolution().y);
		ImGui::Text("VSync: %s", Renderer::GetVSync() ? "Enabled" : "Disabled");
		ImGui::Text("Samples: %d", Renderer::GetSamples());

		if (scene->GetPhysics().GetState() == PhysicsPlayState::Paused)
			ImGui::Text("PHYSICS PAUSED");

		if (ImGui::Button("Reload External Service"))
			Application::GetService<ExternalServices>()->Reload(Application::AssetDir + ExternalServicePath);
	}
	ImGui::End();

	/*
	ImGui::Begin("Light");
	{
		static const char* LightTypeNames[] = { "Point", "Spot", "Directional" };
		if (ImGui::BeginCombo("Type", LightTypeNames[(int)directionalLight->Type]))
		{
			for (int i = 0; i < 3; i++)
				if (ImGui::Selectable(LightTypeNames[i], (int)directionalLight->Type == i))
					directionalLight->Type = (LightType)i;
			ImGui::EndCombo();
		}

		bool castShadows = directionalLight->GetCastShadows();
		ImGui::Checkbox("Cast Shadows", &castShadows);
		directionalLight->SetCastShadows(castShadows);

		ImGui::SliderFloat("Radius", &directionalLight->Radius, 0.1f, 120.0f, "%.1f");
		ImGui::SliderFloat("Distance", &directionalLight->Distance, 0.1f, 120.0f, "%.1f");
		ImGui::SliderFloat("Fade Cutoff Inner", &directionalLight->FadeCutoffInner, 0.1f, 120.0f, "%.1f");
		ImGui::SliderFloat("Intensity", &directionalLight->Intensity, 0.01f, 25.0f, "%.1f");
		ImGui::ColorEdit3("Colour", &directionalLight->Colour[0]);
		ImGui::SliderFloat3("Position", &directionalLight->GetTransform()->Position[0], -100, 100, "%.2f");

		vec3 rotation = degrees(directionalLight->GetTransform()->Rotation);
		ImGui::SliderFloat3("Rotation", &rotation[0], 0.0f, 360.0f, "%.2f");
		directionalLight->GetTransform()->Rotation = radians(rotation);
	}
	ImGui::End();
	*/

	/*
	static bool servicesOpen = true;
	if (ImGui::Begin("Services", &servicesOpen))
	{
		auto& services = Application::GetAllServices();
		for (Service* service : services)
			ImGui::Text(" - %s", typeid(*service).name());
	}
	ImGui::End();
	*/

	static bool graphicsOpen = true;
	if (ImGui::Begin("Graphics"))
	{
		static const char* TonemapperNames[] = { "None", "Aces", "Reinhard" };
		if (ImGui::BeginCombo("Tonemapper", TonemapperNames[(int)PP_Tonemapping->Tonemapper]))
		{
			for (int i = 0; i < 3; i++)
				if (ImGui::Selectable(TonemapperNames[i], (int)PP_Tonemapping->Tonemapper == i))
					PP_Tonemapping->Tonemapper = (Tonemapper)i;
			ImGui::EndCombo();
		}
		ImGui::DragFloat("Gamma", &PP_Tonemapping->Gamma, 0.05f, 1.0f, 3.0f);
		ImGui::DragFloat("Exposure", &PP_Tonemapping->Exposure, 0.05f, 0.1f, 2.5f);

		static int ShadowMapResIndex = 1; // Hardcoded default value in RenderPipeline.cpp is 1024x1024
		static const char* ShadowMapResNames[] = { "512x512", "1024x1024", "2048x2048", "4096x4096", "8192x8192" };
		static glm::vec2 ShadowMapResolutions[] = { { 512, 512 }, { 1024, 1024 }, { 2048, 2048 }, { 4096, 4096 }, { 8192, 8192 } };
		if (ImGui::BeginCombo("Shadow Map Res", ShadowMapResNames[ShadowMapResIndex]))
		{
			for (int i = 0; i < 5; i++)
			{
				if (ImGui::Selectable(ShadowMapResNames[i], i == ShadowMapResIndex))
				{
					Renderer::GetPipeline()->GetShadowMapPass()->SetResolution(ShadowMapResolutions[i]);
					ShadowMapResIndex = i;
				}
			}
			ImGui::EndCombo();
		}
	}
	ImGui::End();
}

void Demo::OnDrawGizmos()
{
#if DRAW_GRID
	// Draw grid as gizmo
	if (GridMeshID == InvalidResourceID)
		GridMeshID = Mesh::Grid(GridSize);

	Gizmos::SetColour(1, 1, 1, GridAlpha);
	Gizmos::Draw(GridMeshID, vec3(-GridSize, 0.0f, -GridSize), vec3(GridSize * 2.0f));

	Gizmos::SetColour(1, 0, 0, GridAlpha);
	Gizmos::SetLineWidth(3.0f);
	Gizmos::DrawLine({ -GridSize, 0, 0 }, { GridSize, 0, 0 });

	Gizmos::SetColour(0, 0, 1, GridAlpha);
	Gizmos::DrawLine({ 0, 0, -GridSize }, { 0, 0, GridSize });

	Gizmos::SetColour(0, 1, 0, GridAlpha);
	Gizmos::DrawLine({ 0, 0, 0 }, { 0, 0.5f, 0 });

	Gizmos::SetLineWidth(1.0f);
#endif

	Gizmos::SetColour(0, 1, 0);
	Gizmos::DrawWireSphere(directionalLight->GetTransform()->Position, 0.1f);

#if CERBERUS && 0
	for (auto& Light : CerberusLights)
	{
		Gizmos::SetColour(Light.Component->Colour);
		Gizmos::DrawWireSphere(Light.Transform->Position, 1.0f);
	}
#endif

#if SPONZA
	for (auto& light : SponzaLights)
	{
		Gizmos::SetColour(light->Colour);
		Gizmos::DrawWireSphere(light->GetTransform()->Position, 0.1f);
	}
#endif
}
