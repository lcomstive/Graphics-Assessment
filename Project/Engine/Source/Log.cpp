#include <iostream>
#include <Engine/Log.hpp>
#include <termcolor/termcolor.hpp>

using namespace std;
using namespace Engine;

int Log::s_LogLevel = (int)Log::LogLevel::Info |
#ifndef NDEBUG // only add debug loglevel if NOT in release mode
						(int)Log::LogLevel::Debug |
#endif
						(int)Log::LogLevel::Warning |
						(int)Log::LogLevel::Error |
						(int)Log::LogLevel::Exception;

void Log::Debug(const char* msg)
{
	if (s_LogLevel & (int)Log::LogLevel::Debug)
		cout << termcolor::green << "[DEBUG] " << msg << endl;
}
void Log::Debug(string msg) { Log::Debug(msg.c_str()); }

void Log::Info(const char* msg)
{
	if (s_LogLevel & (int)Log::LogLevel::Info)
		cout << termcolor::white << "[INFO] " << msg << endl;
}
void Log::Info(string msg) { Log::Info(msg.c_str()); }

void Log::Warning(const char* msg)
{
	if (s_LogLevel & (int)Log::LogLevel::Warning)
		cout << termcolor::yellow << "[WARN] " << msg << endl;
}
void Log::Warning(string msg) { Log::Warning(msg.c_str()); }

void Log::Error(const char* msg)
{
	if (s_LogLevel & (int)Log::LogLevel::Error)
		cout << termcolor::red << "[ERROR] " << msg << endl;
}
void Log::Error(string msg) { Log::Error(msg.c_str()); }

void Log::Exception(const char* msg, exception& ex)
{
	if (s_LogLevel & (int)Log::LogLevel::Exception)
		cout << termcolor::bright_red << "[EXCEPTION] " << msg << endl << endl << "\t" << ex.what() << endl << endl;
}
void Log::Exception(string msg, exception& ex) { Log::Exception(msg.c_str(), ex); }

void Log::Pause(const char* msg)
{
	cout << termcolor::grey << msg << endl;
	(void)getchar(); // cast to void for discarding return value
}
void Log::Pause(string msg) { Log::Pause(msg.c_str()); }

void Log::Assert(bool value, const char* msg)
{
	if (value)
		return;
	cout << termcolor::red << msg << endl;
	cout << endl << endl << "Press any key to exit..." << endl;
	(void)getchar();

	// Exit application
	std::terminate();
}
void Log::Assert(bool value, string msg) { Log::Assert(value, msg.c_str()); }

void Log::SetLogLevel(int level) { s_LogLevel = level; }
void Log::SetLogLevel(LogLevel level) { s_LogLevel = (int)level; }
int Log::GetLogLevel() { return s_LogLevel; }

void Log::AddLogLevel(LogLevel level) { s_LogLevel |= (int)level; }
void Log::RemoveLogLevel(LogLevel level) { s_LogLevel &= ~(int)level; }