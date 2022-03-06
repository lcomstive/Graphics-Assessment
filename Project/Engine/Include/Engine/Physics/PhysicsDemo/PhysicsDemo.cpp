#include <imgui.h>
#include "PhysicsDemo.hpp"
#include <Engine/Utilities.hpp>
#include <Engine/Physics/Shapes.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Physics/Particle.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>
#include <Engine/Components/Physics/BoxCollider.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>
#include <Engine/Components/Physics/PlaneCollider.hpp>
#include <Engine/Components/OrbitCameraController.hpp>
#include <Engine/Components/Physics/SphereCollider.hpp>
#include <Engine/Physics/Broadphase/BroadphaseOctree.hpp>

#pragma warning(disable : 4244)

#define TOWER_DEMO			1
#define TRIGGER_DEMO		0
#define ROPE_DEMO			0
#define PARTICLE_DEMO		0
#define OCTOPUSSS			0

#define DRAW_GRID			0
#define CAMERA_RAY_FORCE	1

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

PhysicsDemo::PhysicsDemo(ApplicationArgs args) : Application(args) { }

// Walls
const float WallDistance = 100.0f, WallThickness = 5;
const vec3 WallOffset = { 0, WallDistance - WallThickness - 10.0f, 0 };
Material WallMaterial = {};

#if DRAW_GRID
// Grid
Mesh* gridMesh = nullptr;
const int GridSize = 250;
#endif

#if ROPE_DEMO || OCTOPUSSS
#include <Engine/Components/Physics/Constraints/Spring.hpp>
GameObject* RopeRoot = nullptr;

int RopeLength = 5;
float RopeMass = 10.0f;
float RopeStiffness = 50.0f;
float RopeRestingLength = 1.0f;
float RopeDampingFactor = 0.1f;

vector<Spring*> RopeComponents;
#endif

#if OCTOPUSSS
int OctopusLegs = 8;
GameObject* OctopusBody = nullptr;
#endif

#if TOWER_DEMO
// Tower is generated in 3D: TowerSize x TowerSize x TowerSize
#ifdef NDEBUG
int TowerSize = 8;
#else
int TowerSize = 2;
#endif
#endif

#if CAMERA_RAY_FORCE
RaycastHit Hit = {};
float RayForce = 10.0f;
#endif

vector<GameObject*> CreatedObjects;

void PhysicsDemo::OnStart()
{
	// Set ImGui Font
	ImGui::GetIO().Fonts->AddFontFromFileTTF((AssetDir + "Fonts/Source Sans Pro/SourceSansPro-Regular.ttf").c_str(), 16.0f);

	ResetScene();
}

