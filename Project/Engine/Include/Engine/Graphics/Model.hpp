#pragma once
#include <string>
#include <vector>
#include <assimp/scene.h>
#include <Engine/GameObject.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Material.hpp>

namespace Engine::Graphics
{
	class Model
	{
	public:
		struct MeshData
		{
			std::string Name = "";
			aiMatrix4x4 Transformation;
			std::vector<MeshData> Children;
			std::vector<unsigned int> MeshIDs; // Index of Model::m_MeshIDs
			std::vector<Material> Materials;
		};

	private:
		MeshData m_Root;
		std::string m_Path;
		std::vector<Mesh*> m_Meshes;

		Model();

		void Load();
		Material CreateMaterial(aiMaterial* material);
		void CreateEntity(MeshData& mesh, Engine::GameObject* parent);
		Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
		void ProcessNode(aiNode* node, MeshData* parent, const aiScene* scene);

	public:
		Model(std::string path);

		MeshData& GetRootMeshData();
		std::vector<Mesh*>& GetMeshes();
		Engine::GameObject* CreateEntity(Engine::GameObject* parent = nullptr, glm::vec3 position = { 0, 0, 0 });
	};
}