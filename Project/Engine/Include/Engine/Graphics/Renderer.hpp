#pragma once
#include <set>
#include <mutex>
#include <vector>
#include <glm/glm.hpp>
#include <Engine/Graphics/Mesh.hpp>
#include <Engine/Graphics/Material.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Components/Transform.hpp>
#include <Engine/Graphics/RenderPipeline.hpp>

namespace Engine::Graphics
{
	struct DrawCall
	{
		Mesh* Mesh;
		Material Material;

		glm::vec3 Position = { 0, 0, 0 };
		glm::vec3 Scale = { 1, 1, 1 };
		glm::mat4 Rotation = glm::mat4(1.0f);

		bool DeleteMeshAfterRender = false;

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
		static uint32_t s_Samples;
		static Application* s_App;
		static glm::ivec2 s_Resolution;
		static bool s_Wireframe, s_VSync;
		static RenderPipeline* s_Pipeline;
		static float s_Time, s_FPS, s_DeltaTime;

		static std::vector<DrawCall> s_DrawQueue;

		static void Shutdown();
		static void Resized(glm::ivec2 newResolution);
		static void SortDrawQueue(DrawSortType sortType);

		friend class Engine::Application;

	public:
		static void Draw(DrawArgs args = {});

		static void ClearDrawQueue();
		static void Submit(DrawCall drawCall);
		static void Submit(Mesh* mesh, Material& material, Components::Transform* transform);
		static void Submit(Mesh* mesh, Material& material, glm::vec3 position, glm::vec3 scale, glm::mat4 rotation);
		static void Submit(Mesh* mesh, Material& material, glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);

#pragma region Setters
		static void SetVSync(bool vsync = true);
		static void SetWireframe(bool wireframe = true);

		template<typename T>
		static T* SetPipeline()
		{
			Log::Assert(std::is_base_of<RenderPipeline, T>(), "Pipeline needs to inherit from Engine::Graphics::RenderPipeline");
			if (s_Pipeline)
				delete s_Pipeline;
			s_Pipeline = new T();
			return (T*)s_Pipeline;
		}
#pragma endregion

#pragma region Getters
		static bool GetVSync();
		static float GetFPS();
		static float GetTime();
		static uint32_t GetSamples();
		static float GetDeltaTime();
		static Application* GetApp();
		static bool GetWireframeMode();
		static glm::ivec2 GetResolution();
		static RenderPipeline* GetPipeline();
#pragma endregion
	};
}
