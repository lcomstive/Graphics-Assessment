#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/RenderTexture.hpp>

namespace Engine::Graphics
{
	struct ENGINE_API FramebufferAttachment
	{
		TextureFormat Format = TextureFormat::RGBA8;
		TexturePixelType PixelType = TexturePixelType::UnsignedByte;

		FramebufferAttachment() {}
		FramebufferAttachment(TextureFormat format, TexturePixelType pixelType = TexturePixelType::UnsignedByte) : Format(format), PixelType(pixelType) { }
	};

	struct ENGINE_API FramebufferSpec
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
		ENGINE_API Framebuffer(FramebufferSpec& specs);
		ENGINE_API ~Framebuffer();

		ENGINE_API void Bind();
		ENGINE_API void Unbind();

		ENGINE_API void SetSamples(unsigned int samples);
		ENGINE_API void SetResolution(glm::ivec2 newResolution);
		
		ENGINE_API void CopyAttachmentTo(RenderTexture* destination, unsigned int colourAttachment = 0);
		ENGINE_API void BlitTo(Framebuffer* other = nullptr, GLbitfield bufferFlags = GL_COLOR_BUFFER_BIT, GLenum filter = GL_NEAREST);

		ENGINE_API unsigned int GetSamples();
		ENGINE_API glm::ivec2 GetResolution();
		ENGINE_API RenderTexture* GetDepthAttachment();
		ENGINE_API RenderTexture* GetColourAttachment(unsigned int index = 0);
		ENGINE_API std::vector<RenderTexture*> GetAttachments();
		ENGINE_API std::vector<RenderTexture*> GetColourAttachments();

		ENGINE_API unsigned int AttachmentCount();
		ENGINE_API unsigned int ColourAttachmentCount();

		ENGINE_API bool HasDepthAttachment();

		ENGINE_API FramebufferSpec& GetSpecs();
	};
}
