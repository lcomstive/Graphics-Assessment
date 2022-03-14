#version 400 core

const float Bias = 0.005;

void main()
{
	gl_FragDepth = gl_FragCoord.z;
	// gl_FragDepth += gl_FrontFacing ? Bias : 0.0;
}