#pragma once
#include <Engine/Components/Component.hpp>

struct MouseLook : public Engine::Components::Component
{
	enum class LookAxis
	{
		Vertical,	// Rotate on X Axis
		Horizontal	// Rotate on Y Axis
	} Axis = LookAxis::Vertical;

	/// <summary>
	/// Speed of rotation relative to mouse input
	/// </summary>
	float Sensitivity = 1.0;

	bool Invert = false;

protected:
	virtual void Update(float deltaTime) override;
};