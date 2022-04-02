#include <Engine/Input.hpp>
#include "Components/MouseLook.hpp"
#include <Engine/Components/Transform.hpp>

using namespace glm;
using namespace Engine;
using namespace Engine::Components;

const vec2 VerticalConstraints =
{
	radians(-60.0f),
	radians( 60.0f)
};

void MouseLook::Update(float deltaTime)
{
	Transform* transform = GetTransform();
	vec2 mouseDelta = Input::GetMouseDelta() * Sensitivity * deltaTime;

	if (Invert)
		mouseDelta *= -1.0f;

	if (Axis == LookAxis::Vertical)
	{
		transform->Rotation.x = clamp(
			transform->Rotation.x + mouseDelta.y,
			VerticalConstraints.x, // Min
			VerticalConstraints.y  // Max
		);
	}
	else
		transform->Rotation.y -= radians(mouseDelta.x);
}