void PhysicsDemo::ResetScene()
{
	// Clean up any pre-existing resources
	for (GameObject* go : CreatedObjects)
		delete go;
	CreatedObjects.clear();

	CurrentScene()->Clear();

	// Default Cube Mesh //
	MeshRenderer::MeshInfo meshInfo;
	meshInfo.Mesh = Mesh::Cube();
	meshInfo.Material = {};

	CreateWalls();

	// Main Camera //
	GameObject* cameraObj = new GameObject(CurrentScene(), "Main Camera");
	cameraObj->AddComponent<OrbitCameraController>();
	cameraObj->GetTransform()->Position = { 0, 0, 20 };
	cameraObj->GetTransform()->Rotation = { 0, radians(-90.0f), 0 }; // From euler angles
	CreatedObjects.emplace_back(cameraObj);

	meshInfo.Material.Albedo = { 1, 0, 0, 1 };

#if TOWER_DEMO
	// Generate tower(s)
	for (int x = 0; x < TowerSize; x++)
	{
		for (int y = 0; y < TowerSize; y++)
		{
			for (int z = 0; z < TowerSize; z++)
			{
				GameObject* go = new GameObject(CurrentScene(), "Tower " + to_string(y));
				go->GetTransform()->Position.x = (x - TowerSize / 2.0f) * 5;
				go->GetTransform()->Position.y = y * 5;
				go->GetTransform()->Position.z = (z - TowerSize / 2.0f) * 5;
				go->GetTransform()->Rotation = radians(vec3{ Random(-180, 180), Random(-180, 180), Random(-180, 180) });

				Rigidbody* rb = go->AddComponent<Rigidbody>();
				rb->SetFriction(0.1f);
				rb->SetRestitution(0.35f);

				// Generate either sphere or cube, randomly
				bool sphere = rand() % 2 == 0;
				if (sphere)
				{
					meshInfo.Mesh = Mesh::Sphere();
					go->AddComponent<SphereCollider>();
				}
				else
				{
					meshInfo.Mesh = Mesh::Cube();
					BoxCollider* bCollider = go->AddComponent<BoxCollider>();
				}

				meshInfo.Material.Albedo = { Random(0.5f, 1.0f), Random(0.5f, 1.0f), Random(0.5f, 1.0f), 1.0f };
				go->AddComponent<MeshRenderer>()->Meshes = { meshInfo };

				CreatedObjects.emplace_back(go);
			}
		}
	}
#endif

#if TRIGGER_DEMO
	// Create trigger
	GameObject* triggerObject = new GameObject(CurrentScene(), "Trigger");
	triggerObject->GetTransform()->Scale = { 2.5f, 0.1f, 2.5f };
	BoxCollider* trigger = triggerObject->AddComponent<BoxCollider>();
	trigger->IsTrigger = true;

	meshInfo.Material.Albedo = { 0, 1, 0, 1 };
	triggerObject->AddComponent<MeshRenderer>()->Meshes = { meshInfo };

	trigger->SetTriggerEnterEvent([=](Collider*) { triggerObject->GetComponent<MeshRenderer>()->Meshes[0].Material.Albedo = { 1, 0, 0, 1 }; });
	trigger->SetTriggerExitEvent([=](Collider*) { triggerObject->GetComponent<MeshRenderer>()->Meshes[0].Material.Albedo = { 0, 1, 0, 1 }; });

	CreatedObjects.emplace_back(triggerObject);

	// Create object to fall through trigger
	GameObject* fallingObject = new GameObject(CurrentScene(), "Falling Object");
	fallingObject->GetTransform()->Position = { 0, 5, 0 };

	meshInfo.Material.Albedo = { 0, 0, 1, 1 };
	fallingObject->AddComponent<MeshRenderer>()->Meshes = { meshInfo };
	fallingObject->AddComponent<Rigidbody>();
	fallingObject->AddComponent<BoxCollider>()->SetCollisionEvent([=](Collider*, Rigidbody*) { fallingObject->GetTransform()->Position = { 0, 5, 0 }; });

	CreatedObjects.emplace_back(fallingObject);
#endif

#if ROPE_DEMO
	Particle* previousParticle = nullptr;
	meshInfo.Mesh = Mesh::Sphere();
	RopeComponents.clear();
	meshInfo.Material.Albedo = { 0.5f, 0.5f, 0.5f, 1.0f };
	for (int i = 0; i < RopeLength; i++)
	{
		// Create rope visual
		GameObject* rope = new GameObject(CurrentScene(), "Rope " + to_string(i));
		rope->GetTransform()->Position = { i * RopeRestingLength, 0, 0.0f };
		rope->GetTransform()->Scale = vec3(0.1f);
		rope->AddComponent<MeshRenderer>()->Meshes = { meshInfo };
		CreatedObjects.emplace_back(rope);

		// Create rope particle
		Particle* particle = rope->AddComponent<Particle>();
		particle->SetMass(RopeMass);
		if (i == 0)
			particle->SetStatic(true);

		if (previousParticle)
		{
			// Attach particle to previously created particle in a chain-like fashion
			Spring* spring = rope->AddComponent<Spring>();
			spring->SetBodies(previousParticle, particle);
			spring->SetConstants(RopeStiffness, RopeDampingFactor);
			spring->SetRestingLength(RopeRestingLength);
			RopeComponents.emplace_back(spring);
		}
		previousParticle = particle;

		if (i == 0)
			RopeRoot = rope;
	}
#endif

#if OCTOPUSSS
	meshInfo.Mesh = Mesh::Sphere();
	meshInfo.Material.Albedo = { 0.2f, 0.5f, 0.92f, 0.25f }; // Lightened blue
	RopeComponents.clear();

	OctopusBody = new GameObject(CurrentScene(), "Octopus");
	OctopusBody->AddComponent<MeshRenderer>()->Meshes = { meshInfo };
	OctopusBody->AddComponent<SphereCollider>();

	// Create legs
	for (int leg = 0; leg < OctopusLegs; leg++)
	{
		Particle* previousParticle = nullptr;
		float x = cos((leg / (float)OctopusLegs) * radians(360.0f));
		float z = sin((leg / (float)OctopusLegs) * radians(360.0f));
		for (int i = 0; i < RopeLength; i++)
		{
			// Create visual
			GameObject* rope = new GameObject(i == 0 ? OctopusBody : &CurrentScene()->Root(), "Rope " + to_string(i) + " " + to_string(leg));
			rope->GetTransform()->Position = { x, i * -RopeRestingLength, z };
			rope->GetTransform()->Scale = vec3(0.1f);
			rope->AddComponent<MeshRenderer>()->Meshes = { meshInfo };
			CreatedObjects.emplace_back(rope);

			// Create particle
			Particle* particle = rope->AddComponent<Particle>();
			particle->SetMass(RopeMass);
			if (i == 0)
				particle->SetStatic(true);

			if (previousParticle)
			{
				// Attach particle to previously created particle, in chain-like fashion
				Spring* spring = rope->AddComponent<Spring>();
				spring->SetBodies(previousParticle, particle);
				spring->SetConstants(RopeStiffness, RopeDampingFactor);
				spring->SetRestingLength(RopeRestingLength);
				RopeComponents.emplace_back(spring);
			}
			previousParticle = particle;

			if (i == 0)
				RopeRoot = rope;
		}
	}

	// Generate random obstacles
	const unsigned int ObstacleCount = 20;
	const vec3 ObstacleSpawnExtents = vec3(10, 5, 10);
	for (int i = 0; i < ObstacleCount; i++)
	{
		GameObject* obstacle = new GameObject(CurrentScene(), "Obstacle " + to_string(i));
		meshInfo.Material.Albedo = { Random(0.5f, 1.0f), Random(0.5f, 1.0f), Random(0.5f, 1.0f), 1.0f };
		obstacle->GetTransform()->Position =
		{
			Random(-ObstacleSpawnExtents.x, ObstacleSpawnExtents.x),
			Random(-ObstacleSpawnExtents.y, ObstacleSpawnExtents.y),
			Random(-ObstacleSpawnExtents.z, ObstacleSpawnExtents.z)
		};
		const float Scale = 1.0f;
		obstacle->GetTransform()->Scale = vec3(Scale);

		if (Random(0.0f, 1.0f) > 0.5f)
		{
			obstacle->AddComponent<BoxCollider>();
			obstacle->GetTransform()->Rotation =
			{
				Random(0.0f, radians(360.0f)),
				Random(0.0f, radians(360.0f)),
				Random(0.0f, radians(360.0f))
			};
			meshInfo.Mesh = Mesh::Cube();
		}
		else
		{
			obstacle->AddComponent<SphereCollider>()->SetRadius(Scale);
			meshInfo.Mesh = Mesh::Sphere();
		}
		obstacle->AddComponent<MeshRenderer>()->Meshes = { meshInfo };
		CreatedObjects.emplace_back(obstacle);
	}
#endif

#if PARTICLE_DEMO
	const float ParticleSize = 0.5f;
#ifndef NDEBUG
	const int ParticleTowerSize = 3;
#else
	const int ParticleTowerSize = 8;
#endif
	// Create tower(s)
	meshInfo.Mesh = Mesh::Sphere();
	for (float x = ParticleTowerSize / -2.0f; x < ParticleTowerSize / 2.0f; x++)
	{
		for (int y = 0; y < ParticleTowerSize; y++)
		{
			for (float z = ParticleTowerSize / -2.0f; z < ParticleTowerSize / 2.0f; z++)
			{
				GameObject* go = new GameObject(CurrentScene(), "Particle (" + to_string(x) + ", " + to_string(y) + ", " + to_string(z) + ")");
				go->GetTransform()->Position = vec3(x, 3.0f + y, z) * ParticleSize * 2.5f;
				go->GetTransform()->Scale = vec3(ParticleSize);

#if 1
				go->AddComponent<Rigidbody>();
				go->AddComponent<SphereCollider>()->SetRadius(ParticleSize);
#else
				go->AddComponent<Particle>()->SetCollisionRadius(ParticleSize);
#endif

				meshInfo.Material.Albedo = { Random(0.5f, 1.0f), Random(0.5f, 1.0f), Random(0.5f, 1.0f), 1.0f };
				go->AddComponent<MeshRenderer>()->Meshes = { meshInfo };

				CreatedObjects.emplace_back(go);
			}
		}
	}

	// Generate spaced grid of obstacles
	const int ObstacleWidth = 6;
	const float ObstacleSize = 0.5f;
	const float ObstacleSpacing = 1.5f;
	meshInfo.Material.Albedo = { 1, 0, 0, 1 };
	for (float x = ObstacleWidth / -2.0f; x < ObstacleWidth / 2.0f; x++)
	{
		for (float z = ObstacleWidth / -2.0f; z < ObstacleWidth / 2.0f; z++)
		{
			GameObject* obstacle = new GameObject(CurrentScene(), "Particle Obstacle");
			obstacle->GetTransform()->Position = vec3
			{
				(x + ObstacleSize) * ObstacleSpacing,
				2.0f,
				(z + ObstacleSize) * ObstacleSpacing
			};
			obstacle->GetTransform()->Scale = vec3(ObstacleSize);
			obstacle->AddComponent<MeshRenderer>()->Meshes = { meshInfo };
			obstacle->AddComponent<SphereCollider>()->SetRadius(ObstacleSize);
			obstacle->AddComponent<Rigidbody>()->SetStatic(true);
			CreatedObjects.emplace_back(obstacle);
		}
	}
#endif
}

