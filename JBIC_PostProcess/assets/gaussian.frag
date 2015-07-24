#version 450

in vec2 UV_VS;

uniform sampler2D RTT;

subroutine vec4 BlurPassType();
subroutine uniform BlurPassType BlurPass;

// See: http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
// Normalized Gaussian coefficients
const float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

// Offset to fetch texel nearby
const int offsets[5] = int[] (0, 1, 2, 3, 4);

out vec4 fragCol;

subroutine(BlurPassType)
vec4 VerticalPass()
{
	ivec2 pix = ivec2(gl_FragCoord.xy);
    vec4 sum = texelFetch(RTT, pix, 0) * weights[0]; // Start fetching central pixel value
	for (int i = 1; i < 5; i++)
	{
		// Sum the vertically sampled texels in both directions by offset multiplied by normalized Gaussian weight 
		sum += texelFetchOffset(RTT, pix, 0, ivec2(0, offsets[i])) * weights[i];
		sum += texelFetchOffset(RTT, pix, 0, ivec2(0, -offsets[i])) * weights[i];
	}

    return sum;
}

subroutine(BlurPassType)
vec4 HorizontalPass()
{
	ivec2 pix = ivec2(gl_FragCoord.xy); // Start fetching central pixel value
    vec4 sum = texelFetch(RTT, pix, 0) * weights[0];
	for (int i = 1; i < 5; i++)
	{
		// Sum the horizontally sampled texels in both directions by offset multiplied by normalized Gaussian weight 
		sum += texelFetchOffset(RTT, pix, 0, ivec2(offsets[i], 0)) * weights[i];
		sum += texelFetchOffset(RTT, pix, 0, ivec2(-offsets[i], 0)) * weights[i];
	}
	
    return sum;
}

void main()
{
	fragCol = BlurPass();
}