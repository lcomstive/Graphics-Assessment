uniform int samples;

vec4 TextureMultisample(sampler2DMS texture, ivec2 coord)
{
	coord = coord * textureSize(texture);
	vec4 colour = vec4(0);

	for(int i = 0; i < samples; i++)
		colour += texelFetch(texture, coord, i);
	
	return colour / float(samples);
}

vec4 TextureMultisample(sampler2DMS texture, vec2 coords) { return TextureMultisample(texture, ivec2(coords)); }