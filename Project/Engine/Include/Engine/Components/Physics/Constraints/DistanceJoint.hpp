#pragma once
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

		virtual void DrawGizmos() override;
		virtual void SolveConstraints(float timestep) override;

	public:
		void Initialise(Particle* a, Particle* b, float length);

		float GetLength();
		void SetLength(float length);
	};
}