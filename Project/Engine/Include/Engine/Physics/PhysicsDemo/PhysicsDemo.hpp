#pragma once
#include <Engine/Application.hpp>
#include <Engine/Physics/Shapes.hpp>

class PhysicsDemo : public Engine::Application
{
	void OnStart() override;
	void OnShutdown() override;

	void OnDraw() override;
	void OnUpdate() override;
	void OnDrawGizmos() override;

	void CreateWall(glm::vec3 axis);
	void CreateWalls();
	void ResetScene();

public:
	PhysicsDemo(Engine::ApplicationArgs args);
};
