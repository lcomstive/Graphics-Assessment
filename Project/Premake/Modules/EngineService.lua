function CreateEngineService(name, copyToOutput)
	copyToOutput = copyToOutput or true
	
	project(name)
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{OutputDir}%{prj.name}")
	objdir ("%{OutputDir}Intermediate/%{prj.name}")
	
	includedirs
	{
		IncludeDir,
		"%{wks.location}/../Engine/Include"
	}

	defines
	{
		EngineDefines,
		"BUILD_SERVICE_DLL"
	}

	links { EngineLinkLibs }

	local fileExt = '.so'

	if EngineType == "SharedLib" then
		defines { "BUILD_SHARED_LIB" }
	end

	filter "system:windows"
		systemversion "latest" -- Windows SDK Version
		debugdir "%{OutputDir}%{prj.name}"
		fileExt = '.dll'

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

		-- Copy output file to Assets directory
		if copyToOutput then
			postbuildcommands
			{
				"{MKDIR} \"%{wks.location}../Applications/Assets/Services/\" >nul",
				"{COPYFILE} \"" .. OutputDir .. "%{prj.name}/%{prj.name}" .. fileExt .. "\" \"%{wks.location}../Applications/Assets/Services/%{prj.name}" .. fileExt .. "\""
			}
		end

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

		-- Copy output file to Assets directory
		if copyToOutput then
			postbuildcommands
			{
				"{MKDIR} \"" .. OutputDir .. "Binaries/Assets/Services/\" >nul",
				"{COPYFILE} \"" .. OutputDir .. "%{prj.name}/%{prj.name}" .. fileExt .. "\" \"" .. OutputDir .. "Binaries/Assets/Services/%{prj.name}" .. fileExt .. "\""
			}
		end

	filter {}
end