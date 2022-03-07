#include <Engine/Scene.hpp>
#include <Engine/GameObject.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Components;

GameObject::GameObject(string name) : GameObject((GameObject*)nullptr, name) { }

GameObject::GameObject(Scene* scene, string name) : m_Scene(scene), m_Name(name)
{
	m_Transform = AddComponent<Transform>();
	m_Transform->SetParent(m_Scene->Root().GetTransform());
}

GameObject::GameObject(Transform* parent, string name) : GameObject(parent->GetGameObject(), name) { }
GameObject::GameObject(GameObject* parent, string name) : m_Name(name), m_Components(), m_Scene(parent ? parent->m_Scene : nullptr)
{
	m_Transform = AddComponent<Transform>();

	// Update transform hierarchy
	if(parent && parent->m_Transform)
		m_Transform->SetParent(parent->m_Transform);
}

GameObject::~GameObject()
{	
	m_Transform->SetParent(nullptr);

	for (auto& pair : m_Components)
	{
		pair.second->Removed();
		delete pair.second;
	}
	m_Transform = nullptr;
	m_Components.clear();
}

string GameObject::GetName() { return m_Name; }
Scene* GameObject::GetScene() { return m_Scene; }
void GameObject::SetName(string name) { m_Name = name; }
Transform* GameObject::GetTransform() { return m_Transform; }

void GameObject::Draw()
{
	for (auto& pair : m_Components)
		pair.second->Draw();

	for (Transform* child : m_Transform->GetChildren())
		child->GetGameObject()->Draw();
}

void GameObject::DrawGizmos()
{
	for (auto& pair : m_Components)
		pair.second->DrawGizmos();

	for (Transform* child : m_Transform->GetChildren())
		child->GetGameObject()->DrawGizmos();
}

void GameObject::Update(float deltaTime)
{
	for (auto& pair : m_Components)
		pair.second->Update(deltaTime);

	for (Transform* child : m_Transform->GetChildren())
		child->GetGameObject()->Update(deltaTime);
}
