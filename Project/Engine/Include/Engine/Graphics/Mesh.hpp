#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <Engine/Api.hpp>
#include <Engine/ResourceID.hpp>

namespace Engine { class Application; }

namespace Engine::Graphics
{
	const unsigned int MeshPrimitiveCount = 4;
	
	class Mesh
	{
	public:
		enum class DrawMode : unsigned char
		{
			Points			= GL_POINTS,
			Lines			= GL_LINES,
			LineStrip		= GL_LINE_STRIP,
			Triangles		= GL_TRIANGLES,
			TriangleStrip	= GL_TRIANGLE_STRIP,
			TriangleFan		= GL_TRIANGLE_FAN,
			Quads			= GL_QUADS,
			QuadStrip		= GL_QUAD_STRIP,
			Patches			= GL_PATCHES
		};

		struct ENGINE_API Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 TexCoords;
			glm::vec3 Tangent;
			glm::vec3 Bitangent;
		};

	private:
		unsigned int m_VBO, m_VAO, m_EBO;

		bool m_Setup;
		DrawMode m_DrawMode;
		std::vector<Vertex> m_Vertices;
		std::vector<unsigned int> m_Indices;

		void Setup();

	public:
		ENGINE_API Mesh();
		ENGINE_API Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices = {}, DrawMode drawMode = DrawMode::Triangles);
		ENGINE_API ~Mesh();

		ENGINE_API void Draw();
		ENGINE_API void SetData(std::vector<Vertex>& vertices, std::vector<unsigned int> indices = {});

		ENGINE_API std::vector<Vertex>& GetVertices() { return m_Vertices; }
		ENGINE_API std::vector<unsigned int>& GetIndices() { return m_Indices; }

		ENGINE_API static ResourceID& Quad();
		ENGINE_API static ResourceID& Cube();
		ENGINE_API static ResourceID& Line();
		ENGINE_API static ResourceID& Sphere();
		ENGINE_API static ResourceID Grid(unsigned int size);
	};
}