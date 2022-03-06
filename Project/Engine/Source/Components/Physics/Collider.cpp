#include <Engine/Scene.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Components/Physics/Collider.hpp>
#include <Engine/Components/Physics/Rigidbody.hpp>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Components;

void Collider::Added()
{
	PhysicsComponent::Added();
	GetGameObject()->GetScene()->GetPhysics().AddCollider(this);
}

void Collider::Removed()
{
	PhysicsComponent::Removed();
	GetGameObject()->GetScene()->GetPhysics().RemoveCollider(this);
}

mat4& Collider::InverseTensor() { return m_DefaultInverseTensor; }

void Collider::ProcessTriggerEntries()
{
	// Check for enter events
	if (m_TriggerEnterEvent)
	{
		for (int i = 0; i < (int)m_CurrentTriggerEntries.size(); i++)
		{
			const auto& it = find(m_PreviousTriggerEntries.begin(), m_PreviousTriggerEntries.end(), m_CurrentTriggerEntries[i]);
			if (it == m_PreviousTriggerEntries.end())
			{
				// Was not inside trigger last frame, but is this frame. Must have entered
				m_TriggerEnterEvent(m_CurrentTriggerEntries[i]);
				continue;
			}
		}
	}

	// Check for exit events
	if (m_TriggerExitEvent)
	{
		for (int i = 0; i < (int)m_PreviousTriggerEntries.size(); i++)
		{
			const auto& it = find(m_CurrentTriggerEntries.begin(), m_CurrentTriggerEntries.end(), m_PreviousTriggerEntries[i]);
			if (it == m_CurrentTriggerEntries.end())
			{
				// Was not inside trigger this frame, but was last frame. Must have exited
				m_TriggerExitEvent(m_PreviousTriggerEntries[i]);
				continue;
			}
		}
	}

	m_PreviousTriggerEntries = m_CurrentTriggerEntries;
	m_CurrentTriggerEntries.clear();
}