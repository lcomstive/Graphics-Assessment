#pragma once
#include <string>

namespace Engine::Graphics
{
	class Texture
	{
		unsigned int m_ID;
		std::string m_Path;

		void GenerateImage(bool hdr);

	public:
		Texture();
		Texture(std::string path, bool hdr = false);
		~Texture();

		unsigned int GetID();
		std::string GetPath();
		
		void Bind(unsigned int index = 0);
	};
}