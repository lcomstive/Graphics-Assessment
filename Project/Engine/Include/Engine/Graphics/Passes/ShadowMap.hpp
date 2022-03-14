#pragma once
#include <Engine/Graphics/Shader.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Components { struct Light; }

namespace Engine::Graphics
{
	class ShadowMapPass
	{
		/// <summary>
		/// Shadow data store
		/// </summary>
		struct LightShadowData
		{
			// Camera view * projection matrices. Stored here instead of recalculating multiple times per frame
			glm::mat4 LightSpaceMatrix;

			/// <summary>
			/// Less than 0 implies that light has not been calculated nor put into the shadow map
			/// </summary>
			int ShadowMapArrayIndex;
		};

		glm::ivec2 m_Resolution;
		RenderPipelinePass m_Pass;
		Shader* m_Shader = nullptr;

		EngineUnorderedMap<Components::Light*, LightShadowData> m_ShadowCasters;

		void SetBorder();
		void DrawCallback(Framebuffer* previous);
		void FillShadowData(Components::Light* light, LightShadowData& shadowData);

		void AddShadowCaster(Components::Light* light);
		void RemoveShadowCaster(Components::Light* light);

		friend struct Components::Light;

	public:
		ENGINE_API ShadowMapPass(glm::ivec2 resolution = { 1024, 1024 });
		ENGINE_API ~ShadowMapPass();

		ENGINE_API glm::ivec2& GetResolution();
		ENGINE_API void SetResolution(glm::ivec2 resolution);

		ENGINE_API RenderPipelinePass& GetPipelinePass();

		ENGINE_API RenderTexture* GetTexture();
	};
}