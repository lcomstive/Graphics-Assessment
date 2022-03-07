#pragma once
#include <iostream>
#include <Engine/Application.hpp>
#include <Engine/Services/ExternalService.hpp>

class TestService : public Engine::Services::Service
{
public:
	void OnStart() override;
	void OnShutdown() override;

	void OnDraw() override;
	void OnDrawGizmos() override;
	void OnUpdate(float dt) override;
};

EXTERNAL_SERVICE_ENTRYPOINT(TestService)