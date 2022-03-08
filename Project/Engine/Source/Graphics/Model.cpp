#include <filesystem>
#include <Engine/Log.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Engine/Utilities.hpp>
#include <Engine/DataStream.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Texture.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>

using namespace glm;
using namespace std;
using namespace Assimp;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

namespace fs = std::filesystem;

#define ENABLE_CACHING 1

Model::Model() : m_Path(""), m_Root(), m_Meshes() { }
Model::Model(string path) : m_Path(path), m_Root(), m_Meshes() { Load(); }

Model::~Model()
{
	for (ResourceID meshID : m_Meshes)
		ResourceManager::Unload(meshID);
	m_Meshes.clear();
}

void ApplyAssimpTransformation(aiMatrix4x4 transformation, Transform* transform)
{
	aiVector3D scale = {};
	aiQuaternion rotation = {};
	aiVector3D position = {};
	transformation.Decompose(scale, rotation, position);

	transform->Scale = { scale.x, scale.y, scale.z };
	transform->Position = { position.x, position.y, position.z };
	transform->Rotation = eulerAngles(quat{ rotation.w, rotation.x, rotation.y, rotation.z });
}

Model::MeshData& Model::GetRootMeshData() { return m_Root; }
std::vector<ResourceID>& Model::GetMeshes() { return m_Meshes; }

GameObject* Model::CreateEntity(GameObject* parent, vec3 position)
{
	if (!parent || (m_Root.MeshIDs.size() == 0 && m_Root.Children.size() == 0))
		return nullptr; // Return invalid entity

	GameObject* root = new GameObject(parent, m_Root.Name);
	root->GetTransform()->Position = position;

	CreateEntity(m_Root, root);

	return root;
}

void Model::CreateEntity(MeshData& mesh, GameObject* parent)
{
	// This mesh is empty, skip it
	if (mesh.MeshIDs.size() == 0)
	{
		for (MeshData& childMesh : mesh.Children)
			CreateEntity(childMesh, parent);
		return;
	}

	// Create entity
	GameObject* entity = new GameObject(parent, mesh.Name);

	// Apply transform properties from mesh
	ApplyAssimpTransformation(mesh.Transformation, entity->GetTransform());

	// Create renderers for all meshes
	MeshRenderer* renderer = entity->AddComponent<MeshRenderer>();
	for (unsigned int i = 0; i < (unsigned int)mesh.MeshIDs.size(); i++)
		renderer->Meshes.emplace_back(MeshRenderer::MeshInfo
		{
			m_Meshes[mesh.MeshIDs[i]],
			mesh.Materials[i]
		});

	// Add children
	for (MeshData& childMesh : mesh.Children)
		CreateEntity(childMesh, entity);
}

void Model::Load()
{
#if ENABLE_CACHING
	if (fs::exists(m_Path + ".cache"))
	{
		LoadFromCache();
		return;
	}
#endif
	Log::Info("Loading model " + m_Path);

	Importer importer;

	const aiScene* scene = nullptr;
	unsigned int postProcessing = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;

	// Read in model file
	scene = importer.ReadFile(m_Path, postProcessing);

	// Safety check
	if (!scene)
	{
		Log::Error("Failed to load model '" + m_Path + "' - " + string(importer.GetErrorString()));
		return;
	}

	// Read in meshes
	m_Meshes.reserve(scene->mNumMeshes);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		m_Meshes.emplace_back(ProcessMesh(scene->mMeshes[i], scene));

	m_Root = {};

	// Create mesh data hierarchy
	ProcessNode(scene->mRootNode, nullptr, scene);
#if ENABLE_CACHING
	SaveCache();
#endif
}

void LoadMaterialTextures(
	string currentDirectory,
	aiTextureType textureType,
	aiMaterial* aiMat,
	ResourceID& materialTexture) // TODO: CHECK THIS REFERENCE
{
	int textureCount = aiMat->GetTextureCount(textureType);

	if (aiMat->GetTextureCount(textureType) <= 0)
	{
		materialTexture = InvalidResourceID;
		return;
	}

	aiString aiTexturePath;
	aiMat->GetTexture(textureType, 0, &aiTexturePath);

	string texturePath = currentDirectory + aiTexturePath.C_Str();
	replace(texturePath.begin(), texturePath.end(), '\\', '/');

	materialTexture = ResourceManager::LoadNamed<Texture>(aiTexturePath.C_Str(), texturePath);
}


Material Model::CreateMaterial(aiMaterial* aiMat)
{
	filesystem::path path(m_Path);
	string currentDirectory = path.parent_path().string() + "/"; // Get directory of model path

	Material material;

	// Albedo / Base Colour
	LoadMaterialTextures(currentDirectory, aiTextureType_DIFFUSE, aiMat, material.AlbedoMap);
	if (material.AlbedoMap == InvalidResourceID)
		LoadMaterialTextures(currentDirectory, aiTextureType_BASE_COLOR, aiMat, material.AlbedoMap);

	LoadMaterialTextures(currentDirectory, aiTextureType_NORMALS, aiMat, material.NormalMap);
	LoadMaterialTextures(currentDirectory, aiTextureType_METALNESS, aiMat, material.MetalnessMap);
	LoadMaterialTextures(currentDirectory, aiTextureType_DIFFUSE_ROUGHNESS, aiMat, material.RoughnessMap);
	LoadMaterialTextures(currentDirectory, aiTextureType_AMBIENT_OCCLUSION, aiMat, material.AmbientOcclusionMap);

	return material;
}

