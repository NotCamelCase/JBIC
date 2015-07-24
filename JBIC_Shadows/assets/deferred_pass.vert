#version 450

// Mesh data structure: POS, NORMAL, UV
layout (location = 0) in vec3 in_POS;
layout (location = 1) in vec3 in_NORMAL;
layout (location = 2) in vec4 in_KD;
layout (location = 3) in vec3 in_KS;
layout (location = 4) in vec3 in_KA;

out vec4 POS_VS_CAM;
out vec3 NORMAL_VS_CAM;
out vec4 KD;
out vec3 KS;
out vec3 KA;

uniform mat4 MODELVIEW;
uniform mat3 NORMAL;
uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(in_POS, 1.0); // WORLD -> CLIP
	
	POS_VS_CAM = MODELVIEW * vec4(in_POS, 1.0);
	NORMAL_VS_CAM = NORMAL * in_NORMAL;
	KD = in_KD;
	KS = in_KS;
	KA = in_KA;
}