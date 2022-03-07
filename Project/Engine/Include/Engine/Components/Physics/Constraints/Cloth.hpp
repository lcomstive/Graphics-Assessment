#pragma once
#include <vector>
#include <Engine/ResourceID.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Material.hpp>
#include <Engine/Components/Physics/Constraints/Spring.hpp>

namespace Engine::Components
{
	class Cloth : public PhysicsComponent
	{
	protected:
		ResourceID m_MeshID;
		Graphics::Mesh* m_Mesh;

		unsigned int m_ClothSize;
		std::vector<SPRING_PARTICLE_TYPE*> m_Vertices;
		std::vector<Spring*> m_BendSprings;
		std::vector<Spring*> m_ShearSprings;
		std::vector<Spring*> m_StructuralSprings;

		virtual void Draw() override;

	public:
		Graphics::Material Material;

		void Initialize(unsigned int size, float spacing);
		void Clear();

		void SetBendSprings(float stiffness, float dampening);
		void SetShearSprings(float stiffness, float dampening);
		void SetStructuralSprings(float stiffness, float dampening);

		void SetParticleMass(float mass);
	};
}