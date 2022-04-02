#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <Engine/Api.hpp>
#include <Engine/DataStream.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Texture.hpp>

namespace Engine::Graphics
{
	struct ENGINE_API Material
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
		ResourceID AlbedoMap = InvalidResourceID;

		/// <summary>
		/// When enabled, albedo alpha's under AlphaClipThreshold are discarded.
		/// 
		/// Default is enabled.
		/// </summary>
		bool AlphaClipping = true;

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
		ResourceID NormalMap = InvalidResourceID;

		/// <summary>
		/// Simulates metal on a mesh
		/// </summary>
		ResourceID MetalnessMap = InvalidResourceID;

		/// <summary>
		/// Simulates roughness on a mesh
		/// </summary>
		ResourceID RoughnessMap = InvalidResourceID;

		/// <summary>
		/// Simulates extra shadows on a mesh
		/// </summary>
		ResourceID AmbientOcclusionMap = InvalidResourceID;

		/// <summary>
		/// Draw attached mesh in wireframe mode
		/// </summary>
		bool Wireframe = false;

		/// <summary>
		/// Attached mesh can cast shadows
		/// </summary>
		bool CanCastShadows = true;

		void FillShader(Shader* shader);
		void Serialize(DataStream& stream);

	private:
		void SerializeTexture(DataStream& stream, ResourceID& texture);
	};
}
