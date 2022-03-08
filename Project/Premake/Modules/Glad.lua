project "Glad"
	kind(DependencyType)
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

	if DependencyType == "SharedLib" then
		defines { "GLAD_GLAPI_EXPORT", "GLAD_GLAPI_EXPORT_BUILD" }
	end

	filter "system:windows"
		systemversion "latest" -- Windows SDK

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		runtime  "Release"
		optimize "On"