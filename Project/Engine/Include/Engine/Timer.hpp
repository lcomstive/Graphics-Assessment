#pragma once
#include <chrono>
#include <thread>
#include <functional>

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

		Timer(unsigned int intervalMS);
		Timer(std::chrono::milliseconds interval = DefaultInterval);
		Timer(TimerCallback callback, std::chrono::milliseconds interval = DefaultInterval);
		~Timer();

		void Start();
		void Stop();
		std::chrono::milliseconds ElapsedTime();

		void Restart();

		bool IsRunning();
	};
}