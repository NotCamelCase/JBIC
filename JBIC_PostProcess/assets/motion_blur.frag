#version 450

in vec2 UV_VS;

uniform sampler2D RTT;
uniform sampler2D VelocityTex;

#define NUM_SAMPLES 4

out vec4 finalCol;

void main()
{
	vec2 velocity = texture(VelocityTex, UV_VS).rg;
	finalCol = texture(RTT, UV_VS);
    for (int i = 1; i < NUM_SAMPLES; ++i)
	{
		vec2 offset = velocity * (i / float(NUM_SAMPLES - 1) - 0.5);
		finalCol += texture(RTT, UV_VS + offset);
	}
   
   finalCol /= float(NUM_SAMPLES);
}