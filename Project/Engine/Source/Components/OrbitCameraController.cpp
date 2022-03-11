#include <Engine/Input.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Components/OrbitCameraController.hpp>

using namespace glm;
using namespace Engine;
using namespace Engine::Components;

void OrbitCameraController::Added()
{
	m_Camera = GetGameObject()->GetComponent<Camera>();
	if (!m_Camera)
		m_Camera = GetGameObject()->AddComponent<Camera>();
}

void OrbitCameraController::Update(float deltaTime)
{
	Transform* transform = GetTransform();

	float speed = Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT) ? SprintSpeed : MoveSpeed;
	if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
		speed = MoveSpeed / 4.0f;
	speed *= deltaTime;

	if (Input::IsKeyDown(GLFW_KEY_A)) transform->Position -= transform->Right() * speed;
	if (Input::IsKeyDown(GLFW_KEY_D)) transform->Position += transform->Right() * speed;
	if (Input::IsKeyDown(GLFW_KEY_W)) transform->Position += transform->Forward() * speed;
	if (Input::IsKeyDown(GLFW_KEY_S)) transform->Position -= transform->Forward() * speed;

	if (Input::IsKeyDown(GLFW_KEY_Q)) transform->Position -= transform->Up() * speed;
	if (Input::IsKeyDown(GLFW_KEY_E)) transform->Position += transform->Up() * speed;

	m_Camera->FieldOfView = std::clamp(m_Camera->FieldOfView - Input::GetScrollDelta(), 10.0f, 120.0f);

	Input::ShowMouse(!Input::IsMouseDown(GLFW_MOUSE_BUTTON_2));
	if (Input::IsMouseDown(GLFW_MOUSE_BUTTON_2))
	{
		vec2 mouseDelta = Input::GetMouseDelta() * RotateSpeed * deltaTime;
		transform->Rotation += radians(vec3{ -mouseDelta.y, mouseDelta.x, 0 });
	}
}