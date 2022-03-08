#include <thread>
#include <unordered_map>
#include <Engine/Log.hpp>
#include <Engine/FileWatcher.hpp>

using namespace std;
using namespace Engine;

FileWatcher::FileWatcher(string path, int intervalMs, bool multithread) :
	m_Path(path),
	m_LoopThread(),
	m_Running(false),
	m_WatchedPaths(),
	m_Interval(intervalMs),
	m_Multithread(multithread)
{
	if (filesystem::is_regular_file(m_Path) && filesystem::exists(m_Path))
	{
		m_WatchedPaths[m_Path] = filesystem::last_write_time(m_Path);
	}
	else if (filesystem::is_directory(m_Path))
	{
		for (auto& file : filesystem::recursive_directory_iterator(m_Path))
			m_WatchedPaths[file.path().string()] = file.last_write_time();
	}
}

FileWatcher::FileWatcher(string path, bool multithread) : FileWatcher(path, 1000, multithread) { }

void FileWatcher::Start(function<void(string, FileWatchStatus)> callback)
{
	m_Callback = callback;
	m_Running.store(true);

	if (m_Multithread)
		m_LoopThread = thread(&FileWatcher::BeginWatching, this);
	else
		BeginWatching(); // Call synchronously
}

void FileWatcher::BeginWatching()
{
	if (!m_Callback)
	{
		Log::Debug("Cannot watch '" + m_Path + "' - no callback was registered");
		return;
	}

	Log::Debug("Watching '" + m_Path + "' for changes");
	while (m_Running.load())
	{
		// Check for deleted files
		for(auto& it = m_WatchedPaths.begin(); it != m_WatchedPaths.end();)
		{
			if (filesystem::exists(it->first))
			{
				it++;
				continue;
			}

			m_Callback(it->first, FileWatchStatus::Removed);
			m_WatchedPaths.erase(it++);
		}

		// Check for modified or created files
		if (filesystem::is_directory(m_Path))
		{
			for (auto& file : filesystem::recursive_directory_iterator(m_Path))
			{
				string filepath = file.path().string();
				if (m_WatchedPaths.find(filepath) == m_WatchedPaths.end())
				{
					// File was created
					m_WatchedPaths[filepath] = file.last_write_time();
					m_Callback(filepath, FileWatchStatus::Created);
				}
				else if (file.last_write_time() != m_WatchedPaths[filepath])
				{
					m_WatchedPaths[filepath] = file.last_write_time();
					m_Callback(filepath, FileWatchStatus::Modified);
				}
			}
		}
		if (filesystem::is_regular_file(m_Path))
		{
			auto lastWriteTime = filesystem::last_write_time(m_Path);
			if (lastWriteTime != m_WatchedPaths[m_Path])
			{
				m_WatchedPaths[m_Path] = lastWriteTime;
				m_Callback(m_Path, FileWatchStatus::Modified);
			}
		}

		this_thread::sleep_for(m_Interval);
	}
}

void FileWatcher::Stop()
{
	m_Running.store(false);

	if (m_Multithread)
		m_LoopThread.join();
}