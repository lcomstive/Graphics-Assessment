#version 330 core
#include "#ASSET_DIR/Shaders/Include/Tonemap.inc"

out vec4 FragColour;

in vec2 TexCoords;

uniform sampler2D inputTexture;

// 0 = None
// 1 = Aces
// 2 = Reinhard
uniform int tonemapper = 1;

void main()
{
	vec3 colour = texture(inputTexture, TexCoords).rgb;	
	switch(tonemapper)
	{
		default:
		case 0:
			// No tonemapping
			break;
		case 1:
			colour = AcesTonemap(colour);
			break;
		case 2:
			colour = ReinhardTonemap(colour);
			break;
	}
	
	// Expose
	colour = Expose(colour);

	// Gamma Correct
	colour = GammaCorrect(colour);

	FragColour = vec4(colour, 1.0);
}