#version 450

uniform sampler2D RTT;

in vec2 UV_VS;

out vec4 fragCol;

void main()
{
	fragCol = vec4(vec3(1.0 - texture(RTT, UV_VS)), 1.0);
}