#ifndef _INCLUDE_CAMERA_
#define _INCLUDE_CAMERA_
struct Camera
{
	vec3 Position;
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
};

uniform Camera camera;

#endif