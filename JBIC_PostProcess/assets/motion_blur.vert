#version 450

// Mesh data structure: POS & UV
layout (location = 0) in vec2 in_POS;
layout (location = 1) in vec2 in_UV;

out vec2 UV_VS;

void main()
{
	gl_Position = vec4(in_POS, 0.0, 1.0);
	
	UV_VS = in_UV;
}