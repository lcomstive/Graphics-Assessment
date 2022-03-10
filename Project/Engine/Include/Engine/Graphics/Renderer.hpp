#pragma once
#include <set>
#include <mutex>
#include <vector>
#include <glm/glm.hpp>
#include <Engine/Api.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Services/Service.hpp>
#include <Engine/Graphics/Material.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Components { struct Transform; }

namespace Engine::Graphics
{
	struct ENGINE_API DrawCall
	{
		ResourceID Mesh;
		Material Material;

		glm::vec3 Position = { 0, 0, 0 };
		glm::vec3 Scale = { 1, 1, 1 };
		glm::mat4 Rotation = glm::mat4(1.0f);

		// For GL_LINES
		float LineWidth = 1.0f;

		bool DeleteMeshAfterRender = false;

		// Copy constructor
		DrawCall& operator =(const DrawCall& other)
		{
			Mesh = other.Mesh;
			Scale = other.Scale;
			Rotation = other.Rotation;
			Material = other.Material;
			Position = other.Position;
			DeleteMeshAfterRender = other.DeleteMeshAfterRender;
			return *this;
		}

		bool operator ==(const DrawCall& b)
		{
			return Mesh == b.Mesh &&
				// Material == b.Material &&
				Position == b.Position &&
				Scale == b.Scale &&
				Rotation == b.Rotation &&
				DeleteMeshAfterRender == b.DeleteMeshAfterRender;
		}
	};

	enum class DrawSortType { None, FrontToBack, BackToFront };

	struct ENGINE_API DrawArgs
	{
		bool ClearQueue = true;
		bool RenderOpaque = true;
		bool RenderTransparent = true;
		DrawSortType DrawSorting = DrawSortType::None;
	};

	class Renderer
	{
		uint32_t m_Samples;
		glm::ivec2 m_Resolution;
		bool m_Wireframe, m_VSync;
		RenderPipeline* m_Pipeline;
		bool m_SupportsTessellation;
		Components::Camera* m_MainCamera;
		float m_Time, m_FPS, m_DeltaTime;
		std::vector<DrawCall> m_DrawQueue;

		static ENGINE_API Renderer* s_Instance;

		static void Shutdown();
		static void Resized(glm::ivec2 newResolution);
		static void SortDrawQueue(DrawSortType sortType);

		Renderer();
		~Renderer();

		friend class Engine::Application;

	public:
		ENGINE_API static void Draw(DrawArgs args = {});

		ENGINE_API static void ClearDrawQueue();
		ENGINE_API static void Submit(DrawCall drawCall);
		ENGINE_API static void Submit(ResourceID& mesh, Material& material, Components::Transform* transform);
		ENGINE_API static void Submit(ResourceID& mesh, Material& material, glm::vec3 position, glm::vec3 scale, glm::mat4 rotation);
		ENGINE_API static void Submit(ResourceID& mesh, Material& material, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);

		ENGINE_API static Components::Camera* GetMainCamera();
		ENGINE_API static void SetMainCamera(Components::Camera* camera);

#pragma region Setters
		ENGINE_API static void ToggleWireframe();
		ENGINE_API static void SetVSync(bool vsync = true);
		ENGINE_API static void SetWireframe(bool wireframe = true);

		template<typename T>
		ENGINE_EXPORT static T* SetPipeline()
		{
			Log::Assert(std::is_base_of<RenderPipeline, T>(), "Pipeline needs to inherit from Engine::Graphics::RenderPipeline");
			if (s_Instance->m_Pipeline)
				delete s_Instance->m_Pipeline;
			s_Instance->m_Pipeline = new T();
			s_Instance->m_Pipeline->OnResized(s_Instance->m_Resolution);
			
			auto& services = Application::GetAllServices();
			for (Engine::Services::Service* service : services)
				service->OnPipelineChanged(s_Instance->m_Pipeline);

			return (T*)s_Instance->m_Pipeline;
		}
#pragma endregion

#pragma region Getters
		ENGINE_API static bool GetVSync();
		ENGINE_API static float GetFPS();
		ENGINE_API static float GetTime();
		ENGINE_API static uint32_t GetSamples();
		ENGINE_API static float GetDeltaTime();
		ENGINE_API static bool GetWireframeMode();
		ENGINE_API static glm::ivec2 GetResolution();
		ENGINE_API static RenderPipeline* GetPipeline();

		/// <summary>
		/// If the current hardware supports the tessellation feature (OpenGL 4.0+
		/// </summary>
		/// <returns></returns>
		ENGINE_API static bool SupportsTessellation();
#pragma endregion
	};
}
