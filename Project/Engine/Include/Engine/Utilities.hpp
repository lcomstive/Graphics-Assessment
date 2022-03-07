#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
	std::string ReadText(std::string path);
	std::vector<unsigned char> Read(std::string path);

	void WriteText(std::string path, std::string contents);
	void Write(std::string path, std::vector<unsigned char>);

	float Magnitude(glm::vec3 vector);
	float MagnitudeSqr(glm::vec3 vector);
	glm::vec3 RotationFromDirection(glm::vec3 direction);

	/// <summary>
	/// Yeeeaaahhh it's close enough
	/// </summary>
	bool BasicallyZero(float f);

	float Random(float min, float max);
}