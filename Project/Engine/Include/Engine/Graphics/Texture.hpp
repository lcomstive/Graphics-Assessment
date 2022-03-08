#pragma once
#include <string>
#include <Engine/Api.hpp>

namespace Engine::Graphics
{
	class Texture
	{
		bool m_HDR;
		unsigned int m_ID;
		std::string m_Path;

		void GenerateImage();

	public:
		ENGINE_API Texture();
		ENGINE_API Texture(std::string path, bool hdr = false);
		ENGINE_API ~Texture();

		ENGINE_API unsigned int GetID();
		ENGINE_API std::string GetPath();
		
		ENGINE_API void Bind(unsigned int index = 0);
	};
}