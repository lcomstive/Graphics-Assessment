#version 400 core
#include "#ASSET_DIR/Shaders/Include/PBRFunctions.inc"

out vec2 FragColour;

in vec2 TexCoords;

void main()
{
	FragColour = IntegrateBRDF(TexCoords.x, TexCoords.y);
}