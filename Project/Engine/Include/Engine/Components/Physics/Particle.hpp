#pragma once
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

		Particle();

		Physics::PhysicsSystem& GetSystem();

		void ApplyForce(glm::vec3 force, ForceMode mode = ForceMode::Acceleration);

		float GetMass();
		void  SetMass(float value);

		float GetFriction();
		void  SetFriction(float value);

		bool IsStatic();
		void SetStatic(bool value);

		glm::vec3 GetVelocity();

		float GetRestitution();
		void  SetRestitution(float value);

		void SetCollisionRadius(float radius);

		float InverseMass();

		void SolveConstraints(float timestep) override;

	protected:
		void Added() override;
		void FixedUpdate(float timestep) override;

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