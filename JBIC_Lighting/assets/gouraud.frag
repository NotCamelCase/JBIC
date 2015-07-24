#version 430

in vec3 gouraudLightIntensity;

out vec4 out_fragCol;

void main()
{
	out_fragCol = vec4(gouraudLightIntensity, 1.0);
}