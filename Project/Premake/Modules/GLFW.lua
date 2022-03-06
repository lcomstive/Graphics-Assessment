project "GLFW"
	kind "StaticLib"
	language "C"
	staticruntime "Off"

	targetdir ("%{OutputDir}%{prj.name}")
	objdir ("%{OutputDir}Intermediate/%{prj.name}")

	files
	{
		"%{DependencyDir}GLFW/include/GLFW/glfw3.h",
		"%{DependencyDir}GLFW/include/GLFW/glfw3native.h",
		"%{DependencyDir}GLFW/src/internal.h",
		"%{DependencyDir}GLFW/src/platform.h",
		"%{DependencyDir}GLFW/src/mappings.h",
		"%{DependencyDir}GLFW/src/context.c",
		"%{DependencyDir}GLFW/src/init.c",
		"%{DependencyDir}GLFW/src/input.c",
		"%{DependencyDir}GLFW/src/monitor.c",
		"%{DependencyDir}GLFW/src/platform.c",
		"%{DependencyDir}GLFW/src/vulkan.c",
		"%{DependencyDir}GLFW/src/window.c",
		"%{DependencyDir}GLFW/src/egl_context.c",
		"%{DependencyDir}GLFW/src/osmesa_context.c",
		"%{DependencyDir}GLFW/src/null_platform.h",
		"%{DependencyDir}GLFW/src/null_joystick.h",
		"%{DependencyDir}GLFW/src/null_init.c",
	
		"%{DependencyDir}GLFW/src/null_monitor.c",
		"%{DependencyDir}GLFW/src/null_window.c",
		"%{DependencyDir}GLFW/src/null_joystick.c",
	
	}

	systemversion "latest"
	staticruntime "On"

	filter "system:windows"		
		files
		{
			"%{DependencyDir}GLFW/src/win32_init.c",
			"%{DependencyDir}GLFW/src/win32_module.c",
			"%{DependencyDir}GLFW/src/win32_joystick.c",
			"%{DependencyDir}GLFW/src/win32_monitor.c",
			"%{DependencyDir}GLFW/src/win32_time.h",
			"%{DependencyDir}GLFW/src/win32_time.c",
			"%{DependencyDir}GLFW/src/win32_thread.h",
			"%{DependencyDir}GLFW/src/win32_thread.c",
			"%{DependencyDir}GLFW/src/win32_window.c",
			"%{DependencyDir}GLFW/src/wgl_context.c",
			"%{DependencyDir}GLFW/src/egl_context.c",
			"%{DependencyDir}GLFW/src/osmesa_context.c"
			}

		defines
		{
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "system:linux"
		pic "On" -- Position Independent Code

		files
		{
			"%{DependencyDir}GLFW/src/x11_init.c",
			"%{DependencyDir}GLFW/src/x11_monitor.c",
			"%{DependencyDir}GLFW/src/x11_window.c",
			"%{DependencyDir}GLFW/src/xkb_unicode.c",
			"%{DependencyDir}GLFW/src/posix_time.c",
			"%{DependencyDir}GLFW/src/posix_thread.c",
			"%{DependencyDir}GLFW/src/glx_context.c",
			"%{DependencyDir}GLFW/src/egl_context.c",
			"%{DependencyDir}GLFW/src/osmesa_context.c",
			"%{DependencyDir}GLFW/src/linux_joystick.c"
		}

		defines { "_GLFW_X11" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		runtime  "Release"
		optimize "On"