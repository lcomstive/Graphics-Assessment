/*
#include <glad/glad.h>
#include <Engine/Graphics/RenderPass.hpp>

using namespace Engine::Graphics;

RenderPass::RenderPass(FramebufferSpec& specs) : m_Framebuffer(new Framebuffer(specs)) { }

RenderPass::~RenderPass()
{
	if (!m_Framebuffer)
		return;

	delete m_Framebuffer;
	m_Framebuffer = nullptr;
}

Framebuffer* RenderPass::GetFramebuffer() { return m_Framebuffer; }

void RenderPass::Begin() { m_Framebuffer->Bind(); }
void RenderPass::End() { m_Framebuffer->Unbind(); }

unsigned int RenderPass::GetColourAttachmentCount() { return m_Framebuffer ? m_Framebuffer->ColourAttachmentCount() : 0; }
RenderTexture* RenderPass::GetDepthAttachment() { return m_Framebuffer ? m_Framebuffer->GetDepthAttachment() : nullptr; }
RenderTexture* RenderPass::GetColourAttachment(unsigned int index) { return m_Framebuffer ? m_Framebuffer->GetColourAttachment(index) : nullptr; }
*/