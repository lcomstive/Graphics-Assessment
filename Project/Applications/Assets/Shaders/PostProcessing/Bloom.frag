#version 330 core
#include "#ASSET_DIR/Shaders/Include/Tonemap.inc"

out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D input;

void main()
{
	vec3 colour = texture(input, TexCoords).rgb;
	// TODO: Bloom
	FragColour = vec4(colour, 1.0);
}