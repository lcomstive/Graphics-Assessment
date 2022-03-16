#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <Engine/Api.hpp>

namespace Engine
{
	ENGINE_API std::string ReadText(std::string path);
	ENGINE_API std::vector<unsigned char> Read(std::string path);

	ENGINE_API void WriteText(std::string path, std::string& contents);
	ENGINE_API void Write(std::string path, std::vector<unsigned char>&);

	ENGINE_API float Magnitude(glm::vec3 vector);
	ENGINE_API float MagnitudeSqr(glm::vec3 vector);
	ENGINE_API glm::vec3 RotationFromDirection(glm::vec3 direction);

	/// <summary>
	/// Yeeeaaahhh it's close enough
	/// </summary>
	ENGINE_API bool BasicallyZero(float f);

	ENGINE_API float Random(float min, float max);
}