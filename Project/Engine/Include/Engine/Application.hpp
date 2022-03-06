#pragma once
#include <string>
#include <chrono>
#include <glm/glm.hpp>		// Maths library
#include <glad/glad.h>		// Modern OpenGL loader
#include <GLFW/glfw3.h>		// Window & input handling
#include <Engine/Log.hpp>	// Console logging
#include <Engine/Input.hpp> // Handles keyboard & mouse input
#include <Engine/Scene.hpp> // Holds game objects & state

using namespace std::chrono_literals; // seconds in literal

namespace Engine
{
	struct ApplicationArgs
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

	class Application
	{
		Scene m_Scene;

		friend class Engine::Physics::PhysicsSystem;

	protected:
		/// <summary>
		/// When a frame is ready to be drawn
		/// </summary>
		virtual void OnDraw() { }

		/// <summary>
		/// When gizmos are ready to be drawn
		/// </summary>
		virtual void OnDrawGizmos() { }

		/// <summary>
		/// Called once before each draw call
		/// </summary>
		/// <param name="deltaTime">Time, in milliseconds, since last frame</param>
		virtual void OnUpdate() { }

		/// <summary>
		/// When the class has been initialised.
		/// Typical usage is pre-loading assets.
		/// </summary>
		virtual void OnStart() { }

		/// <summary>
		/// Application is being closed
		/// </summary>
		virtual void OnShutdown() { }

		/// <summary>
		/// Called when the window has been resized
		/// </summary>
		/// <param name="resolution">New width & height in pixels</param>
		virtual void OnResized(glm::ivec2 resolution) { }

	public:
		/// <summary>
		/// Local directory containing assets intended for use with the application.
		/// Always has a leading slash.
		/// </summary>
		const static std::string AssetDir;

		Application(ApplicationArgs args = {});

		/// <summary>
		/// Launches application and begins rendering loop on the current thread.
		/// </summary>
		void Run();

		/// <summary>
		/// Informs the application to close and release resources
		/// </summary>
		void Exit();

		/// <summary>
		/// The active scene
		/// </summary>
		Scene* CurrentScene();

		/// <summary>
		/// Renames the application window title
		/// </summary>
		void SetTitle(std::string title);

		/// <returns>True if currently fullscreen mode</returns>
		bool GetFullscreen();

		/// <summary>
		/// If fullscreen, make not fullscreen... and vice versa
		/// </summary>
		void ToggleFullscreen();

		/// <summary>
		/// Sets the application window fullscreen or windowed
		/// </summary>
		void SetFullscreen(bool fullscreen);
	private:
		GLFWwindow* m_Window;
		ApplicationArgs m_Args;

		// Cache windowed resolution, for when coming out of fullscreen mode
		glm::ivec2 m_WindowedResolution;

		static Application* s_Instance;

		void SetupGizmos();
		void CreateAppWindow();

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

#define APPLICATION_MAIN(type)	\
	int main() {				\
		type().Run();			\
		return 0;				\
	}
