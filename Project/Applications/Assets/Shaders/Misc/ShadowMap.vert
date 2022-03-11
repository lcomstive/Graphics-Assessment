#version 330 core
const int MaxLightViews = 1;

layout(location = 0) in vec3 position;

uniform mat4 modelMatrix;
uniform int lightSpaceIndex;

struct Light { mat4 LightSpaceMatrix[MaxLightViews]; };
uniform Light lights[1];

void main()
{
	// Only one light is filled into the lights array, so we can use index 0
	gl_Position = lights[0].LightSpaceMatrix[lightSpaceIndex] * modelMatrix * vec4(position, 1.0);
}