#pragma once
#include <queue>
#include <string>
#include <imgui.h>
#include <Engine/Api.hpp>
#include <Engine/Types.hpp>
#include <Engine/Application.hpp>
#include <Engine/Services/Service.hpp>

#ifndef NDEBUG
#include <Engine/FileWatcher.hpp>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
// Unix shared library loading library thingo
#endif

#define EXTERNAL_SERVICE_ENTRYPOINT(className) extern "C" SERVICE_API Engine::Services::Service* CreateExternalService(Engine::Application* app) { \
		app->UpdateGlobals(); \
		return new className(); }

namespace Engine::Services
{
	struct ExternalServices : Service
	{
		ENGINE_API virtual void OnStart() override;
		ENGINE_API virtual void OnShutdown() override;

		ENGINE_API virtual void OnDraw() override;
		ENGINE_API virtual void OnUpdate(float deltaTime) override;
		ENGINE_API virtual void OnDrawGizmos() override;

		ENGINE_API virtual void OnResized(glm::ivec2 resolution) override;
		ENGINE_API virtual void OnPipelineChanged(Graphics::RenderPipeline* pipeline) override;

		ENGINE_API Service* Add(std::string path);
		ENGINE_API void Remove(std::string path);
		ENGINE_API Service* Get(std::string path);
		ENGINE_API Service* Reload(std::string path);

		ENGINE_API void RemoveAll();

	private:
		struct ServiceInstance
		{
			std::string Path = "";
			Service* Instance = nullptr;

#if _WIN32
			HMODULE NativeHandle = NULL;
#else
			void* NativeHandle = nullptr;
#endif

#ifndef NDEBUG
			std::string OriginalPath = "";
			FileWatcher* FileWatcher = nullptr;
#endif
		};

		EngineUnorderedMap<std::string, ServiceInstance> m_Instances;

		/// <summary>
		/// Loads shared library from file
		/// </summary>
		/// <param name="instance"></param>
		/// <returns>Newly created instance, or nullptr if failure</returns>
		Service* Load(ServiceInstance& instance);

		/// <summary>
		/// Release resources of shared library
		/// </summary>
		/// <param name="instance"></param>
		void Unload(ServiceInstance& instance);

#ifndef NDEBUG
		std::queue<std::pair<std::string, FileWatchStatus>> m_FileModifications; // Queue of file that have been changed in the filewatcher thread
#endif
	};
}