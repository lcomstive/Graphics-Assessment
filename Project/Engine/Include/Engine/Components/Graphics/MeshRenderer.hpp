#pragma once
#include <vector>
#include <Engine/Api.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Material.hpp>
#include <Engine/Components/Component.hpp>

namespace Engine::Components
{
	struct MeshRenderer : public Component
	{
		struct ENGINE_API MeshInfo
		{
			ResourceID Mesh = InvalidResourceID;
			Graphics::Material Material = {};
		};

		std::vector<MeshInfo> Meshes;

	protected:
		ENGINE_API void Draw() override;
	};
}