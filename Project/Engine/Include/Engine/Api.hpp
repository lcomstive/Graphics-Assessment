#pragma once

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
#else
	#define ENGINE_EXPORT
	#define ENGINE_IMPORT
#endif

#if EXPORT_DLL
	#define ENGINE_API ENGINE_EXPORT
#else
	#define ENGINE_API ENGINE_IMPORT
#endif