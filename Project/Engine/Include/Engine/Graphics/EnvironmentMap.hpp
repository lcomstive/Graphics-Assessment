#pragma once
#include <string>
#include <Engine/Api.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Framebuffer.hpp>

namespace Engine::Graphics
{
	class EnvironmentMap
	{
		ResourceID m_Texture;
		glm::ivec2 m_Resolution;
		Framebuffer* m_Framebuffer;
		ResourceID m_EquiToCubeShader;

		void Generate();

	public:
		ENGINE_API EnvironmentMap(std::string filepath, glm::ivec2 resolution = { 2048, 2048 });
		ENGINE_API ~EnvironmentMap();

		ENGINE_API ResourceID GetTexture();
		ENGINE_API RenderTexture* GetCubemap();
	};
}