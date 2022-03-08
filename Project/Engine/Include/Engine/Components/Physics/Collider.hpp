#pragma once
#include <functional>
#include <Engine/Api.hpp>
#include <Engine/Physics/Shapes.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Physics/CollisionInfo.hpp>

namespace Engine::Components
{
	struct BoxCollider;
	struct PlaneCollider;
	struct SphereCollider;

	/// <summary>
	/// Abstract class for colliders
	/// </summary>
	struct Collider : public PhysicsComponent
	{
		/// <summary>
		/// Triggers the collision event on collision, but does not apply any collision resolution from this object
		/// </summary>
		bool IsTrigger = false;

		ENGINE_API virtual Physics::OBB& GetBounds() = 0;
		ENGINE_API virtual bool LineTest(Physics::Line& line) = 0;
		ENGINE_API virtual bool IsPointInside(glm::vec3& point) const = 0;
		ENGINE_API virtual bool Raycast(Physics::Ray& ray, Physics::RaycastHit* outResult = nullptr) = 0;

		ENGINE_API virtual bool CheckCollision(Collider* other) = 0;
		ENGINE_API virtual bool CheckCollision(BoxCollider* other) = 0;
		ENGINE_API virtual bool CheckCollision(PlaneCollider* other) = 0;
		ENGINE_API virtual bool CheckCollision(SphereCollider* other) = 0;

		ENGINE_API virtual glm::mat4& InverseTensor();

		ENGINE_API void SetTriggerExitEvent(std::function<void(Collider*)> callback) { m_TriggerExitEvent = callback; }
		ENGINE_API void SetTriggerEnterEvent(std::function<void(Collider*)> callback) { m_TriggerEnterEvent = callback; }
		ENGINE_API void SetCollisionEvent(std::function<void(Collider*, Rigidbody*)> callback) { m_CollisionEvent = callback; }

	protected:
		ENGINE_API virtual void Added() override;
		ENGINE_API virtual void Removed() override;

	private:
		std::function<void(Collider*)> m_TriggerExitEvent;
		std::function<void(Collider*)> m_TriggerEnterEvent;
		std::function<void(Collider*, Rigidbody*)> m_CollisionEvent;

		/// <summary>
		/// If IsTrigger, all colliders that are currently inside of this collider
		/// </summary>
		std::vector<Collider*> m_CurrentTriggerEntries;

		/// <summary>
		/// If IsTrigger, all colliders that are were inside of this collider last frame
		/// </summary>
		std::vector<Collider*> m_PreviousTriggerEntries;

		glm::mat4 m_DefaultInverseTensor = glm::mat4(0.0f);

		void ProcessTriggerEntries();

		friend struct Engine::Components::Rigidbody;
		friend class Engine::Physics::PhysicsSystem;
	};
}