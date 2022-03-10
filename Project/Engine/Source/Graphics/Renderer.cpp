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
	m_Pipeline(nullptr),
	m_MainCamera(nullptr),
	m_SupportsTessellation(false)
{
	if (!s_Instance)
		s_Instance = this;
	
	// Check for tessellation support
	// OpenGL 4.0+ is required
	if (GLVersion.major >= 4)
	{
		GLint maxPatchVertices = 0;
		glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);

		m_SupportsTessellation = maxPatchVertices >= 3;
		m_SupportsTessellation = false;

		if (m_SupportsTessellation)
		{
			Log::Debug("Setting up tessellation with patch vertices: 3 (max " + to_string(maxPatchVertices) + ")");
			glPatchParameteri(GL_PATCH_VERTICES, 3);
		}
	}
}

Renderer::~Renderer()
{
	if (s_Instance == this)
		s_Instance = nullptr;
}

void Renderer::ToggleWireframe() { SetWireframe(!s_Instance->m_Wireframe); }
void Renderer::SetVSync(bool vsync) { glfwSwapInterval(s_Instance->m_VSync = vsync ? 1 : 0); }
void Renderer::SetWireframe(bool wireframe) { glPolygonMode(GL_FRONT_AND_BACK, (s_Instance->m_Wireframe = wireframe) ? GL_LINE : GL_FILL); }

float Renderer::GetFPS() { return s_Instance->m_FPS; }
float Renderer::GetTime() { return s_Instance->m_Time; }
bool Renderer::GetVSync() { return s_Instance->m_VSync; }
uint32_t Renderer::GetSamples() { return s_Instance->m_Samples; }
float Renderer::GetDeltaTime() { return s_Instance->m_DeltaTime; }
ivec2 Renderer::GetResolution() { return s_Instance->m_Resolution; }
bool Renderer::GetWireframeMode() { return s_Instance->m_Wireframe; }
Camera* Renderer::GetMainCamera() { return s_Instance->m_MainCamera; }
RenderPipeline* Renderer::GetPipeline() { return s_Instance->m_Pipeline; }
void Renderer::SetMainCamera(Camera* camera) { s_Instance->m_MainCamera = camera; }
bool Renderer::SupportsTessellation() { return s_Instance->m_SupportsTessellation; }

void Renderer::Shutdown()
{
	if (s_Instance->m_Pipeline)
		delete s_Instance->m_Pipeline;
	s_Instance = nullptr;
}

void Renderer::SortDrawQueue(DrawSortType sortType)
{
	vec3 cameraPos = s_Instance->m_MainCamera->GetTransform()->GetGlobalPosition();
	if (sortType == DrawSortType::None || !s_Instance->m_MainCamera)
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

	glPolygonMode(GL_FRONT_AND_BACK, s_Instance->m_Wireframe ? GL_LINE : GL_FILL);

	for (DrawCall& drawCall : s_Instance->m_DrawQueue)
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

		glLineWidth(drawCall.LineWidth);		

		if(drawCall.Material.Wireframe && !s_Instance->m_Wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		mesh->Draw();

		if(drawCall.Material.Wireframe && !s_Instance->m_Wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (drawCall.DeleteMeshAfterRender)
			ResourceManager::Unload(drawCall.Mesh);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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