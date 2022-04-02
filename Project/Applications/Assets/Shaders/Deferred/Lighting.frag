#version 330 core
#include "#ASSET_DIR/Shaders/Include/PBR.inc"
#include "#ASSET_DIR/Shaders/Include/Denoise.inc"
#include "#ASSET_DIR/Shaders/Include/Material.inc"

out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D inputDepth;
uniform sampler2D inputAlbedo;
uniform sampler2D inputNormalMetalness;
uniform sampler2D inputPositionRoughness;

void main()
{
	vec3 albedo = texture(inputAlbedo, TexCoords).rgb;
	vec4 normalMetalness = texture(inputNormalMetalness, TexCoords);
	vec4 positionRoughness = texture(inputPositionRoughness, TexCoords);

	PBRInput input;

	input.Albedo = albedo;
	input.Normals = normalMetalness.rgb;
	input.Metalness = normalMetalness.a;
	input.WorldPos = positionRoughness.rgb;
	input.Roughness = positionRoughness.a;

	FragColour = vec4(PBRLighting(input), 1.0);
}