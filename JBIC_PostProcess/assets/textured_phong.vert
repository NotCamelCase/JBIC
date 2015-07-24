#version 450

// Mesh data structure: POS & NORMAL & UV
layout (location = 0) in vec3 in_POS;
layout (location = 1) in vec3 in_NORMAL;
layout (location = 2) in vec2 in_UV;

out vec4 POS_VS_CAM;
out vec3 NORMAL_VS_CAM;
out vec2 UV_VS;

uniform mat4 MODELVIEW;
uniform mat3 NORMAL;
uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(in_POS, 1.0);
	
	POS_VS_CAM = MODELVIEW * vec4(in_POS, 1.0);
	NORMAL_VS_CAM = NORMAL * in_NORMAL;
	UV_VS = in_UV;
}