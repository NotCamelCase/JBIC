#version 450

in vec4 POS_VS_CAM;
in vec3 NORMAL_VS_CAM;
in vec2 UV_VS;
in vec4 COORDS_VS_SHADOW;

uniform sampler2D MainTex;
uniform sampler2DShadow ShadowMap;

#define NUM_PCF_SAMPLE 1 / 4

const int MAX_NUM_LIGHTS = 8;
const int NUM_LIGHTS = 1;

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

struct MaterialBlock
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shininess;
	float padding[2];
};
uniform MaterialBlock Material;

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

layout (location = 0) out vec4 outCol;

#define PCF_ENABLED 0

/** Pseudo-BRDF Blinn-Phong shading. It bears __improvement__ */
subroutine(RenderPassType)
void BRDF_BlinnPhong()
{
	vec3 s;
	vec3 normal = gl_FrontFacing ? NORMAL_VS_CAM : -NORMAL_VS_CAM;
	vec4 pos = POS_VS_CAM;
	float lightAtten = 1.0;
	vec3 lightVec = (LightList[0].lightPos_CAM - pos).xyz;
	float dist = length(lightVec);
	lightAtten = 1 / ((LightList[0].atten.constant) + (LightList[0].atten.linear * dist) + (LightList[0].atten.exponent * dist * dist));
	s = normalize(lightVec);
	
#if PCF_ENABLED
	// Sample 4 neighbouring texels contributions
    float texelCont += textureProjOffset(ShadowMap, COORDS_VS_SHADOW, ivec2(-1,-1));
    texelCont += textureProjOffset(ShadowMap, COORDS_VS_SHADOW, ivec2(-1,1));
    texelCont += textureProjOffset(ShadowMap, COORDS_VS_SHADOW, ivec2(1,1));
    texelCont += textureProjOffset(ShadowMap, COORDS_VS_SHADOW, ivec2(1,-1));
	
    float inShadow = texelCont * NUM_PCF_SAMPLE;
#else
	float inShadow = textureProj(ShadowMap, COORDS_VS_SHADOW);
#endif

	vec3 ambient = (Material.Ka * LightList[0].lightIntensity);
	vec3 diffuse = inShadow * ((Material.Kd * LightList[0].lightColor) * (lightAtten * LightList[0].lightIntensity * max(dot(s, normal), 0.0)));
	vec3 v = vec3(-pos);
	vec3 h = normalize(v + s);
	vec3 spec = inShadow * ((Material.Ks * LightList[0].lightColor) * (LightList[0].lightIntensity * pow(max(dot(h, normal), 0.0), Material.shininess)));
	
	vec4 texCol = texture(MainTex, UV_VS);
	
	outCol = vec4(ambient + diffuse + spec, 1.0) * texCol;
}

subroutine(RenderPassType)
void ShadowPass()
{
	// Light-space depth written to texture automatically
}

void main()
{
	RenderPass();
}