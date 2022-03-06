#pragma once
#include <string>
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

		static Scene* s_Active;

	public:
		Scene(Application* app, std::string name = "Scene");

		GameObject& Root();
		Physics::PhysicsSystem& GetPhysics();

		void Draw();
		void Clear();
		void DrawGizmos();
		void Update(float deltaTime);

		void SetActive();
		static Scene* GetActive();
	};
}