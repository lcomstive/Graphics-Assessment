#include <Engine/Timer.hpp>

using namespace std;
using namespace Engine;
using namespace std::chrono;

const milliseconds Timer::DefaultInterval = milliseconds(1000);

Timer::Timer(milliseconds interval) : Timer(nullptr, interval) { }
Timer::Timer(unsigned int intervalMS) : Timer(nullptr, milliseconds(intervalMS)) { }
Timer::Timer(TimerCallback callback, milliseconds interval) : m_Interval(interval), m_Callback(callback), m_IsRunning(false) { }

Timer::~Timer()
{
	Stop();
}

void Timer::Start()
{
	if (m_IsRunning)
		return;
	
	m_IsRunning = true;
	m_StartTime = high_resolution_clock::now();

	m_Thread = thread([&]()
		{
			time_point start = high_resolution_clock::now();

			while (m_IsRunning)
			{
				this_thread::sleep_for(m_Interval);

				time_point end = high_resolution_clock::now();
				start = end;

				if(m_Callback)
					m_Callback();
			}
		});
}

void Timer::Stop()
{
	if (!m_IsRunning)
		return;
	m_IsRunning = false;
	m_StopTime = high_resolution_clock::now();
	
	if (m_Thread.joinable())
		m_Thread.join();

	m_Thread.~thread();
}

void Timer::Restart()
{
	Stop();
	Start();
}

bool Timer::IsRunning() { return m_IsRunning; }

milliseconds Timer::ElapsedTime()
{
	if (m_IsRunning)
		return duration_cast<milliseconds>(high_resolution_clock::now() - m_StartTime);
	return duration_cast<milliseconds>(m_StopTime - m_StartTime);
}