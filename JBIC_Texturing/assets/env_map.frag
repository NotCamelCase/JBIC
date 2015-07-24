#version 430

in vec3 REFLECT_WORLD;

layout (binding = 0) uniform samplerCube environment;

out vec4 fragCol;

void main()
{
	fragCol = texture(environment, REFLECT_WORLD);
}