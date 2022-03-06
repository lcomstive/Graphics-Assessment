project "Demo"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{OutputDir}%{prj.name}")
	objdir ("%{OutputDir}Intermediate/%{prj.name}")
	
	files
	{
		"Include/**.hpp",
		"Source/**.cpp"
	}

	includedirs
	{
		"Include",
		"%{IncludeDir.STB}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGUI}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.termcolor}",
		"%{IncludeDir.termcolor}",
		"%{wks.location}/../Engine/Include"
	}

	links
	{
		"Engine",
		"Glad",
		"GLFW",
		"ImGUI",
		"Assimp"
	}

	filter "system:windows"
		systemversion "latest" -- Windows SDK Version
		debugdir "%{OutputDir}%{prj.name}"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

		postbuildcommands
		{
			"{COPYDIR} \"%{wks.location}../Applications/Assets\" \"%{OutputDir}%{prj.name}/Assets/\""
		}