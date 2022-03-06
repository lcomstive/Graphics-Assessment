#include <Engine/Utilities.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Components/Physics/SphereCollider.hpp>
#include <Engine/Components/Physics/Constraints/Cloth.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Graphics;
using namespace Engine::Components;

void Cloth::Initialize(unsigned int size, float distance)
{
	float k = 1.0f;
	float b = 0.1f;

	// Cloth could be reused, clear values
	Clear();

	m_ClothSize = std::max(size, 3u);
	m_Vertices.resize((size_t)(m_ClothSize * m_ClothSize));

	// Create vertex particles w/ default values
	for (unsigned int x = 0; x < m_ClothSize; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize; z++)
		{
			int i = z * m_ClothSize + x;
			float xPos = (x - (m_ClothSize / 2.0f)) * distance;
			float zPos = (z - (m_ClothSize / 2.0f)) * distance;

			GameObject* go = new GameObject(GetGameObject(), "Particle " + to_string(i));
			m_Vertices[i] = go->AddComponent<SPRING_PARTICLE_TYPE>();
			m_Vertices[i]->GetTransform()->Position = { xPos, 0, zPos };
			m_Vertices[i]->SetMass(1.0f);
			m_Vertices[i]->SetRestitution(0.0f);
			m_Vertices[i]->SetFriction(0.9f);

#if SPRING_USE_RIGIDBODIES
			// go->AddComponent<SphereCollider>()->SetRadius(distance * 0.5f);
			go->AddComponent<SphereCollider>()->SetRadius(0.1f);
#else
			// m_Vertices[i]->SetCollisionRadius(distance * 0.5f);
			m_Vertices[i]->SetCollisionRadius(0.1f);
#endif
		}
	}

	// Create left-to-right structural springs
	for (unsigned int x = 0; x < m_ClothSize; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize - 1; z++)
		{
			int i = z * m_ClothSize + x;
			int j = (z + 1) * m_ClothSize + x;

			vec3 iPos = m_Vertices[i]->GetTransform()->Position;
			vec3 jPos = m_Vertices[j]->GetTransform()->Position;
			float restingLength = Magnitude(iPos - jPos);

			Spring* spring = (new GameObject(GetGameObject(), "Spring LR " + to_string(i)))->AddComponent<Spring>();
			spring->SetBodies(m_Vertices[i], m_Vertices[j]);
			spring->SetRestingLength(restingLength);
			spring->SetConstants(k, b);
			m_StructuralSprings.emplace_back(spring);
		}
	}

	// Create up-to-down structural springs
	for (unsigned int x = 0; x < m_ClothSize - 1; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize; z++)
		{
			int i = z * m_ClothSize + x;
			int j = z * m_ClothSize + (x + 1);

			vec3 iPos = m_Vertices[i]->GetTransform()->Position;
			vec3 jPos = m_Vertices[j]->GetTransform()->Position;
			float restingLength = Magnitude(iPos - jPos);

			Spring* spring = (new GameObject(GetGameObject(), "Spring UD " + to_string(i)))->AddComponent<Spring>();
			spring->SetBodies(m_Vertices[i], m_Vertices[j]);
			spring->SetRestingLength(restingLength);
			spring->SetConstants(k, b);
			m_StructuralSprings.emplace_back(spring);
		}
	}

	// Create the left-to-right shear springs
	for (unsigned int x = 0; x < m_ClothSize - 1; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize - 1; z++)
		{
			int i = z * m_ClothSize + x;
			int j = (z + 1) * m_ClothSize + (x + 1);

			vec3 iPos = m_Vertices[i]->GetTransform()->Position;
			vec3 jPos = m_Vertices[j]->GetTransform()->Position;
			float restingLength = Magnitude(iPos - jPos);

			Spring* spring = (new GameObject(GetGameObject(), "Spring LR Shear " + to_string(i)))->AddComponent<Spring>();
			spring->SetBodies(m_Vertices[i], m_Vertices[j]);
			spring->SetRestingLength(restingLength);
			spring->SetConstants(k, b);
			m_ShearSprings.emplace_back(spring);
		}
	}

	// Create the up-to-down shear springs
	for (unsigned int x = 1; x < m_ClothSize - 1; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize - 1; z++)
		{
			int i = z * m_ClothSize + x;
			int j = (z + 1) * m_ClothSize + (x - 1);

			vec3 iPos = m_Vertices[i]->GetTransform()->Position;
			vec3 jPos = m_Vertices[j]->GetTransform()->Position;
			float restingLength = Magnitude(iPos - jPos);

			Spring* spring = (new GameObject(GetGameObject(), "Spring UD Shear " + to_string(i)))->AddComponent<Spring>();
			spring->SetBodies(m_Vertices[i], m_Vertices[j]);
			spring->SetRestingLength(restingLength);
			spring->SetConstants(k, b);
			m_ShearSprings.emplace_back(spring);
		}
	}

	// Create the left-to-right bend springs
	for (unsigned int x = 0; x < m_ClothSize; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize - 2; z++)
		{
			int i = z * m_ClothSize + x;
			int j = (z + 2) * m_ClothSize + x;

			vec3 iPos = m_Vertices[i]->GetTransform()->Position;
			vec3 jPos = m_Vertices[j]->GetTransform()->Position;
			float restingLength = Magnitude(iPos - jPos);

			Spring* spring = (new GameObject(GetGameObject(), "Spring LR Bend " + to_string(i)))->AddComponent<Spring>();
			spring->SetBodies(m_Vertices[i], m_Vertices[j]);
			spring->SetRestingLength(restingLength);
			spring->SetConstants(k, b);
			m_BendSprings.emplace_back(spring);
		}
	}

	// Create the up-to-down bend springs
	for (unsigned int x = 0; x < m_ClothSize - 2; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize; z++)
		{
			int i = z * m_ClothSize + x;
			int j = z * m_ClothSize + (x + 2);

			vec3 iPos = m_Vertices[i]->GetTransform()->Position;
			vec3 jPos = m_Vertices[j]->GetTransform()->Position;
			float restingLength = Magnitude(iPos - jPos);

			Spring* spring = (new GameObject(GetGameObject(), "Spring UD Bend " + to_string(i)))->AddComponent<Spring>();
			spring->SetBodies(m_Vertices[i], m_Vertices[j]);
			spring->SetRestingLength(restingLength);
			spring->SetConstants(k, b);
			m_BendSprings.emplace_back(spring);
		}
	}

	// Generate mesh data
	vector<Mesh::Vertex> meshVerts;
	for (unsigned int x = 0; x < m_ClothSize - 1; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize - 1; z++)
		{
			int tl = z * m_ClothSize + x;
			int bl = (z + 1) * m_ClothSize + x;
			int tr = z * m_ClothSize + (x + 1);
			int br = (z + 1) * m_ClothSize + (x + 1);

			meshVerts.emplace_back(Mesh::Vertex{ m_Vertices[tl]->GetTransform()->Position });
			meshVerts.emplace_back(Mesh::Vertex{ m_Vertices[br]->GetTransform()->Position });
			meshVerts.emplace_back(Mesh::Vertex{ m_Vertices[bl]->GetTransform()->Position });
			meshVerts.emplace_back(Mesh::Vertex{ m_Vertices[tl]->GetTransform()->Position });
			meshVerts.emplace_back(Mesh::Vertex{ m_Vertices[tr]->GetTransform()->Position });
			meshVerts.emplace_back(Mesh::Vertex{ m_Vertices[br]->GetTransform()->Position });
		}
	}

	m_Mesh = new Mesh(meshVerts);
}

