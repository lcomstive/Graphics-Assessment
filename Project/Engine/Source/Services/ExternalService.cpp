#include <iostream>
#include <Engine/Application.hpp>
#include <termcolor/termcolor.hpp>
#include <Engine/Services/ExternalService.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windef.h>
#include <libloaderapi.h>
#else
// Unix shared library loading library thingo
#endif

using namespace std;
using namespace Engine;
using namespace Engine::Services;

#define CALL_INSTANCE_FN(fnName, paramType, paramName) \
		void ExternalServices::fnName(paramType paramName) { \
			for(auto& pair : m_Instances) \
				pair.second.Instance->fnName(paramName); \
		}

CALL_INSTANCE_FN(OnDraw)
CALL_INSTANCE_FN(OnStart)
CALL_INSTANCE_FN(OnShutdown)
CALL_INSTANCE_FN(OnDrawGizmos)
CALL_INSTANCE_FN(OnUpdate, float, deltaTime)
CALL_INSTANCE_FN(OnResized, glm::ivec2, resolution)

typedef Service*(*CreateServiceFn)(Application*); // Dynamic library entrypoint

Service* ExternalServices::Add(string path)
{
	Service* service = Get(path);
	if (service)
		return service;

	ServiceInstance instance;
	instance.Path = path;

#if _WIN32
	// Load .dll
	instance.NativeHandle = LoadLibrary(wstring(path.begin(), path.end()).c_str());
	if (!instance.NativeHandle)
	{
		Log::Error("Failed to load service '" + path + "' - failed to load file");
		return nullptr;
	}

	// Get pointer to function
	CreateServiceFn pfnCreateService = (CreateServiceFn)GetProcAddress(instance.NativeHandle, "CreateExternalService");
	if (!pfnCreateService)
	{
		Log::Error("Failed to load service '" + path + "' - entrypoint not found");
		FreeLibrary(instance.NativeHandle);
		return nullptr;
	}

	// Call pointer, load service
	instance.Instance = (*pfnCreateService)(Application::Get());
#else
	// TODO: Unix support
#endif

	if (!instance.Instance)
	{
		Log::Error("Failed to load service '" + path + "' - error during service creation");
		return nullptr;
	}

	if(Application::GetState() == ApplicationState::Running)
		instance.Instance->OnStart();

	m_Instances.emplace(path, instance);
	return instance.Instance;
}

Service* ExternalServices::Get(string path)
{
	auto& it = m_Instances.find(path);
	return it == m_Instances.end() ? nullptr : it->second.Instance;
}

void ExternalServices::Remove(string path)
{
	auto& it = m_Instances.find(path);
	if (it == m_Instances.end())
		return;

	it->second.Instance->OnShutdown();
	delete it->second.Instance;

#if _WIN32
	FreeLibrary(it->second.NativeHandle);
#else

#endif

	m_Instances.erase(it);
}

Service* ExternalServices::Reload(string path)
{
	Remove(path);
	return Add(path);
}