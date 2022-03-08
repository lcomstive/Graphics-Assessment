#pragma once
#include <string>
#include <vector>
#include <assimp/scene.h>
#include <Engine/Api.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/DataStream.hpp>
#include <Engine/Graphics/Material.hpp>

namespace Engine::Graphics
{
	class Model
	{
	public:
		struct ENGINE_API MeshData
		{
			std::string Name = "";
			aiMatrix4x4 Transformation;
			std::vector<MeshData> Children;
			std::vector<unsigned int> MeshIDs; // Index of m_Meshes
			std::vector<Material> Materials;
		};

	private:
		MeshData m_Root;
		std::string m_Path;
		std::vector<ResourceID> m_Meshes;

		Model();

		void Load();
		Material CreateMaterial(aiMaterial* material);
		void CreateEntity(MeshData& mesh, Engine::GameObject* parent);
		ResourceID ProcessMesh(aiMesh* mesh, const aiScene* scene);
		void ProcessNode(aiNode* node, MeshData* parent, const aiScene* scene);

		void SaveCache();
		void LoadFromCache();
		void Serialize(DataStream& stream);

	public:
		ENGINE_API Model(std::string path);
		ENGINE_API ~Model();

		ENGINE_API MeshData& GetRootMeshData();
		ENGINE_API std::vector<ResourceID>& GetMeshes();
		ENGINE_API Engine::GameObject* CreateEntity(Engine::GameObject* parent = nullptr, glm::vec3 position = { 0, 0, 0 });
	};
}