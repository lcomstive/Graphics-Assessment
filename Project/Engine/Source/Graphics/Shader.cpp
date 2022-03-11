#include <regex>
#include <vector>
#include <string>
#include <filesystem>
#include <glad/glad.h>
#include <Engine/Log.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Engine/Utilities.hpp>
#include <Engine/FileWatcher.hpp>
#include <Engine/Application.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Renderer.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

const unsigned int MaxIncludeRecursion = 5;

void ReplaceAll(string& text, string search, string replacement)
{
	// regex expression for pattern to be searched
	regex regexp(search);
	smatch match;

	// regex_search that searches pattern regexp in the string mystr
	while (regex_search(text, match, regexp))
		text = text.replace(
			match.prefix().length(),
			match.length(),
			replacement
		);
}

// Replace all instances of '#include "<path>"' with contents of file at path
void ProcessIncludeFiles(string& text, unsigned int recursions)
{
	if (recursions <= 0)
		return;

	ReplaceAll(text, "#ASSET_DIR", Application::AssetDir);

	// regex expression for pattern to be searched
	regex regexp("\\#include\\s+\\\"(.*)\\\"");
	smatch match;

	// regex_search that searches pattern regexp in the string mystr
	while (regex_search(text, match, regexp))
	{
		// match.str(0) is the entire captured string
		// match.str(1) is first capture group, in this case filename

		string includeText = ReadText(match.str(1));
		ProcessIncludeFiles(includeText, recursions - 1);

		text = text.replace(
			match.prefix().length(),
			match.length(),
			includeText
		);
	}
}

// Create a shader from source code
unsigned int Shader::CreateShader(const string& source, const unsigned int type)
{
	if (source.empty())
		return GL_INVALID_VALUE;

	// Create shader object
	const GLuint shader = glCreateShader(type);

	// Add source text & compile
	string readSource = ReadText(source);
	if (readSource.empty())
		return GL_INVALID_VALUE;

	ProcessIncludeFiles(readSource, MaxIncludeRecursion);
	ReplaceAll(readSource, "#SUPPORTS_TESSELLATION", Renderer::SupportsTessellation() ? "1" : "0");

	const char* sourceRaw = readSource.c_str();
	glShaderSource(shader, 1, &sourceRaw, nullptr);
	glCompileShader(shader);

	// Check success status
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		// Get and print error message
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
		glDeleteShader(shader);

		Log::Error("Failed to compile '" + source + "' - " + infoLog.data());
		return GL_INVALID_VALUE;
	}
	return shader;
}

Shader::Shader() : m_ShaderStages({}), m_IsDirty(false), m_Program(GL_INVALID_VALUE) { }
Shader::Shader(ShaderStageInfo stageInfo) : m_Program(GL_INVALID_VALUE), m_ShaderStages(stageInfo), m_IsDirty(true) { }

Shader::~Shader()
{
	WatchShader(m_ShaderStages.VertexPath, false);
	WatchShader(m_ShaderStages.FragmentPath, false);
	WatchShader(m_ShaderStages.GeometryPath, false);
	WatchShader(m_ShaderStages.ComputePath, false);

	Destroy();

	for (auto& pair : m_Watchers)
		delete pair.second;
}

void Shader::Destroy()
{
	if (m_Program != GL_INVALID_VALUE)
		glDeleteProgram(m_Program);
	m_Program = GL_INVALID_VALUE;
}

ShaderStageInfo& Shader::GetStages() { return m_ShaderStages; }

void Shader::WatchShader(string path, bool watch)
{
#ifndef NDEBUG
	if (path.empty())
		return;

	const auto& it = m_Watchers.find(path);
	if (watch && it == m_Watchers.end())
	{
		function<void(string, FileWatchStatus)> callback = std::bind(&Shader::ShaderSourceChangedCallback, this, std::placeholders::_1, std::placeholders::_2);

		m_Watchers.emplace(path, new FileWatcher(path, true));
		m_Watchers[path]->Start(callback);
	}
	else if (!watch && it != m_Watchers.end())
	{
		it->second->Stop();
		m_Watchers.erase(path);
	}
#endif
}

