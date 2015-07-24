#version 450

in vec2 UV_VS;

uniform sampler2D RTT;

out vec4 fragCol;

const vec3 stdLuminance = vec3(0.2126, 0.7152, 0.0722);
float luminance(vec3 col)
{
	return dot(stdLuminance, col);
}

void main()
{
	float lmnc = luminance(texture(RTT, UV_VS).rgb);
	
	fragCol = vec4(lmnc, lmnc, lmnc, 1.0);
}