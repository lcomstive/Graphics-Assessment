#pragma once
#include <Engine/Api.hpp>

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
			ENGINE_API virtual void Added() { }

			/// <summary>
			/// Component is about to be removed from object
			/// </summary>
			ENGINE_API virtual void Removed();

			ENGINE_API virtual void Draw() { }
			ENGINE_API virtual void DrawGizmos() { }
			ENGINE_API virtual void Update(float deltaTime) { }

		public:
			ENGINE_API Rigidbody* GetRigidbody();
			ENGINE_API Transform* GetTransform();
			ENGINE_API GameObject* GetGameObject() const;
		};

		class PhysicsComponent : public Component
		{
		protected:
			ENGINE_API virtual void Added() override;
			ENGINE_API virtual void Removed() override;

			ENGINE_API virtual void FixedUpdate(float timestep) { }
			ENGINE_API virtual void ApplyForces(float timestep) { }
			ENGINE_API virtual void ApplyWorldForces(float timestep) { }
			ENGINE_API virtual void SolveConstraints(float timestep) { }

		private:
			friend class Engine::Physics::PhysicsSystem;
		};
	}
}