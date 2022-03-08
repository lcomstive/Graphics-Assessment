#pragma once
#include <string>
#include <Engine/Api.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Physics/Octree.hpp>
#include <Engine/Physics/PhysicsSystem.hpp>

namespace Engine
{
	class Application;
	class Scene
	{
		std::string m_Name;
		GameObject m_Root;
		Physics::PhysicsSystem m_Physics;

	public:
		ENGINE_API Scene(std::string name = "Scene");

		ENGINE_API GameObject& Root();
		ENGINE_API Physics::PhysicsSystem& GetPhysics();

		ENGINE_API void Draw();
		ENGINE_API void Clear();
		ENGINE_API void DrawGizmos();
		ENGINE_API void Update(float deltaTime);
	};
}