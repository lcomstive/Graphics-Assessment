#version 400 core
#include "#ASSET_DIR/Shaders/Include/PBRFunctions.inc"

out vec4 FragColour;

in vec3 WorldPos;

uniform float roughness;
uniform vec2 resolution;
uniform samplerCube environmentMap;

void main()
{
	vec3 N = normalize(WorldPos);
	vec3 R = N;
	vec3 V = R;

	const uint SampleCount = 1024u;
	float totalWeight = 0;
	vec3 prefilteredColour = vec3(0);
	for(uint i = 0u; i < SampleCount; i++)
	{
		vec2 xi = Hammersley(i, SampleCount);
		vec3 H = ImportanceSampleGGX(xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if(NdotL > 0)
		{
			// Sample environment's mip level based on roughness
			float D = DistributionGGX(N, H, roughness);
			float NdotH = max(dot(N, H), 0);
			float HdotV = max(dot(H, V), 0);
			float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

			float saTexel = 4.0 * PBR_PI / (6.0 * resolution.x * resolution.y);
			float saSample = 1.0 / (float(SampleCount) * pdf + 0.0001);

			float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

			prefilteredColour += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	prefilteredColour /= totalWeight;

	FragColour = vec4(prefilteredColour, 1.0);
}