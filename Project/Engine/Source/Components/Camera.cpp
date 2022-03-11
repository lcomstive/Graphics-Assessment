#include <glm/glm.hpp>
#include <Engine/GameObject.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Engine/Components/Transform.hpp>

using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

void Camera::Added()
{
	if (!Renderer::GetMainCamera())
		Renderer::SetMainCamera(this);
}

void Camera::SetMainCamera() { Renderer::SetMainCamera(this); }
Camera* Camera::GetMainCamera() { return Renderer::GetMainCamera(); }

mat4 Camera::GetViewMatrix() { return m_ViewMatrix; }
mat4 Camera::GetProjectionMatrix() { return m_ProjectionMatrix; }

void Camera::FillShader(Shader* shader)
{
	shader->Set("camera.Position", GetTransform()->GetGlobalPosition());
	shader->Set("camera.ViewMatrix", GetViewMatrix());
	shader->Set("camera.ProjectionMatrix", GetProjectionMatrix());
}

void Camera::Removed()
{
	Component::Removed();
	if (Renderer::GetMainCamera() == this)
		Renderer::SetMainCamera(nullptr);
}

void Camera::Update(float deltaTime)
{
	Transform* transform = GetTransform();

	// Create view matrix
	m_ViewMatrix = lookAt(
		transform->GetGlobalPosition(),
		transform->GetGlobalPosition() + transform->Forward(),
		transform->Up()
	);

	// Create projection matrix
	ivec2 res = Renderer::GetResolution();
	res.x = max(res.x, 1);
	res.y = max(res.y, 1);
	float aspectRatio = res.x / (float)res.y;
	if (Orthographic)
	{
		float width = OrthoSize * aspectRatio;
		float height = OrthoSize;

		m_ProjectionMatrix = ortho(-width, width, -height, height, ClipNear, ClipFar);
	}
	else
		m_ProjectionMatrix = perspective(radians(FieldOfView), aspectRatio, ClipNear, ClipFar);
}