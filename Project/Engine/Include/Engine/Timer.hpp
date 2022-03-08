#pragma once
#include <chrono>
#include <thread>
#include <functional>
#include <Engine/Api.hpp>

namespace Engine
{
	typedef std::function<void()> TimerCallback;

	class Timer
	{
		bool m_IsRunning;
		std::thread m_Thread;
		TimerCallback m_Callback;
		std::chrono::milliseconds m_Interval;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime, m_StopTime;

	public:
		static const std::chrono::milliseconds DefaultInterval;

		ENGINE_API Timer(unsigned int intervalMS);
		ENGINE_API Timer(std::chrono::milliseconds interval = DefaultInterval);
		ENGINE_API Timer(TimerCallback callback, std::chrono::milliseconds interval = DefaultInterval);
		ENGINE_API ~Timer();

		ENGINE_API void Start();
		ENGINE_API void Stop();
		ENGINE_API std::chrono::milliseconds ElapsedTime();

		ENGINE_API void Restart();

		ENGINE_API bool IsRunning();
	};
}