#pragma once
#pragma once
#include <glm/glm.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Physics/Shapes.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Material.hpp>

namespace Engine::Graphics
{
	class Gizmos
	{
		static Material s_Material;

	public:
		static glm::vec4 Colour;

		static void Draw(Mesh* mesh, glm::vec3 position = { 0, 0, 0 }, glm::vec3 scale = { 1, 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });
		
		static void DrawGrid(glm::vec3 position, unsigned int gridSize, glm::vec3 scale = { 1, 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });

		static void DrawRay(Physics::Ray ray);
		static void DrawLine(Physics::Line line);
		static void DrawLine(glm::vec3 start, glm::vec3 end);

		static void DrawQuad(glm::vec3 position, glm::vec2 scale = { 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });
		static void DrawCube(glm::vec3 position, glm::vec3 scale = { 1, 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });
		static void DrawSphere(glm::vec3 position, float radius);

		static void DrawWireQuad(glm::vec3 position, glm::vec2 scale, glm::vec3 rotation);
		static void DrawWireQuad(glm::vec3 position, glm::vec2 scale = { 1, 1 }, glm::mat4 rotation = glm::mat4(1.0f));

		static void DrawWireCube(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);
		static void DrawWireCube(glm::vec3 position, glm::vec3 scale = { 1, 1, 1 }, glm::mat4 rotation = glm::mat4(1.0f));

		static void DrawWireSphere(glm::vec3 position, float radius = 1.0f);
	};
}