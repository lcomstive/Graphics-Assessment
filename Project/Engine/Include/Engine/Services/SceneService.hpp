#pragma once
#include <unordered_map>
#include <Engine/Scene.hpp>
#include <Engine/Services/Service.hpp>

namespace Engine::Services
{
	struct SceneService : public Service
	{
		virtual void OnShutdown() override;
		virtual void OnDraw() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnDrawGizmos() override;

		Scene* CurrentScene();
		void MakeCurrent(std::string name);

		Scene* GetScene(std::string name);
		Scene* NewScene(std::string name, bool active = true);
		void UnloadScene(std::string name);
		void UnloadAllScenes();

		void SetActive(std::string name, bool active = true);

	private:
		struct SceneState
		{
			Scene* Scene;
			bool Active; // Callbacks will be applied
		};

		Scene* m_Current = nullptr;
		std::unordered_map<std::string, SceneState> m_Scenes;

		void SelectActiveScene();
	};
}