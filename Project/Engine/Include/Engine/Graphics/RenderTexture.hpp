#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <Engine/Api.hpp>

namespace Engine::Graphics
{
	enum class ENGINE_API TextureFormat
	{
		None = 0,

		// Colour
		RGB8,
		RGBA8,
		RGBA16,
		RGBA16F,
		RedInteger,

		Cubemap,

		// Depth & Stencil
		Depth24Stencil8,

		// Aliases
		Depth = Depth24Stencil8
	};

	enum class ENGINE_API TexturePixelType
	{
		Byte			= GL_BYTE,
		Short			= GL_SHORT,
		UnsignedShort	= GL_UNSIGNED_SHORT,
		UnsignedByte	= GL_UNSIGNED_BYTE,
		Float			= GL_FLOAT,
		Int				= GL_INT,
		UnsignedInt		= GL_UNSIGNED_INT
	};

	ENGINE_API bool IsDepthFormat(TextureFormat format);
	ENGINE_API unsigned int GetTextureTarget(TextureFormat format, bool multisampled = false);
	ENGINE_API unsigned int TextureFormatToGLFormat(TextureFormat format);
	ENGINE_API unsigned int TextureFormatToInternalGLFormat(TextureFormat format);

	class RenderTexture
	{
		glm::ivec2 m_Resolution;
		TexturePixelType m_PixelType;
		unsigned int m_ID, m_Samples = 1;
		TextureFormat m_Format = TextureFormat::RGBA8;

		void Recreate();

		void CreateDepthTexture();
		void CreateColourTexture();

	public:
		ENGINE_API RenderTexture(
			glm::ivec2 resolution,
			TextureFormat format = TextureFormat::RGBA8,
			unsigned int samples = 1,
			TexturePixelType pixelType = TexturePixelType::UnsignedByte
			);
		ENGINE_API ~RenderTexture();

		ENGINE_API unsigned int GetID();

		ENGINE_API void Bind(unsigned int textureIndex = 0);
		ENGINE_API void Unbind();

		ENGINE_API void CopyTo(RenderTexture* destination);

		ENGINE_API unsigned int GetSamples();
		ENGINE_API void SetSamples(unsigned int samples);

		ENGINE_API TextureFormat GetFormat();
		ENGINE_API void SetFormat(TextureFormat format);

		ENGINE_API glm::ivec2 GetResolution();
		ENGINE_API void SetResolution(glm::ivec2 newResolution);
	};
}