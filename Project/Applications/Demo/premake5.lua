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
		"%{wks.location}/../Engine/Include"
	}

	links
	{
		"Engine"
	}

	local fileExt = ''

	filter "system:windows"
		systemversion "latest" -- Windows SDK Version
		fileExt = '.exe'

	filter { "system:windows", "configurations:Debug" }
		debugdir "%{OutputDir}%{prj.name}"

	filter { "system:windows", "configurations:Release" }
		kind "WindowedApp"
		debugdir(OutputDir .. "Binaries/")

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

		postbuildcommands
		{
			"{MKDIR} \"" .. OutputDir .. "Binaries/\"",
			"{COPYFILE} \"" .. OutputDir .. "%{prj.name}/%{prj.name}" .. fileExt .. "\" \"" .. OutputDir .. "Binaries/%{prj.name}" .. fileExt .. "\"",
			"{COPYDIR} \"%{wks.location}../Applications/Assets/\" \"" .. OutputDir .. "Binaries/Assets/\""
		}
