#pragma once
#include <string>
#include <Engine/Api.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Framebuffer.hpp>

namespace Engine::Graphics
{
	struct EnvironmentMapArgs
	{
		/// <summary>
		/// Image to be shown on the skybox.
		/// Expect equirectangular map/panorama.
		/// </summary>
		std::string Background;

		/// <summary>
		/// Image shown in reflections, typically HDR for lighting.
		/// If empty, the background image will be used.
		/// </summary>
		std::string Reflection;
	};

	class EnvironmentMap
	{
		glm::ivec2 m_Resolution;
		EnvironmentMapArgs m_Args;
		Framebuffer* m_Framebuffer;
		RenderTexture* m_BRDFTexture;
		RenderTexture* m_CubemapTexture;
		RenderTexture* m_PreFilterTexture;
		RenderTexture* m_IrradianceTexture;
		RenderTexture* m_ReflectionCubemapTexture;
		ResourceID m_Texture, m_ReflectionTexture;
		ResourceID m_BRDFShader,
					m_PreFilterShader,
					m_EquiToCubeShader,
					m_IrradianceShader;

		void Generate();

	public:
		ENGINE_API EnvironmentMap(EnvironmentMapArgs maps, glm::ivec2 resolution = { 1024, 1024 });
		ENGINE_API ~EnvironmentMap();

		ENGINE_API ResourceID GetTexture();
		ENGINE_API RenderTexture* GetBRDF();
		ENGINE_API RenderTexture* GetCubemap();
		ENGINE_API RenderTexture* GetIrradianceMap();
		ENGINE_API RenderTexture* GetReflectionCubemap();
		ENGINE_API RenderTexture* GetPrefilteredCubemap();
	};
}