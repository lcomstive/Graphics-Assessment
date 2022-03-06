#include <Engine/Log.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/quaternion.hpp>
#include <Engine/Graphics/Model.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Texture.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>

using namespace glm;
using namespace std;
using namespace Assimp;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

Model::Model() : m_Path(""), m_Root(), m_Meshes() { }
Model::Model(string path) : m_Path(path), m_Root(), m_Meshes() { Load(); }

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
std::vector<Mesh*>& Model::GetMeshes() { return m_Meshes; }

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
}

void LoadMaterialTextures(
	string currentDirectory,
	aiTextureType textureType,
	aiMaterial* aiMat,
	Texture** materialTexture)
{
	int textureCount = aiMat->GetTextureCount(textureType);

	if (aiMat->GetTextureCount(textureType) <= 0)
	{
		materialTexture = nullptr;
		return;
	}

	aiString aiTexturePath;
	aiMat->GetTexture(textureType, 0, &aiTexturePath);

	string texturePath = currentDirectory + aiTexturePath.C_Str();
	replace(texturePath.begin(), texturePath.end(), '\\', '/');

	*materialTexture = new Texture(texturePath);
}

Material Model::CreateMaterial(aiMaterial* aiMat)
{
	filesystem::path path(m_Path);
	string currentDirectory = path.parent_path().string() + "/"; // Get directory of model path

	Material material;

	// Albedo / Base Colour
	LoadMaterialTextures(currentDirectory, aiTextureType_DIFFUSE, aiMat, &material.AlbedoMap);
	if (material.AlbedoMap == nullptr)
		LoadMaterialTextures(currentDirectory, aiTextureType_BASE_COLOR, aiMat, &material.AlbedoMap);

	LoadMaterialTextures(currentDirectory, aiTextureType_NORMALS, aiMat, &material.NormalMap);
	LoadMaterialTextures(currentDirectory, aiTextureType_METALNESS, aiMat, &material.MetalnessMap);
	LoadMaterialTextures(currentDirectory, aiTextureType_DIFFUSE_ROUGHNESS, aiMat, &material.RoughnessMap);
	LoadMaterialTextures(currentDirectory, aiTextureType_AMBIENT_OCCLUSION, aiMat, &material.AmbientOcclusionMap);

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

Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
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

	return new Mesh(vertices, indices);
}