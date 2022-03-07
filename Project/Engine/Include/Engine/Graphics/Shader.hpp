#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include <Engine/FileWatcher.hpp>

namespace Engine::Graphics
{
	struct ShaderStageInfo
	{
		std::string VertexPath;
		std::string FragmentPath;
		std::string ComputePath;
		std::string GeometryPath;
	};

	struct ShaderUniform
	{		
		int Location = -1;
		std::string Name;
		unsigned int Type;
	};

	class Shader
	{
		bool m_IsDirty;
		unsigned int m_Program;
		ShaderStageInfo m_ShaderStages;
		std::vector<ShaderUniform> m_Uniforms;
		std::unordered_map<std::string, FileWatcher*> m_Watchers;

		void Destroy();
		void CreateShaders();
		void CacheUniformLocations();
		void WatchShader(std::string path, bool watch = true);
		unsigned int CreateShader(const std::string & source, const unsigned int type);
		void ShaderSourceChangedCallback(std::string path, FileWatchStatus changeType);

	public:
		Shader();
		Shader(ShaderStageInfo stageInfo);
		~Shader();

		ShaderStageInfo& GetStages();
		void UpdateStages(ShaderStageInfo stageInfo);

		void Bind();
		void Unbind();

		unsigned int GetProgram();
		unsigned int GetUniformCount();

		void Set(int& location, int value) const;
		void Set(int& location, bool value) const;
		void Set(int& location, float value) const;
		void Set(int& location, double value) const;
		void Set(int& location, glm::vec2 value) const;
		void Set(int& location, glm::vec3 value) const;
		void Set(int& location, glm::vec4 value) const;
		void Set(int& location, glm::mat3 value) const;
		void Set(int& location, glm::mat4 value) const;

		void Set(std::string locationName, int value);
		void Set(std::string locationName, bool value);
		void Set(std::string locationName, float value);
		void Set(std::string locationName, double value);
		void Set(std::string locationName, glm::vec2 value);
		void Set(std::string locationName, glm::vec3 value);
		void Set(std::string locationName, glm::vec4 value);
		void Set(std::string locationName, glm::mat3 value);
		void Set(std::string locationName, glm::mat4 value);

		/// <returns>Information about the uniform at location, or an invalid struct if outside of bounds</returns>
		ShaderUniform& GetUniformInfo(int location);

		/// <returns>Information about the uniform at locationName, or an invalid struct if not found</returns>
		ShaderUniform& GetUniformInfo(std::string& locationName);
	};
}
