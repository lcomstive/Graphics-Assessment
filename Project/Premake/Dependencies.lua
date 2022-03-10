DependencyDir = "%{wks.location}/../Dependencies/"

IncludeDir = {}
IncludeDir["STB"] = "%{DependencyDir}stb"
IncludeDir["GLM"] = "%{DependencyDir}glm"
IncludeDir["ImGUI"] = "%{DependencyDir}imgui"
IncludeDir["Glad"] = "%{DependencyDir}glad/include"
IncludeDir["GLFW"] = "%{DependencyDir}glfw/include"
IncludeDir["string_id"] = "%{DependencyDir}string-id/src/lib/sid"
IncludeDir["Assimp"] = "%{DependencyDir}assimp/include"
IncludeDir["termcolor"] = "%{DependencyDir}termcolor/include"
IncludeDir["robin-hood-hash"] = "%{DependencyDir}robin-hood-hashing/src/include"

group "Dependencies"
	include "./Modules/Glad.lua"
	include "./Modules/GLFW.lua"
	include "./Modules/ImGUI.lua"
	include "./Modules/Assimp.lua"
group ""