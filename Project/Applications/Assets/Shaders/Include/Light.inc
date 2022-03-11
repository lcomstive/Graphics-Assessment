#ifndef _INCLUDE_LIGHT_
#define _INCLUDE_LIGHT_
#include "#ASSET_DIR/Shaders/Include/Camera.inc"

const int MAX_LIGHTS = 32;
const int MaxLightViews = 1;

uniform sampler2DArray shadowMap;

#define LIGHT_POINT 		0
#define LIGHT_SPOT 			1
#define LIGHT_DIRECTIONAL 	2

struct Light
{
	int Type;
	vec3 Colour;
	float Radius;
	vec3 Position;
	vec3 Direction;
	float Intensity;
	bool CastShadows;

	int ShadowMapIndex[MaxLightViews];
	mat4 LightSpaceMatrix[MaxLightViews];
};

uniform int lightCount = 0;
uniform Light lights[MAX_LIGHTS];
uniform vec3 ambientLighting = vec3(0.1, 0.1, 0.1);

// Returns a value between [0.0-1.0] as a percentage value OUT of shadow.
// 1.0 = Not in shadow at all
// 0.0 = Completely covered by shadow
float GetShadowAmount(int lightIndex, int viewIndex, vec3 surfaceNormal, vec3 fragPos)
{
	if(lights[lightIndex].ShadowMapIndex[viewIndex] < 0)
		return 1.0; // Not in shadow
	vec4 lightSpaceCoords = lights[lightIndex].LightSpaceMatrix[viewIndex] * vec4(fragPos, 1.0);
	vec3 normalDeviceCoords = lightSpaceCoords.xyz / lightSpaceCoords.w;
	normalDeviceCoords = normalDeviceCoords * 0.5 + 0.5; // Maps range [-1,1] to [0,1]

	if(normalDeviceCoords.z > 1.0)
		return 1.0; // Outside of texture coordinates, no shadow

	float shadow = 0.0;
	float fragDepth = normalDeviceCoords.z;
	float bias = max(0.005 * (1.0 - dot(surfaceNormal, lights[lightIndex].Direction)), 0.005);

#if 0
	// Percentage Closer Filtering
	vec3 texelSize = 1.0 / textureSize(shadowMap, 0).xyz;
	vec3 textureCoords = vec3(normalDeviceCoords.xy, lights[lightIndex].ShadowMapIndex[viewIndex]);
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
	vec3 textureCoords = vec3(normalDeviceCoords.xy, lights[lightIndex].ShadowMapIndex[viewIndex]);
	float currentDepth = texture(shadowMap, textureCoords).r;
	shadow = fragDepth - bias > currentDepth ? 1.0 : 0.0;
#endif

	return shadow; 
}

vec3 BlinnPhong(vec3 albedo, vec3 fragPosition, vec3 normals)
{
	vec3 lighting = albedo * ambientLighting;
	vec3 viewDir = normalize(camera.Position - fragPosition);
	for(int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
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