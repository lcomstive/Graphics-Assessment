#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <stb_image.h>
#include <Engine/Log.hpp>
#include <Engine/Graphics/Texture.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

Texture::Texture() : m_Path(""), m_ID(GL_INVALID_VALUE), m_Args() { }
Texture::Texture(string path, TextureArgs args) : m_Path(path), m_ID(GL_INVALID_VALUE), m_Args(args) { }

Texture::~Texture()
{
	if (m_ID != GL_INVALID_VALUE)
		glDeleteTextures(1, &m_ID);
}

void Texture::GenerateImage()
{
	if (m_Path.empty())
	{
		Log::Warning("Cannot generate texture with empty path");
		return;
	}

	// Read image data
	int width, height, channelCount;
	void* data = nullptr;
	
	stbi_set_flip_vertically_on_load(m_Args.FlipVertically);
	// Load image data from file
	if(!m_Args.HDR)
		data = stbi_load(m_Path.c_str(), &width, &height, &channelCount, 0);
	else
		data = stbi_loadf(m_Path.c_str(), &width, &height, &channelCount, 0);

	// Check for validity
	if (!data)
	{
		// Release resources, these are now stored inside OpenGL's texture buffer
		stbi_image_free(data);

		Log::Warning("Failed to load '" + m_Path + "' - " + stbi_failure_reason());
		return;
	}

	// Generate texture ID
	glGenTextures(1, &m_ID);
	// Set as 2D Texture
	glBindTexture(GL_TEXTURE_2D, m_ID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_Args.Wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_Args.Wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Args.MinFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_Args.MagFilter);

	// Get texture format based on channels
	GLenum internalFormat = GL_INVALID_ENUM, textureFormat = GL_INVALID_ENUM;
	switch (channelCount)
	{
	case 1:
		textureFormat = GL_RED;
		internalFormat = m_Args.HDR ? GL_R16F : GL_RED;
		break;
	case 3:
		textureFormat = GL_RGB;
		internalFormat = m_Args.HDR ? GL_RGB16F : GL_RGB;
		break;
	case 4:
		textureFormat = GL_RGBA;
		internalFormat = m_Args.HDR ? GL_RGBA16F : GL_RGBA;
		break;
	}

	// Fill OpenGL texture data with binary data
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, textureFormat, m_Args.HDR ? GL_FLOAT : GL_UNSIGNED_BYTE, data);

	// Release resources, these are now stored inside OpenGL's texture buffer
	stbi_image_free(data);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

string Texture::GetPath() { return m_Path; }
unsigned int Texture::GetID() { return m_ID; }

void Texture::Bind(unsigned int index)
{
	if (m_ID == GL_INVALID_VALUE)
	{
		if (!m_Path.empty())
			GenerateImage();
		if(m_ID == GL_INVALID_VALUE)
			return; // Double check
	}
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}
