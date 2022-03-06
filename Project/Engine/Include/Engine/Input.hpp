#pragma once
#include <map>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace Engine
{
	class Input
	{
		enum class KeyState
		{
			/// <summary>
			/// Key is not pressed at all
			/// </summary>
			Up,

			/// <summary>
			/// Key is held down, pressed or released this frame
			/// </summary>
			Down,

			/// <summary>
			/// Key was pressed down this frame
			/// </summary>
			Pressed,

			/// <summary>
			/// Key is released this frame
			/// </summary>
			Released
		};

		static float s_ScrollDelta;
		static glm::vec2 s_MousePos, s_LastMousePos;
		static GLFWwindow* s_MainWindow;
		static std::map<int, KeyState> s_KeyStates;
		static std::map<int, KeyState> s_MouseStates;

		static void Update();

		friend class Application;
	public:
#pragma region Keys
		/// <returns>True if the key is not pressed down</returns>
		static bool IsKeyUp(int key);

		/// <returns>True if the key is currently pressed down</returns>
		static bool IsKeyDown(int key);

		/// <returns>True if the key is was pressed down this frame</returns>
		static bool IsKeyPressed(int key);

		/// <returns>True if the key is was released this frame</returns>
		static bool IsKeyReleased(int key);
#pragma endregion

#pragma region Mouse
		/// <returns>Mouse position relative to the top-left of the window</returns>
		static glm::vec2 GetMousePosition();

		/// <returns>Pixels moved since last frame</returns>
		static glm::vec2 GetMouseDelta();

		/// <returns>Scroll offset since last frame</returns>
		static float GetScrollDelta();

		static bool IsMouseUp(int button);
		static bool IsMouseDown(int button);
		static bool IsMousePressed(int button);
		static bool IsMouseReleased(int button);

		/// <summary>
		/// Can hide or show the mouse. When hidden mouse is locked and cannot exit window.
		/// </summary>
		static void ShowMouse(bool show = true);
#pragma endregion
	};
}