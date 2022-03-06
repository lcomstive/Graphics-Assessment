#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Engine::Graphics
{
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
			QuadStrip		= GL_QUAD_STRIP
		};

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 TexCoords;
			glm::vec3 Tangent;
			glm::vec3 Bitangent;
		};

	private:
		unsigned int m_VBO, m_VAO, m_EBO;

		DrawMode m_DrawMode;
		std::vector<Vertex> m_Vertices;
		std::vector<unsigned int> m_Indices;

		void Setup();

	public:
		Mesh();
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices = {}, DrawMode drawMode = DrawMode::Triangles);
		~Mesh();

		void Draw();
		void SetData(std::vector<Vertex>& vertices, std::vector<unsigned int> indices = {});

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<unsigned int>& GetIndices() { return m_Indices; }

		static Mesh* Quad();
		static Mesh* Cube();
		static Mesh* Line();
		static Mesh* Grid(unsigned int size);
		static Mesh* Sphere();
	};
}