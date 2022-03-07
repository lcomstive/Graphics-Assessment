#include <Engine/Scene.hpp>
#include <Engine/Application.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Physics;

Scene::Scene(string name) : m_Name(name), m_Root(this, "Root"), m_Physics() { }

GameObject& Scene::Root() { return m_Root; }
PhysicsSystem& Scene::GetPhysics() { return m_Physics; }

void Scene::Draw() { m_Root.Draw(); }
void Scene::Update(float deltaTime) { m_Root.Update(deltaTime); }

void Scene::Clear() { m_Root.GetTransform()->ClearChildren(); }

void Scene::DrawGizmos()
{
	m_Root.DrawGizmos();
	m_Physics.DrawGizmos();
}