void PhysicsDemo::OnShutdown()
{
#if DRAW_GRID
	delete gridMesh;
#endif
}

const float CameraSpeed = 5.0f;
const float CameraRotationSpeed = 5.0f;
void PhysicsDemo::OnUpdate()
{
	if (Input::IsKeyPressed(GLFW_KEY_F11))
		ToggleFullscreen();

	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
		Exit();

	if (Input::IsKeyPressed(GLFW_KEY_SPACE))
		CurrentScene()->GetPhysics().TogglePause();

#if OCTOPUSSS
	// Octopus movement
	const float OctopusSpeed = 5.0f;
	if (Input::IsKeyDown(GLFW_KEY_LEFT))		OctopusBody->GetTransform()->Position += vec3{ -1,  0,  0 } * OctopusSpeed * Renderer::GetDeltaTime();
	if (Input::IsKeyDown(GLFW_KEY_RIGHT))		OctopusBody->GetTransform()->Position += vec3{  1,  0,  0 } * OctopusSpeed * Renderer::GetDeltaTime();
	if (Input::IsKeyDown(GLFW_KEY_UP))			OctopusBody->GetTransform()->Position += vec3{  0,  0, -1 } * OctopusSpeed * Renderer::GetDeltaTime();
	if (Input::IsKeyDown(GLFW_KEY_DOWN))		OctopusBody->GetTransform()->Position += vec3{  0,  0,  1 } * OctopusSpeed * Renderer::GetDeltaTime();
	if (Input::IsKeyDown(GLFW_KEY_PAGE_UP))		OctopusBody->GetTransform()->Position += vec3{  0,  1,  0 } * OctopusSpeed * Renderer::GetDeltaTime();
	if (Input::IsKeyDown(GLFW_KEY_PAGE_DOWN))	OctopusBody->GetTransform()->Position += vec3{  0, -1,  0 } * OctopusSpeed * Renderer::GetDeltaTime();
#endif

#if CAMERA_RAY_FORCE
	Camera* camera = Camera::GetMainCamera();

	// Shoot ray from camera, if hit's an object apply force in direction of ray
	Ray ray;
	ray.Origin = camera->GetTransform()->Position;
	ray.Direction = camera->GetForwardDirection();

	Collider* hitCollider = CurrentScene()->GetPhysics().Raycast(ray, &Hit);

	if (hitCollider && Input::IsKeyPressed(GLFW_KEY_F))
	{
		// Test rigidbody
		Rigidbody* rb = hitCollider->GetRigidbody();
		if (rb)
			rb->ApplyForce(ray.Direction * RayForce, Hit.Point, ForceMode::Impulse);

		// Test particle
		Particle* particle = hitCollider->GetGameObject()->GetComponent<Particle>();
		if (particle)
			particle->ApplyForce(ray.Direction * RayForce, ForceMode::Impulse);
	}
#endif
}

