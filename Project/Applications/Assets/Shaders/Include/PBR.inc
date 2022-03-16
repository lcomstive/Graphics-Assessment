#ifndef _INCLUDE_PBR_
#define _INCLUDE_PBR_
#include "#ASSET_DIR/Shaders/Include/Light.inc"
#include "#ASSET_DIR/Shaders/Include/Camera.inc"
#include "#ASSET_DIR/Shaders/Include/PBRFunctions.inc"

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
	vec3 R = reflect(-viewDir, input.Normals);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, input.Albedo, input.Metalness);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < min(lightCount, MaxLights); i++)
	{
		float dist = length(lights[i].Position - input.WorldPos);
		
		if((lights[i].Type == LIGHT_POINT && dist > lights[i].Radius))
			continue;
		
		vec3 L = normalize(lights[i].Position - input.WorldPos);
		vec3 H = normalize(viewDir + L);

		if(lights[i].Type == LIGHT_DIRECTIONAL)
			dist = 0.0;

		float spotlightTheta = dot(L, normalize(-lights[i].Direction));
		if(lights[i].Type == LIGHT_SPOT &&
			(spotlightTheta < cos(radians(lights[i].Radius))) ||
			(dist > lights[i].Distance))
			continue;

		const float Linear = 0.7;
		const float Quadratic = 1.8;

		float attenuation = 1.0 / (1.0 + Linear * dist + Quadratic * dist * dist);

		if(lights[i].Type != LIGHT_DIRECTIONAL)
		{
			float epsilon = lights[i].FadeCutoffInner - lights[i].Radius;
			attenuation = clamp((spotlightTheta - lights[i].Radius) / epsilon, 0.0, 1.0);
		}

		vec3 radiance = lights[i].Colour * attenuation * lights[i].Intensity;

		float NDF = DistributionGGX(input.Normals, H, input.Roughness);
		vec3 F = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);
		float G = GeometrySmith(input.Normals, viewDir, L, input.Roughness);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(input.Normals, viewDir), 0.0) * max(dot(input.Normals, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		float shadow = 0.0; // No shadow
		
		if(lights[i].CastShadows)
			shadow = GetShadowAmount(i, input.Normals, input.WorldPos);
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - input.Metalness;

		float NdotL = max(dot(input.Normals, L), 0.0);

		Lo += ((kD * input.Albedo / PBR_PI + specular) * radiance * NdotL) * (1.0 - shadow);
	}
	
	// Calculate ambient lighting
	vec3 F = FresnelSchlickRoughness(max(dot(input.Normals, viewDir), 0.0), F0, input.Roughness);
	vec3 kD = 1.0 - F;
	kD *= 1.0 - input.Metalness;

	vec3 irradiance = texture(environment.IrradianceMap, input.Normals).rgb;
	vec3 diffuse = irradiance * input.Albedo;

	const float MaxReflectionLOD = 1;
	vec3 prefilteredColour = textureLod(environment.PrefilterMap, R, input.Roughness * MaxReflectionLOD).rgb;
	vec2 brdf = texture(environment.BRDFMap, vec2(max(dot(input.Normals, viewDir), 0.0), input.Roughness)).rg;
	vec3 specular = prefilteredColour * (F * brdf.x + brdf.y);
	
	vec3 ambient = (kD * diffuse + specular) * environment.AmbientLightStrength;
	vec3 colour = ambient + Lo;

	return colour;
}

#endif