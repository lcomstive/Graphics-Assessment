#ifndef _INCLUDE_PBR_
#define _INCLUDE_PBR_
#include "#ASSET_DIR/Shaders/Include/Light.inc"
#include "#ASSET_DIR/Shaders/Include/Camera.inc"

const float PBR_PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PBR_PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

struct PBRInput
{
	vec3 Albedo;
	float Metalness;
	float Roughness;

	vec3 Normals; // Normalized pls
	vec3 WorldPos;
};

vec3 PBRLighting(PBRInput input)
{
	vec3 viewDir = normalize(camera.Position - input.WorldPos);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, input.Albedo, input.Metalness);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		float dist = length(lights[i].Position - input.WorldPos);

		if(lights[i].Type == LIGHT_DIRECTIONAL)
			dist = 0.0;
		
		if(lights[i].Type != LIGHT_DIRECTIONAL && dist > lights[i].Radius)
			continue;
		
		vec3 L = normalize(lights[i].Position - input.WorldPos);
		vec3 H = normalize(viewDir + L);

		if(lights[i].Type == LIGHT_DIRECTIONAL)
			dist = 0.0;

		const float Linear = 0.7;
		const float Quadratic = 1.8;

		float attenuation = 1.0 / (1.0 + Linear * dist + Quadratic * dist * dist);
		// float attenuation = 1.0 / (dist * dist);

		vec3 radiance = lights[i].Colour * attenuation * lights[i].Intensity;

		float NDF = DistributionGGX(input.Normals, H, input.Roughness);
		vec3 F = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);
		float G = GeometrySmith(input.Normals, viewDir, L, input.Roughness);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(input.Normals, viewDir), 0.0) * max(dot(input.Normals, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		float shadow = 0.0; // No shadow
		
		if(lights[i].Type == LIGHT_DIRECTIONAL && lights[i].CastShadows)
			shadow = GetShadowAmount(i, 0, input.Normals, input.WorldPos);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - input.Metalness;

		float NdotL = max(dot(input.Normals, L), 0.0);
		Lo += (kD * input.Albedo / PBR_PI + specular) * radiance * NdotL * (1.0 - shadow);
	}
	
	vec3 ambient = vec3(0.001) * input.Albedo;
	vec3 colour = ambient + Lo;

	return colour;
}

#endif