#include <Engine/Scene.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>

using namespace Engine;
using namespace Engine::Components;

GameObject* Component::GetGameObject() const { return m_GameObject; }

void Component::Removed()
{
	m_CachedRB = nullptr;
	m_GameObject = nullptr;
	m_CachedTransform = nullptr;
}

Transform* Component::GetTransform() 
{
	if (m_CachedTransform)
		return m_CachedTransform;
	else
		return (m_CachedTransform = m_GameObject->GetTransform());
}

Rigidbody* Component::GetRigidbody()
{
	if (m_CachedRB)
		return m_CachedRB;
	else
		return (m_CachedRB = m_GameObject->GetComponent<Rigidbody>());
}

#pragma region Physics Component
void PhysicsComponent::Added()  { GetGameObject()->GetScene()->GetPhysics().AddPhysicsComponent(this); }
void PhysicsComponent::Removed() { GetGameObject()->GetScene()->GetPhysics().RemovePhysicsComponent(this); }
#pragma endregion

void Engine::Components::OverrideComponentInternals(
	Component* component,
	GameObject* go,
	Transform* transform,
	Rigidbody* rb)
{
	component->m_CachedRB = rb;
	component->m_GameObject = go;
	component->m_CachedTransform = transform;
}
