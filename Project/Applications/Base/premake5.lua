project "Application Base"
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

	defines(EngineDefines)

	local fileExt = ''
	local dllExt = '.so'

	filter "system:windows"
		systemversion "latest" -- Windows SDK Version
		fileExt = '.exe'
		dllExt = '.dll'

	filter { "system:windows", "configurations:Debug" }
		debugdir "%{OutputDir}%{prj.name}"

	filter { "system:windows", "configurations:Release" }
		kind "WindowedApp"
		debugdir(OutputDir .. "Binaries/")

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

		if DependencyType == 'SharedLib' then
			-- Shared library binaries
			postbuildcommands
			{
				"{COPYFILE} \"%{OutputDir}Glad/Glad" .. dllExt .. "\" \"%{OutputDir}%{prj.name}/Glad" .. dllExt .. "\"",
				"{COPYFILE} \"%{OutputDir}GLFW/GLFW" .. dllExt .. "\" \"%{OutputDir}%{prj.name}/GLFW" .. dllExt .. "\"",
				"{COPYFILE} \"%{OutputDir}ImGUI/ImGUI" .. dllExt .. "\" \"%{OutputDir}%{prj.name}/ImGUI" .. dllExt .. "\""
			}
		end

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

		postbuildcommands
		{
			"{MKDIR} \"" .. OutputDir .. "Binaries/\"",
			"{COPYFILE} \"" .. OutputDir .. "%{prj.name}/%{prj.name}" .. fileExt .. "\" \"" .. OutputDir .. "Binaries/%{prj.name}" .. fileExt .. "\"",
		}
		
		for i, directory in ipairs({
			"Fonts",
			"Models",
			"Shaders",
			"Textures"
		}) do
			postbuildcommands("{COPYDIR} \"%{wks.location}../Applications/Assets/" .. directory .. "\" \"" .. OutputDir .. "Binaries/Assets/" .. directory .. "\"")
		end

		if EngineType == 'SharedLib' then
			postbuildcommands("{COPYFILE} \"%{OutputDir}Engine/Engine" .. dllExt .. "\" \"%{OutputDir}Binaries/Engine" .. dllExt .. "\"")
		end

		if DependencyType == 'SharedLib' then
			-- Shared library binaries
			postbuildcommands
			{
				"{COPYFILE} \"%{OutputDir}Glad/Glad" .. dllExt .. "\" \"%{OutputDir}Binaries/Glad" .. dllExt .. "\"",
				"{COPYFILE} \"%{OutputDir}GLFW/GLFW" .. dllExt .. "\" \"%{OutputDir}Binaries/GLFW" .. dllExt .. "\"",
				"{COPYFILE} \"%{OutputDir}ImGUI/ImGUI" .. dllExt .. "\" \"%{OutputDir}Binaries/ImGUI" .. dllExt .. "\""
			}
		end

		filter {}

	if EngineType == 'SharedLib' then
		defines { "BUILD_SHARED_LIB" }

		-- Engine Shared Binary
		postbuildcommands
		{
			"{COPYFILE} \"%{OutputDir}Engine/Engine" .. dllExt .. "\" \"%{OutputDir}%{prj.name}/Engine" .. dllExt .. "\""
		}

		filter "system:windows"
			-- Engine debug symbols
			postbuildcommands
			{
				"{COPYFILE} \"%{OutputDir}Engine/Engine.pdb\" \"%{OutputDir}%{prj.name}/Engine.pdb\""
			}
		filter {}
	end

	
