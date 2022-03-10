project "Engine"
	kind(EngineType)
	language "C++"
	cppdialect "C++17"

	targetdir ("%{OutputDir}%{prj.name}")
	objdir ("%{OutputDir}Intermediate/%{prj.name}")

	files
	{
		"Include/**.hpp",
		"Source/**.cpp",
	}

	defines
	{
		EngineDefines,
		"_CRT_SECURE_NO_WARNINGS",
	}

	includedirs
	{
		"Include",
		IncludeDir
	}

	links
	{
		"Glad",
		"GLFW",
		"ImGUI",
		"Assimp"
	}

	if DependencyType == 'SharedLib' then
		defines { "IMGUI_API=__declspec(dllimport)" }
	end

	filter "system:windows"
		systemversion "latest" -- Windows SDK Version

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

	filter "kind:SharedLib"
		defines { "BUILD_SHARED_LIB", "ENGINE_EXPORT_DLL" }
