#pragma once
#include <string>
#include <chrono>
#include <glm/glm.hpp>		// Maths library
#include <glad/glad.h>		// Modern OpenGL loader
#include <GLFW/glfw3.h>		// Window & input handling
#include <Engine/Log.hpp>	// Console logging
#include <Engine/Api.hpp>	// For API exporting
#include <Engine/Input.hpp> // Handles keyboard & mouse input
#include <Engine/Scene.hpp> // Holds game objects & state
#include <imgui.h>			// Immediate Mode UI Library

#include <Engine/Services/Service.hpp>

using namespace std::chrono_literals; // seconds in literal

namespace Engine::Graphics
{
	class Mesh;
	class Gizmos;
	class Renderer;
}

namespace Engine
{
	class ResourceManager; // Forward declaration

	struct ENGINE_API ApplicationArgs
	{
		/// <summary>
		/// Vertical sync
		/// </summary>
		bool VSync = true;

		/// <summary>
		/// Anti-aliasing samples.
		/// Range is 0-16
		/// </summary>
		unsigned int Samples = 1;

		/// <summary>
		/// Window title
		/// </summary>
		std::string Title = "Application";

		/// <summary>
		/// Window resolution
		/// </summary>
		glm::ivec2 Resolution = { 1280, 720 };

		/// <summary>
		/// Physics fixed timestep
		/// </summary>
		std::chrono::milliseconds FixedTimestep = 50ms;

		/// <summary>
		/// Take up the entire primary monitor
		/// </summary>
		bool Fullscreen = false;
	};

	enum class ENGINE_API ApplicationState { Starting, Running, Stopping };
	class Application
	{
		Input* m_Input;
		ApplicationState m_State;
		Graphics::Gizmos* m_Gizmos;
		void* m_ImGUIContext = nullptr;
		GLADloadproc m_GladProc = nullptr;
		Graphics::Renderer* m_Renderer;
		ImGuiContext* m_ImGuiContext = nullptr;
		ResourceManager* m_ResourceManager = nullptr;

		std::unordered_map<std::type_index, Services::Service*> m_Services = {};

	public:
		/// <summary>
		/// Local directory containing assets intended for use with the application.
		/// Always has a leading slash.
		/// </summary>
		ENGINE_API const static std::string AssetDir;

		ENGINE_API Application(ApplicationArgs args = {});

		/// <summary>
		/// Launches application and begins rendering loop on the current thread.
		/// </summary>
		ENGINE_API void Run();

		/// <summary>
		/// Call from separate threads/DLLs to update global values
		/// </summary>
		ENGINE_API void UpdateGlobals();

		/// <summary>
		/// Informs the application to close and release resources
		/// </summary>
		ENGINE_API static void Exit();

		/// <summary>
		/// Renames the application window title
		/// </summary>
		ENGINE_API static void SetTitle(std::string title);

		/// <returns>True if currently fullscreen mode</returns>
		ENGINE_API static bool GetFullscreen();

		/// <summary>
		/// If fullscreen, make not fullscreen... and vice versa
		/// </summary>
		ENGINE_API static void ToggleFullscreen();

		/// <summary>
		/// Sets the application window fullscreen or windowed
		/// </summary>
		ENGINE_EXPORT static void SetFullscreen(bool fullscreen) { s_Instance->_SetFullscreen(fullscreen); }

		ENGINE_API static void ShowMouse(bool show);

		ENGINE_API static ApplicationState GetState();

		template<typename T>
		ENGINE_EXPORT static T* AddService() { return s_Instance->_AddService<T>(); }

		template<typename T>
		ENGINE_EXPORT static T* GetService() { return s_Instance->_GetService<T>(); }

		template<typename T>
		ENGINE_EXPORT static void RemoveService() { return s_Instance->_RemoveService<T>(); }

		ENGINE_EXPORT static Application* Get() { return s_Instance; }

	protected:
		ENGINE_API virtual void InitServices();

	private:
		GLFWwindow* m_Window;
		ApplicationArgs m_Args;

		// Cache windowed resolution, for when coming out of fullscreen mode
		glm::ivec2 m_WindowedResolution;

		ENGINE_API static Application* s_Instance;

		void SetupGizmos();
		void CreateAppWindow();

		// Functions starting with _ are to be declared statically and accessed through s_Instance

		template<typename T>
		ENGINE_EXPORT T* _GetService()
		{
			const auto& it = m_Services.find(typeid(T));
			return (T*)(it == m_Services.end() ? nullptr : it->second);
		}

		template<typename T>
		ENGINE_EXPORT void _RemoveService()
		{
			const auto& it = m_Services.find(typeid(T));
			if (it == m_Services.end())
				return;

			it->second->OnShutdown();
			delete it->second;
			m_Services.erase(it);
		}

		template<typename T>
		ENGINE_EXPORT T* _AddService()
		{
			Log::Assert(std::is_base_of<Services::Service, T>(), "Added services need to derive from Engine::Service");

			const std::type_index& type = typeid(T);
			if (m_Services.find(type) != m_Services.end())
				return (T*)m_Services[type];

			T* service = new T();
			m_Services.emplace(type, (Services::Service*)service);
			if (m_State == ApplicationState::Running)
				((Services::Service*)service)->OnStart();
			return service;
		}

		ENGINE_API void _SetFullscreen(bool fullscreen);

#pragma region GLFW Callbacks
		static void GLFW_WindowCloseCallback(GLFWwindow* window);
		static void GLFW_ErrorCallback(int error, const char* description);
		static void GLFW_ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
		static void GLFW_MouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void GLFW_CursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
		static void GLFW_FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void GLFW_DebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* msg, const void* userParam);
#pragma endregion
	};
}