void Shader::UpdateStages(ShaderStageInfo stageInfo)
{
	// Check for same stages, in that case won't need to recreate shader program
	if (stageInfo.VertexPath.compare(m_ShaderStages.VertexPath) == 0 &&
		stageInfo.FragmentPath.compare(m_ShaderStages.FragmentPath) == 0 &&
		stageInfo.GeometryPath.compare(m_ShaderStages.GeometryPath) == 0 &&
		stageInfo.ComputePath.compare(m_ShaderStages.ComputePath) == 0 &&
		stageInfo.TessellationControl.compare(m_ShaderStages.TessellationControl) == 0 &&
		stageInfo.TessellationEvaluate.compare(m_ShaderStages.TessellationEvaluate) == 0)
		return;

	// Unwatch shaders
	WatchShader(m_ShaderStages.VertexPath, false);
	WatchShader(m_ShaderStages.FragmentPath, false);
	WatchShader(m_ShaderStages.GeometryPath, false);
	WatchShader(m_ShaderStages.ComputePath, false);
	WatchShader(m_ShaderStages.TessellationControl, false);
	WatchShader(m_ShaderStages.TessellationEvaluate, false);

	// Update & create new shaders
	m_IsDirty = true;
	m_ShaderStages = stageInfo;
	CreateShaders();
}

void Shader::ShaderSourceChangedCallback(std::string path, FileWatchStatus changeType)
{
	Log::Debug("Shader file(s) changed, reloading..");
	m_IsDirty = true;
}

void Shader::CreateShaders()
{
	if (!m_IsDirty)
		return;

	m_IsDirty = false;
	unsigned int programID = 0;

	vector<GLuint> shaders =
	{
		CreateShader(m_ShaderStages.VertexPath, GL_VERTEX_SHADER),
		CreateShader(m_ShaderStages.FragmentPath, GL_FRAGMENT_SHADER),
		CreateShader(m_ShaderStages.ComputePath, GL_COMPUTE_SHADER),
		CreateShader(m_ShaderStages.GeometryPath, GL_GEOMETRY_SHADER),
		CreateShader(m_ShaderStages.TessellationControl, GL_TESS_CONTROL_SHADER),
		CreateShader(m_ShaderStages.TessellationEvaluate, GL_TESS_EVALUATION_SHADER)
	};

	// Debug info
	Log::Debug("Creating shader with following sources:");
	if (!m_ShaderStages.VertexPath.empty())   Log::Debug("  Vertex:   " + m_ShaderStages.VertexPath);
	if (!m_ShaderStages.FragmentPath.empty()) Log::Debug("  Fragment: " + m_ShaderStages.FragmentPath);
	if (!m_ShaderStages.ComputePath.empty())  Log::Debug("  Compute:  " + m_ShaderStages.ComputePath);
	if (!m_ShaderStages.GeometryPath.empty()) Log::Debug("  Geometry: " + m_ShaderStages.GeometryPath);
	if (!m_ShaderStages.GeometryPath.empty()) Log::Debug("  Tess Control: " + m_ShaderStages.TessellationControl);
	if (!m_ShaderStages.GeometryPath.empty()) Log::Debug("  Tess Eval: " + m_ShaderStages.TessellationEvaluate);

	WatchShader(m_ShaderStages.VertexPath);
	WatchShader(m_ShaderStages.FragmentPath);
	WatchShader(m_ShaderStages.GeometryPath);
	WatchShader(m_ShaderStages.ComputePath);
	WatchShader(m_ShaderStages.TessellationControl);
	WatchShader(m_ShaderStages.TessellationEvaluate);

	const bool invalidShaders = shaders.size() == 0 || shaders[0] == GL_INVALID_VALUE || shaders[1] == GL_INVALID_VALUE;
	programID = invalidShaders ? GL_INVALID_VALUE : glCreateProgram();

	for (GLuint shader : shaders)
	{
		if (shader == GL_INVALID_VALUE)
			continue;
		if (invalidShaders)
			glDeleteShader(shader);
		else
			glAttachShader(programID, shader);
	}

	if (invalidShaders)
		return;

	// Finalise shader program
	glLinkProgram(programID);

	// Check success status
	int success = 0;
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		// Get and print error message
		int maxLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

		vector<char> infoLog(maxLength);
		glGetProgramInfoLog(programID, maxLength, &maxLength, &infoLog[0]);

		// Clean up resources
		glDeleteProgram(programID);
		for (GLuint shader : shaders)
			if (shader != GL_INVALID_VALUE)
				glDeleteShader(shader);

		Log::Error("Failed to link shader program - " + string(infoLog.data()));
		return;
	}

	// The shaders aren't needed once they've been linked into the program
	for (GLuint shader : shaders)
		if (shader != GL_INVALID_VALUE)
			glDeleteShader(shader);

	// Compilation successful, destroy old program 
	Destroy();
	m_Program = programID;

	CacheUniformLocations();

	Log::Debug("Created shader program successfully [" + to_string(m_Program) + "]");
}

