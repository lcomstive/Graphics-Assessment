#pragma once
#include <Engine/Services/ExternalService.hpp>

class Demo : public Engine::Services::Service
{
protected:
	void OnStart() override;
	void OnShutdown() override;

	void OnDraw() override;
	void OnDrawGizmos() override;
	void OnUpdate(float deltaTime) override;

	void ResetScene();
};

EXTERNAL_SERVICE_ENTRYPOINT(Demo)