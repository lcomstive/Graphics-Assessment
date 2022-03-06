#include <Engine/Utilities.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Components/Transform.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Physics;
using namespace Engine::Graphics;
using namespace Engine::Components;

vec4 Gizmos::Colour = { 1, 1, 1, 1 };
Material Gizmos::s_Material = {};

void Gizmos::Draw(Mesh* mesh, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			mesh,
			s_Material,
			position,
			scale,
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z)
		});
}

void Gizmos::DrawQuad(vec3 position, vec2 scale, vec3 rotation)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			Mesh::Quad(),
			s_Material,
			position,
			{ scale.x, scale.y, 1 },
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z)
		});
}

void Gizmos::DrawCube(vec3 position, vec3 scale, vec3 rotation)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			Mesh::Cube(),
			s_Material,
			position,
			scale,
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z)
		});
}

void Gizmos::DrawSphere(vec3 position, float radius)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			Mesh::Sphere(),
			s_Material,
			position,
			{ radius, radius, radius },
			mat4(1.0f)
		});
}

void Gizmos::DrawWireQuad(vec3 position, vec2 scale, vec3 rotation) { DrawWireQuad(position, scale, eulerAngleXYZ(rotation.x, rotation.y, rotation.z)); }
void Gizmos::DrawWireQuad(vec3 position, vec2 scale, mat4 rotation)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = true;
	Renderer::Submit(DrawCall
		{
			Mesh::Quad(),
			s_Material,
			position,
			{ scale.x, scale.y, 1 },
			rotation
		});
}

void Gizmos::DrawWireCube(vec3 position, vec3 scale, vec3 rotation) { DrawWireCube(position, scale, eulerAngleXYZ(rotation.x, rotation.y, rotation.z)); }
void Gizmos::DrawWireCube(vec3 position, vec3 scale, mat4 rotation)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = true;
	Renderer::Submit(DrawCall
		{
			Mesh::Cube(),
			s_Material,
			position,
			scale,
			rotation
		});
}

void Gizmos::DrawWireSphere(vec3 position, float radius)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = true;
	Renderer::Submit(DrawCall{ Mesh::Sphere(), s_Material, position, { radius, radius, radius } });
}

void Gizmos::DrawLine(vec3 start, vec3 end)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			new Mesh(vector<Mesh::Vertex>
			{
				{ start }, { end } // Vertices
			}, {}, Mesh::DrawMode::Lines),
			s_Material,
			vec3(0),
			{ 1, 1, 1 },
			mat4(1.0f),
			true // Delete mesh after rendered
		});
}

void Gizmos::DrawGrid(glm::vec3 position, unsigned int gridSize, glm::vec3 scale, glm::vec3 rotation)
{
	s_Material.Albedo = Colour;
	s_Material.Wireframe = false;
	Renderer::Submit(DrawCall
		{
			Mesh::Grid(gridSize),
			s_Material,
			position,
			scale,
			eulerAngleXYZ(rotation.x, rotation.y, rotation.z),
			true // Delete mesh after draw
		});
}

void Gizmos::DrawLine(Line line) { DrawLine(line.Start, line.End); }
void Gizmos::DrawRay(Ray ray) { DrawLine(ray.Origin, ray.Origin + ray.Direction * 1000.0f); }
