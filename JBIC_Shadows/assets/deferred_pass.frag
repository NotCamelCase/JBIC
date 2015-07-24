#version 450

in vec4 POS_VS_CAM;
in vec3 NORMAL_VS_CAM;
in vec4 KD;
in vec3 KS;
in vec3 KA;

layout (location = 1) out vec4 POS_CAM;
layout (location = 2) out vec3 NORMAL_CAM;
layout (location = 3) out vec4 KD_MAT;
layout (location = 4) out vec3 KS_MAT;
layout (location = 5) out vec3 KA_MAT;

void main()
{
	POS_CAM = POS_VS_CAM;
	NORMAL_CAM = NORMAL_VS_CAM;
	KD_MAT = KD;
	KS_MAT = KS;
	KA_MAT = KA;
}