#version 450 core
#include "#ASSET_DIR/Shaders/Include/Light.inc"

layout(triangles, invocations = MaxLights) in;
layout(triangle_strip, max_vertices = 3) out;

void main()
{
	if(lights[gl_InvocationID].ShadowMapIndex < 0)
		return;

	gl_Layer = lights[gl_InvocationID].ShadowMapIndex;
	for(int v = 0; v < gl_in.length(); v++)
	{
		gl_Position = lights[gl_InvocationID].LightSpaceMatrix * gl_in[v].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}