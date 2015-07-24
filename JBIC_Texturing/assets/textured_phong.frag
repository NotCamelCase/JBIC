#version 450

in vec4 POS_VS_CAM;
in vec3 NORMAL_VS_CAM;
in vec2 UV_VS;

// glActiveTexture(GL_TEXTURE0) == specifying binding at 0
layout (binding = 0)uniform sampler2D Tex0;

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

void phongShading(in vec4 pos, in vec3 normal, out vec3 ambient, out vec3 diffuse, out vec3 spec)
{	
	ambient = Material.Ka * Light.lightIntensity;
	vec3 s = normalize(vec3(Light.lightPos_CAM - pos));
	diffuse = Material.Kd * Light.lightColor * Light.lightIntensity * max(dot(s, normal), 0.0);
	vec3 v = vec3(-pos);
	vec3 h = normalize(v + s);
	spec = Material.Ks * Light.lightColor * Light.lightIntensity * pow(max(dot(h, normal), 0.0), Material.shininess);
}

void main()
{
	vec4 texCol = texture(Tex0, UV_VS);
		
	vec3 ambient, diffuse, spec;
	if (gl_FrontFacing)// Two-sided lighting
	{		
		phongShading(POS_VS_CAM, NORMAL_VS_CAM, ambient, diffuse, spec);
	}
	else
	{
		phongShading(POS_VS_CAM, -NORMAL_VS_CAM, ambient, diffuse, spec);
	}
	
	fragCol = vec4(ambient + diffuse, 1.0) * texCol + vec4(spec, 1.0);
}





