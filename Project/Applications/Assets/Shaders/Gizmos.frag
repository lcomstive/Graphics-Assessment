#version 330 core
#include "#ASSET_DIR/Shaders/Include/Material.inc"

out vec4 FragColour;

void main()
{
	FragColour = vec4(material.AlbedoColour);
}
