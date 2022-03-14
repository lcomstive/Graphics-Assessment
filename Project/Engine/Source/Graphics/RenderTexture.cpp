#include <glad/glad.h>
#include <Engine/Log.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;

GLenum Engine::Graphics::GetTextureTarget(TextureFormat target, bool multisampled, RenderTextureDepth depth)
{
	switch (target)
	{
	default:
	case TextureFormat::None:
		return GL_INVALID_ENUM;
	case TextureFormat::RedInteger:
		return GL_TEXTURE_1D;
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16F:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
		if (depth.Depth <= 1 && !depth.Force3D)
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		else if (depth.Is3D)
			return GL_TEXTURE_3D;
		return multisampled ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
	case TextureFormat::Cubemap:
		return GL_TEXTURE_CUBE_MAP;
	}
	Log::Assert(false, "Shouldn't be here?");
}

GLenum Engine::Graphics::TextureFormatToInternalGLFormat(TextureFormat format)
{
	switch (format)
	{
	default: return GL_INVALID_ENUM;
	case TextureFormat::RGB8:		return GL_RGB8;
	case TextureFormat::RGBA8:		return GL_RGBA8;
	case TextureFormat::RGBA16:		return GL_RGBA16;
	case TextureFormat::RGBA16F:	return GL_RGBA16F;
	case TextureFormat::RedInteger:	return GL_R32I;
	case TextureFormat::Cubemap:	return GL_RGB16F;
	case TextureFormat::Depth16:	return GL_DEPTH_COMPONENT16;
	case TextureFormat::Depth24:	return GL_DEPTH_COMPONENT24;
	}
}

GLenum Engine::Graphics::TextureFormatToGLFormat(TextureFormat format)
{
	switch (format)
	{
	default: return GL_INVALID_ENUM;
	case TextureFormat::RGB8:			return GL_RGB;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16F:		return GL_RGBA;
	case TextureFormat::RedInteger:		return GL_RED_INTEGER;
	case TextureFormat::Depth16:		return GL_DEPTH_COMPONENT;
	case TextureFormat::RenderBuffer:	return GL_DEPTH24_STENCIL8;
	case TextureFormat::Cubemap:		return GL_RGB;
	}
}

RenderTexture::RenderTexture(RenderTextureArgs args) : m_ID(GL_INVALID_VALUE), m_Args(args)
{
	if (m_Args.Format == TextureFormat::RenderBuffer)
		CreateRenderbuffer();
	else
		CreateColourTexture();
}

RenderTexture::~RenderTexture()
{
	if (m_ID == GL_INVALID_VALUE)
		return;

	glDeleteTextures(1, &m_ID);
	m_ID = GL_INVALID_VALUE;
}

RenderTextureArgs& RenderTexture::GetArgs() { return m_Args; }

void RenderTexture::SetResolution(ivec2 resolution)
{
	if (m_Args.Resolution == resolution)
		return;

	m_Args.Resolution = resolution;
	Recreate();
}

void RenderTexture::SetSamples(unsigned int samples)
{
	if (samples == m_Args.Samples)
		return;

	m_Args.Samples = samples;
	Recreate();
}

void RenderTexture::SetFormat(TextureFormat format)
{
	if (format == m_Args.Format)
		return;

	glDeleteTextures(1, &m_ID);

	// Create in new format
	m_Args.Format = format;
	Recreate();
}

void RenderTexture::SetDirty() { m_Dirty = true; }
unsigned int RenderTexture::GetID() { return m_ID; }
ivec2 RenderTexture::GetResolution() { return m_Args.Resolution; }
TextureFormat RenderTexture::GetFormat() { return m_Args.Format; }
unsigned int RenderTexture::GetSamples() { return m_Args.Samples; }

void RenderTexture::Recreate()
{
	m_Dirty = false;
	if (m_Args.Format == TextureFormat::RenderBuffer)
		CreateRenderbuffer();
	else
		CreateColourTexture();
}

