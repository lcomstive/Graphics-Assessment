#version 330 core
#include "ASSET_DIR/Shaders/Include/Tonemap.inc"

out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D input;

void main()
{
	FragColour = vec4(GammaCorrect(texture(input, TexCoords).rgb), 1.0);
}