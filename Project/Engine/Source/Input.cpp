#include <Engine/Input.hpp>

using namespace glm;
using namespace std;
using namespace Engine;

float Input::s_ScrollDelta = 0;
vec2 Input::s_MousePos = { 0, 0 };
vec2 Input::s_LastMousePos = { 0, 0 };
GLFWwindow* Input::s_MainWindow = nullptr;
map<int, Input::KeyState> Input::s_KeyStates;
map<int, Input::KeyState> Input::s_MouseStates;

#pragma region Keys
bool Input::IsKeyUp(int key)
{
	auto it = s_KeyStates.find(key);
	return it != s_KeyStates.end() ? it->second == KeyState::Up : true;
}

bool Input::IsKeyDown(int key)
{
	auto it = s_KeyStates.find(key);
	return it != s_KeyStates.end() ? it->second == KeyState::Down : false;
}

bool Input::IsKeyPressed(int key)
{
	auto it = s_KeyStates.find(key);
	return it != s_KeyStates.end() ? it->second == KeyState::Pressed : false;
}

bool Input::IsKeyReleased(int key)
{
	auto it = s_KeyStates.find(key);
	return it != s_KeyStates.end() ? it->second == KeyState::Released : false;
}
#pragma endregion

#pragma region Mouse Buttons
bool Input::IsMouseUp(int key)
{
	auto it = s_MouseStates.find(key);
	return it != s_MouseStates.end() ? it->second == KeyState::Up : true;
}

bool Input::IsMouseDown(int key)
{
	auto it = s_MouseStates.find(key);
	return it != s_MouseStates.end() ? it->second == KeyState::Down : false;
}

bool Input::IsMousePressed(int key)
{
	auto it = s_MouseStates.find(key);
	return it != s_MouseStates.end() ? it->second == KeyState::Pressed : false;
}

bool Input::IsMouseReleased(int key)
{
	auto it = s_MouseStates.find(key);
	return it != s_MouseStates.end() ? it->second == KeyState::Released : false;
}
#pragma endregion

#pragma region Mouse
vec2 Input::GetMousePosition() { return s_MousePos; }
vec2 Input::GetMouseDelta() { return (s_MousePos - s_LastMousePos); }

float Input::GetScrollDelta() { return s_ScrollDelta; }

void Input::ShowMouse(bool show)
{
	glfwSetInputMode(s_MainWindow, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
#pragma endregion

void Input::Update()
{
	// Loop through all keystates and check their current state
	for (auto& pair : s_KeyStates)
	{
		switch (pair.second)
		{
		// Keys currently pressed are now classed as down/held
		case KeyState::Pressed:
			s_KeyStates[pair.first] = KeyState::Down;
			break;

		// Keys that have been released are now class as up
		case KeyState::Released:
			s_KeyStates[pair.first] = KeyState::Up;
			break;
		default: break;
		}
	}
	
	// Loop through all mousestates and check their current state
	for (auto& pair : s_MouseStates)
	{
		switch (pair.second)
		{
		// Keys currently pressed are now classed as down/held
		case KeyState::Pressed:
			s_MouseStates[pair.first] = KeyState::Down;
			break;

		// Keys that have been released are now class as up
		case KeyState::Released:
			s_MouseStates[pair.first] = KeyState::Up;
			break;
		default: break;
		}
	}

	// Set current mouse position as "last" mouse position
	s_LastMousePos = s_MousePos;

	// Reset scroll delta every frame
	s_ScrollDelta = 0;
}
