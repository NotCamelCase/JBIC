#version 430

layout (location = 0) in vec3 in_POS;
layout (location = 1) in vec3 in_NORMAL;
layout (location = 2) in vec2 in_UV;

out vec3 REFLECT_WORLD;

uniform mat4 MODELVIEW;
uniform mat4 MVP;
uniform mat3 NORMAL;
uniform vec3 camPOS;

uniform bool drawSkyBox;

void main()
{
	gl_Position = MVP * vec4(in_POS, 1.0);
	
	if (drawSkyBox) // Simply render skybox
	{
		REFLECT_WORLD = in_POS;
	}
	else // Simulate reflection using vertex pos in camera-space & surface normal
	{
		vec3 viewPos = (MODELVIEW * vec4(in_POS, 1.0)).xyz;
		viewPos = viewPos - camPOS;
		vec3 normalCam = NORMAL * in_NORMAL;
		vec3 N = normalize(normalCam);
	
		REFLECT_WORLD = reflect(viewPos, N);
	}
}