void PhysicsDemo::OnDraw()
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
#if OCTOPUSSS
		ImGui::Text("ARROW KEYS:  Move octopus");
		ImGui::Text("PAGE UP/DOWN:Raise/Lower octopus");
#endif

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

		if (CurrentScene()->GetPhysics().GetState() == PhysicsPlayState::Paused)
			ImGui::Text("PHYSICS PAUSED");

		ImGui::End();
	}

#if TOWER_DEMO
	static bool towerWindowOpen = true;
	if (ImGui::Begin("Tower Demo", &towerWindowOpen))
	{
		ImGui::SliderInt("Tower Width", &TowerSize, 1, 20);

		if (ImGui::Button("Reset"))
			ResetScene();
		ImGui::End();
	}
#endif

#if ROPE_DEMO || OCTOPUSSS
	static bool ropeWindowOpen = true;
#if ROPE_DEMO
	if (ImGui::Begin("Rope Demo", &ropeWindowOpen))
#else
	if (ImGui::Begin("Octopus Demo", &ropeWindowOpen))
#endif
	{
		bool ropeDirty = false;

		if (ImGui::SliderFloat("Stiffness", &RopeStiffness, 0.0f, 1000.0f)) ropeDirty = true;
		if (ImGui::SliderFloat("Dampening", &RopeDampingFactor, 0.0f, 1.0f)) ropeDirty = true;
		if (ImGui::SliderFloat("Resting Length", &RopeRestingLength, 0.1f, 10.0f)) ropeDirty = true;
		if (ImGui::SliderFloat("Particle Mass", &RopeMass, 0.1f, 100.0f)) ropeDirty = true;

#if CAMERA_RAY_FORCE
		ImGui::SliderFloat("Raycast Force", &RayForce, 0.0f, 100.0f);
#endif

#if OCTOPUSSS
		ImGui::SliderInt("Tentacles", &OctopusLegs, 1, 20);
#endif

		ImGui::SliderInt("Rope Length", &RopeLength, 2, 20);
		if (ImGui::Button("Reset"))
			ResetScene();

		ImGui::End();

		if (ropeDirty)
			for (Spring* rope : RopeComponents)
			{
				rope->GetPoint1()->SetMass(RopeMass);
				rope->GetPoint2()->SetMass(RopeMass);
				rope->SetRestingLength(RopeRestingLength);
				rope->SetConstants(RopeStiffness, RopeDampingFactor);
			}
}
#endif
}

