project "Glad"
	kind "StaticLib"
	language "C"
	staticruntime "Off"

	targetdir ("%{OutputDir}%{prj.name}")
	objdir ("%{OutputDir}Intermediate/%{prj.name}")
  
	files
	{
		"%{DependencyDir}glad/include/glad/glad.h",
		"%{DependencyDir}glad/include/KHR/khrplatform.h",
		"%{DependencyDir}glad/src/glad.c"
	}

	includedirs { "%{DependencyDir}glad/include" }

	filter "system:windows"
		systemversion "latest" -- Windows SDK

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		runtime  "Release"
		optimize "On"