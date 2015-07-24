#version 450

in vec4 POS_VS_CAM;
in vec3 NORMAL_VS_CAM;
in vec2 UV_VS;

layout (binding = 0) uniform sampler2D Tex0;

struct LightBlock
{
	vec4 lightPos_CAM;
	vec3 lightColor;
	float lightIntensity;
	uint type;
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

/** Pseudo-BRDF Blinn-Phong shading. It bears __improvement__ */
void BRDF_BlinnPhong(in vec4 pos, in vec3 normal, out vec3 ambient, out vec3 diffuse, out vec3 spec)
{
	vec3 s;
	if (Light.type == 0) // Directional
	{
		s = normalize(vec3(Light.lightPos_CAM));
	}
	else if (Light.type == 1) // Point
	{
		s = normalize(vec3(Light.lightPos_CAM - pos));
	}
	else if (Light.type == 2) // Spot
	{
		//TODO: Re-write spot lighting
	}
	
	ambient = Material.Ka * Light.lightIntensity;
	diffuse = Material.Kd * Light.lightColor * Light.lightIntensity * max(dot(s, normal), 0.0);
	vec3 v = vec3(-pos);
	vec3 h = normalize(v + s);
	spec = Material.Ks * Light.lightColor * Light.lightIntensity * pow(max(dot(h, normal), 0.0), Material.shininess);
}

void main()
{
	vec3 ambient, diffuse, spec;
	if (gl_FrontFacing)// Two-sided lighting
	{		
		BRDF_BlinnPhong(POS_VS_CAM, NORMAL_VS_CAM, ambient, diffuse, spec);
	}
	else
	{
		BRDF_BlinnPhong(POS_VS_CAM, -NORMAL_VS_CAM, ambient, diffuse, spec);
	}
	
	vec4 texCol = texture(Tex0, UV_VS);
	fragCol = vec4(ambient + diffuse, 1.0) * texCol + vec4(spec, 1.0);
}





