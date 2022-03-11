#pragma once
#include <glm/glm.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

namespace Engine::Graphics { class Shader; } // Forward declaration

namespace Engine::Components
{
	const unsigned int LightMaxViews = 1; // Maximum cameras per light (6 = cubemap)

	/// <summary>
	/// Data calculated by the ShadowMapPass class
	/// </summary>
	struct LightShadowData
	{
		Camera Cameras[LightMaxViews];

		// Camera view * projection matrices. Stored here instead of recalculating multiple times per frame
		glm::mat4 LightSpaceMatrices[LightMaxViews];
		
		/// <summary>
		/// Less than 0 implies that light has not been calculated nor put into the shadow map
		/// </summary>
		int ShadowMapArrayIndex[LightMaxViews];
	};

	enum class ENGINE_API LightType
	{
		Point = 0,
		Spot,
		Directional
	};

	struct Light : public Component
	{
		LightType Type = LightType::Point;

		/// <summary>
		/// RGB ranging 0.0-1.0
		/// </summary>
		glm::vec3 Colour = { 1.0f, 1.0f, 1.0f };

		/// <summary>
		/// Point lights - effective radius		<para/> <!-- para/ effectively new line -->
		/// Spot lights - maximum radius		<para/>
		/// Directional lights - no effect		<para/>
		/// </summary>
		float Radius = 10.0f;

		float Intensity = 1.0f;

		bool CastShadows = true;

		LightShadowData ShadowData;

		void FillShadowData();
		void FillShader(unsigned int lightIndex, Engine::Graphics::Shader* shader);
	};
}