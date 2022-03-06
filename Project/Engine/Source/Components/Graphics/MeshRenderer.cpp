#include <Engine/GameObject.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Components/Graphics/MeshRenderer.hpp>

using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

void MeshRenderer::Draw()
{
	for (auto& meshInfo : Meshes)
		Renderer::Submit(
			meshInfo.Mesh,
			meshInfo.Material,
			GetTransform()
		);
}
