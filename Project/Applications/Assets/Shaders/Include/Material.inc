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
};

uniform Material material;