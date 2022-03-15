#version 400 core
#include "#ASSET_DIR/Shaders/Include/Camera.inc"

layout(location = 0) in vec3 position;

out vec3 WorldPos;

void main()
{
	WorldPos = position;
	
	mat4 rotView = mat4(mat3(camera.ViewMatrix)); // Remove translation from viewmatrix
	vec4 clipPos = camera.ProjectionMatrix * rotView * vec4(position, 1.0);

	gl_Position = clipPos.xyww;
}