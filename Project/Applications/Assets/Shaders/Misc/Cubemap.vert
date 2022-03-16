#version 400 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoords;

out vec3 WorldPos;
out vec2 TexCoords;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	TexCoords = texCoords;
	WorldPos = position;
	gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
}