#version 330 core
#include "#ASSET_DIR/Shaders/Include/Material.inc"
#include "#ASSET_DIR/Shaders/Include/PBR.inc"

out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D inputPositionRoughness;
uniform sampler2D inputNormalMetalness;
uniform sampler2D inputAlbedo;
uniform sampler2D inputDepth;

void main()
{
	vec4 positionRoughness = texture(inputPositionRoughness, TexCoords);
	vec4 normalMetalness = texture(inputNormalMetalness, TexCoords);

	PBRInput input;

	input.Albedo = texture(inputAlbedo, TexCoords).rgb;
	input.Normals = normalMetalness.rgb;
	input.Metalness = normalMetalness.a;
	input.WorldPos = positionRoughness.rgb;
	input.Roughness = positionRoughness.a;

	FragColour = vec4(PBRLighting(input), 1.0);

	// FragColour.rgb = vec3(texture(shadowMap, vec3(TexCoords, 0)).r);
}