#pragma once
#include <Engine/GameObject.hpp>
#include <Engine/Components/Transform.hpp>

namespace MV // ModelViewer
{
	void DrawHierarchy(Engine::Scene* scene);
	void DrawHierarchy(Engine::GameObject* go);
	void DrawGizmo(Engine::Components::Transform* transform);
}