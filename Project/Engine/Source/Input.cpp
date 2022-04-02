#include <Engine/Input.hpp>
#include <Engine/Application.hpp>

using namespace glm;
using namespace std;
using namespace Engine;

Input* Input::s_Instance = nullptr;

Input::Input()
{
	if(!s_Instance)
		s_Instance = this;
}

Input::~Input() { s_Instance = nullptr; }

#pragma region Keys
bool Input::IsKeyUp(int key)
{
	auto it = s_Instance->m_KeyStates.find(key);
	return it != s_Instance->m_KeyStates.end() ? it->second == KeyState::Up : true;
}

bool Input::IsKeyDown(int key)
{
	auto it = s_Instance->m_KeyStates.find(key);
	return it != s_Instance->m_KeyStates.end() ? it->second == KeyState::Down : false;
}

bool Input::IsKeyPressed(int key)
{
	auto it = s_Instance->m_KeyStates.find(key);
	return it != s_Instance->m_KeyStates.end() ? it->second == KeyState::Pressed : false;
}

bool Input::IsKeyReleased(int key)
{
	auto it = s_Instance->m_KeyStates.find(key);
	return it != s_Instance->m_KeyStates.end() ? it->second == KeyState::Released : false;
}
#pragma endregion

#pragma region Mouse Buttons
bool Input::IsMouseUp(int key)
{
	auto it = s_Instance->m_MouseStates.find(key);
	return it != s_Instance->m_MouseStates.end() ? it->second == KeyState::Up : true;
}

bool Input::IsMouseDown(int key)
{
	auto it = s_Instance->m_MouseStates.find(key);
	return it != s_Instance->m_MouseStates.end() ? it->second == KeyState::Down : false;
}

bool Input::IsMousePressed(int key)
{
	auto it = s_Instance->m_MouseStates.find(key);
	return it != s_Instance->m_MouseStates.end() ? it->second == KeyState::Pressed : false;
}

bool Input::IsMouseReleased(int key)
{
	auto it = s_Instance->m_MouseStates.find(key);
	return it != s_Instance->m_MouseStates.end() ? it->second == KeyState::Released : false;
}
#pragma endregion

#pragma region Mouse
vec2 Input::GetMousePosition() { return s_Instance->m_MousePos; }
vec2 Input::GetMouseDelta() { return (s_Instance->m_MousePos - s_Instance->m_LastMousePos); }

float Input::GetScrollDelta() { return s_Instance->m_ScrollDelta; }

void Input::ToggleShowMouse() { Application::ToggleShowMouse(); }
void Input::ShowMouse(bool show) { Application::ShowMouse(show); }
bool Input::IsMouseShowing() { return Application::IsMouseShowing(); }
#pragma endregion

void Input::Update()
{
	// Loop through all keystates and check their current state
	for (auto& pair : m_KeyStates)
	{
		switch (pair.second)
		{
		// Keys currently pressed are now classed as down/held
		case KeyState::Pressed:
			m_KeyStates[pair.first] = KeyState::Down;
			break;

		// Keys that have been released are now class as up
		case KeyState::Released:
			m_KeyStates[pair.first] = KeyState::Up;
			break;
		default: break;
		}
	}
	
	// Loop through all mousestates and check their current state
	for (auto& pair : m_MouseStates)
	{
		switch (pair.second)
		{
		// Keys currently pressed are now classed as down/held
		case KeyState::Pressed:
			m_MouseStates[pair.first] = KeyState::Down;
			break;

		// Keys that have been released are now class as up
		case KeyState::Released:
			m_MouseStates[pair.first] = KeyState::Up;
			break;
		default: break;
		}
	}

	// Set current mouse position as "last" mouse position
	m_LastMousePos = m_MousePos;

	// Reset scroll delta every frame
	m_ScrollDelta = 0;
}
