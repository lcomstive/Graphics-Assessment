#pragma once
#include <string>
#include <glad/glad.h>
#include <Engine/Api.hpp>

namespace Engine::Graphics
{
	struct TextureArgs
	{
		bool HDR = false;
		bool FlipVertically = false;
		GLenum Wrap		 = GL_REPEAT;
		GLenum MinFilter = GL_LINEAR;
		GLenum MagFilter = GL_LINEAR;
	};

	class Texture
	{
		unsigned int m_ID;
		std::string m_Path;
		TextureArgs m_Args;

		void GenerateImage();

	public:
		ENGINE_API Texture();
		ENGINE_API Texture(std::string path, TextureArgs args = {});
		ENGINE_API ~Texture();

		ENGINE_API unsigned int GetID();
		ENGINE_API std::string GetPath();
		
		ENGINE_API void Bind(unsigned int index = 0);
	};
}