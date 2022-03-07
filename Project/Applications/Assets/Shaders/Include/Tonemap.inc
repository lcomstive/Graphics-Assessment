uniform float gamma = 2.2;
uniform float exposure = 1.0;

vec3 Expose(vec3 input) { return vec3(1.0) - exp(-input * exposure); }
vec3 ReinhardTonemap(vec3 input) { return input / (input + vec3(1.0)); }

vec3 GammaCorrect(vec3 input) { return pow(input, vec3(1.0 / gamma)); }

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 AcesTonemap(vec3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}