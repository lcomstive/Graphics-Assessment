#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace Engine::Graphics
{
	enum class TextureFormat
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

	enum class TexturePixelType
	{
		Byte			= GL_BYTE,
		Short			= GL_SHORT,
		UnsignedShort	= GL_UNSIGNED_SHORT,
		UnsignedByte	= GL_UNSIGNED_BYTE,
		Float			= GL_FLOAT,
		Int				= GL_INT,
		UnsignedInt		= GL_UNSIGNED_INT
	};

	bool IsDepthFormat(TextureFormat format);
	unsigned int GetTextureTarget(TextureFormat format, bool multisampled = false);
	unsigned int TextureFormatToGLFormat(TextureFormat format);
	unsigned int TextureFormatToInternalGLFormat(TextureFormat format);

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
		RenderTexture(
			glm::ivec2 resolution,
			TextureFormat format = TextureFormat::RGBA8,
			unsigned int samples = 1,
			TexturePixelType pixelType = TexturePixelType::UnsignedByte
			);
		~RenderTexture();

		unsigned int GetID();

		void Bind(unsigned int textureIndex = 0);
		void Unbind();

		void CopyTo(RenderTexture* destination);

		unsigned int GetSamples();
		void SetSamples(unsigned int samples);

		TextureFormat GetFormat();
		void SetFormat(TextureFormat format);

		glm::ivec2 GetResolution();
		void SetResolution(glm::ivec2 newResolution);
	};
}