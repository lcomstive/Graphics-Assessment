EngineType	   = "SharedLib" -- SharedLib / StaticLib
DependencyType = "SharedLib" -- SharedLib / StaticLib

EngineLinkLibs = { "Engine", "ImGUI", "Assimp", "GLFW", "Glad" }
include "./Modules/EngineService.lua"
EngineDefines = {}

if DependencyType == 'SharedLib' then
	EngineDefines =
	{
		EngineDefines,
		"GLAD_GLAPI_EXPORT",
		"GLFW_DLL",
		"IMGUI_API="
	}

	filter "system:windows"
		EngineDefines = { EngineDefines, "IMGUI_API=__declspec(dllimport)" }
end

workspace "Graphics Engine"
	location "../Build"
	architecture "x86_64"
	configurations { "Debug", "Release" }

	startproject "Application Base"

	flags { "MultiProcessorCompile" }

	OutputDir = "%{wks.location}/%{cfg.buildcfg}/"

	filter "configurations:Release"
		defines { "NDEBUG" }

	filter {}

	include "./Dependencies.lua"
	include "../Engine"

	group "Applications"
		include "../Applications/Demo"
		include "../Applications/Base"
		-- include "../Applications/Editor"
		include "../Applications/TestService"
	group ""