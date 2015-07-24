#version 450

in vec2 UV_VS;

uniform sampler2D RTT;

out vec4 fragCol;

#define KERNEL_SIZE 9

void main()
{
	ivec2 pix = ivec2(gl_FragCoord.xy);
		
	ivec2 offsets[9] = ivec2[]
	(
		ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1),
		ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0),
		ivec2(-1, -1), ivec2(0, -1), ivec2(1, 1)
	);
	
	float kernel[9] = float[]
	(
		-1, -1, -1,
		-1, 8, -1,
		-1, -1, -1
	);
	
	vec3 sampleTex[9];
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		sampleTex[i] = texelFetchOffset(RTT, pix, 0, offsets[i]).rgb;
	}
	
	vec3 col;
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		col += sampleTex[i] * kernel[i];
	}
	
	fragCol = vec4(col, 1.0);
}




