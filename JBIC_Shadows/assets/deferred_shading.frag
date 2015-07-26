#version 450

layout (binding = 1) uniform sampler2D PosTex;
layout (binding = 2) uniform sampler2D NormalTex;
layout (binding = 3) uniform sampler2D MatKaTex;
layout (binding = 4) uniform sampler2D MatKdTex;
layout (binding = 5) uniform sampler2D MatKsTex;

in vec2 UV_VS;

#define NUM_LIGHTS 1

struct Attenuation
{
	float linear;
	float exponent;
	float constant;
};

struct LightBlock
{
	Attenuation atten;
	vec4 lightPos_CAM;
	vec3 lightColor;
	float lightIntensity;
	uint type;
	float padding;
};
uniform LightBlock LightList[NUM_LIGHTS];

out vec4 outColor;

void BRDF_BlinnPhong()
{
	vec3 s;
	vec3 normal = texture(NormalTex, UV_VS).xyz;
	normal = gl_FrontFacing ? normal : -normal;
	vec3 pos = texture(PosTex, UV_VS).xyz;
	float lightAtten = 1.0;
	vec3 lightVec = (vec3(LightList[0].lightPos_CAM) - pos).xyz;
	float dist = length(lightVec);
	lightAtten = 1 / ((LightList[0].atten.constant) + (LightList[0].atten.linear * dist) + (LightList[0].atten.exponent * dist * dist));
	s = normalize(lightVec);
	vec3 Ka = texture(MatKaTex, UV_VS).rgb;
	vec3 Kd = texture(MatKdTex, UV_VS).rgb;
	vec3 Ks = texture(MatKsTex, UV_VS).rgb;
	float shininess = 96.0;
	vec3 ambient = (Ka * LightList[0].lightIntensity);
	vec3 diffuse = ((Kd * LightList[0].lightColor) * (lightAtten * LightList[0].lightIntensity * max(dot(s, normal), 0.0)));
	vec3 v = vec3(-pos);
	vec3 h = normalize(v + s);
	vec3 spec = ((Ks * LightList[0].lightColor) * (LightList[0].lightIntensity * pow(max(dot(h, normal), 0.0), shininess)));
	
	outColor = vec4(ambient + diffuse + spec, 1.0);
}

void main()
{
	BRDF_BlinnPhong();
}