#version 430

#define SPOTLIGHT_ENABLED 0
#define FOG_ENABLED 0

in vec4 out_POS;
in vec3 out_NORMAL;

struct MaterialInfo
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shininess;
};

uniform MaterialInfo u_material;

struct LightInfo
{
	vec4 lightPos_camSpace;
	vec3 intensity;
};

uniform LightInfo u_light;

#if SPOTLIGHT_ENABLED
struct SpotLightInfo
{
	vec3 direction_camSpace;
	float exponent;
	float cutoff;
};
uniform SpotLightInfo u_spotLight;

vec3 phongSpotLightShading(vec4 pos, vec3 normal)
{
	vec3 s = normalize(vec3(u_light.lightPos_camSpace - pos));
	vec3 spotDir = normalize(u_spotLight.direction_camSpace);
	float angle = acos(dot(-s, spotDir));
	float cutoffClamped = radians(clamp(u_spotLight.cutoff, 0.0, 90.0));
	
	vec3 ambient = u_light.intensity * u_material.Ka;
	
	if (angle > cutoffClamped)
	{
		float spotFactor = pow(dot(-s, spotDir), u_spotLight.exponent);
		vec3 v = normalize(vec3(-pos));
		vec3 h = normalize(v + s);
		
		vec3 diffuse = u_material.Kd * max(dot(s, normal), 0.0);
		vec3 spec = u_material.Ks * pow(max(dot(h, normal), 0.0), u_material.shininess);
		
		return ambient + spotFactor * (diffuse + spec);
	}
	else
	{
		return ambient;
	}
}
#endif

out vec4 out_fragCol;

vec3 phongShading(vec4 pos, vec3 normal)
{
	vec3 ambient = u_light.intensity * u_material.Ka;
	vec3 s;
	if (u_light.lightPos_camSpace.w == 0)
	{
		s = normalize(vec3(u_light.lightPos_camSpace));
	}
	else
	{
		s = normalize(vec3(u_light.lightPos_camSpace - pos));
	}
	
	vec3 v = vec3(-pos);
	vec3 diffuse = u_material.Kd * max(dot(s, normal), 0.0);
	vec3 h = normalize(v + s);
	vec3 specular = u_material.Ks * pow(max(dot(h, normal), 0.0), u_material.shininess);
	
	return ambient + diffuse + specular;
}

const int levels = 3;
const float scale = 1.0 / levels;
vec3 toonShading(vec4 pos, vec3 normal)
{
	vec3 s = normalize(vec3(u_light.lightPos_camSpace - pos));
	float cosine = max(dot(s, normal), 0.0);
	vec3 diffuse = u_material.Kd * floor(cosine * levels) * scale;
	
	return u_light.intensity * (u_material.Ka + diffuse);
}

#if FOG_ENABLED
struct FogInfo
{
	float maxDist;
	float minDist;
	vec3 color;
};

uniform FogInfo u_fog;
#endif

void main()
{
#if SPOTLIGHT_ENABLED
	out_fragCol = vec4(phongSpotLightShading(out_POS, out_NORMAL), 1.0);
#elif FOG_ENABLED
	vec4 col = vec4(phongShading(out_POS, out_NORMAL), 1.0);
	
	float dist = length(out_POS.xyz);
	float fogFactor = (u_fog.maxDist - dist) / (u_fog.maxDist - u_fog.minDist);
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	out_fragCol = mix(vec4(u_fog.color, 1.0), col, fogFactor);
	
#else
	out_fragCol = vec4(phongShading(out_POS, out_NORMAL), 1.0);
#endif
}