void RenderTexture::CreateColourTexture()
{
	if (m_ID == GL_INVALID_VALUE)
		glGenTextures(1, &m_ID);
	m_Dirty = false;

	GLenum textureTarget = GetTextureTarget(m_Args.Format, m_Args.Samples > 1, m_Args.Depth);
	GLenum GLFormat = TextureFormatToGLFormat(m_Args.Format);
	GLenum internalGLFormat = TextureFormatToInternalGLFormat(m_Args.Format);
	glBindTexture(textureTarget, m_ID);

	switch (m_Args.Format)
	{
	case TextureFormat::RGB8:
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16F:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
		if (m_Args.Samples <= 1)
		{
			if (m_Args.Depth.Depth <= 1 && !m_Args.Depth.Force3D) // 2D Texture
			{
				glTexImage2D(
					GL_TEXTURE_2D,			// Target
					0,						// Mipmap Level
					internalGLFormat,		// Internal Format
					m_Args.Resolution.x,	// Resolution
					m_Args.Resolution.y,
					0,						// Border
					GLFormat,				// Format
					(GLenum)m_Args.PixelType,// Type
					nullptr					// Data/Pixels
				);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else // 3D Texture or 2D Texture Array
			{
				GLenum textureTarget = m_Args.Depth.Is3D ? GL_TEXTURE_3D : GL_TEXTURE_2D_ARRAY;
				glTexImage3D(
					textureTarget,			// Target
					0,						// Mipmap Level
					internalGLFormat,		// Internal Format
					m_Args.Resolution.x,	// Resolution
					m_Args.Resolution.y,
					m_Args.Depth.Depth,		// depth
					0,						// Border
					GLFormat,				// Format
					(GLenum)m_Args.PixelType, // Type
					nullptr					// Data/Pixels
				);

				glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(textureTarget, GL_TEXTURE_MAX_LEVEL, 0);
				glTexParameteri(textureTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
		else // Multisampled
		{
			if (m_Args.Depth.Depth <= 1 && !m_Args.Depth.Force3D) // 2D
				glTexImage2DMultisample(
					GL_TEXTURE_2D_MULTISAMPLE,
					m_Args.Samples,
					internalGLFormat,
					m_Args.Resolution.x,
					m_Args.Resolution.y,
					GL_TRUE				// Fixed sample locations
				);
			else // 2D Array (3D multisampling isn't a thing)
				glTexImage3DMultisample(
					GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
					m_Args.Samples,
					internalGLFormat,
					m_Args.Resolution.x,
					m_Args.Resolution.y,
					m_Args.Depth.Depth,
					GL_TRUE				// Fixed sample locations
				);
		}
		break;
	case TextureFormat::Cubemap:
		for (unsigned int i = 0; i < 6; i++)
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, // Mip level
				internalGLFormat,
				m_Args.Resolution.x,
				m_Args.Resolution.y,
				0, // Border
				GLFormat,
				GL_FLOAT,
				nullptr);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}

	glBindTexture(textureTarget, 0);
}

void RenderTexture::CreateRenderbuffer()
{
	if (m_ID == GL_INVALID_VALUE)
		glGenRenderbuffers(1, &m_ID);

	glBindRenderbuffer(GL_RENDERBUFFER, m_ID);

	if (m_Args.Samples <= 1)
		glRenderbufferStorage(
			GL_RENDERBUFFER,
			TextureFormatToGLFormat(m_Args.Format),
			m_Args.Resolution.x,
			m_Args.Resolution.y
		);
	else
		glRenderbufferStorageMultisample(
			GL_RENDERBUFFER,
			m_Args.Samples,
			TextureFormatToGLFormat(m_Args.Format),
			m_Args.Resolution.x,
			m_Args.Resolution.y
		);

	// Unbind
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RenderTexture::Bind(unsigned int textureIndex)
{
	if (m_Dirty)
		Recreate();

	if (m_Args.Format != TextureFormat::RenderBuffer)
	{
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		glBindTexture(GetTextureTarget(m_Args.Format, m_Args.Samples > 1, m_Args.Depth), m_ID);
	}
	else
		glBindRenderbuffer(GL_RENDERBUFFER, m_ID);
}

void RenderTexture::Unbind()
{
	if (m_Args.Format != TextureFormat::RenderBuffer)
		glBindTexture(GetTextureTarget(m_Args.Format, m_Args.Samples > 1, m_Args.Depth), 0);
	else
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RenderTexture::CopyTo(RenderTexture* destination)
{
	ivec2 outputResolution =
	{
		min(m_Args.Resolution.x, destination->m_Args.Resolution.x),
		min(m_Args.Resolution.y, destination->m_Args.Resolution.y)
	};

	glCopyImageSubData(
		m_ID,
		GetTextureTarget(m_Args.Format, m_Args.Samples > 1, m_Args.Depth),
		0,	// Mip level
		0, 0, 0,

		destination->m_ID,
		GetTextureTarget(destination->m_Args.Format, destination->m_Args.Samples > 1, m_Args.Depth),
		0,	// Mip level
		0, 0, 0,

		outputResolution.x,
		outputResolution.y,
		1 // Source Depth
	);
}