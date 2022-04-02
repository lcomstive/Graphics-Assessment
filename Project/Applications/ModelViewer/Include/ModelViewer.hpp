#pragma once
#include <Engine/ResourceID.hpp>
#include <Engine/GameObject.hpp>
#include <Engine/Components/Light.hpp>
#include <Engine/Services/ExternalService.hpp>

const std::string DefaultModelPath = "Models/Cerberus/Cerberus_LP.FBX";

namespace MV
{
	class ModelViewer : public Engine::Services::Service
	{
		Engine::Scene* m_Scene = nullptr;

		// Grid
		bool m_DrawGrid = true;
		Engine::ResourceID m_LineID = Engine::InvalidResourceID;
		Engine::ResourceID m_GridMeshID = Engine::InvalidResourceID;

		// Model
		std::string m_NewModelPath;
		std::string m_CurrentModelPath;
		Engine::ResourceID m_ModelID = Engine::InvalidResourceID;

		// GameObjects
		Engine::GameObject* m_GameObject = nullptr;
		Engine::Components::Light* m_Light = nullptr;

		/// <summary>
		/// Loads model at path
		/// </summary>
		void LoadModel(std::string path);

	protected:
		void OnStart() override;
		void OnShutdown() override;

		void OnDraw() override;
		void OnDrawGizmos() override;
		void OnUpdate(float deltaTime) override;
	};
}