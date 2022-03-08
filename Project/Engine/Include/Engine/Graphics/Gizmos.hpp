#pragma once
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Physics/Shapes.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Material.hpp>

namespace Engine { class Application; }

namespace Engine::Graphics
{
	class Gizmos
	{
		Material m_Material;

		static Gizmos* s_Instance;

		Gizmos();
		~Gizmos();

		friend class Application;

	public:
		ENGINE_API static void SetColour(glm::vec3 colour);
		ENGINE_API static void SetColour(glm::vec4 colour);
		ENGINE_API static void SetColour(float r, float g, float b, float a = 1.0f);

		ENGINE_API static void Draw(ResourceID& mesh, glm::vec3 position = { 0, 0, 0 }, glm::vec3 scale = { 1, 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });
		
		ENGINE_API static void DrawGrid(glm::vec3 position, unsigned int gridSize, glm::vec3 scale = { 1, 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });

		ENGINE_API static void DrawRay(Physics::Ray ray);
		ENGINE_API static void DrawLine(Physics::Line line);
		ENGINE_API static void DrawLine(glm::vec3 start, glm::vec3 end);

		ENGINE_API static void DrawQuad(glm::vec3 position, glm::vec2 scale = { 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });
		ENGINE_API static void DrawCube(glm::vec3 position, glm::vec3 scale = { 1, 1, 1 }, glm::vec3 rotation = { 0, 0, 0 });
		ENGINE_API static void DrawSphere(glm::vec3 position, float radius);

		ENGINE_API static void DrawWireQuad(glm::vec3 position, glm::vec2 scale, glm::vec3 rotation);
		ENGINE_API static void DrawWireQuad(glm::vec3 position, glm::vec2 scale = { 1, 1 }, glm::mat4 rotation = glm::mat4(1.0f));

		ENGINE_API static void DrawWireCube(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);
		ENGINE_API static void DrawWireCube(glm::vec3 position, glm::vec3 scale = { 1, 1, 1 }, glm::mat4 rotation = glm::mat4(1.0f));

		ENGINE_API static void DrawWireSphere(glm::vec3 position, float radius = 1.0f);
	};
}