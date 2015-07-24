#version 430

layout (location = 0) in vec3 in_POS;
layout (location = 1) in vec3 in_NORMAL;
layout (location = 2) in vec2 in_UV;
layout (location = 3) in vec4 in_TANGENTS;

out vec3 LightDir_VS_TANGENT;
out vec3 ViewDir_VS_TANGENT;
out vec2 UV_VS;

struct LightBlock
{
	vec4 lightPos_CAM;
	vec3 lightColor;
	float lightIntensity;
};
uniform LightBlock Light;

uniform mat4 MODELVIEW;
uniform mat3 NORMAL;
uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(in_POS, 1.0);
	
	// Transform normal and tangent into eye-space
	vec3 norm = normalize(NORMAL * in_NORMAL);
	vec3 tang = normalize(NORMAL * in_TANGENTS.xyz);
	vec3 bitang = normalize(cross(norm, tang)) * in_TANGENTS.w;
	
	// Matrix for transformation to tangent space
    mat3 toObjectLocal = mat3(
        tang.x, bitang.x, norm.x,
        tang.y, bitang.y, norm.y,
        tang.z, bitang.z, norm.z);

    // Transform light direction and view direction to tangent space
    vec3 pos = vec3(MODELVIEW * vec4(in_POS, 1.0));
    LightDir_VS_TANGENT = normalize(toObjectLocal * (Light.lightPos_CAM.xyz - pos));
    ViewDir_VS_TANGENT = toObjectLocal * normalize(-pos);

    UV_VS = in_UV;
}