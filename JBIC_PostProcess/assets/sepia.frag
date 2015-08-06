#version 450

in vec2 UV_VS;

layout (binding = 2) uniform sampler2D RTT;

out vec4 fragCol;

uniform mat3 TO_SEPIA = mat3
(
	0.299, 0.587, 0.114,
	0.596, -0.275, -0.321,
	0.212, -0.523, 0.311
);

void main()
{
	vec4 texCol = texture(RTT, UV_VS);
	vec3 sepia = TO_SEPIA[0] * texCol.rgb;
	sepia.r += .191;
	sepia.g -= 0.054;
	sepia.b -= .221;
	
	fragCol = vec4(sepia, texCol.a);
}