#version 450

in vec2 UV_VS;

// High-res HDR texture
uniform sampler2D RTT;

/**
* Overbright areas extracted from RTT texture will be written to this texture
* to apply Gaussian blurring and later blend with RTT HDR texture
*/
uniform sampler2D BloomTex;

const float gammaVal = 1 / 2.0;

uniform float exposure = 0.40;
uniform float averagedLuminance;
uniform float maxLightIntensity = 1.28;

// See: http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
// Normalized Gaussian coefficients
const float weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

// Offset to fetch texel nearby
const int offsets[5] = int[] (0, 1, 2, 3, 4);

const vec3 stdLuminance = vec3(0.2126, 0.7152, 0.0722);
float luminance(vec3 col)
{
	return dot(stdLuminance, col);
}

uniform float brightPassThreshold = 4.5;

uniform mat3 TO_XYZ = mat3
(
  0.4124564, 0.2126729, 0.0193339,
  0.3575761, 0.7151522, 0.1191920,
  0.1804375, 0.0721750, 0.9503041
);

uniform mat3 TO_RGB = mat3
(
  3.2404542, -0.9692660, 0.0556434,
  -1.5371385, 1.8760108, -0.2040259,
  -0.4985314, 0.0415560, 1.0572252
);

subroutine vec4 BloomPassType();
subroutine uniform BloomPassType BloomPass;

// Pass to apply vertical Gaussian blur
subroutine(BloomPassType)
vec4 VerticalBlurPass()
{
	ivec2 pix = ivec2(gl_FragCoord.xy);
    vec4 sum = texelFetch(BloomTex, pix, 0) * weights[0]; // Start fetching central pixel value
	for (int i = 1; i < 5; i++)
	{
		// Sum the vertically sampled texels in both directions by offset multiplied by normalized Gaussian weight 
		sum += texelFetchOffset(BloomTex, pix, 0, ivec2(0, offsets[i])) * weights[i];
		sum += texelFetchOffset(BloomTex, pix, 0, ivec2(0, -offsets[i])) * weights[i];
	}

    return sum;
}

// Pass to apply horizontal Gaussian blur
subroutine(BloomPassType)
vec4 HorizontalBlurPass()
{
	ivec2 pix = ivec2(gl_FragCoord.xy); // Start fetching central pixel value
    vec4 sum = texelFetch(BloomTex, pix, 0) * weights[0];
	for (int i = 1; i < 5; i++)
	{
		// Sum the horizontally sampled texels in both directions by offset multiplied by normalized Gaussian weight 
		sum += texelFetchOffset(BloomTex, pix, 0, ivec2(offsets[i], 0)) * weights[i];
		sum += texelFetchOffset(BloomTex, pix, 0, ivec2(-offsets[i], 0)) * weights[i];
	}
	
    return sum;
}

// Separate overbright pixels from scene
subroutine(BloomPassType)
vec4 BrightPass()
{
	vec4 texcol = texture(RTT, UV_VS);
    if(luminance(texcol.rgb) > brightPassThreshold)
        return texcol;
    else
        return vec4(0.0);
}

// #define HDR_OFF
subroutine(BloomPassType)
vec4 ToneMappingPass()
{
	// Look up HDR color from GL_RGB32F texture
	vec4 hdrCol = texture(RTT, UV_VS);
#ifdef HDR_OFF
	return hdrCol;
#else
	vec3 xyzCol = TO_XYZ * vec3(hdrCol);
    float sum = xyzCol.x + xyzCol.y + xyzCol.z;
    vec3 xyYCol = vec3(xyzCol.x / sum, xyzCol.y / sum, xyzCol.y);
	
    float lum = (exposure * xyYCol.z) / averagedLuminance;
    lum = (lum * (1 + lum / (maxLightIntensity * maxLightIntensity))) / (1 + lum);

    xyzCol.x = (lum * xyYCol.x) / (xyYCol.y);
    xyzCol.y = lum;
    xyzCol.z = (lum * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;
	
	vec4 toneMapped = vec4(pow((TO_RGB * xyzCol), vec3(gammaVal)), 1.0);
	vec4 blurred = texture(BloomTex, UV_VS);
		
    return toneMapped + blurred;
#endif
}

out vec4 finalColor;

void main()
{
	finalColor = BloomPass();
}