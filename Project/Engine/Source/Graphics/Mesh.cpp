#include <glad/glad.h>
#include <Engine/Application.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/ResourceManager.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

Mesh::Mesh() : m_Vertices(), m_Setup(true), m_Indices(), m_VAO(GL_INVALID_VALUE), m_VBO(), m_EBO(), m_DrawMode(DrawMode::Triangles) { }

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, DrawMode drawMode) : Mesh()
{
	m_Setup = false;
	m_DrawMode = drawMode;
	m_Vertices = vertices;
	m_Indices = indices;
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

	m_Setup = true;
}

void Mesh::Draw()
{
	if (!m_Setup)
		Setup();

	glBindVertexArray(m_VAO);
	if (m_Indices.size() > 0)
		glDrawElements((GLenum)m_DrawMode, (GLsizei)m_Indices.size(), GL_UNSIGNED_INT, 0);
	else
		glDrawArrays((GLenum)m_DrawMode, 0, (GLint)m_Vertices.size());
	glBindVertexArray(0);
}

ResourceID& Mesh::Quad()
{
	static ResourceID quadMeshID = InvalidResourceID;
	if (quadMeshID != InvalidResourceID)
		return quadMeshID;

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

	const string PrimitiveQuadName = "PrimitiveQuadName";
	if (ResourceManager::IsValid(PrimitiveQuadName))
		quadMeshID = *ResourceManager::Get<ResourceID>(PrimitiveQuadName);
	else
	{
		quadMeshID = ResourceManager::Load<Mesh>(vertices, indices);
		ResourceManager::LoadNamed<ResourceID>(PrimitiveQuadName, quadMeshID);
	}
	return quadMeshID;
}

ResourceID& Mesh::Line()
{
	static ResourceID lineMeshID = InvalidResourceID;
	if (lineMeshID != InvalidResourceID)
		return lineMeshID;

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

	const string PrimitiveLineName = "PrimitiveLineID";
	if (ResourceManager::IsValid(PrimitiveLineName))
		lineMeshID = *ResourceManager::Get<ResourceID>(PrimitiveLineName);
	else
	{
		lineMeshID = ResourceManager::Load<Mesh>(vertices, indices, DrawMode::Lines);
		ResourceManager::LoadNamed<ResourceID>(PrimitiveLineName, lineMeshID);
	}
	return lineMeshID;
}

ResourceID Mesh::Grid(unsigned int size)
{
	static ResourceID gridMeshID = InvalidResourceID;
	if (gridMeshID != InvalidResourceID)
		return gridMeshID;

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

	return (gridMeshID = ResourceManager::Load<Mesh>(vertices, indices, DrawMode::Lines));
}

ResourceID& Mesh::Cube()
{
	static ResourceID cubeMeshID = InvalidResourceID;
	if (cubeMeshID != InvalidResourceID)
		return cubeMeshID;

	ResourceID modelID = ResourceManager::LoadNamed<Model>("PrimitiveCube", Application::AssetDir + "Models/Primitives/Cube.fbx");
	Model* model = ResourceManager::Get<Model>(modelID);
	return (cubeMeshID = model->GetMeshes()[0]);
}

ResourceID& Mesh::Sphere()
{
	static ResourceID sphereMeshID = InvalidResourceID;
	if (sphereMeshID != InvalidResourceID)
		return sphereMeshID;

	ResourceID modelID = ResourceManager::LoadNamed<Model>("PrimitiveSphere", Application::AssetDir + "Models/Primitives/Sphere.fbx");
	Model* model = ResourceManager::Get<Model>(modelID);
	return (sphereMeshID = model->GetMeshes()[0]);
}