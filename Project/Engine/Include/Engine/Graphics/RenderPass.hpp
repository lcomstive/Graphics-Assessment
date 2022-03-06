#pragma once
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/Framebuffer.hpp>

namespace Engine::Graphics
{
	class RenderPass
	{
		Framebuffer* m_Framebuffer;

	public:
		RenderPass(FramebufferSpec& framebufferSpecs);
		~RenderPass();

		Framebuffer* GetFramebuffer();

		void Begin();
		void End();

		RenderTexture* GetDepthAttachment();
		unsigned int GetColourAttachmentCount();
		RenderTexture* GetColourAttachment(unsigned int index = 0);
	};
}