void Cloth::Clear()
{
	m_Vertices.clear();
	m_BendSprings.clear();
	m_ShearSprings.clear();
	m_StructuralSprings.clear();

	for (SPRING_PARTICLE_TYPE* vertex : m_Vertices)
	{
		vertex->GetGameObject()->RemoveComponent<SPRING_PARTICLE_TYPE>();
		delete vertex->GetGameObject();
	}
	m_Vertices.clear();

	for (Spring* spring : m_StructuralSprings)
	{
		spring->GetGameObject()->RemoveComponent<Spring>();
		delete spring->GetGameObject();
	}
	m_StructuralSprings.clear();

	for (Spring* spring : m_BendSprings)
	{
		spring->GetGameObject()->RemoveComponent<Spring>();
		delete spring->GetGameObject();
	}
	m_BendSprings.clear();

	for (Spring* spring : m_ShearSprings)
	{
		spring->GetGameObject()->RemoveComponent<Spring>();
		delete spring->GetGameObject();
	}
	m_ShearSprings.clear();

	if (m_Mesh)
	{
		delete m_Mesh;
		m_Mesh = nullptr;
	}
}

void Cloth::SetStructuralSprings(float k, float b)
{
	for (Spring* spring : m_StructuralSprings)
		spring->SetConstants(k, b);
}

void Cloth::SetBendSprings(float k, float b)
{
	for (Spring* spring : m_BendSprings)
		spring->SetConstants(k, b);
}

void Cloth::SetShearSprings(float k, float b)
{
	for (Spring* spring : m_ShearSprings)
		spring->SetConstants(k, b);
}

void Cloth::SetParticleMass(float mass)
{
	for (SPRING_PARTICLE_TYPE* particle : m_Vertices)
		particle->SetMass(mass);
}

void Cloth::Draw()
{
	if (!m_Mesh)
		return;

	unsigned int i = 0;
	auto& verts = m_Mesh->GetVertices();
	for (unsigned int x = 0; x < m_ClothSize - 1; x++)
	{
		for (unsigned int z = 0; z < m_ClothSize - 1; z++)
		{
			int tl = z * m_ClothSize + x;
			int bl = (z + 1) * m_ClothSize + x;
			int tr = z * m_ClothSize + (x + 1);
			int br = (z + 1) * m_ClothSize + (x + 1);

			verts[i++].Position = m_Vertices[tl]->GetTransform()->Position;
			verts[i++].Position = m_Vertices[br]->GetTransform()->Position;
			verts[i++].Position = m_Vertices[bl]->GetTransform()->Position;
			verts[i++].Position = m_Vertices[tl]->GetTransform()->Position;
			verts[i++].Position = m_Vertices[tr]->GetTransform()->Position;
			verts[i++].Position = m_Vertices[br]->GetTransform()->Position;
		}
	}

	m_Mesh->SetData(verts);

	Renderer::Submit(m_Mesh, Material, GetTransform());
}