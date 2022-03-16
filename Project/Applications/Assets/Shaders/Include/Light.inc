#ifndef _INCLUDE_LIGHT_
#define _INCLUDE_LIGHT_
#include "#ASSET_DIR/Shaders/Include/Camera.inc"

const int MaxLights 	= 32;
const int MaxLightViews = 1;

#define LIGHT_POINT 		0
#define LIGHT_SPOT 			1
#define LIGHT_DIRECTIONAL 	2

// LIGHTS //
struct Light
{
	int Type;
	float Radius;
	float Distance;
	float Intensity;
	bool CastShadows;
	float FadeCutoffInner;
	float FadeCutoffOuter;

	vec3 Colour;
	vec3 Position;
	vec3 Direction;

	int ShadowMapIndex;
	mat4 LightSpaceMatrix;
};

uniform int lightCount = 0;
uniform Light lights[MaxLights];

// SHADOWS //
uniform sampler2DArray shadowMap;

// ENVIRONMENT MAP //
struct Environment
{
	sampler2D BRDFMap;
	samplerCube IrradianceMap;
	samplerCube EnvironmentMap;
	samplerCube ReflectionMap;
	samplerCube PrefilterMap;

	float AmbientLightStrength;
};

uniform Environment environment;

// FUNCTIONS //
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // NDC

	z = (2.0 * camera.NearPlane * camera.FarPlane) /
			(camera.FarPlane + camera.NearPlane - z * (camera.FarPlane - camera.NearPlane));
	return z;
}

// Returns a value between [0.0-1.0] as a percentage value OUT of shadow.
// 1.0 = Not in shadow at all
// 0.0 = Completely covered by shadow
float GetShadowAmount(int lightIndex, vec3 surfaceNormal, vec3 fragPos)
{
	if(lights[lightIndex].ShadowMapIndex < 0)
		return 0.0; // Not in shadow
	vec4 lightSpaceCoords = lights[lightIndex].LightSpaceMatrix * vec4(fragPos, 1.0);
	vec3 normalDeviceCoords = lightSpaceCoords.xyz / lightSpaceCoords.w;
	normalDeviceCoords = normalDeviceCoords * 0.5 + 0.5; // Maps range [-1,1] to [0,1]

	if(normalDeviceCoords.z > 1.0)
		return 0.0; // Outside of texture coordinates, no shadow

	float shadow = 0.0;
	float fragDepth = normalDeviceCoords.z;
	float bias = max(0.005 * (1.0 - dot(surfaceNormal, lights[lightIndex].Direction)), 0.005);

#if 0
	// Percentage Closer Filtering
	vec3 texelSize = 1.0 / textureSize(shadowMap, 0).xyz;
	vec3 textureCoords = vec3(normalDeviceCoords.xy, lights[lightIndex].ShadowMapIndex);
	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(shadowMap, textureCoords + vec3(x, y, 0) * texelSize).r;
			shadow += fragDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
#else
	vec3 textureCoords = vec3(normalDeviceCoords.xy, lights[lightIndex].ShadowMapIndex);
	float currentDepth = texture(shadowMap, textureCoords).r;
	/*
	if(lights[lightIndex].Type != LIGHT_DIRECTIONAL)
	{
		fragDepth = LinearizeDepth(fragDepth);
		currentDepth = LinearizeDepth(currentDepth);
	}
	*/
	shadow = fragDepth - bias > currentDepth ? 1.0 : 0.0;
#endif

	return shadow; 
}

vec3 BlinnPhong(vec3 albedo, vec3 fragPosition, vec3 normals)
{
	vec3 lighting = albedo * vec3(0.1) * environment.AmbientLightStrength;
	vec3 viewDir = normalize(camera.Position - fragPosition);
	for(int i = 0; i < min(lightCount, MaxLights); i++)
	{
		float distance = length(lights[i].Position - fragPosition);
		if(distance >= lights[i].Radius)
			continue;
			
		vec3 lightDir = normalize(lights[i].Position - fragPosition);
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normals, halfwayDir), 0.0), 16.0);

		vec3 diffuse = max(dot(normals, lightDir), 0.0) * albedo;

		lighting += (diffuse + spec) * lights[i].Colour * lights[i].Intensity;
	}

	return lighting;
}

#endif