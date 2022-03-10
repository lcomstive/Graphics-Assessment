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
	sampler2D AmbientOcclusionMap;

	bool HasAlbedoMap;
	bool HasNormalMap;
	bool HasRoughnessMap;
	bool HasMetalnessMap;
	bool HasAmbientOcclusionMap;

	bool Tessellate;
	int TessellationDistances[2];
};

uniform Material material;

#endif