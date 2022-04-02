#version 330 core
#include "#ASSET_DIR/Shaders/Include/Camera.inc"
#include "#ASSET_DIR/Shaders/Include/Material.inc"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

// x = Albedo Map
// y = Normal Map
// z = Roughness Map
// w = Metalness Map
layout(location = 5) in vec4 TextureIndices;
// layout(location = 6) in mat4 ModelMatrix;

uniform mat4 modelMatrix;

#if #SUPPORTS_TESSELLATION
#define TBN_NAME TBN_Tess
#define WORLDPOS_NAME WorldPos_Tess
#define TEXCOORDS_NAME TexCoords_Tess
#define TEXINDICES_NAME TexIndices_Tess
#else
#define TBN_NAME TBN
#define WORLDPOS_NAME WorldPos
#define TEXCOORDS_NAME TexCoords
#define TEXINDICES_NAME TexIndices
#endif

out mat3 TBN_NAME; // Tangent, Bitangent, Normal
out vec3 WORLDPOS_NAME;
out vec2 TEXCOORDS_NAME;
out vec4 TEXINDICES_NAME;

void main()
{
	mat4 ModelMatrix = modelMatrix;
	vec3 T = normalize(vec3(ModelMatrix * vec4(tangent,   0.0)));
	vec3 B = normalize(vec3(ModelMatrix * vec4(bitangent, 0.0)));
	vec3 N = normalize(vec3(ModelMatrix * vec4(normals,   0.0)));
	TBN_NAME = mat3(T, B, N);

	TEXINDICES_NAME = TextureIndices;
	WORLDPOS_NAME = vec3(ModelMatrix * vec4(position, 1.0));
	TEXCOORDS_NAME = (texCoords * material.TextureCoordScale) + material.TextureCoordOffset;

#if !#SUPPORTS_TESSELLATION
	gl_Position = camera.ProjectionMatrix * camera.ViewMatrix * modelMatrix * vec4(position, 1.0);
#endif
}