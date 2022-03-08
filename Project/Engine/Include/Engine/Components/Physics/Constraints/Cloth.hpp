#pragma once
#include <vector>
#include <Engine/Api.hpp>
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

		ENGINE_API virtual void Draw() override;

	public:
		Graphics::Material Material;

		ENGINE_API void Initialize(unsigned int size, float spacing);
		ENGINE_API void Clear();

		ENGINE_API void SetBendSprings(float stiffness, float dampening);
		ENGINE_API void SetShearSprings(float stiffness, float dampening);
		ENGINE_API void SetStructuralSprings(float stiffness, float dampening);

		ENGINE_API void SetParticleMass(float mass);
	};
}