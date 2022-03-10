#pragma once
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine { class Application; } // Forward declaration

namespace Engine::Services
{
	struct ENGINE_API Service
	{
		virtual ~Service() { }

		virtual void OnStart() { }
		virtual void OnShutdown() { }

		virtual void OnDraw() { }
		virtual void OnUpdate(float deltaTime) { }
		virtual void OnDrawGizmos() { }

		/// <summary>
		/// Called when the pipeline changes, including during application setup
		/// </summary>
		virtual void OnPipelineChanged(Graphics::RenderPipeline* pipeline) { }

		virtual void OnResized(glm::ivec2 resolution) { }
	};
}