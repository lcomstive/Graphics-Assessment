// Based on https://solarianprogrammer.com/2019/01/13/cpp-17-filesystem-write-file-watcher-monitor/
#pragma once
#include <chrono>
#include <thread>
#include <string>
#include <functional>
#include <filesystem>
#include <unordered_map>

namespace Engine
{
	enum class FileWatchStatus { Created, Removed, Modified };

	class FileWatcher
	{
		std::string m_Path;
		std::thread m_LoopThread;
		bool m_Running, m_Multithread;
		std::chrono::duration<int, std::milli> m_Interval; // Time between checking for changes
		std::function<void(std::string, FileWatchStatus)> m_Callback;
		std::unordered_map<std::string, std::filesystem::file_time_type> m_WatchedPaths;

		void BeginWatching();

	public:
		/// <param name="intervalMs">Time between checking for file changes, in milliseconds</param>
		FileWatcher(std::string path, int intervalMs = 1000, bool multithread = true);

		// Constructor using default interval of 1000ms
		FileWatcher(std::string path, bool multithread);

		void Start(std::function<void(std::string, FileWatchStatus)> callback);
		void Stop();
	};
}