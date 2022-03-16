#version 400 core
#include "#ASSET_DIR/Shaders/Include/Tonemap.inc"

out vec4 FragColour;

in vec3 WorldPos;
in vec2 TexCoords;

struct Environment
{
	sampler2D BRDFMap;
	samplerCube PrefilterMap;
	samplerCube EnvironmentMap;
};

uniform Environment environment;

void main()
{
	vec3 envColour = texture(environment.EnvironmentMap, WorldPos).rgb;
	FragColour = vec4(envColour, 1);
}