void Shader::CacheUniformLocations()
{
	int uniformCount = 0;
	glGetProgramiv(m_Program, GL_ACTIVE_UNIFORMS, &uniformCount);

	m_Uniforms.clear();
	m_Uniforms.resize(uniformCount);

	m_NamedUniforms.clear();
	m_NamedUniforms.reserve(uniformCount);

	for (GLuint i = 0; i < (GLuint)uniformCount; i++)
	{
		GLint nameLength;
		glGetActiveUniformsiv(m_Program, 1, &i, GL_UNIFORM_NAME_LENGTH, &nameLength);

		m_Uniforms[i].Name.resize(nameLength - 1);

		GLint size;
		glGetActiveUniform(m_Program, i, nameLength, &nameLength, &size, &m_Uniforms[i].Type, m_Uniforms[i].Name.data());

#if USE_STRING_ID
		m_NamedUniforms.emplace(make_pair(StringId(m_Uniforms[i].Name.c_str()).GetValue(), &m_Uniforms[i]));
#else
		m_NamedUniforms.emplace(make_pair(m_Uniforms[i].Name, &m_Uniforms[i]));
#endif

		m_Uniforms[i].Location = glGetUniformLocation(m_Program, m_Uniforms[i].Name.c_str());

		// Log::Debug(" [" + to_string(m_Uniforms[i].Location) + "] " + m_Uniforms[i].Name);
	}

	Log::Debug("Cached " + to_string(uniformCount) + " shader uniforms");
}

void Shader::Bind()
{
	if (m_IsDirty)
		UpdateStages(m_ShaderStages);

	if (m_Program != GL_INVALID_VALUE)
		glUseProgram(m_Program);
}

void Shader::Unbind()
{
	glUseProgram(0);

	if (m_IsDirty)
		CreateShaders();
}

unsigned int Shader::GetProgram() { return m_Program; }
unsigned int Shader::GetUniformCount() { return (unsigned int)m_Uniforms.size(); }

void Shader::Set(int& location, int value)	  const { if (m_Program != GL_INVALID_VALUE) glProgramUniform1i(m_Program, location, value); }
void Shader::Set(int& location, bool value)   const { if (m_Program != GL_INVALID_VALUE) glProgramUniform1i(m_Program, location, value); }
void Shader::Set(int& location, float value)  const { if (m_Program != GL_INVALID_VALUE) glProgramUniform1f(m_Program, location, value); }
void Shader::Set(int& location, double value) const { Set(location, (float)value); }
void Shader::Set(int& location, vec2 value)   const { if (m_Program != GL_INVALID_VALUE) glProgramUniform2f(m_Program, location, value.x, value.y); }
void Shader::Set(int& location, vec3 value)   const { if (m_Program != GL_INVALID_VALUE) glProgramUniform3f(m_Program, location, value.x, value.y, value.z); }
void Shader::Set(int& location, vec4 value)   const { if (m_Program != GL_INVALID_VALUE) glProgramUniform4f(m_Program, location, value.x, value.y, value.z, value.w); }
void Shader::Set(int& location, mat3 value)   const { if (m_Program != GL_INVALID_VALUE) glProgramUniformMatrix3fv(m_Program, location, 1, GL_FALSE, value_ptr(value)); }
void Shader::Set(int& location, mat4 value)   const { if (m_Program != GL_INVALID_VALUE) glProgramUniformMatrix4fv(m_Program, location, 1, GL_FALSE, value_ptr(value)); }

void Shader::Set(string locationName, int value)	{ ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, bool value)	{ ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, float value)  { ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, double value) { ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, vec2 value)	{ ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, vec3 value)	{ ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, vec4 value)	{ ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, mat3 value)	{ ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }
void Shader::Set(string locationName, mat4 value)	{ ShaderUniform* uniform = GetUniformInfo(locationName); if(uniform) Set(uniform->Location, value); }

ShaderUniform* Shader::GetUniformInfo(int location)
{
	location++; // First cached uniform is an invalid one
	return (location >= 0 && location < m_Uniforms.size() - 1 ? &m_Uniforms[location] : nullptr);
}

ShaderUniform* Shader::GetUniformInfo(std::string& locationName)
{
	auto& it = m_NamedUniforms.find(
#if USE_STRING_ID
		StringId(locationName.c_str()).GetValue()
#else
		locationName
#endif
	);
	return it != m_NamedUniforms.end() ? it->second : nullptr;
}
