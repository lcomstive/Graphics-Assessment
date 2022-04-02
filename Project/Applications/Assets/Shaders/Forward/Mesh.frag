#version 330 core
#include "#ASSET_DIR/Shaders/Include/Material.inc"
#include "#ASSET_DIR/Shaders/Include/PBR.inc"

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
	vec4 albedo = material.AlbedoColour * texture(material.AlbedoMap, TexCoords);
	input.Albedo = albedo.rgb;

	// Metalness
	input.Metalness = material.Metalness * texture(material.MetalnessMap, TexCoords).r;

	// Roughness
	input.Roughness = material.Roughness * texture(material.RoughnessMap, TexCoords).r;

	// Calculate lighting
	FragColour.rgb = PBRLighting(input);
	FragColour.a = albedo.a;
		
	// Check for alpha clipping
	if(material.AlphaClipping && FragColour.a <= material.AlphaClipThreshold)
		discard;
}