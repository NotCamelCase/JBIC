#version 450

in vec2 UV_VS;

uniform sampler2D RTT;

out vec4 fragCol;

// Squared threshold value for edges
const float edgeThreshold = 0.1;

const float pixelOffset = 1 / 450.0;

#define KERNEL_SIZE 9

void main()
{
	vec2 offsets[9] = vec2[]
	(
		vec2(-pixelOffset, pixelOffset), vec2(0.0, pixelOffset), vec2(pixelOffset, pixelOffset),
		vec2(-pixelOffset, 0.0), vec2(0.0, 0.0), vec2(pixelOffset, 0.0),
		vec2(-pixelOffset, -pixelOffset), vec2(0.0, -pixelOffset), vec2(pixelOffset, -pixelOffset)
	);
	
	float kernel[9] = float[]
	(
		-1, -1, -1,
		-1, 9, -1,
		-1, -1, -1
	);
	
	vec3 sampleTex[9];
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		sampleTex[i] = vec3(texture(RTT, UV_VS.st + offsets[i]));
	}
	
	vec3 col;
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		col += sampleTex[i] * kernel[i];
	}
	
	fragCol = vec4(col, 1.0);
}





