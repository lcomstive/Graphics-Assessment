#include <regex>
#include <imgui.h>
#include "Inspector.hpp"
#include <Engine/Scene.hpp>
#include <Engine/Input.hpp>
#include <Engine/Application.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Renderer.hpp> // For screen resolution
#include <Engine/Components/Transform.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>
#include <ImGuizmo.h>

using namespace MV;
using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

Transform* m_GizmoTransform = nullptr;

void MV::DrawHierarchy(Scene* scene)
{
	if (!scene)
		return;

	ImGui::Begin("Hierarchy");
	for (Transform* child : scene->Root().GetTransform()->GetChildren())
		DrawHierarchy(child->GetGameObject());
	ImGui::End();

	DrawGizmo(m_GizmoTransform);
}

string GetMapPath(ResourceID textureID)
{
	Texture* texture = ResourceManager::Get<Texture>(textureID);
	string path = texture ? texture->GetPath() : "";

	if (!path.empty())
		path = regex_replace(
			path,
			regex(Application::AssetDir),
			""
		);

	return path;
}

void MV::DrawHierarchy(GameObject* go)
{
	Transform* transform = go->GetTransform();

	vector<Transform*> children = transform->GetChildren();

	if (ImGui::TreeNode(go->GetName().c_str()))
	{
		/// TRANSFORM ///
		ImGui::Text("Transform");

		// Gizmo
		if (m_GizmoTransform != transform &&
			ImGui::Button("Enable Gizmo"))
			m_GizmoTransform = transform;
		else if (m_GizmoTransform == transform &&
			ImGui::Button("Disable Gizmo"))
			m_GizmoTransform = nullptr;

		// Transform Properties
		ImGui::DragFloat3("Position", &transform->Position[0], 0.1f, -1000, 1000, "%.2f");

		vec3 euler = degrees(transform->Rotation);
		ImGui::DragFloat3("Rotation", &euler[0], 0.1f, -1000, 1000, "%.2f");
		transform->Rotation = radians(euler);

		ImGui::DragFloat3("Scale", &transform->Scale[0], 0.1f, -1000, 1000, "%.2f");

		ImGui::Spacing();

		/// MESH RENDERER ///

		MeshRenderer* renderer = go->GetComponent<MeshRenderer>();
		if (renderer)
		{
			unsigned int meshCount = 0;
			for (auto& meshInfo : renderer->Meshes)
			{
				if (ImGui::TreeNode(("Material [" + to_string(meshCount) + "]").c_str()))
				{
					// Albedo
					ImGui::ColorEdit3("Albedo", &meshInfo.Material.Albedo[0]);

					// Alpha Clipping & Threshold
					ImGui::Checkbox("Alpha Clipping", &meshInfo.Material.AlphaClipping);
					ImGui::DragFloat("Alpha Clip Threshold", &meshInfo.Material.AlphaClipThreshold, 0.01f, 0.0f, 1.0f, "%.2f");

					// Texture Coordinate Scale & Offset
					ImGui::DragFloat2("UV Scale", &meshInfo.Material.TextureCoordinateScale[0], 0.01f, -100, 100, "%.2f");
					ImGui::DragFloat2("UV Offset", &meshInfo.Material.TextureCoordinateOffset[0], 0.01f, -100, 100, "%.2f");

					// Roughness & Metalness
					ImGui::DragFloat("Roughness", &meshInfo.Material.Roughness, 0.01f, 0.0f, 1.0f, "%.2f");
					ImGui::DragFloat("Metalness", &meshInfo.Material.Metalness, 0.01f, 0.0f, 1.0f, "%.2f");

					// Wireframe
					ImGui::Checkbox("Wireframe Mode", &meshInfo.Material.Wireframe);

					// Can Cast Shadows
					ImGui::Checkbox("Shadow Caster", &meshInfo.Material.CanCastShadows); // Actually not sure if this is used..

					Texture* texture = nullptr;

					// Albedo Map
					string albedoMapPath = GetMapPath(meshInfo.Material.AlbedoMap);
					bool valueChanged = ImGui::InputText("Albedo Map", &albedoMapPath, ImGuiInputTextFlags_EnterReturnsTrue);
					ImGui::SameLine();
					if (ImGui::Button("->") || valueChanged)
						meshInfo.Material.AlbedoMap = ResourceManager::LoadNamed<Texture>(albedoMapPath, Application::AssetDir + albedoMapPath);

					// Normal Map
					string normalMapPath = GetMapPath(meshInfo.Material.NormalMap);
					valueChanged = ImGui::InputText("Normal Map", &normalMapPath, ImGuiInputTextFlags_EnterReturnsTrue);
					ImGui::SameLine();
					if (ImGui::Button("->") || valueChanged)
						meshInfo.Material.NormalMap = ResourceManager::LoadNamed<Texture>(normalMapPath, Application::AssetDir + normalMapPath);

					// Roughness Map
					string roughnessMapPath = GetMapPath(meshInfo.Material.RoughnessMap);
					valueChanged = ImGui::InputText("Roughness Map", &roughnessMapPath, ImGuiInputTextFlags_EnterReturnsTrue);
					ImGui::SameLine();
					if (ImGui::Button("->") || valueChanged)
						meshInfo.Material.RoughnessMap = ResourceManager::LoadNamed<Texture>(roughnessMapPath, Application::AssetDir + roughnessMapPath);

					// Metalness Map
					string metalnessMapPath = GetMapPath(meshInfo.Material.MetalnessMap);
					valueChanged = ImGui::InputText("Metalness Map", &metalnessMapPath, ImGuiInputTextFlags_EnterReturnsTrue);
					ImGui::SameLine();
					if (ImGui::Button("->") || valueChanged)
						meshInfo.Material.MetalnessMap = ResourceManager::LoadNamed<Texture>(metalnessMapPath, Application::AssetDir + metalnessMapPath);

					ImGui::TreePop();
				}
				meshCount++;
			}
		}

		ImGui::Spacing();

		/// RECURSIVELY DRAW CHILDREN ///

		for (Transform* child : children)
			DrawHierarchy(child->GetGameObject());

		ImGui::TreePop();
	}

	if (children.empty())
		ImGui::Separator();
}

