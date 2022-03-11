#include <glad/glad.h>
#include <Engine/Log.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Framebuffer.hpp>

using namespace glm;
using namespace std;
using namespace Engine;
using namespace Engine::Graphics;

Framebuffer::Framebuffer(FramebufferSpec& specs) : m_Specs(specs), m_ID(GL_INVALID_VALUE), m_DepthAttachment(nullptr)
{
	if (m_Specs.SwapchainTarget)
		m_ID = 0;
}

Framebuffer::~Framebuffer() { Destroy(); }

void Framebuffer::Destroy()
{
	// Check if already destroyed
	if (m_ID == GL_INVALID_VALUE)
		return;

	Log::Debug("Destroying framebuffer [" + to_string(m_ID) + "]");

	glDeleteFramebuffers(1, &m_ID);
	m_ID = GL_INVALID_VALUE;

	for (auto& texture : m_ColourAttachments)
	{
		if (texture == m_DepthAttachment)
			m_DepthAttachment = nullptr;

		delete texture;
	}
	m_ColourAttachments.clear();

	if (m_DepthAttachment)
		delete m_DepthAttachment;
	m_DepthAttachment = nullptr;
}

void Framebuffer::Create()
{
	if (m_Specs.SwapchainTarget)
	{
		m_ID = 0;
		return;
	}

	glGenFramebuffers(1, &m_ID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

	for (unsigned int i = 0; i < m_Specs.Attachments.size(); i++)
	{
		if (m_Specs.Attachments[i].Format == TextureFormat::RenderBuffer)
		{
			RenderTexture* texture = new RenderTexture(
				{
					m_Specs.Samples,
					m_Specs.Resolution,
					m_Specs.Attachments[i].Format,
					m_Specs.Attachments[i].PixelType
				}
			);
			m_DepthAttachment = texture;
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, texture->GetID());
		}
		else
		{
			RenderTexture* texture = new RenderTexture(
				{
					m_Specs.Samples,
					m_Specs.Resolution,
					m_Specs.Attachments[i].Format,
					m_Specs.Attachments[i].PixelType,
					m_Specs.Attachments[i].DepthInfo
				}
			);
			texture->Bind();

			m_ColourAttachments.emplace_back(texture);

			GLenum attachmentType = (GLenum)(GL_COLOR_ATTACHMENT0 + (m_ColourAttachments.size() - 1));
			if (m_Specs.Attachments[i].Format == TextureFormat::Depth16 ||
				m_Specs.Attachments[i].Format == TextureFormat::Depth24 ||
				m_Specs.Attachments[i].Format == TextureFormat::Depth32)
				attachmentType = GL_DEPTH_ATTACHMENT;

			if (attachmentType == GL_DEPTH_ATTACHMENT && !m_DepthAttachment)
				m_DepthAttachment = texture;

			int depth3D = 0;
			GLenum textureTarget = GetTextureTarget(m_Specs.Attachments[i].Format, m_Specs.Samples > 1, m_Specs.Attachments[i].DepthInfo);
			switch (m_Specs.Attachments[i].Format)
			{
			default:
				if (m_Specs.Attachments[i].DepthInfo.Depth <= 1)
					glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						attachmentType,
						textureTarget,
						texture->GetID(),
						0 // Mipmap Level
					);
				else if (m_Specs.Attachments[i].DepthInfo.Is3D)
					glFramebufferTexture3D(
						GL_FRAMEBUFFER,
						attachmentType,
						textureTarget,
						texture->GetID(),
						0, // Mipmap level
						0  // Z Offset - NOT SURE IF THIS SHOULD BE ZERO??
					);
				else
				{
					for (unsigned int j = 0; j < m_Specs.Attachments[i].DepthInfo.Depth; j++)
						glFramebufferTextureLayer(
							GL_FRAMEBUFFER,
							attachmentType,
							texture->GetID(),
							0, // Mipmap level
							j  // Array layer
						);
				}
				break;
			case TextureFormat::Cubemap:
				for (int i = 0; i < 6; i++)
					glFramebufferTexture2D(
						GL_FRAMEBUFFER,
						(GLenum)(GL_COLOR_ATTACHMENT0 + (m_ColourAttachments.size() - 1)),
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						texture->GetID(),
						0 // Mipmap level
					);
				break;
			}

			texture->Unbind();
		}
	}

	// Draw buffers
	vector<GLenum> colourBuffers;
	for (unsigned int i = 0; i < m_ColourAttachments.size(); i++)
		colourBuffers.emplace_back(GL_COLOR_ATTACHMENT0 + i);

	if (colourBuffers.size() > 1)
		glDrawBuffers((GLsizei)colourBuffers.size(), colourBuffers.data());
	else if (colourBuffers.empty())
	{
		glReadBuffer(GL_NONE);
		glDrawBuffer(GL_NONE); // Only depth-pass
	}

	// Finalise
	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
		Log::Error("Failed to create framebuffer");
	Log::Assert(framebufferStatus == GL_FRAMEBUFFER_COMPLETE, "OpenGL Framebuffer Error: " + to_string(framebufferStatus));

	Log::Debug("Created framebuffer [" + to_string(m_ID) + "][" + to_string(m_Specs.Attachments.size()) + " attachments]");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Recreate()
{
	if (m_Specs.SwapchainTarget)
		return;

	if (m_ID == GL_INVALID_VALUE)
		Create();

	// Create attachments if they don't exist, otherwise alter existing ones
	for (auto& attachment : m_ColourAttachments)
	{
		attachment->SetSamples(m_Specs.Samples);
		attachment->SetResolution(m_Specs.Resolution);
	}

	if (m_DepthAttachment)
	{
		m_DepthAttachment->SetSamples(m_Specs.Samples);
		m_DepthAttachment->SetResolution(m_Specs.Resolution);
	}
}

