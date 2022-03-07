#pragma once
#include <glm/glm.hpp>
#include <Engine/Api.hpp>

namespace Engine { class Application; } // Forward declaration

namespace Engine::Services
{
	struct Service
	{
		virtual ~Service() { }

		virtual void OnStart() { }
		virtual void OnShutdown() { }

		virtual void OnDraw() { }
		virtual void OnUpdate(float deltaTime) { }
		virtual void OnDrawGizmos() { }

		virtual void OnResized(glm::ivec2 resolution) { }
	};
}