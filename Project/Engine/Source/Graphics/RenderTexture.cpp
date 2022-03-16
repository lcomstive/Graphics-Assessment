#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <Engine/Log.hpp>
#include <Engine/Utilities.hpp>
#include <Engine/DataStream.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include <stb_image.h>
#include <stb_image_write.h>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;

unsigned int GetChannelCount(TextureFormat format)
{
	switch (format)
	{
	default:
	case TextureFormat::None:
		return 0;
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::RedInteger:
		return 1;
	case TextureFormat::RGB8:
	case TextureFormat::Cubemap:
		return 3;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16F:
		return 4;
	}
}

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
	case TextureFormat::RG16F:
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
	case TextureFormat::RedInteger:	return GL_R32I;
	case TextureFormat::RG16F:		return GL_RG16F;
	case TextureFormat::RGBA8:		return GL_RGBA8;
	case TextureFormat::RGBA16:		return GL_RGBA16;
	case TextureFormat::RGBA16F:	return GL_RGBA16F;
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
	case TextureFormat::RG16F:			return GL_RG;
	case TextureFormat::RGB8:			return GL_RGB;
	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16F:		return GL_RGBA;
	case TextureFormat::RedInteger:		return GL_RED_INTEGER;
	case TextureFormat::Depth16:		return GL_DEPTH_COMPONENT;
	case TextureFormat::Depth24:		return GL_DEPTH_COMPONENT24;
	case TextureFormat::RenderBuffer:	return GL_DEPTH24_STENCIL8;
	case TextureFormat::Cubemap:		return GL_RGB;
	}
}

RenderTexture::RenderTexture(RenderTextureArgs args, void* pixelData) : m_ID(GL_INVALID_VALUE), m_Args(args)
{
	if (m_Args.Format == TextureFormat::RenderBuffer)
		CreateRenderbuffer();
	else
		CreateColourTexture(pixelData);
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

	samples = std::clamp(samples, 1u, 32u);
	if ((m_Args.Samples == 1 && samples  > 1) || // Non-Multisampled -> Multisampled
		(m_Args.Samples  > 1 && samples == 1)) // Multisampled -> Non-Multisampled
	{
		glDeleteTextures(1, &m_ID);
		m_ID = GL_INVALID_VALUE;
		Log::Debug("Changed samples from " + to_string(m_Args.Samples) + " to " + to_string(samples));
	}

	m_Args.Samples = samples;
	m_Dirty = true;
}

void RenderTexture::SetFormat(TextureFormat format)
{
	if (format == m_Args.Format)
		return;

	glDeleteTextures(1, &m_ID);
	m_ID = GL_INVALID_VALUE;

	// Create in new format
	m_Args.Format = format;
	m_Dirty = true;
}

void RenderTexture::SetDirty() { m_Dirty = true; }
unsigned int RenderTexture::GetID() { return m_ID; }
ivec2 RenderTexture::GetResolution() { return m_Args.Resolution; }
TextureFormat RenderTexture::GetFormat() { return m_Args.Format; }
unsigned int RenderTexture::GetSamples() { return m_Args.Samples; }

void RenderTexture::Recreate(void* data)
{
	m_Dirty = false;
	if (m_Args.Format == TextureFormat::RenderBuffer)
		CreateRenderbuffer();
	else
		CreateColourTexture(data);
}

void RenderTexture::CreateColourTexture(void* data)
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
	case TextureFormat::RG16F:
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
					data					// Data/Pixels
				);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Args.MinFilter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_Args.MagFilter);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, m_Args.Wrap);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_Args.Wrap);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_Args.Wrap);

				if (m_Args.GenerateMipmaps)
					glGenerateMipmap(GL_TEXTURE_2D);
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
					data					// Data/Pixels
				);

				glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, m_Args.MinFilter);
				glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, m_Args.MagFilter);
				glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(textureTarget, GL_TEXTURE_MAX_LEVEL, 0);
				glTexParameteri(textureTarget, GL_TEXTURE_WRAP_R, m_Args.Wrap);
				glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, m_Args.Wrap);
				glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, m_Args.Wrap);

				if (m_Args.GenerateMipmaps)
					glGenerateMipmap(textureTarget);
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
		unsigned int faceSize = m_Args.Resolution.x * m_Args.Resolution.y * GetChannelCount(m_Args.Format);

		switch (m_Args.PixelType)
		{
		default:
		case TexturePixelType::Int:				faceSize *= sizeof(int);			break;
		case TexturePixelType::Byte:			faceSize *= sizeof(char);			break;
		case TexturePixelType::Short:			faceSize *= sizeof(short);			break;
		case TexturePixelType::Float:			faceSize *= sizeof(float);			break;
		case TexturePixelType::UnsignedInt:		faceSize *= sizeof(unsigned int);	break;
		case TexturePixelType::UnsignedByte:	faceSize *= sizeof(unsigned char);	break;
		case TexturePixelType::UnsignedShort:	faceSize *= sizeof(unsigned short); break;
		}

		for (unsigned int i = 0; i < 6; i++)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, // Mip level
				internalGLFormat,
				m_Args.Resolution.x,
				m_Args.Resolution.y,
				0, // Border
				GLFormat,
				(GLenum)m_Args.PixelType,
				data ? ((char*)data + i * faceSize) : nullptr
			);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, m_Args.Wrap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, m_Args.Wrap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, m_Args.Wrap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_Args.MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_Args.MagFilter);

		if (m_Args.GenerateMipmaps)
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
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
		std::min(m_Args.Resolution.x, destination->m_Args.Resolution.x),
		std::min(m_Args.Resolution.y, destination->m_Args.Resolution.y)
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

