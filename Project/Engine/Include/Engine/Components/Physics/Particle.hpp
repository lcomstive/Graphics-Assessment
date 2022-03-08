#pragma once
#include <Engine/Api.hpp>
#include <Engine/Log.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Physics/PhysicsSystem.hpp>

namespace Engine::Components
{
	enum class ForceMode { Impulse, Acceleration };

	struct Particle : public PhysicsComponent
	{
		bool UseGravity = true;
		bool IsTrigger = false;

		ENGINE_API Particle();

		ENGINE_API Physics::PhysicsSystem& GetSystem();

		ENGINE_API void ApplyForce(glm::vec3 force, ForceMode mode = ForceMode::Acceleration);

		ENGINE_API float GetMass();
		ENGINE_API void  SetMass(float value);

		ENGINE_API float GetFriction();
		ENGINE_API void  SetFriction(float value);

		ENGINE_API bool IsStatic();
		ENGINE_API void SetStatic(bool value);

		ENGINE_API glm::vec3 GetVelocity();

		ENGINE_API float GetRestitution();
		ENGINE_API void  SetRestitution(float value);

		ENGINE_API void SetCollisionRadius(float radius);

		ENGINE_API float InverseMass();

		ENGINE_API void SolveConstraints(float timestep) override;

	protected:
		ENGINE_API void Added() override;
		ENGINE_API void FixedUpdate(float timestep) override;

	private:
		float m_Radius;
		bool m_IsStatic = false;
		SphereCollider* m_Collider;

		glm::vec3 m_Force, m_Velocity;
		float m_Mass, m_Restitution, m_Friction;
		glm::vec3 m_PreviousPosition;

		void ApplyWorldForces(float timestep) override;
	};
}