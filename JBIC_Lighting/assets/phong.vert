#version 430

layout (location = 0) in vec3 in_POS;
layout (location = 1) in vec3 in_NORMAL;
layout (location = 2) in vec2 in_UV;
layout (location = 3) in vec4 in_TANGENTS;

out vec4 out_POS;
out vec3 out_NORMAL;

uniform mat4 MODELVIEW;
uniform mat4 MVP;
uniform mat3 NORMAL;

void main()
{
	gl_Position = MVP * vec4(in_POS, 1.0);
	
	out_POS = MVP * vec4(in_POS, 1.0);
	out_NORMAL = NORMAL * in_NORMAL;
}











