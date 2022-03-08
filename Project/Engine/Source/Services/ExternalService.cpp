#include <iostream>
#include <filesystem>
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

namespace fs = std::filesystem;

#define CALL_INSTANCE_FN(fnName) \
		void ExternalServices::fnName() { \
			for(auto& pair : m_Instances) \
				pair.second.Instance->fnName(); \
		}
#define CALL_INSTANCE_FN_PARAM(fnName, paramType, paramName) \
		void ExternalServices::fnName(paramType paramName) { \
			for(auto& pair : m_Instances) \
				pair.second.Instance->fnName(paramName); \
		}

CALL_INSTANCE_FN(OnDraw)
CALL_INSTANCE_FN(OnStart)
CALL_INSTANCE_FN(OnDrawGizmos)
CALL_INSTANCE_FN_PARAM(OnResized, glm::ivec2, resolution)

void ExternalServices::OnShutdown() { RemoveAll(); }

void ExternalServices::OnUpdate(float deltaTime)
{
#ifndef NDEBUG
	// Files that have been removed in the filewatcher thread
	while (!m_FileModifications.empty())
	{
		auto [ path, status ] = m_FileModifications.front();
		switch (status)
		{
		case FileWatchStatus::Modified:
			Reload(path);
			break;
		case FileWatchStatus::Removed:
			Remove(path);
			break;
		}
		m_FileModifications.pop();
	}
#endif

	for (auto& pair : m_Instances)
		pair.second.Instance->OnUpdate(deltaTime);
}

typedef Service*(*CreateServiceFn)(Application*); // Dynamic library entrypoint

Service* ExternalServices::Add(string path)
{
	Service* service = Get(path);
	if (service)
		return service;

	if (!fs::exists(path))
	{
		Log::Error("Failed to load service '" + path + "' - file not found");
		return nullptr;
	}

	ServiceInstance instance;
	instance.Path = path;

	Log::Debug("Loading external service '" + path + "'");

#ifndef NDEBUG
	instance.Path += ".locked";
	instance.OriginalPath = path;

	fs::copy_file(path, instance.Path, fs::copy_options::overwrite_existing); // Copy file to lock & load
#endif

	instance.Instance = Load(instance);

#ifndef NDEBUG
	instance.FileWatcher = new FileWatcher(path);
	instance.FileWatcher->Start([=](string path, FileWatchStatus status) { m_FileModifications.emplace(make_pair(path, status)); });
#endif

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

	Log::Debug("Removing external service '" + path + "'");

#ifndef NDEBUG
	it->second.FileWatcher->Stop();
	delete it->second.FileWatcher;
#endif

	it->second.Instance->OnShutdown();
	delete it->second.Instance;

	Unload(it->second);

#ifndef NDEBUG
	if (fs::exists(it->second.Path))
		fs::remove(it->second.Path); // Delete temporary, locked file
#endif

	m_Instances.erase(it);
}

Service* ExternalServices::Reload(string path)
{
	auto& it = m_Instances.find(path);
	if (it == m_Instances.end())
		return Add(path);

	Log::Debug("Reloading external service '" + path + "'");
	
	it->second.Instance->OnShutdown();
	delete it->second.Instance;
	Unload(it->second);

#ifndef NDEBUG
	fs::copy(it->second.OriginalPath, it->second.Path, fs::copy_options::overwrite_existing);
#endif
	
	it->second.Instance = Load(it->second);
	if(it->second.Instance)
		it->second.Instance->OnStart();
	return it->second.Instance;
}

Service* ExternalServices::Load(ServiceInstance& instance)
{
#if _WIN32
	// Load .dll
	instance.NativeHandle = LoadLibrary(wstring(instance.Path.begin(), instance.Path.end()).c_str());
	if (!instance.NativeHandle)
	{
		Log::Error("Failed to load service '" + instance.Path + "' - failed to load file");
		return nullptr;
	}

	// Get pointer to function
	CreateServiceFn pfnCreateService = (CreateServiceFn)GetProcAddress(instance.NativeHandle, "CreateExternalService");
	if (!pfnCreateService)
	{
		Log::Error("Failed to load service '" + instance.Path + "' - entrypoint not found");
		FreeLibrary(instance.NativeHandle);
		return nullptr;
	}

	// Call pointer, load service
	instance.Instance = (*pfnCreateService)(Application::Get());

	if (!instance.Instance)
	{
		Log::Error("Failed to load service '" + instance.Path + "' - error during service creation");
		FreeLibrary(instance.NativeHandle);
		return nullptr;
	}
#else
	// TODO: Unix support
#endif

	return instance.Instance;
}

void ExternalServices::Unload(ServiceInstance& instance)
{
#if _WIN32
	FreeLibrary(instance.NativeHandle);
#else
	// TODO: Unix support
#endif
}

void ExternalServices::RemoveAll()
{
	while(!m_Instances.empty())
		Remove(m_Instances.cbegin()->first);
}