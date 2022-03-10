#pragma once
#include <unordered_map>
#include <Engine/Scene.hpp>
#include <Engine/Types.hpp>
#include <Engine/Services/Service.hpp>

namespace Engine::Services
{
	struct SceneService : public Service
	{
		ENGINE_API virtual void OnShutdown() override;
		ENGINE_API virtual void OnDraw() override;
		ENGINE_API virtual void OnUpdate(float deltaTime) override;
		ENGINE_API virtual void OnDrawGizmos() override;

		ENGINE_API Scene* CurrentScene();
		ENGINE_API void MakeCurrent(std::string name);

		ENGINE_API Scene* GetScene(std::string name);
		ENGINE_API Scene* NewScene(std::string name, bool active = true);
		ENGINE_API void UnloadScene(std::string name);
		ENGINE_API void UnloadAllScenes();

		ENGINE_API void SetActive(std::string name, bool active = true);

	private:
		struct SceneState
		{
			Scene* Scene;
			bool Active; // Callbacks will be applied
		};

		Scene* m_Current = nullptr;
		EngineUnorderedMap<std::string, SceneState> m_Scenes;

		void SelectActiveScene();
	};
}