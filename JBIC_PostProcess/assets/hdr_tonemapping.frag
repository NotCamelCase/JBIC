#version 450

in vec2 UV_VS;

/** High-res HDR texture */
uniform sampler2D RTT;

uniform float exposure = 0.40;
uniform float averagedLuminance;
uniform float maxLightIntensity = 0.98;

const float gammaVal = 1 / 2.0;

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
  
// #define HDR_OFF

out vec4 finalColor;

void main()
{
	// Look up HDR color from GL_RGB32F texture
	vec4 hdrCol = texture(RTT, UV_VS);
#ifdef HDR_OFF
	finalColor = hdrCol;
#else
	vec3 xyzCol = TO_XYZ * vec3(hdrCol);
    float sum = xyzCol.x + xyzCol.y + xyzCol.z;
    vec3 xyYCol = vec3(xyzCol.x / sum, xyzCol.y / sum, xyzCol.y);
	
    float lum = (exposure * xyYCol.z) / averagedLuminance;
    lum = (lum * (1 + lum / (maxLightIntensity * maxLightIntensity))) / (1 + lum);

    xyzCol.x = (lum * xyYCol.x) / (xyYCol.y);
    xyzCol.y = lum;
    xyzCol.z = (lum * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;
		
    finalColor = vec4(pow((TO_RGB * xyzCol), vec3(gammaVal)), 1.0);
#endif
}