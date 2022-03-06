workspace "Graphics Engine"
	location "../Build"
	architecture "x86_64"
	configurations { "Debug", "Release" }

	startproject "Demo"

	flags { "MultiProcessorCompile" }

OutputDir = "%{wks.location}/%{cfg.buildcfg}/"

include "./Dependencies.lua"
include "../Engine"
include "../Applications/Demo"
-- include "../Applications/Editor"