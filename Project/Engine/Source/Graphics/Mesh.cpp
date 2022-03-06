#include <glad/glad.h>
#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Model.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

Mesh::Mesh() : m_Vertices(), m_Indices(), m_VAO(GL_INVALID_VALUE), m_VBO(), m_EBO(), m_DrawMode(DrawMode::Triangles) { }

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, DrawMode drawMode) : Mesh()
{
	m_DrawMode = drawMode;
	SetData(vertices, indices);
}

Mesh::~Mesh()
{
	if (m_VAO == GL_INVALID_VALUE)
		return; // Not set up

	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void Mesh::SetData(std::vector<Vertex>& vertices, std::vector<unsigned int> indices)
{
	m_Vertices = vertices;
	m_Indices = indices;

	if (m_VAO == GL_INVALID_VALUE)
	{
		Setup();
		return;
	}

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0]);

	if (m_Indices.size() > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Indices.size() * sizeof(unsigned int), &m_Indices[0]);
	}

	glBindVertexArray(0);
}

void Mesh::Setup()
{
	// Generate buffers
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Fill vertex data
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

	// Fill index data
	if (m_Indices.size() > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);
	}

	// Vertex data layout
	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// Normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// Tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// Bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	// Unbind VAO to prevent data being overriden accidentally
	glBindVertexArray(0);
}

void Mesh::Draw()
{
	glBindVertexArray(m_VAO);
	if (m_Indices.size() > 0)
		glDrawElements((GLenum)m_DrawMode, (GLsizei)m_Indices.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays((GLenum)m_DrawMode, 0, (GLint)m_Vertices.size());
	glBindVertexArray(0);
}


Mesh* LineInstance = nullptr;
Mesh* GridInstance = nullptr;
Mesh* QuadInstance = nullptr;
Model* CubeInstance = nullptr;
Model* SphereInstance = nullptr;

const string CubePath = "Models/Primitives/Cube.fbx";
const string QuadPath = "Models/Primitives/Quad.fbx";
const string SpherePath = "Models/Primitives/Sphere.fbx";

Mesh* Mesh::Quad()
{
	if (QuadInstance)
		return QuadInstance;

	vector<Vertex> vertices =
	{
		{
			{  1.0f,  1.0f, 0.0f }, // Position
			{  1.0f,  0.0f, 0.0f }, // Normals
			{  1.0f,  1.0f }		// Texture Coordinates
		},
		{
			{  1.0f, -1.0f, 0.0f }, // Position
			{  0.0f,  1.0f, 0.0f }, // Normals
			{  1.0f,  0.0f }		// Texture Coordinates
		},
		{
			{ -1.0f, -1.0f, 0.0f }, // Position
			{  0.0f,  0.0f, 1.0f }, // Normals
			{  0.0f,  0.0f }		// Texture Coordinates
		},
		{
			{ -1.0f,  1.0f, 0.0f }, // Position
			{  0.0f,  0.0f, 1.0f }, // Normals
			{  0.0f,  1.0f }		// Texture Coordinates
		}
	};
	vector<unsigned int> indices =
	{
		0, 1, 3, // Triangle 1
		1, 2, 3  // Triangle 2
	};

	QuadInstance = new Mesh(vertices, indices);

	return QuadInstance;
}

Mesh* Mesh::Line()
{
	if (LineInstance)
		return LineInstance;

	vector<Vertex> vertices =
	{
		{
			{  0.0f,  0.0f, 0.0f }, // Position
			{  0.0f,  0.0f, 0.0f }, // Normals
			{  0.0f,  0.0f }		// Texture Coordinates
		},
		{
			{  1.0f,  1.0f, 0.0f }, // Position
			{  0.0f,  0.0f, 0.0f }, // Normals
			{  0.0f,  0.0f }		// Texture Coordinates
		}
	};
	vector<unsigned int> indices = { 0, 1 };

	LineInstance = new Mesh(vertices, indices, DrawMode::Lines);
	return LineInstance;
}

Mesh* Mesh::Grid(unsigned int size)
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;

	for (unsigned int i = 0; i <= size; i++)
	{
		for (unsigned int j = 0; j <= size; j++)
		{
			float x = ((float)i / (float)size);
			float z = ((float)j / (float)size);
			vertices.emplace_back(Vertex
				{
					{ x, 0, z }, // Positions
					{ 0, 1, 0 }, // Normals
					{ 0, 0 }	 // Texture Coordinates
				});
		}
	}

	for (unsigned int j = 0; j < size; j++)
	{
		for (unsigned int i = 0; i < size; i++)
		{
			int row1 = j * (size + 1);
			int row2 = (j + 1) * (size + 1);

			indices.emplace_back(row1 + i);
			indices.emplace_back(row1 + i + 1);
			indices.emplace_back(row1 + i + 1);
			indices.emplace_back(row2 + i + 1);
			indices.emplace_back(row2 + i + 1);
			indices.emplace_back(row2 + i);
			indices.emplace_back(row2 + i);
			indices.emplace_back(row1 + i);
		}
	}

	return new Mesh(vertices, indices, DrawMode::Lines);
}

