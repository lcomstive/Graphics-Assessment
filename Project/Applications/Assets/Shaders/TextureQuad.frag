#version 330 core
out vec4 fragColour;

in vec2 TexCoords;

uniform sampler2D inputTexture;

void main()
{
	fragColour = texture(inputTexture, TexCoords);
}