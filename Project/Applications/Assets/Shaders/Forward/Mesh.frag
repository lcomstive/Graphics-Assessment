#version 330 core
#include "ASSET_DIR/Shaders/Include/Camera.inc"
#include "ASSET_DIR/Shaders/Include/Light.inc"
#include "ASSET_DIR/Shaders/Include/Material.inc"
#include "ASSET_DIR/Shaders/Include/PBR.inc"

out vec4 FragColour;

in vec2 TexCoords;
in vec3 WorldPos;
in mat3 TBN; // Tangent, Bitangent, Normal

void main()
{
	PBRInput input;

	// Normals
	input.Normals = TBN[2];
	if(material.HasNormalMap)
	{
		input.Normals = texture(material.NormalMap, TexCoords).rgb;
		input.Normals = input.Normals * 2.0 - 1.0; // Remap to range [-1, 1]
		input.Normals = normalize(TBN * input.Normals); // Tangent-to-World space
	}

	// World Pos
	input.WorldPos = WorldPos;

	// Albedo
	input.Albedo = material.AlbedoColour.rgb;
	if(material.HasAlbedoMap)
		input.Albedo = texture(material.AlbedoMap, TexCoords).rgb;

	// Metalness
	input.Metalness = material.Metalness;
	if(material.HasMetalnessMap)
		input.Metalness = texture(material.MetalnessMap, TexCoords).r;

	// Roughness
	input.Roughness = material.Roughness;
	if(material.HasRoughnessMap)
		input.Roughness = texture(material.RoughnessMap, TexCoords).r;

	// Calculate lighting
	FragColour.rgb = PBRLighting(input);
	FragColour.a = material.AlbedoColour.a;
	
	// Check for alpha clipping
	if(material.AlphaClipping && FragColour.a <= material.AlphaClipThreshold)
		discard;
}