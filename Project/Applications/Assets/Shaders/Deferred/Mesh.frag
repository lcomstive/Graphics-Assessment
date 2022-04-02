#version 330 core

#ifndef _INCLUDE_MATERIAL_
#define _INCLUDE_MATERIAL_

struct Material
{
	vec4  AlbedoColour;
	float Roughness;
	float Metalness;

	bool  AlphaClipping;
	float AlphaClipThreshold;

	vec2 TextureCoordScale;
	vec2 TextureCoordOffset;

	bool Transparent;

	sampler2D AlbedoMap;
	sampler2D NormalMap;
	sampler2D RoughnessMap;
	sampler2D MetalnessMap;
	// sampler2D AmbientOcclusionMap;

	bool HasAlbedoMap;
	bool HasNormalMap;
	bool HasRoughnessMap;
	bool HasMetalnessMap;
	// bool HasAmbientOcclusionMap;

	bool Tessellate;
	int TessellationDistances[2];
};

uniform Material material;

#endif

layout (location = 0) out vec4 gPositionRoughness;
layout (location = 1) out vec4 gNormalMetalness;
layout (location = 2) out vec3 gAlbedo;

in vec2 TexCoords;
in vec3 WorldPos;
in mat3 TBN; // Tangent, Bitangent, Normal
in vec4 TexIndices;

void main()
{
	// Albedo
	vec4 albedo = material.AlbedoColour * texture(material.AlbedoMap, TexCoords);
	if(material.AlphaClipping && albedo.a < material.AlphaClipThreshold)
		discard;
	gAlbedo = albedo.rgb;

	// Position
	gPositionRoughness.rgb = WorldPos;

	// Roughness
	gPositionRoughness.a = material.Roughness * texture(material.RoughnessMap, TexCoords).r;

	// Metalness
	gNormalMetalness.a = material.Metalness * texture(material.MetalnessMap, TexCoords).r;

	// Normals
	gNormalMetalness.rgb = TBN[2];
	if(material.HasNormalMap)
	{
		gNormalMetalness.rgb = texture(material.NormalMap, TexCoords).rgb;
		gNormalMetalness.rgb = gNormalMetalness.rgb * 2.0 - 1.0; // Remap to range [-1, 1]
		gNormalMetalness.rgb = normalize(TBN * gNormalMetalness.rgb);
	}
}
