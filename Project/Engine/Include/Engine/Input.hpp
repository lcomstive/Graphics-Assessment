#pragma once
#include <map>
#include <glm/glm.hpp>
#include <glad/glad.h>
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

		float m_ScrollDelta = 0;
		glm::vec2 m_MousePos = glm::vec2(0), m_LastMousePos = glm::vec2(0);
		std::map<int, KeyState> m_KeyStates;
		std::map<int, KeyState> m_MouseStates;

		static Input* s_Instance;

		void Update();

		Input();
		~Input();

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