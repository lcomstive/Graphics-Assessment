#include <regex>
#include <fstream>
#include <filesystem>
#include <Engine/Log.hpp>
#include <Engine/Utilities.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace std;
using namespace glm;
using namespace Engine;

namespace fs = std::filesystem;

string Engine::ReadText(std::string path)
{
	if (!fs::exists(path))
	{
		Log::Warning("Cannot read '" + path + "' - file does not exist");
		return "";
	}

	// Open file for read and get size
	ifstream filestream(path, ios::in | ios::binary);

	// Get file length
	streampos size = fs::file_size(path);

	// Read in contents of file
	string contents(size, '\0');
	filestream.read(contents.data(), size);

	// Convert CRLF -> LF
	contents = regex_replace(contents, regex("\r\n"), "\n");

	return contents;
}

vector<unsigned char> Engine::Read(std::string path)
{
	if (!fs::exists(path))
	{
		Log::Warning("Cannot read '" + path + "' - file does not exist");
		return vector<unsigned char>();
	}

	ifstream filestream(path, ios::in | ios::binary);

	// Get file length
	filestream.seekg(0, ios::end);
	size_t filesize = filestream.tellg();

	filestream.seekg(0, ios::beg); // Go back to start

	// Prepare vector
	vector<unsigned char> contents(filesize);

	filestream.read((char*)&contents[0], filesize);
	filestream.close();

	return contents;
}

float Engine::Magnitude(vec3 vector) { return sqrt(MagnitudeSqr(vector)); }
float Engine::MagnitudeSqr(vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

bool Engine::BasicallyZero(float f) { return fabsf(f) < 0.00001f; }

float Engine::Random(float min, float max) { return min + ((float)rand() / (float)RAND_MAX) * (max - min); }

vec3 Engine::RotationFromDirection(vec3 direction)
{
	direction = normalize(direction);
	return eulerAngles(quatLookAt(direction, { 0, 1, 0 }));
}