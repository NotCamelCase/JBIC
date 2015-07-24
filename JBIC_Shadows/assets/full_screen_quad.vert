#version 450

layout (location = 0) in vec4 in_POS; // x-y pos of full-screen quad
layout (location = 1) in vec2 in_UV; // texcoords

out vec2 UV_VS;

void main()
{
	gl_Position = in_POS;
	
	UV_VS = in_UV;
}