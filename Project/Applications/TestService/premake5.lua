project "Test Service"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

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

	defines { "BUILD_SHARED_LIB", "EXPORT_DLL" }

	links { "Engine", "ImGUI" }

	local fileExt = '.so'

	filter "system:windows"
		systemversion "latest" -- Windows SDK Version
		debugdir "%{OutputDir}%{prj.name}"
		fileExt = '.dll'

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

		-- Copy output file to Assets directory
		postbuildcommands
		{
			-- "{MKDIR} \"%{wks.location}../Applications/Assets/Services/\"",
			"{COPYFILE} \"%{OutputDir}%{prj.name}/%{prj.name}" .. fileExt .. "\" \"%{wks.location}../Applications/Assets/Services/%{prj.name}" .. fileExt .. "\""
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

		-- Copy output file to Assets directory
		postbuildcommands
		{
			"{MKDIR} \"%{OutputDir}Binaries/Assets/Services/\" >nul",
			"{COPYFILE} \"%{OutputDir}%{prj.name}/%{prj.name}" .. fileExt .. "\" \"%{OutputDir}Binaries/Assets/Services/%{prj.name}" .. fileExt .. "\""
		}

	filter {}