void MV::DrawGizmo(Transform* transform)
{
	if (!transform)
		return;

	static ImGuizmo::MODE GizmoMode = ImGuizmo::WORLD;
	static ImGuizmo::OPERATION GizmoOperation = ImGuizmo::TRANSLATE;

	if (Input::IsKeyPressed(GLFW_KEY_Z)) GizmoOperation = ImGuizmo::TRANSLATE;
	if (Input::IsKeyPressed(GLFW_KEY_X)) GizmoOperation = ImGuizmo::ROTATE;
	if (Input::IsKeyPressed(GLFW_KEY_C)) GizmoOperation = ImGuizmo::SCALE;
	if (Input::IsKeyPressed(GLFW_KEY_V)) GizmoMode = GizmoMode == ImGuizmo::WORLD ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

	ivec2 resolution = Graphics::Renderer::GetResolution();

	ImGuizmo::SetRect(0, 0, (float)resolution.x, (float)resolution.y);
	
	Camera* camera = Camera::GetMainCamera();
	mat4 modelMatrix = mat4(1.0f);

	// ImGuizmo flips Y & Z ;-;
	
	// Create model matrix
	vec3 rotation = { transform->Rotation.x, transform->Rotation.z, transform->Rotation.y };
	vec3 scale = { transform->Scale.x, transform->Scale.z, transform->Scale.y };
	ImGuizmo::RecomposeMatrixFromComponents(
		&transform->Position[0],
		&rotation[0],
		&scale[0],
		&modelMatrix[0][0]
	);

	// Display gizmo
	ImGuizmo::Manipulate(
		&camera->GetViewMatrix()[0][0],
		&camera->GetProjectionMatrix()[0][0],
		GizmoOperation,
		GizmoMode,
		&modelMatrix[0][0]
	);

	// Model matrix back into transform
	ImGuizmo::DecomposeMatrixToComponents(
		&modelMatrix[0][0],
		&transform->Position[0],
		&rotation[0],
		&scale[0]
	);
	transform->Rotation = { rotation.x, rotation.z, rotation.y }; // Flip Y & Z
	transform->Scale = { scale.x, scale.z, scale.y };	 // Flip Y & Z
}