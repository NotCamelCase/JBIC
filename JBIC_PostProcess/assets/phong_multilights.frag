#version 450

in vec4 POS_VS_CAM;
in vec3 NORMAL_VS_CAM;
in vec2 UV_VS;

uniform sampler2D MainTex;

const int MAX_NUM_LIGHTS = 8;
const int NUM_LIGHTS = 3;

struct Attenuation
{
	float linear;
	float constant;
	float exponent;
};

struct LightBlock
{
	Attenuation atten;
	vec4 lightPos_CAM;
	vec3 lightColor;
	float lightIntensity;
	uint type;
};
uniform LightBlock LightList[NUM_LIGHTS];

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
void BRDF_BlinnPhong(in int i, in vec4 pos, in vec3 normal, out vec3 ambient, out vec3 diffuse, out vec3 spec)
{
	vec3 s;
	float lightAtten = 1.0;
	if (LightList[i].type == 0) // Directional
	{
		s = normalize(vec3(LightList[i].lightPos_CAM));
	}
	else if (LightList[i].type == 1) // Point
	{
		vec3 lightVec = (LightList[i].lightPos_CAM - pos).xyz;
		float dist = length(lightVec);
		lightAtten = 1 / ((LightList[i].atten.constant) + (LightList[i].atten.linear * dist) + (LightList[i].atten.exponent * dist * dist));
		s = normalize(lightVec);
	}
	else if (LightList[i].type == 2) // Spot
	{
		//TODO: Re-write spot lighting
	}

	ambient = (Material.Ka * LightList[i].lightIntensity);
	diffuse = ((Material.Kd * LightList[i].lightColor) * (lightAtten * LightList[i].lightIntensity * max(dot(s, normal), 0.0)));
	vec3 v = vec3(-pos);
	vec3 h = normalize(v + s);
	spec = ((Material.Ks * LightList[i].lightColor) * (LightList[i].lightIntensity * pow(max(dot(h, normal), 0.0), Material.shininess)));
}

void main()
{
	vec3 norm = gl_FrontFacing ? NORMAL_VS_CAM : -NORMAL_VS_CAM;
	vec3 ambient = vec3(0.0);
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		vec3 amb, diff, spec;
		BRDF_BlinnPhong(i, POS_VS_CAM, norm, amb, diff, spec);
		ambient += amb;
		diffuse += diff;
		specular += spec;
	}
	
	vec4 texCol = texture(MainTex, UV_VS);
	fragCol = vec4(ambient + diffuse, 1.0) * texCol + vec4(specular, 1.0);
}





