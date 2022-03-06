#pragma once
#define SPRING_USE_RIGIDBODIES 0

#if SPRING_USE_RIGIDBODIES
#include <Engine/Components/Physics/Rigidbody.hpp>
#define SPRING_PARTICLE_TYPE Engine::Components::Rigidbody
#else
#include <Engine/Components/Physics/Particle.hpp>
#define SPRING_PARTICLE_TYPE Engine::Components::Particle
#endif

namespace Engine::Components
{
	struct Spring : PhysicsComponent
	{
		SPRING_PARTICLE_TYPE* GetPoint1();
		SPRING_PARTICLE_TYPE* GetPoint2();

		void SetBodies(SPRING_PARTICLE_TYPE* a, SPRING_PARTICLE_TYPE* b);

		/// <param name="stiffness">How stiff the spring is. Range is [0, -inf]</param>
		/// <param name="dampening">Dampening force on the spring. Range is [0.0-1.0]</param>
		void SetConstants(float stiffness, float dampening);

		void SetRestingLength(float length);
		float GetRestingLength();

		void DrawGizmos() override;

	protected:
		SPRING_PARTICLE_TYPE* m_Point1 = nullptr;
		SPRING_PARTICLE_TYPE* m_Point2 = nullptr;

		float m_Stiffness = 0; // [0 - inf] Higher = stiffer
		float m_Dampening = 0.1f; // [0-1]
		float m_RestingLength = 1.0f;

		void ApplyForces(float timestep) override;
	};
}