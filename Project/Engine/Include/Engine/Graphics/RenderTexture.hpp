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
		Depth16,
		Depth24,
		RenderBuffer, // Creates renderbuffer object with Depth24_Stencil8 format

		// Aliases
		Depth = RenderBuffer
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

	struct RenderTextureDepth
	{
		/// <summary>
		/// When <= 1 generated texture is 2D.
		/// If larger than 1 texture is 3D or 2D Array.
		/// </summary>
		unsigned int Depth = 1;

		/// <summary>
		/// If true generates 3D texture.
		/// When false, generates 2D texture array.
		/// </summary>
		bool Is3D = false;

		/// <summary>
		/// If true creates 2D texture array or 3D texture, even if depth <= 1
		/// </summary>
		bool Force3D = false;
	};

	ENGINE_API unsigned int GetTextureTarget(TextureFormat format, bool multisampled = false, RenderTextureDepth depth = {});
	ENGINE_API unsigned int TextureFormatToGLFormat(TextureFormat format);
	ENGINE_API unsigned int TextureFormatToInternalGLFormat(TextureFormat format);

	struct RenderTextureArgs
	{
		unsigned int Samples = 1;
		glm::ivec2 Resolution = glm::ivec2(1, 1);
		TextureFormat Format = TextureFormat::RGBA8;
		TexturePixelType PixelType = TexturePixelType::UnsignedByte;
		RenderTextureDepth Depth;
		GLenum Wrap = GL_REPEAT;
		GLenum MinFilter = GL_LINEAR;
		GLenum MagFilter = GL_LINEAR;
	};

	class RenderTexture
	{
		bool m_Dirty;
		unsigned int m_ID;
		RenderTextureArgs m_Args;

		void Recreate();

		void CreateRenderbuffer();
		void CreateColourTexture();

	public:
		ENGINE_API RenderTexture(const RenderTextureArgs args);
		ENGINE_API ~RenderTexture();

		/// <summary>
		/// Flag recreation of texture based on RenderTextureArgs (see RenderTexture::GetArgs())
		/// </summary>
		/// <returns></returns>
		ENGINE_API void SetDirty();

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

		ENGINE_API RenderTextureArgs& GetArgs();
	};
}