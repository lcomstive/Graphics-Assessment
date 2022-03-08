#pragma once
#include <Engine/Api.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Components/Physics/Particle.hpp>

namespace Engine::Components
{
	class DistanceJoint : public PhysicsComponent
	{
	protected:
		Particle* m_Body1 = nullptr;
		Particle* m_Body2 = nullptr;
		float m_Length = 1.0f;

		ENGINE_API virtual void DrawGizmos() override;
		ENGINE_API virtual void SolveConstraints(float timestep) override;

	public:
		ENGINE_API void Initialise(Particle* a, Particle* b, float length);

		ENGINE_API float GetLength();
		ENGINE_API void SetLength(float length);
	};
}