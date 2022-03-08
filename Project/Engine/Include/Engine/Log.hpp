#pragma once
#include <string>
#include <exception>
#include <Engine/Api.hpp>

namespace Engine
{
	class ENGINE_API Log
	{
		static int s_LogLevel;
	public:
		enum class LogLevel : int
		{
			Debug		= (1 << 0),
			Info		= (1 << 1),
			Warning		= (1 << 2),
			Error		= (1 << 3),
			Exception	= (1 << 4),

			All = (int)Debug | (int)Info | (int)Warning | (int)Error | (int)Exception
		};

		static void Debug(const char* msg);
		static void Info(const char* msg);
		static void Warning(const char* msg);
		static void Error(const char* msg);
		static void Exception(const char* msg, std::exception& exception);
		static void Pause(const char* msg = "Press any key to continue..");
		static void Assert(bool value, const char* msg);

		static void Debug(std::string msg);
		static void Info(std::string msg);
		static void Warning(std::string msg);
		static void Error(std::string msg);
		static void Exception(std::string msg, std::exception& exception);
		static void Pause(std::string msg);
		static void Assert(bool value, std::string msg);

		static void SetLogLevel(int level);
		static void SetLogLevel(LogLevel level);
		static int GetLogLevel();

		static void AddLogLevel(LogLevel level);
		static void RemoveLogLevel(LogLevel level);
	};
}