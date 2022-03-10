#version 330 core
#include "ASSET_DIR/Shaders/Include/Camera.inc"
#include "ASSET_DIR/Shaders/Include/Material.inc"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out mat3 TBN; // Tangent, Bitangent, Normal
out vec3 WorldPos;
out vec2 TexCoords;

uniform mat4 modelMatrix;

void main()
{
	vec3 T = normalize(vec3(modelMatrix * vec4(tangent,   0.0)));
	vec3 B = normalize(vec3(modelMatrix * vec4(bitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * vec4(normals,   0.0)));
	TBN = mat3(T, B, N);

	WorldPos = vec3(modelMatrix * vec4(position, 1.0));
	TexCoords = (texCoords * material.TextureCoordScale) + material.TextureCoordOffset;

	gl_Position = camera.ProjectionMatrix * camera.ViewMatrix * modelMatrix * vec4(position, 1.0);
}