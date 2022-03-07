#pragma once
#include <string>
#include <imgui.h>
#include <unordered_map>
#include <Engine/Api.hpp>
#include <Engine/Services/Service.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
// Unix shared library loading library thingo
#endif

#define EXTERNAL_SERVICE_ENTRYPOINT(className) extern "C" ENGINE_EXPORT Engine::Services::Service* CreateExternalService(Engine::Application* app) { \
		app->UpdateGlobals(); \
		return new className(); }

namespace Engine::Services
{
	struct ExternalServices : Service
	{
		virtual void OnStart() override;
		virtual void OnShutdown() override;

		virtual void OnDraw() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnDrawGizmos() override;

		virtual void OnResized(glm::ivec2 resolution) override;

		Service* Add(std::string path);
		void Remove(std::string path);
		Service* Get(std::string path);
		Service* Reload(std::string path);

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
		};

		std::unordered_map<std::string, ServiceInstance> m_Instances;
	};
}