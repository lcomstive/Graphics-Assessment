#pragma once
#include <vector>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Material.hpp>
#include <Engine/Components/Component.hpp>

namespace Engine::Components
{
	struct MeshRenderer : public Component
	{
		struct MeshInfo
		{
			ResourceID Mesh = InvalidResourceID;
			Graphics::Material Material = {};
		};

		std::vector<MeshInfo> Meshes;

	protected:
		void Draw() override;
	};
}