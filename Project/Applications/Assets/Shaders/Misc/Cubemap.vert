#version 400 core

layout(location = 0) in vec3 position;

out vec3 WorldPos;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	WorldPos = position;
	gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
}