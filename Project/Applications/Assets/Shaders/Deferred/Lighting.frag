#version 330 core
#include "ASSET_DIR/Shaders/Include/Camera.inc"
#include "ASSET_DIR/Shaders/Include/Light.inc"
#include "ASSET_DIR/Shaders/Include/Material.inc"
#include "ASSET_DIR/Shaders/Include/PBR.inc"

out vec4 fragColour;

in vec2 passTexCoords;

uniform sampler2D inputPositionRoughness;
uniform sampler2D inputNormalMetalness;
uniform sampler2D inputAlbedo;
uniform sampler2D inputDepth;

void main()
{
	vec4 positionRoughness = texture(inputPositionRoughness, passTexCoords);
	vec4 normalMetalness = texture(inputNormalMetalness, passTexCoords);

	PBRInput input;

	input.Albedo = texture(inputAlbedo, passTexCoords).rgb;
	input.Normals = normalMetalness.rgb;
	input.Metalness = normalMetalness.a;
	input.WorldPos = positionRoughness.rgb;
	input.Roughness = positionRoughness.a;

	fragColour = vec4(PBRLighting(input), 1.0);
}