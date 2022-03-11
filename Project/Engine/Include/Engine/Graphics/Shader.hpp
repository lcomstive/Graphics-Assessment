#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/Types.hpp>
#include <Engine/FileWatcher.hpp>

namespace Engine::Graphics
{
	struct ENGINE_API ShaderStageInfo
	{
		std::string VertexPath;
		std::string FragmentPath;
		std::string ComputePath;
		std::string GeometryPath;
		std::string TessellationControl;
		std::string TessellationEvaluate;

		bool Empty()
		{
			return VertexPath.empty() &&
				FragmentPath.empty() &&
				ComputePath.empty() &&
				GeometryPath.empty() &&
				TessellationControl.empty() &&
				TessellationEvaluate.empty();
		}
	};

	struct ENGINE_API ShaderUniform
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
		std::vector<ShaderUniform> m_Uniforms = { {} };
		EngineUnorderedMap<std::string, FileWatcher*> m_Watchers;

#if USE_STRING_ID
		EngineUnorderedMap<StringId::Storage, ShaderUniform*> m_NamedUniforms;
#else
		EngineUnorderedMap<std::string, ShaderUniform*> m_NamedUniforms;
#endif

		void Destroy();
		void CreateShaders();
		void CacheUniformLocations();
		void WatchShader(std::string path, bool watch = true);
		unsigned int CreateShader(const std::string & source, const unsigned int type);
		void ShaderSourceChangedCallback(std::string path, FileWatchStatus changeType);

	public:
		ENGINE_API Shader();
		ENGINE_API Shader(ShaderStageInfo stageInfo);
		ENGINE_API ~Shader();

		ENGINE_API ShaderStageInfo& GetStages();
		ENGINE_API void UpdateStages(ShaderStageInfo stageInfo);

		ENGINE_API void Bind();
		ENGINE_API void Unbind();

		ENGINE_API unsigned int GetProgram();
		ENGINE_API unsigned int GetUniformCount();

		ENGINE_API void Set(int& location, int value) const;
		ENGINE_API void Set(int& location, bool value) const;
		ENGINE_API void Set(int& location, float value) const;
		ENGINE_API void Set(int& location, double value) const;
		ENGINE_API void Set(int& location, glm::vec2 value) const;
		ENGINE_API void Set(int& location, glm::vec3 value) const;
		ENGINE_API void Set(int& location, glm::vec4 value) const;
		ENGINE_API void Set(int& location, glm::mat3 value) const;
		ENGINE_API void Set(int& location, glm::mat4 value) const;

		ENGINE_API void Set(std::string locationName, int value);
		ENGINE_API void Set(std::string locationName, bool value);
		ENGINE_API void Set(std::string locationName, float value);
		ENGINE_API void Set(std::string locationName, double value);
		ENGINE_API void Set(std::string locationName, glm::vec2 value);
		ENGINE_API void Set(std::string locationName, glm::vec3 value);
		ENGINE_API void Set(std::string locationName, glm::vec4 value);
		ENGINE_API void Set(std::string locationName, glm::mat3 value);
		ENGINE_API void Set(std::string locationName, glm::mat4 value);

		/// <returns>Information about the uniform at location, or an invalid struct if outside of bounds</returns>
		ENGINE_API ShaderUniform* GetUniformInfo(int location);

		/// <returns>Information about the uniform at locationName, or an invalid struct if not found</returns>
		ENGINE_API ShaderUniform* GetUniformInfo(std::string& locationName);
	};
}
