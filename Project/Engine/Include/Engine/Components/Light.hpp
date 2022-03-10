#pragma once
#include <glm/glm.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

namespace Engine::Components
{
	/// <summary>
	/// Point light
	/// </summary>
	struct Light : public Component
	{
		/// <summary>
		/// RGB ranging 0.0-1.0
		/// </summary>
		glm::vec3 Colour = { 1.0f, 1.0f, 1.0f };

		float Radius = 10.0f;
		float Intensity = 1.0f;

		Graphics::RenderTexture* ShadowMap;
	};

	/// <summary>
	/// Directional light. Direction is transform's rotation
	/// </summary>
	struct DirectionalLight : public Component
	{
		/// <summary>
		/// RGB ranging 0.0-1.0
		/// </summary>
		glm::vec3 Colour = { 1.0f, 1.0f, 1.0f };

		float Intensity = 1.0f;

		Graphics::RenderTexture* ShadowMap;
	};

	/// <summary>
	/// Spot light. Direction is transform's rotation
	/// </summary>
	struct SpotLight : public Component
	{
		/// <summary>
		/// RGB ranging 0.0-1.0
		/// </summary>
		glm::vec3 Colour = { 1.0f, 1.0f, 1.0f };

		float Radius = 1.0f;
		float Intensity = 1.0f;
		float Distance = 10.0f;

		Graphics::RenderTexture* ShadowMap;
	};
}