void* RenderTexture::GetPixels(size_t* length)
{
	size_t bufferSize = m_Args.Resolution.x * m_Args.Resolution.y;
	bufferSize *= GetChannelCount(m_Args.Format);

	size_t pixelBufferSize = bufferSize;

	if (m_Args.Format == TextureFormat::Cubemap)
		pixelBufferSize *= 6;
	void* pixels = nullptr;

	switch (m_Args.PixelType)
	{
	default:
	case TexturePixelType::Int:				bufferSize *= sizeof(int);				pixels = new int[pixelBufferSize *= sizeof(int)];						break;
	case TexturePixelType::Byte:			bufferSize *= sizeof(char);				pixels = new char[pixelBufferSize *= sizeof(char)];						break;
	case TexturePixelType::Short:			bufferSize *= sizeof(short);			pixels = new short[pixelBufferSize *= sizeof(short)];					break;
	case TexturePixelType::Float:			bufferSize *= sizeof(float);			pixels = new float[pixelBufferSize *= sizeof(float)];					break;
	case TexturePixelType::UnsignedInt:		bufferSize *= sizeof(unsigned int);		pixels = new unsigned int[pixelBufferSize *= sizeof(unsigned int)];		break;
	case TexturePixelType::UnsignedByte:	bufferSize *= sizeof(unsigned char);	pixels = new unsigned char[pixelBufferSize *= sizeof(unsigned char)];	break;
	case TexturePixelType::UnsignedShort:	bufferSize *= sizeof(unsigned short);	pixels = new unsigned short[pixelBufferSize *= sizeof(unsigned short)]; break;
	}

	memset(pixels, 0, pixelBufferSize);

	Bind(0);

	GLenum GLFormat = TextureFormatToGLFormat(m_Args.Format);
	if (m_Args.Format != TextureFormat::Cubemap)
		// Load pixels from image
		glGetTexImage(
			GetTextureTarget(m_Args.Format, m_Args.Samples > 1, m_Args.Depth),
			0, // Mipmap level
			GLFormat,
			(GLenum)m_Args.PixelType,
			pixels
		);
	else
		for (unsigned int i = 0; i < 6; i++)
			// Load pixels from image
			glGetTexImage(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, // Mipmap level
				GLFormat,
				(GLenum)m_Args.PixelType,
				(char*)pixels + i * bufferSize
			);

	if (length)
		*length = pixelBufferSize;
	return pixels;
}

void RenderTexture::SaveHDR(std::string path)
{
	void* pixels = GetPixels();

	int totalHeight = m_Args.Resolution.y;
	if (m_Args.Format == TextureFormat::Cubemap)
		totalHeight *= 6;

	stbi_write_hdr(
		path.c_str(),
		m_Args.Resolution.x,
		totalHeight,
		GetChannelCount(m_Args.Format),
		(const float*)pixels
	);

	delete[] pixels;
}

void RenderTexture::SavePNG(std::string path)
{
	void* pixels = GetPixels();
	unsigned int channels = GetChannelCount(m_Args.Format);

	int totalHeight = m_Args.Resolution.y;
	if (m_Args.Format == TextureFormat::Cubemap)
		totalHeight *= 6;

	stbi_write_png(
		path.c_str(),
		m_Args.Resolution.x,
		totalHeight,
		channels,
		pixels,
		0 // Let stb decide stride
	);

	delete[] pixels;
}

void RenderTexture::SaveJPG(std::string path, unsigned int quality)
{
	void* pixels = GetPixels();
	unsigned int channels = GetChannelCount(m_Args.Format);

	int totalHeight = m_Args.Resolution.y;
	if (m_Args.Format == TextureFormat::Cubemap)
		totalHeight *= 6;

	stbi_write_jpg(
		path.c_str(),
		m_Args.Resolution.x,
		totalHeight,
		channels,
		pixels,
		(int)quality
	);

	delete[] pixels;
}

RenderTexture* RenderTexture::LoadFrom(std::string path, RenderTextureArgs args)
{
	int channels = GetChannelCount(args.Format);

	unsigned char* pixels = stbi_load(path.c_str(), &args.Resolution.x, &args.Resolution.y, &channels, channels);

	if (args.Format == TextureFormat::Cubemap)
		args.Resolution.y /= 6;

	RenderTexture* texture = new RenderTexture(args, pixels);
	stbi_image_free(pixels);
	return texture;
}

RenderTexture* RenderTexture::LoadFromHDR(std::string path, RenderTextureArgs args)
{
	int channels = GetChannelCount(args.Format);

	// stbi_set_flip_vertically_on_load(true);
	float* pixels = stbi_loadf(path.c_str(), &args.Resolution.x, &args.Resolution.y, &channels, channels);

	if (args.Format == TextureFormat::Cubemap)
		args.Resolution.y /= 6;

	RenderTexture* texture = new RenderTexture(args, pixels);
	stbi_image_free(pixels);
	return texture;
}

void RenderTexture::SerializeArgs(RenderTextureArgs& args, DataStream& stream)
{
	stream.Serialize(&args.Samples);
	stream.Serialize(&args.Resolution.x);
	stream.Serialize(&args.Resolution.y);

	int data = (int)args.Format;
	stream.Serialize(&data);
	args.Format = (TextureFormat)data;

	data = (int)args.PixelType;
	stream.Serialize(&data);
	args.PixelType = (TexturePixelType)data;

	stream.Serialize(&args.Depth.Is3D);
	stream.Serialize(&args.Depth.Force3D);
	stream.Serialize(&args.Depth.Depth);

	stream.Serialize(&args.Wrap);
	stream.Serialize(&args.MinFilter);
	stream.Serialize(&args.MagFilter);
	stream.Serialize(&args.GenerateMipmaps);
}