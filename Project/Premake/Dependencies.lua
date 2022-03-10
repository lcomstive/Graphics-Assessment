DependencyDir = "%{wks.location}/../Dependencies/"

IncludeDir = {}
IncludeDir["STB"] = "%{DependencyDir}stb"
IncludeDir["GLM"] = "%{DependencyDir}glm"
IncludeDir["ImGUI"] = "%{DependencyDir}imgui"
IncludeDir["Glad"] = "%{DependencyDir}glad/include"
IncludeDir["GLFW"] = "%{DependencyDir}glfw/include"
IncludeDir["Assimp"] = "%{DependencyDir}assimp/include"
IncludeDir["termcolor"] = "%{DependencyDir}termcolor/include"
IncludeDir["robin-hood-hash"] = "%{DependencyDir}robin-hood-hashing/src/include"

group "Dependencies"
	include "./Modules/Assimp.lua"
	include "./Modules/Glad.lua"
	include "./Modules/GLFW.lua"
	include "./Modules/ImGUI.lua"
group ""