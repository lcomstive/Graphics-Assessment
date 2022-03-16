#pragma once
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>
#include <Engine/Graphics/EnvironmentMap.hpp>

namespace Engine::Graphics
{
	class Skybox
	{ 
		Shader* m_Shader;
		ResourceID m_ShaderID;

	public:
		/// <summary>
		/// Multiplier for ambient lighting applied from
		/// the environment map
		/// </summary>
		float AmbientLightStrength;

		EnvironmentMap* EnvironmentMap;

		ENGINE_API Skybox();
		ENGINE_API ~Skybox();

		ENGINE_API void Draw();
		ENGINE_API void FillShaderData(Shader* shader);
	};
}