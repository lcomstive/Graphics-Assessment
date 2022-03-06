#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <Engine/Application.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Texture.hpp>

namespace Engine::Graphics
{
	struct Material
	{
		/// <summary>
		/// This color is multiplied with other colors, such as the albedoMap or vertex colors.
		/// The alpha channel is used to adjust the opacity, with 1 meaning fully opaque and 0 meaning fully transparent.
		/// 
		/// Default is white.
		/// </summary>
		glm::vec4 Albedo = { 1.0f, 1.0f, 1.0f, 1.0f };

		/// <summary>
		/// Per-pixel albedo values
		/// </summary>
		Texture* AlbedoMap = nullptr;

		/// <summary>
		/// When enabled, albedo alpha's under AlphaClipThreshold are discarded.
		/// 
		/// Default is disabled.
		/// </summary>
		bool AlphaClipping = false;

		/// <summary>
		/// When AlphaClipping is enabled, alpha values below this threshold are discarded.
		/// 
		/// Default is 0.1.
		/// </summary>
		float AlphaClipThreshold = 0.1f;

		/// <summary>
		/// Scales UV texture coordinates.
		/// 
		/// Default is (1.0, 1.0).
		/// </summary>
		glm::vec2 TextureCoordinateScale = { 1.0f, 1.0f };

		/// <summary>
		/// Shifts UV texture coordinates.
		/// 
		/// Default is (0.0, 0.0).
		/// </summary>
		glm::vec2 TextureCoordinateOffset = { 0.0f, 0.0f };

		/// <summary>
		/// Overall roughness of surface.
		/// 
		/// Range [0.0 - 1.0].
		/// Default is 0.5.
		/// </summary>
		float Roughness = 0.5f;
		
		/// <summary>
		/// Overall metalness of surface.
		/// 
		/// Range [0.0 - 1.0].
		/// Default is 0.5.
		/// </summary>
		float Metalness = 0.5f;

		/// <summary>
		/// Simulates extra details on a mesh
		/// </summary>
		Texture* NormalMap = nullptr;

		/// <summary>
		/// Simulates metal on a mesh
		/// </summary>
		Texture* MetalnessMap = nullptr;

		/// <summary>
		/// Simulates roughness on a mesh
		/// </summary>
		Texture* RoughnessMap = nullptr;

		/// <summary>
		/// Simulates extra shadows on a mesh
		/// </summary>
		Texture* AmbientOcclusionMap = nullptr;

		bool Wireframe = false;

		void FillShader(Shader* shader) const;
	};
}