void Framebuffer::BlitTo(Framebuffer* other, GLbitfield bufferFlags, GLenum filter)
{
	ivec2 destRes = other ? other->m_Specs.Resolution : m_Specs.Resolution;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_ID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other ? other->m_ID : 0);
	glBlitFramebuffer(
		// Source x,y, width, height
		0, 0,
		m_Specs.Resolution.x,
		m_Specs.Resolution.y,
		// Destination x,y, width, height
		0, 0,
		destRes.x,
		destRes.y,
		// Mask (e.g. Colour, Depth)
		bufferFlags,
		// Filter
		filter
	);
}

void Framebuffer::CopyAttachmentTo(RenderTexture* destination, unsigned int colourAttachment)
{
	// GetColourAttachment(colourAttachment)->CopyTo(destination);

	Bind();
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	destination->Bind();
	glCopyTexSubImage2D(
		GetTextureTarget(destination->GetFormat(), destination->GetSamples() > 1),
		0, // Level
		0, 0, // (x, y) offset
		0, 0, // (x, y) / (left, top)
		destination->GetResolution().x,
		destination->GetResolution().y
	);

	destination->Unbind();
	Unbind();
}

void Framebuffer::Bind()
{
	if (m_ID == GL_INVALID_VALUE)
		Create();

#ifndef NDEBUG
	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	Log::Assert(framebufferStatus == GL_FRAMEBUFFER_COMPLETE, "OpenGL Framebuffer Error: " + to_string(framebufferStatus));
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
	glViewport(0, 0, m_Specs.Resolution.x, m_Specs.Resolution.y);
}

void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::SetResolution(ivec2 newResolution)
{
	m_Specs.Resolution = newResolution;
	m_Specs.Resolution.x = std::max(m_Specs.Resolution.x, 1);
	m_Specs.Resolution.y = std::max(m_Specs.Resolution.y, 1);
	Recreate();
}

void Framebuffer::SetSamples(unsigned int samples)
{
	m_Specs.Samples = std::clamp(samples, 1u, 32u);
	Recreate();
}

ivec2 Framebuffer::GetResolution() { return m_Specs.Resolution; }
unsigned int Framebuffer::GetSamples() { return m_Specs.Samples; }
RenderTexture* Framebuffer::GetDepthAttachment() { return m_DepthAttachment; }
RenderTexture* Framebuffer::GetColourAttachment(unsigned int index)
{
	if (m_ColourAttachments.empty())
		return nullptr;
	return m_ColourAttachments[std::clamp(index, 0u, (unsigned int)m_ColourAttachments.size() - 1)];
}

std::vector<RenderTexture*> Framebuffer::GetColourAttachments() { return m_ColourAttachments; }
std::vector<RenderTexture*> Framebuffer::GetAttachments()
{
	vector<RenderTexture*> attachments;
	attachments.insert(attachments.end(), m_ColourAttachments.begin(), m_ColourAttachments.end());
	attachments.emplace_back(m_DepthAttachment);
	return attachments;
}

unsigned int Framebuffer::AttachmentCount() { return (unsigned int)(m_ColourAttachments.size() + (m_DepthAttachment ? 0 : 1)); }
unsigned int Framebuffer::ColourAttachmentCount() { return (unsigned int)m_ColourAttachments.size(); }

bool Framebuffer::HasDepthAttachment() { return m_DepthAttachment != nullptr; }
FramebufferSpec& Framebuffer::GetSpecs() { return m_Specs; }