void PhysicsDemo::OnDrawGizmos()
{
#if DRAW_GRID
	// Draw grid as gizmo
	if (!gridMesh)
		gridMesh = Mesh::Grid(GridSize);

	Gizmos::Colour = { 1, 1, 1, 0.2f };
	Gizmos::Draw(gridMesh, vec3(-GridSize, -WallOffset.y / 2.0f, -GridSize), vec3(GridSize * 2.0f));
#endif

#if CAMERA_RAY_FORCE
	// Draw camera raycast hit
	if (Hit.Distance > 0)
	{
		Gizmos::Colour = { 0, 1, 1, 1 };
		Gizmos::DrawCube(Hit.Point, vec3(0.025f));
	}
#endif
}

void PhysicsDemo::CreateWall(vec3 axis)
{
	WallMaterial.Albedo.a = 0.1f;

	GameObject* wall = new GameObject(CurrentScene(), "Wall");
	// wall->AddComponent<MeshRenderer>()->Meshes = { MeshRenderer::MeshInfo { Mesh::Cube(), WallMaterial }};
	wall->AddComponent<BoxCollider>();

	Transform* transform = wall->GetTransform();
	transform->Scale = vec3(WallDistance) - abs(axis) * vec3(WallDistance - WallThickness);
	transform->Rotation = axis * radians(90.0f); // Rotate 90 degrees on axis
	transform->Position = -axis * WallDistance + WallOffset;

	CreatedObjects.emplace_back(wall);
}

void PhysicsDemo::CreateWalls()
{
	CreateWall(vec3{ 0,  1,  0 });
	CreateWall(vec3{ 0, -1,  0 });
	CreateWall(vec3{ 1,  0,  0 });
	CreateWall(vec3{ -1,  0,  0 });
	CreateWall(vec3{ 0,  0,  1 });
	CreateWall(vec3{ 0,  0, -1 });
}
