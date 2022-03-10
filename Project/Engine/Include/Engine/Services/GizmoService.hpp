#pragma once
#include <Engine/Services/Service.hpp>

namespace Engine::Graphics { class Framebuffer; }

namespace Engine::Services
{
	struct ENGINE_API GizmoService : public Service
	{
		virtual void OnStart() override;
		virtual void OnShutdown() override;

		virtual void OnPipelineChanged(Graphics::RenderPipeline* pipeline) override;

	private:
		Graphics::RenderPipelinePass m_Pass;

		void DrawCallback(Engine::Graphics::Framebuffer* previous);
	};
}