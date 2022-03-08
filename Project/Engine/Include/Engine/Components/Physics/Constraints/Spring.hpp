#pragma once
#define SPRING_USE_RIGIDBODIES 0

#include <Engine/Api.hpp>

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
		ENGINE_API SPRING_PARTICLE_TYPE* GetPoint1();
		ENGINE_API SPRING_PARTICLE_TYPE* GetPoint2();

		ENGINE_API void SetBodies(SPRING_PARTICLE_TYPE* a, SPRING_PARTICLE_TYPE* b);

		/// <param name="stiffness">How stiff the spring is. Range is [0, -inf]</param>
		/// <param name="dampening">Dampening force on the spring. Range is [0.0-1.0]</param>
		ENGINE_API void SetConstants(float stiffness, float dampening);

		ENGINE_API void SetRestingLength(float length);
		ENGINE_API float GetRestingLength();

		ENGINE_API void DrawGizmos() override;

	protected:
		SPRING_PARTICLE_TYPE* m_Point1 = nullptr;
		SPRING_PARTICLE_TYPE* m_Point2 = nullptr;

		float m_Stiffness = 0; // [0 - inf] Higher = stiffer
		float m_Dampening = 0.1f; // [0-1]
		float m_RestingLength = 1.0f;

		ENGINE_API void ApplyForces(float timestep) override;
	};
}