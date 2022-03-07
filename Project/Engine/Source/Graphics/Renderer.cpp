#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Transform.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

Renderer* Renderer::s_Instance = nullptr;

Renderer::Renderer() :
	m_FPS(0),
	m_Time(0),
	m_Samples(1),
	m_DrawQueue(),
	m_DeltaTime(0),
	m_VSync(false),
	m_Resolution(0),
	m_Wireframe(false),
	m_Pipeline(nullptr)
{
	if (!s_Instance)
		s_Instance = this;
}

Renderer::~Renderer()
{
	if (s_Instance == this)
		s_Instance = nullptr;
}

void Renderer::SetVSync(bool vsync) { glfwSwapInterval(s_Instance->m_VSync = vsync ? 1 : 0); }
void Renderer::SetWireframe(bool wireframe) { glPolygonMode(GL_FRONT_AND_BACK, (s_Instance->m_Wireframe = wireframe) ? GL_LINE : GL_FILL); }

float Renderer::GetFPS() { return s_Instance->m_FPS; }
float Renderer::GetTime() { return s_Instance->m_Time; }
bool Renderer::GetVSync() { return s_Instance->m_VSync; }
uint32_t Renderer::GetSamples() { return s_Instance->m_Samples; }
float Renderer::GetDeltaTime() { return s_Instance->m_DeltaTime; }
ivec2 Renderer::GetResolution() { return s_Instance->m_Resolution; }
bool Renderer::GetWireframeMode() { return s_Instance->m_Wireframe; }
RenderPipeline* Renderer::GetPipeline() { return s_Instance->m_Pipeline; }

void Renderer::Shutdown()
{
	if (s_Instance->m_Pipeline)
		delete s_Instance->m_Pipeline;
	s_Instance = nullptr;
}

void Renderer::SortDrawQueue(DrawSortType sortType)
{
	Camera* camera = Camera::GetMainCamera();
	vec3 cameraPos = camera->GetTransform()->GetGlobalPosition();
	if (sortType == DrawSortType::None || !camera)
		return;

	sort(s_Instance->m_DrawQueue.begin(), s_Instance->m_DrawQueue.end(), [=](DrawCall& a, DrawCall& b)
		{
			float distanceA = glm::distance(cameraPos, a.Position);
			float distanceB = glm::distance(cameraPos, b.Position);
			return sortType == DrawSortType::BackToFront ? (distanceA > distanceB) : (distanceB > distanceA);
		});
}

void Renderer::Draw(DrawArgs args)
{
	SortDrawQueue(args.DrawSorting);
	Shader* shader = s_Instance->m_Pipeline->CurrentShader();

	for (const DrawCall& drawCall : s_Instance->m_DrawQueue)
	{
		if (drawCall.Mesh == InvalidResourceID)
			continue;
		Mesh* mesh = ResourceManager::Get<Mesh>(drawCall.Mesh);
		if (!mesh) continue;

		if (!args.RenderOpaque && drawCall.Material.Albedo.a >= 1.0f)
			continue;
		if (!args.RenderTransparent && drawCall.Material.Albedo.a < 1.0f)
			continue;

		// Generate model matrix
		mat4 translationMatrix = translate(mat4(1.0f), drawCall.Position);
		mat4 scaleMatrix = scale(mat4(1.0f), drawCall.Scale);

		shader->Set("modelMatrix", translationMatrix * drawCall.Rotation * scaleMatrix);

		// Fill material values
		drawCall.Material.FillShader(shader);

		Renderer::SetWireframe(drawCall.Material.Wireframe);

		mesh->Draw();

		if (drawCall.DeleteMeshAfterRender)
			ResourceManager::Unload(drawCall.Mesh);
	}

	if (args.ClearQueue)
		ClearDrawQueue();
}

void Renderer::Resized(glm::ivec2 newResolution)
{
	s_Instance->m_Resolution = newResolution;
	if (s_Instance->m_Pipeline)
		s_Instance->m_Pipeline->OnResized(s_Instance->m_Resolution);
}

void Renderer::ClearDrawQueue()
{
	s_Instance->m_DrawQueue.clear();
}

void Renderer::Submit(ResourceID& mesh, Material& material, vec3 position, vec3 scale, vec3 rotation)
{
	Submit(mesh, material, position, scale, eulerAngleXYZ(rotation.x, rotation.y, rotation.z));
}

void Renderer::Submit(ResourceID& mesh, Material& material, vec3 position, vec3 scale, mat4 rotation)
{
	Submit(DrawCall
		{
			mesh,
			material,
			position,
			scale,
			rotation
		});
}


void Renderer::Submit(ResourceID& mesh, Material& material, Components::Transform* transform)
{
	Submit(DrawCall
		{
			mesh,
			material,
			transform->GetGlobalPosition(),
			transform->GetGlobalScale(),
			transform->GetGlobalRotationMatrix()
		});
}

void Renderer::Submit(DrawCall drawCall) { s_Instance->m_DrawQueue.emplace_back(drawCall); }