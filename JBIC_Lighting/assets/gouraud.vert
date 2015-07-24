#version 430

layout (location = 0) in vec3 in_POS;
layout (location = 1) in vec3 in_NORMAL;
layout (location = 2) in vec2 in_UV;
layout (location = 3) in vec4 in_TANGENTS;

struct MaterialBlock
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	
	float shininess;
};

uniform MaterialBlock Material;

struct LightInfo
{
	vec4 lightPos_CamSpace;
	vec3 La;
};

uniform LightInfo Light;

uniform mat4 MODELVIEW;
uniform mat4 MVP;
uniform mat3 NORMAL;

out vec3 gouraudLightIntensity;

/**
* I = Ia + Id + Is
* Ia = La * Ka
* Id = Kd * (s.n)
* Is = Ks * (h.s)
*/
vec3 calcGroudShading(vec4 pos_CamSpace, vec3 normal_CamSpace)
{
	vec3 s;
	if (Light.lightPos_CamSpace.w == 0.0) { s = normalize(vec3(Light.lightPos_CamSpace)); }
	else { s = normalize(vec3(Light.lightPos_CamSpace - pos_CamSpace)); }
	
	vec3 v = normalize(vec3(-pos_CamSpace));
	vec3 r = reflect(-s, normal_CamSpace);
	
	return Light.La * Material.Ka + Material.Kd * max(dot(s, normal_CamSpace), 0.0)
		+ Material.Ks * pow(max(dot(r, v), 0.0), Material.shininess);
}

void main()
{
	gl_Position = MVP * vec4(in_POS, 1.0);
	
	vec4 posEyeSpace = MODELVIEW * vec4(in_POS, 1.0);
	vec3 normalEyeSpace = NORMAL * in_NORMAL;
	gouraudLightIntensity = calcGroudShading(posEyeSpace, normalEyeSpace);
}











