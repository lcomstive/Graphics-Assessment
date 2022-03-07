#include <Engine/Utilities.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Components/Transform.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

Gizmos* Gizmos::s_Instance = nullptr;

Gizmos::Gizmos() : m_Material()
{
	if (!s_Instance)
		s_Instance = this;
}

Gizmos::~Gizmos()
{
	if (s_Instance == this)
		s_Instance = nullptr;
}

void Gizmos::SetColour(glm::vec3 colour) { s_Instance->m_Material.Albedo = vec4(colour.r, colour.g, colour.b, 1.0f); }
void Gizmos::SetColour(glm::vec4 colour) { s_Instance->m_Material.Albedo = colour; }
void Gizmos::SetColour(float r, float g, float b, float a) { s_Instance->m_Material.Albedo = { r, g, b, a }; }

void Gizmos::Draw(ResourceID& mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation)
{
	s_Instance->m_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			mesh,
			s_Instance->m_Material,
			position,
			scale,
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z)
		});
}

void Gizmos::DrawQuad(vec3 position, vec2 scale, vec3 rotation)
{
	s_Instance->m_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			Mesh::Quad(),
			s_Instance->m_Material,
			position,
			{ scale.x, scale.y, 1 },
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z)
		});
}

void Gizmos::DrawCube(vec3 position, vec3 scale, vec3 rotation)
{
	s_Instance->m_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			Mesh::Cube(),
			s_Instance->m_Material,
			position,
			scale,
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z)
		});
}

void Gizmos::DrawSphere(vec3 position, float radius)
{
	s_Instance->m_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			Mesh::Sphere(),
			s_Instance->m_Material,
			position,
			{ radius, radius, radius },
			mat4(1.0f)
		});
}

void Gizmos::DrawWireQuad(vec3 position, vec2 scale, vec3 rotation) { DrawWireQuad(position, scale, eulerAngleXYZ(rotation.x, rotation.y, rotation.z)); }
void Gizmos::DrawWireQuad(vec3 position, vec2 scale, mat4 rotation)
{
	s_Instance->m_Material.Wireframe = true;
	Renderer::Submit(DrawCall
		{
			Mesh::Quad(),
			s_Instance->m_Material,
			position,
			{ scale.x, scale.y, 1 },
			rotation
		});
}

void Gizmos::DrawWireCube(vec3 position, vec3 scale, vec3 rotation) { DrawWireCube(position, scale, eulerAngleXYZ(rotation.x, rotation.y, rotation.z)); }
void Gizmos::DrawWireCube(vec3 position, vec3 scale, mat4 rotation)
{
	s_Instance->m_Material.Wireframe = true;
	Renderer::Submit(DrawCall
		{
			Mesh::Cube(),
			s_Instance->m_Material,
			position,
			scale,
			rotation
		});
}

void Gizmos::DrawWireSphere(vec3 position, float radius)
{
	s_Instance->m_Material.Wireframe = true;
	Renderer::Submit(DrawCall{ Mesh::Sphere(), s_Instance->m_Material, position, { radius, radius, radius } });
}

void Gizmos::DrawLine(vec3 start, vec3 end)
{
	s_Instance->m_Material.Wireframe = false;
	ResourceID id = ResourceManager::Load<Mesh>(vector<Mesh::Vertex>
		{
			{ start }, { end } // Vertices
		}, vector<unsigned int>(), Mesh::DrawMode::Lines);
	Renderer::Submit(DrawCall
		{
			id,
			s_Instance->m_Material,
			vec3(0),
			vec3 { 1, 1, 1 },
			mat4(1.0f),
			true // Delete mesh after rendered
		});
}

void Gizmos::DrawGrid(glm::vec3 position, unsigned int gridSize, glm::vec3 scale, glm::vec3 rotation)
{
	s_Instance->m_Material.Wireframe = false;
	ResourceID gridID = Mesh::Grid(gridSize);
	Renderer::Submit(DrawCall
		{
			gridID,
			s_Instance->m_Material,
			position,
			scale,
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z),
			true // Delete mesh after draw
		});
}

void Gizmos::DrawLine(Line line) { DrawLine(line.Start, line.End); }
void Gizmos::DrawRay(Ray ray) { DrawLine(ray.Origin, ray.Origin + ray.Direction * 1000.0f); }
