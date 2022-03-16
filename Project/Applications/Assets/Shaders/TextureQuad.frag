#version 330 core
#include "#ASSET_DIR/Shaders/Include/Multisample.inc"

out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D inputTexture;
uniform sampler2DMS inputTexture;

void main()
{
	FragColour = texture(inputTexture, TexCoords);
	if(samples > 1)
		FragColour = TextureMultisample(inputTexture, TexCoords);
}