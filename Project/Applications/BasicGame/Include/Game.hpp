#pragma once
#include <Engine/ResourceID.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Services/ExternalService.hpp>

class Game : public Engine::Services::Service
{
	Engine::Scene* m_Scene = nullptr;

#pragma region GameObjects
	Engine::GameObject* m_Player = nullptr;
	Engine::Components::Light* m_Light = nullptr;

	// All spawned GameObjects
	std::vector<Engine::GameObject*> m_GameObjects;
#pragma endregion

	void ResetScene();

protected:
	void OnStart() override;
	void OnShutdown() override;

	void OnDraw() override;
	void OnUpdate(float deltaTime) override;
};