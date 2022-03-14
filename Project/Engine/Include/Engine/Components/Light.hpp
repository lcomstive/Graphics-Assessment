#pragma once
#include <glm/glm.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Components/Component.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

namespace Engine::Graphics { class Shader; } // Forward declaration

namespace Engine::Components
{
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
		/// Point lights		- effective radius		<para/> <!-- para/ effectively new line -->
		/// Spot lights			- outter cutoff			<para/>
		/// Directional lights	- no effect				<para/>
		/// </summary>
		float Radius = 10.0f;

		/// <summary>
		/// Point lights		- No effect							<para/>
		/// Spot lights			- Reach, from light origin			<para/>
		/// Directional lights	- Size of orthographic projection	<para />
		/// </summary>
		float Distance = 10.0f;

		float Intensity = 1.0f;

		/// <summary>
		/// Angle, in degrees
		/// </summary>
		float FadeCutoffInner = 12.5f;

		ENGINE_API bool GetCastShadows();
		ENGINE_API void SetCastShadows(bool canCast);
		ENGINE_API void FillShader(unsigned int lightIndex, Engine::Graphics::Shader* shader);

	private:
		bool m_CastShadows = false;
	};
}