#version 330 core
#include "ASSET_DIR/Shaders/Include/Material.inc"

layout (location = 0) out vec4 gPositionRoughness;
layout (location = 1) out vec4 gNormalMetalness;
layout (location = 2) out vec3 gAlbedo;

in vec2 TexCoords;
in vec3 WorldPos;
in mat3 TBN; // Tangent, Bitangent, Normal

void main()
{
	// Albedo
	gAlbedo = material.AlbedoColour.rgb;
	if(material.HasAlbedoMap)
		gAlbedo = texture(material.AlbedoMap, TexCoords).rgb;

	// Position
	gPositionRoughness.rgb = WorldPos;

	// Roughness
	gPositionRoughness.a = material.Roughness;
	if(material.HasRoughnessMap)
		gPositionRoughness.a = texture(material.RoughnessMap, TexCoords).r;

	// Metalness
	gNormalMetalness.a = material.Metalness;
	if(material.HasMetalnessMap)
		gNormalMetalness.a = texture(material.MetalnessMap, TexCoords).r;

	// Normals
	gNormalMetalness.rgb = TBN[2];
	if(material.HasNormalMap)
	{
		gNormalMetalness.rgb = texture(material.NormalMap, TexCoords).rgb;
		gNormalMetalness.rgb = gNormalMetalness.rgb * 2.0 - 1.0; // Remap to range [-1, 1]
		gNormalMetalness.rgb = normalize(TBN * gNormalMetalness.rgb);
	}
}
