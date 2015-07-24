#version 450

in vec2 UV_VS;

uniform sampler2D RTT;

out vec4 fragCol;

void main()
{
	vec4 texCol = texture(RTT, UV_VS);
	
	fragCol = texCol;
}