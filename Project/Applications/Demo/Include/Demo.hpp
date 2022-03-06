#pragma once
#include <Engine/Application.hpp>
#include <Engine/Physics/Shapes.hpp>

class Demo : public Engine::Application
{
	void OnStart() override;
	void OnShutdown() override;

	void OnDraw() override;
	void OnUpdate() override;
	void OnDrawGizmos() override;

	void ResetScene();

public:
	Demo(Engine::ApplicationArgs args);
};
