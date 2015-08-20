#version 450

in vec3 POS_VS_CAM;
in vec3 NORMAL_VS_CAM;

// Material parameters
uniform vec3 KA;
uniform vec3 KD;
uniform vec3 KS;

layout (location = 1) out vec3 POS_CAM;
layout (location = 2) out vec3 NORMAL_CAM;
layout (location = 3) out vec3 MAT_KA;
layout (location = 4) out vec3 MAT_KD;
layout (location = 5) out vec3 MAT_KS;

void main()
{
	POS_CAM = POS_VS_CAM;
	NORMAL_CAM = NORMAL_VS_CAM;
	
	MAT_KA = KA;
	MAT_KD = KD;
	MAT_KS = KS;
}