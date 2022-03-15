#version 400 core
#include "#ASSET_DIR/Shaders/Include/Tonemap.inc"

out vec4 FragColour;

in vec3 WorldPos;
in vec2 TexCoords;

uniform samplerCube environmentMap;

void main()
{
	vec3 envColour = texture(environmentMap, WorldPos).rgb;

	// TODO: Tonemap ??

	FragColour = vec4(envColour, 1);
}