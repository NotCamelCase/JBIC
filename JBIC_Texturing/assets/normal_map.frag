#version 430

in vec3 LightDir_VS_TANGENT;
in vec3 ViewDir_VS_TANGENT;
in vec2 UV_VS;

// glActiveTexture(GL_TEXTURE0) == specifying binding at 0
layout (binding = 0) uniform sampler2D Tex0; // Main texture
layout (binding = 1) uniform sampler2D Tex1; // Normal map

struct LightBlock
{
	vec4 lightPos_CAM;
	vec3 lightColor;
	float lightIntensity;
};
uniform LightBlock Light;

struct MaterialBlock
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shininess;
};
uniform MaterialBlock Material;

out vec4 fragCol;

void phongShading(vec3 norm, out vec3 ambient, out vec3 diffuse, out vec3 spec)
{
    vec3 r = reflect(-LightDir_VS_TANGENT, norm);
    ambient = Light.lightIntensity * Material.Ka;
    float sDotN = max(0.0, dot(LightDir_VS_TANGENT, norm));
    diffuse = Light.lightColor * Light.lightIntensity * Material.Kd * sDotN;
    spec = vec3(0.0);
    if(sDotN > 0.0)
		spec = Light.lightColor * Light.lightIntensity * Material.Ks * pow(max(0.0, dot(r, ViewDir_VS_TANGENT)), Material.shininess);
}

void main()
{
	vec4 texCol = texture(Tex0, UV_VS);
	vec4 normal = 2.0 * texture(Tex1, UV_VS) - 1.0; // [0, 1] -> [-1, 1]
	
	vec3 ambient, diffuse, spec;
	if (gl_FrontFacing) // Two-sided lighting
	{		
		phongShading(normal.xyz, ambient, diffuse, spec);
	}
	else
	{
		phongShading(-normal.xyz, ambient, diffuse, spec);
	}
	
	fragCol = vec4(ambient + diffuse, 1.0) * texCol + vec4(spec, 1.0);
}