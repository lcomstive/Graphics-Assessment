project "Engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("%{OutputDir}%{prj.name}")
	objdir ("%{OutputDir}Intermediate/%{prj.name}")

	files
	{
		"Include/**.hpp",
		"Source/**.cpp"
	}

	defines { "_CRT_SECURE_NO_WARNINGS" }

	includedirs
	{
		"Include",
		"%{IncludeDir.STB}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGUI}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.termcolor}"
	}

	links
	{
		"Glad",
		"GLFW",
		"ImGUI",
		"Assimp"
	}

	filter "system:windows"
		systemversion "latest" -- Windows SDK Version

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

	filter "kind:SharedLib"
		defines { "BUILD_SHARED_LIB", "EXPORT_DLL" }