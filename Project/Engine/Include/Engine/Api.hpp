#pragma once

#pragma warning(disable : 4251) // 'Engine::*': struct 'glm::vec<*>' needs to have dll-interface to be used by clients of struct 'Engine::*'
#pragma warning(disable : 4005) // 'APIENTRY': macro redefinition (defined in both "C:\Program Files (x86)\Windows Kits\10\Include\x.x.x.x\shared\minwindef.h" & "Dependencies\glad\include\glad\glad.h"

#if BUILD_SHARED_LIB
	#if defined(_MSC_VER)
		#define ENGINE_EXPORT __declspec(dllexport)
		#define ENGINE_IMPORT __declspec(dllimport)
	#elif defined(__GNUC__)
		#define ENGINE_EXPORT __attribute__((visibility("default")))
		#define ENGINE_IMPORT
	#else
		#define ENGINE_EXPORT
		#define ENGINE_IMPORT
		#error Unknown dynamic link environment
	#endif
	#if ENGINE_EXPORT_DLL
		#define ENGINE_API ENGINE_EXPORT
	#else
		#define ENGINE_API ENGINE_IMPORT
	#endif
#else
	#define ENGINE_EXPORT
	#define ENGINE_IMPORT
	#define ENGINE_API
#endif

#if BUILD_SERVICE_DLL
	#if _WIN32
		#define SERVICE_API __declspec(dllexport)
	#else
		#define SERVICE_API __attribute__((visibility("default")))
	#endif
#elif _WIN32
	#define SERVICE_API __declspec(dllimport)
#else
	#define SERVICE_API
#endif