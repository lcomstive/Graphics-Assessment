#pragma once
#include <set>
#include <mutex>
#include <vector>
#include <glm/glm.hpp>
#include <Engine/ResourceID.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Graphics/Material.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Components { struct Transform; }

namespace Engine::Graphics
{
	struct DrawCall
	{
		ResourceID Mesh;
		Material Material;

		glm::vec3 Position = { 0, 0, 0 };
		glm::vec3 Scale = { 1, 1, 1 };
		glm::mat4 Rotation = glm::mat4(1.0f);

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

	struct DrawArgs
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
		float m_Time, m_FPS, m_DeltaTime;
		std::vector<DrawCall> m_DrawQueue;

		static Renderer* s_Instance;

		static void Shutdown();
		static void Resized(glm::ivec2 newResolution);
		static void SortDrawQueue(DrawSortType sortType);

		Renderer();
		~Renderer();

		friend class Engine::Application;

	public:
		static void Draw(DrawArgs args = {});

		static void ClearDrawQueue();
		static void Submit(DrawCall drawCall);
		static void Submit(ResourceID& mesh, Material& material, Components::Transform* transform);
		static void Submit(ResourceID& mesh, Material& material, glm::vec3 position, glm::vec3 scale, glm::mat4 rotation);
		static void Submit(ResourceID& mesh, Material& material, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);

#pragma region Setters
		static void SetVSync(bool vsync = true);
		static void SetWireframe(bool wireframe = true);

		template<typename T>
		static T* SetPipeline()
		{
			Log::Assert(std::is_base_of<RenderPipeline, T>(), "Pipeline needs to inherit from Engine::Graphics::RenderPipeline");
			if (s_Instance->m_Pipeline)
				delete s_Instance->m_Pipeline;
			s_Instance->m_Pipeline = new T();
			return (T*)s_Instance->m_Pipeline;
		}
#pragma endregion

#pragma region Getters
		static bool GetVSync();
		static float GetFPS();
		static float GetTime();
		static uint32_t GetSamples();
		static float GetDeltaTime();
		static bool GetWireframeMode();
		static glm::ivec2 GetResolution();
		static RenderPipeline* GetPipeline();
#pragma endregion
	};
}
