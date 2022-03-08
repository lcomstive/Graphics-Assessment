#pragma once
#include <mutex>
#include <chrono>
#include <thread>
#include <functional>
#include <glm/glm.hpp>
#include <unordered_map>
#include <Engine/Api.hpp>
#include <Engine/Log.hpp>
#include <Engine/Physics/Octree.hpp>
#include <Engine/Components/Physics/Collider.hpp>
#include <Engine/Physics/Broadphase/Broadphase.hpp>

using namespace std::chrono_literals;

// Forward declarations
namespace Engine { class Application; }
namespace Engine::Components { struct Rigidbody; }

namespace Engine::Physics
{
	enum class ENGINE_API PhysicsPlayState : int { Stopped = 0, Playing, Paused };

	class PhysicsSystem
	{
		std::thread m_Thread;
		std::mutex m_VariableMutex;
		std::mutex m_CollidersMutex;
		std::mutex m_PhysicsStateMutex;
		std::atomic_int m_ThreadState; // Corresponds to ThreadState
		std::condition_variable m_PauseConditional; // Notifies thread when to unpause

		Broadphase* m_Broadphase;
		PhysicsPlayState m_PhysicsState;
		glm::vec3 m_Gravity = { 0, -9.81f, 0 };
		std::chrono::milliseconds m_LastTimestep;
		std::chrono::milliseconds m_FixedTimestep;
		std::vector<CollisionFrame> m_Collisions;
		std::vector<Components::Collider*> m_Colliders;
		std::vector<Components::PhysicsComponent*> m_Components;

		/// <summary>
		/// How much positional correction to apply,
		/// smaller values allow objects to penetrate more.
		/// Typically 0.2-0.8
		/// </summary>
		float m_LinearProjectionPercent = 0.3f;

		/// <summary>
		/// How much to allow objects to penetrate.
		/// The larger the number, the less jitter in the system.
		/// Range 0.01-0.1.
		/// </summary>
		float m_PenetrationSlack = 0.02f;

		void PhysicsLoop();

		void AddCollider(Components::Collider* collider);
		void RemoveCollider(Components::Collider* collider);

		void AddPhysicsComponent(Components::PhysicsComponent* component);
		void RemovePhysicsComponent(Components::PhysicsComponent* component);

		void NarrowPhase();
		void ApplyImpulse();
		void PositionalCorrect();

		friend struct Components::Collider;
		friend class Components::PhysicsComponent;

	protected:
		/// <summary>
		/// Impulse iterations per update.
		/// Typically 6-8 works well
		/// </summary>
		int m_ImpulseIteration;

		/// <summary>
		/// Physics iterations per update.
		/// Typically 6-8 works well
		/// </summary>
		int m_Substeps;

	public:
		ENGINE_API PhysicsSystem(std::chrono::milliseconds fixedTimestep = 50ms);
		ENGINE_API ~PhysicsSystem();

		ENGINE_API void Start();
		ENGINE_API void Stop();

		ENGINE_API void DrawGizmos();

		ENGINE_API void Resume();
		ENGINE_API void Pause();
		ENGINE_API void TogglePause();

		ENGINE_API std::chrono::milliseconds Timestep();
		ENGINE_API std::chrono::milliseconds LastTimestep();
		ENGINE_API void SetTimestep(std::chrono::milliseconds timestep);

		ENGINE_API void SetGravity(glm::vec3 gravity);
		ENGINE_API glm::vec3 GetGravity();

		ENGINE_API PhysicsPlayState GetState();

		template<typename T, class... Args>
		ENGINE_EXPORT T* SetBroadphase(Args... args)
		{
			Log::Assert(std::is_base_of<Broadphase, T>(), "Broadphase needs to have base class of Engine::Phyics::Broadphase");

			if (m_Broadphase)
				delete m_Broadphase;

			m_Broadphase = new T(args...);
			return (T*)m_Broadphase;
		}

		ENGINE_API bool LineTest(Line line, Components::Collider* ignoreCollider);
		ENGINE_API Components::Collider* Raycast(Ray ray, RaycastHit* outResult = nullptr);
		ENGINE_API Components::Collider* Raycast(Ray ray, Components::Collider* ignoreCollider, RaycastHit* outResult = nullptr);
		ENGINE_API std::vector<Components::Collider*> Query(Sphere& bounds);
		ENGINE_API std::vector<Components::Collider*> Query(AABB& bounds);
	};
}