Mesh* Mesh::Cube()
{
	if (CubeInstance)
		return CubeInstance->GetMeshes()[0];

	/*
	vector<Vertex> vertices = {
		// back face
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } }, // bottom-left
		{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } }, // top-right
		{ {  1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f } }, // bottom-right
		{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } }, // top-right
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } }, // bottom-left
		{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f } }, // top-left
		// front face
		{ { -1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,   1.0f }, { 0.0f, 0.0f } }, // bottom-left
		{ {  1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,   1.0f }, { 1.0f, 0.0f } }, // bottom-right
		{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,   1.0f }, { 1.0f, 1.0f } }, // top-right
		{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,   1.0f }, { 1.0f, 1.0f } }, // top-right
		{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,   1.0f }, { 0.0f, 1.0f } }, // top-left
		{ { -1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,   1.0f }, { 0.0f, 0.0f } }, // bottom-left
		// left face
		{ { -1.0f,  1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // top-right
		{ { -1.0f,  1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } }, // top-left
		{ { -1.0f, -1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // bottom-left
		{ { -1.0f, -1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // bottom-left
		{ { -1.0f, -1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } }, // bottom-right
		{ { -1.0f,  1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // top-right
		// right face
		{ {  1.0f,  1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // top-left
		{ {  1.0f, -1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // bottom-right
		{ {  1.0f,  1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } }, // top-right
		{ {  1.0f, -1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // bottom-right
		{ {  1.0f,  1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // top-left
		{ {  1.0f, -1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } }, // bottom-left
		// bottom face
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } }, // top-right
		{ {  1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } }, // top-left
		{ {  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } }, // bottom-left
		{ {  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } }, // bottom-left
		{ { -1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } }, // bottom-right
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } }, // top-right
		// top face
		{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } }, // top-left
		{ {  1.0f,  1.0f , 1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } }, // bottom-right
		{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f } }, // top-right
		{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } }, // bottom-right
		{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } }, // top-left
		{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } }// bottom-left
	};

	CubeInstance = new Mesh(vertices, vector<unsigned int>());
	*/
	CubeInstance = new Model(Application::AssetDir + CubePath);
	Log::Assert(CubeInstance->GetMeshes().size() > 0, "Failed to load cube primitive from '" + Application::AssetDir + CubePath + "'");
	return CubeInstance->GetMeshes()[0];
}

Mesh* Mesh::Sphere()
{
	if (SphereInstance)
		return SphereInstance->GetMeshes()[0];

	/*
	vector<Vertex> vertices;
	vector<unsigned int> indices;

	const unsigned int segmentsX = 16;
	const unsigned int segmentsY = 16;
	const float PI = 3.14159265359f;
	for (unsigned int x = 0; x <= segmentsX; x++)
	{
		for (unsigned int y = 0; y <= segmentsY; y++)
		{
			float xSegment = (float)x / (float)segmentsX;
			float ySegment = (float)y / (float)segmentsY;
			float xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
			float yPos = cos(ySegment * PI);
			float zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);

			vertices.emplace_back(Vertex
				{
					{ xPos, yPos, zPos },  // Position
					{ xPos, yPos, zPos },  // Normal
					{ xSegment, ySegment } // UVs
				});
		}
	}

	bool oddRow = false;
	for (unsigned int y = 0; y < segmentsY; y++)
	{
		if (!oddRow)
		{
			for (unsigned int x = 0; x <= segmentsX; x++)
			{
				indices.emplace_back(y * (segmentsX + 1) + x);
				indices.emplace_back((y + 1) * (segmentsX + 1) + x);
			}
		}
		else
		{
			for (int x = segmentsX; x >= 0; x--)
			{
				indices.emplace_back((y + 1) * (segmentsX + 1) + x);
				indices.emplace_back(y * (segmentsX + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	// ComputeTangents(vertices, indices);
	SphereInstance = new Mesh(vertices, indices);
	*/
	SphereInstance = new Model(Application::AssetDir + SpherePath);
	Log::Assert(SphereInstance->GetMeshes().size() > 0, "Failed to load sphere primitive from '" + Application::AssetDir + SpherePath + "'");
	return SphereInstance->GetMeshes()[0];
}