#include <Engine/Services/SceneService.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Services;

void SceneService::OnShutdown() { UnloadAllScenes(); }

void SceneService::UnloadAllScenes()
{
	for (auto& it : m_Scenes)
		delete it.second.Scene;
	m_Current = nullptr;
}

void SceneService::OnDraw()
{
	for (auto& it : m_Scenes)
		it.second.Scene->Draw();
}

void SceneService::OnDrawGizmos()
{
	for (auto& it : m_Scenes)
		it.second.Scene->DrawGizmos();
}

void SceneService::OnUpdate(float deltaTime)
{
	for (auto& it : m_Scenes)
		it.second.Scene->Update(deltaTime);
}

void SceneService::UnloadScene(string name)
{
	auto& it = m_Scenes.find(name);
	if (it == m_Scenes.end())
		return;
	if (m_Current == it->second.Scene)
		m_Current = nullptr;
	delete it->second.Scene;
	m_Scenes.erase(it);

	SelectActiveScene();
}

void SceneService::SetActive(string name, bool active)
{
	auto& it = m_Scenes.find(name);
	if (it == m_Scenes.end())
		return;
	it->second.Active = active;

	if (it->second.Scene == m_Current)
		m_Current = nullptr; // Select new active scene
	SelectActiveScene();
}

Scene* SceneService::GetScene(string name)
{
	auto& it = m_Scenes.find(name);
	return it == m_Scenes.end() ? nullptr : it->second.Scene;
}

Scene* SceneService::NewScene(string name, bool active)
{
	SceneState state;
	state.Active = active;
	state.Scene = new Scene();
	
	if (!active)
		state.Scene->GetPhysics().Pause();

	m_Scenes.emplace(name, state);
	SelectActiveScene();
	return state.Scene;
}

Scene* SceneService::CurrentScene() { return m_Current; }

void SceneService::MakeCurrent(string name)
{
	Scene* scene = GetScene(name);
	if (scene)
		m_Current = scene;
}

void SceneService::SelectActiveScene()
{
	if (m_Current)
		return;

	for (auto& pair : m_Scenes)
	{
		if (pair.second.Active )
		{
			m_Current = pair.second.Scene;
			break;
		}
	}
}