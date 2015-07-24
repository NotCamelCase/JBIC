#version 450

uniform sampler2D PosTex;
uniform sampler2D NormalTex;
uniform sampler2D MatKDTex;
uniform sampler2D MatKSTex;
uniform sampler2D MatKATex;

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
	vec4 pos = texture(PosTex, UV_VS);
	float lightAtten = 1.0;
	vec3 lightVec = (LightList[0].lightPos_CAM - pos).xyz;
	float dist = length(lightVec);
	lightAtten = 1 / ((LightList[0].atten.constant) + (LightList[0].atten.linear * dist) + (LightList[0].atten.exponent * dist * dist));
	s = normalize(lightVec);
	vec3 Ka = texture(MatKATex, UV_VS).rgb;
	vec3 Ks = texture(MatKSTex, UV_VS).rgb;
	vec4 Kd_shininess = texture(MatKDTex, UV_VS);
	vec3 Kd = Kd_shininess.rgb; float shininess = Kd_shininess.a;
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