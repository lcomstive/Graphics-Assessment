#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

namespace Engine::Graphics
{
	struct FramebufferAttachment
	{
		TextureFormat Format = TextureFormat::RGBA8;
		TexturePixelType PixelType = TexturePixelType::UnsignedByte;

		FramebufferAttachment() {}
		FramebufferAttachment(TextureFormat format, TexturePixelType pixelType = TexturePixelType::UnsignedByte) : Format(format), PixelType(pixelType) { }
	};

	struct FramebufferSpec
	{
		glm::ivec2 Resolution = { 0, 0 };

		/// <summary>
		/// When set to true, framebuffer target is the screen
		/// </summary>
		bool SwapchainTarget = false;

		/// <summary>
		/// Multisampling, or disable by setting to 1
		/// </summary>
		unsigned int Samples = 1;
		
		std::vector<FramebufferAttachment> Attachments;
	};

	class Framebuffer
	{
		unsigned int m_ID;
		FramebufferSpec m_Specs;

		RenderTexture* m_DepthAttachment;
		std::vector<RenderTexture*> m_ColourAttachments;

		void Destroy();
		void Create();
		void Recreate(); // Usually called when resized or samples change, attachment count & formats remain constant

	public:
		Framebuffer(FramebufferSpec& specs);
		~Framebuffer();

		void Bind();
		void Unbind();

		void SetSamples(unsigned int samples);
		void SetResolution(glm::ivec2 newResolution);
		
		void CopyAttachmentTo(RenderTexture* destination, unsigned int colourAttachment = 0);
		void BlitTo(Framebuffer* other = nullptr, GLbitfield bufferFlags = GL_COLOR_BUFFER_BIT, GLenum filter = GL_NEAREST);

		unsigned int GetSamples();
		glm::ivec2 GetResolution();
		RenderTexture* GetDepthAttachment();
		RenderTexture* GetColourAttachment(unsigned int index = 0);
		std::vector<RenderTexture*> GetAttachments();
		std::vector<RenderTexture*> GetColourAttachments();

		unsigned int AttachmentCount();
		unsigned int ColourAttachmentCount();

		bool HasDepthAttachment();

		FramebufferSpec& GetSpecs();
	};
}