void Model::ProcessNode(aiNode* node, MeshData* parent, const aiScene* scene)
{
	MeshData meshData = { node->mName.C_Str(), node->mTransformation };

	// Process meshes in node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		meshData.MeshIDs.emplace_back(node->mMeshes[i]);

		aiMaterial* material = scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex];
		meshData.Materials.emplace_back(CreateMaterial(material));
	}

	// Process mesh's children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], &meshData, scene);

	if (parent)
		parent->Children.emplace_back(meshData);
	else
		m_Root = meshData;
}

ResourceID Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<Mesh::Vertex> vertices;
	vector<unsigned int> indices;

	// Process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Mesh::Vertex vertex =
		{
			/* Position    */ { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z },
			/* Normals     */ { mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z  },
			/* TexCoords   */ { 0, 0 },
			/* Tangents    */ { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z },
			/* Bitangents  */ { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z }
 		};

		if (mesh->mTextureCoords[0])
			vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

		vertices.emplace_back(vertex);
	}

	// Process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			indices.emplace_back(mesh->mFaces[i].mIndices[j]);

	return ResourceManager::Load<Mesh>(vertices, indices);
}

void Model::LoadFromCache()
{
	DataStream stream(Engine::Read(m_Path + ".cache"));
	Serialize(stream);
	Log::Debug("Loaded '" + m_Path + "' from cache");
}

void Model::SaveCache()
{
	DataStream stream;
	Serialize(stream);
	stream.SaveTo(m_Path + ".cache");
	Log::Debug("Cached '" + m_Path + "'");
}

void SerializeMeshData(DataStream& stream, Model::MeshData& mesh)
{
	stream.Serialize(&mesh.Name);

	// Transform
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			stream.Serialize(&mesh.Transformation[i][j]);

	// Mesh IDs
	unsigned int meshIDCount = (unsigned int)mesh.MeshIDs.size();
	stream.Serialize(&meshIDCount);
	if (stream.IsReading())
		mesh.MeshIDs.resize(meshIDCount);
	for (unsigned int i = 0; i < meshIDCount; i++)
		stream.Serialize(&mesh.MeshIDs[i]);

	// Material Paths
	unsigned int materialCount = (unsigned int)mesh.Materials.size();
	stream.Serialize(&materialCount);
	if (stream.IsReading())
		mesh.Materials.resize(materialCount);
	for (unsigned int i = 0; i < materialCount; i++)
		mesh.Materials[i].Serialize(stream);

	// Children
	unsigned int childCount = (unsigned int)mesh.Children.size();
	stream.Serialize(&childCount);
	if (stream.IsReading())
		mesh.Children.resize(childCount);
	for (unsigned int i = 0; i < childCount; i++)
		SerializeMeshData(stream, mesh.Children[i]);
}

void Model::Serialize(DataStream& stream)
{
	stream.Serialize(&m_Path);

	unsigned int meshCount = (unsigned int)m_Meshes.size();
	stream.Serialize(&meshCount);
	if (stream.IsReading())
		m_Meshes.resize(meshCount);

	for (unsigned int i = 0; i < meshCount; i++)
	{
		Mesh* mesh = nullptr;
		if (stream.IsWriting())
			mesh = ResourceManager::Get<Mesh>(m_Meshes[i]);

		vector<Mesh::Vertex> vertices = mesh ? mesh->GetVertices() : vector<Mesh::Vertex>();
		vector<unsigned int> indices = mesh ? mesh->GetIndices() : vector<unsigned int>();

		unsigned int vertexCount = (unsigned int)vertices.size();
		unsigned int indexCount = (unsigned int)indices.size();

		stream.Serialize(&vertexCount);
		if (stream.IsReading())
			vertices.resize(vertexCount);
		else
			stream.Reserve(vertexCount * sizeof(Mesh::Vertex));

		for (unsigned int v = 0; v < vertexCount; v++)
		{
			stream.Serialize(&vertices[v].Position);
			stream.Serialize(&vertices[v].Normal);
			stream.Serialize(&vertices[v].TexCoords);
			stream.Serialize(&vertices[v].Tangent);
			stream.Serialize(&vertices[v].Bitangent);
		}

		stream.Serialize(&indexCount);
		if (stream.IsReading())
			indices.resize(indexCount);

		for (unsigned int j = 0; j < indexCount; j++)
			stream.Serialize(&indices[j]);

		if (stream.IsReading())
			m_Meshes[i] = ResourceManager::Load<Mesh>(vertices, indices);
	}

	SerializeMeshData(stream, m_Root);
}