#pragma once

namespace Engine
{
	class GameObject;
	namespace Physics { class PhysicsSystem; }

	namespace Components
	{
		struct Transform;
		struct Rigidbody;

		class Component
		{
			GameObject* m_GameObject = nullptr;

			Rigidbody* m_CachedRB = nullptr;
			Transform* m_CachedTransform = nullptr;

			friend class Engine::GameObject;

		protected:
			/// <summary>
			/// Component has been added and initialised
			/// </summary>
			virtual void Added() { }

			/// <summary>
			/// Component is about to be removed from object
			/// </summary>
			virtual void Removed();

			virtual void Draw() { }
			virtual void DrawGizmos() { }
			virtual void Update(float deltaTime) { }

		public:
			Rigidbody* GetRigidbody();
			Transform* GetTransform();
			GameObject* GetGameObject() const;
		};

		class PhysicsComponent : public Component
		{
		protected:
			virtual void Added() override;
			virtual void Removed() override;

			virtual void FixedUpdate(float timestep) { }
			virtual void ApplyForces(float timestep) { }
			virtual void ApplyWorldForces(float timestep) { }
			virtual void SolveConstraints(float timestep) { }

			friend class Engine::Physics::PhysicsSystem;
		};
	}
}