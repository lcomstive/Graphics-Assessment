project "ImGUI"
	language "C++"
	kind "StaticLib"
	cppdialect "C++17"

	targetdir ("%{OutputDir}%{prj.name}")
	objdir ("%{OutputDir}Intermediate/%{prj.name}")

	files
	{
		"%{DependencyDir}imgui/*.h",
		"%{DependencyDir}imgui/*.cpp",
		"%{DependencyDir}imgui/backends/imgui_impl_glfw.h",
		"%{DependencyDir}imgui/backends/imgui_impl_glfw.cpp",
		"%{DependencyDir}imgui/backends/imgui_impl_opengl3.h",
		"%{DependencyDir}imgui/backends/imgui_impl_opengl3.cpp"
	}

	includedirs
	{
		"%{DependencyDir}imgui",
		"%{DependencyDir}glfw/include"
	}

	filter "system:windows"
		systemversion "latest" -- Windows SDK

		files
		{
			"%{DependencyDir}imgui/backends/imgui_impl_win32.h",
			"%{DependencyDir}imgui/backends/imgui_impl_win32.cpp"
		}

	filter "system:linux"
		pic "On" -- Position Independent Code

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		runtime  "Release"
		optimize "On"