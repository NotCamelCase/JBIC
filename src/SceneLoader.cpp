#include <SceneLoader.h>

#include <fstream>
#include <sstream>

#include <common.h>

#include <tinyxml2.h>

#include <glm\glm.hpp>

#include <TextureManager.h>
#include <ShaderProgram.h>
#include <SceneObject.h>
#include <Material.h>
#include <Texture.h>
#include <Camera.h>
#include <Scene.h>
#include <Light.h>
#include <Mesh.h>
#include <App.h>

SceneLoader::SceneLoader(Scene* parent)
	: m_xmlDoc(new tinyxml2::XMLDocument()), m_scene(parent)
{
}

SceneLoader::~SceneLoader()
{
	SAFE_DELETE(m_xmlDoc);
}

void SceneLoader::import(const char* filename)
{
	tinyxml2::XMLError res = m_xmlDoc->LoadFile(filename);
	assert(res == tinyxml2::XML_SUCCESS && "tinxml parsing error!");

	const tinyxml2::XMLElement* sceneElem = m_xmlDoc->FirstChildElement();
	m_scene->getApp()->setTitle(sceneElem->FindAttribute("name")->Value());

	tinyxml2::XMLNode* root = m_xmlDoc->FirstChildElement()->FirstChild();
	for (; root; root = root->NextSibling())
	{
		const char* tag = root->Value();
		if (strcmp(tag, TAG_CAMERA) == 0)
		{
			processCamera(root);
		}
		else if (strcmp(tag, TAG_LIGHT) == 0)
		{
			processLight(root);
		}
		else if (strcmp(tag, TAG_SCENEOBJECT) == 0)
		{
			processSceneObject(root);
		}
		else if (strcmp(tag, TAG_SHADERPROGRAM) == 0)
		{
			processShaderProgram(root);
		}
	}
}

/*const char* SceneLoader::readContent(const char* filename)
{
std::ifstream scenefile(filename, std::ios::in);
scenefile.unsetf(std::ios::skipws);
if (!scenefile.good())
{
return nullptr;
}
else
{
std::stringstream source;
source << scenefile.rdbuf();
scenefile.close();

return source.str().c_str();
}
}*/

glm::vec3 SceneLoader::parseVec3(tinyxml2::XMLElement* node)
{
	float x, y, z;
	node->QueryFloatAttribute("x", &x);
	node->QueryFloatAttribute("y", &y);
	node->QueryFloatAttribute("z", &z);

	return glm::vec3(x, y, z);
}

glm::vec4 SceneLoader::parseVec4(tinyxml2::XMLElement* node)
{
	float x, y, z, w;
	node->QueryFloatAttribute("x", &x);
	node->QueryFloatAttribute("y", &y);
	node->QueryFloatAttribute("z", &z);
	node->QueryFloatAttribute("w", &w);

	return glm::vec4(x, y, z, w);
}

bool SceneLoader::parseBool(tinyxml2::XMLAttribute* attr)
{
	return (attr->IntValue() == 1);
}

void SceneLoader::processCamera(tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLElement* const elem = node->ToElement();

	float aspect;
	elem->QueryFloatAttribute("aspectRatio", &aspect);
	if (aspect == 0.0f) // Auto-calculate aspect ratio if not provided
	{
		aspect = m_scene->getAspectRatio();
	}

	float near, far, fov;
	elem->QueryFloatAttribute("near", &near);
	elem->QueryFloatAttribute("far", &far);
	elem->QueryFloatAttribute("fov", &fov);

	Camera* cam = new Camera(aspect, near, far, fov);
	cam->setPosition(parseVec3(node->FirstChildElement("position")));
	m_scene->setCamera(cam);
}

void SceneLoader::processLight(tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLElement* const elem = node->ToElement();

	LightFrustumParams::ProjectionType projType;
	LightType type = LightType::DIRECTIONAL;
	const char* lt = elem->FindAttribute("type")->Value();
	if (strcmp(lt, "POINT") == 0)
	{
		type = LightType::POINT;
		projType = LightFrustumParams::ProjectionType::PERSPECTIVE;
	}
	else if (strcmp(lt, "DIRECTIONAL") == 0)
	{
		type = LightType::DIRECTIONAL;
		projType = LightFrustumParams::ProjectionType::ORTHO;
	}
	else if (strcmp(lt, "SPOT") == 0)
	{
		type = LightType::SPOT;
		projType = LightFrustumParams::ProjectionType::PERSPECTIVE;
	}

	float intensity;
	elem->QueryFloatAttribute("intensity", &intensity);
	const glm::vec4 pos = parseVec4(node->FirstChildElement("position"));
	const glm::vec3 col = parseVec3(node->FirstChildElement("color"));

	LightFrustumParams* frustumParams = nullptr;
	const tinyxml2::XMLElement* frustumNode = node->FirstChildElement("frustum");
	bool castShadows = false;
	if (frustumNode)
	{
		LOG_ME("Light casting shadows");
		frustumParams = new LightFrustumParams(projType);
		assert(frustumNode && "Failed to parse light frustum parameters!");
		frustumNode->QueryFloatAttribute("near", &frustumParams->nearPlane);
		frustumNode->QueryFloatAttribute("far", &frustumParams->farPlane);
		frustumNode->QueryFloatAttribute("fovY", &frustumParams->fovY);
		castShadows = true;
	}

	Light* light = new Light(m_scene, type, pos, frustumParams, castShadows);
	light->setLightColor(col);
	light->setLightIntensity(intensity);

	const tinyxml2::XMLElement* attenNode = node->FirstChildElement("attenuation");
	if (attenNode)
	{
		float constAtten, linearAtten, expAtten;
		attenNode->QueryFloatAttribute("linear", &linearAtten);
		attenNode->QueryFloatAttribute("constant", &constAtten);
		attenNode->QueryFloatAttribute("exponent", &expAtten);
		light->setLinearAttenuation(linearAtten);
		light->setConstantAttenuation(constAtten);
		light->setExponentAttenuation(expAtten);
	}

	m_scene->addLight(light);
}

void SceneLoader::processSceneObject(tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLElement* const sceneElem = node->ToElement();
	const tinyxml2::XMLElement* meshElem = node->FirstChildElement("mesh");

	const char* name = meshElem->FindAttribute("name")->Value();
	bool hasUV = meshElem->BoolAttribute("hasUV");
	bool genTang = meshElem->BoolAttribute("genTangents");
	Mesh* mesh = new Mesh(name, hasUV, genTang);
	SceneObject* obj = new SceneObject(m_scene);
	obj->setMesh(mesh);

	const tinyxml2::XMLElement* transElem = meshElem->FirstChildElement("translate");
	if (transElem)
	{
		glm::vec3 translation = parseVec3(const_cast<tinyxml2::XMLElement*> (transElem));
		obj->translate(translation);
	}

	const tinyxml2::XMLElement* rotElem = meshElem->FirstChildElement("rotate");
	if (rotElem)
	{
		glm::vec3 rotAxis = parseVec3(const_cast<tinyxml2::XMLElement*> (rotElem));
		obj->rotate(rotElem->FloatAttribute("angle"), rotAxis);
	}

	const tinyxml2::XMLElement* scaleElem = meshElem->FirstChildElement("scale");
	if (scaleElem)
	{
		glm::vec3 scale = parseVec3(const_cast<tinyxml2::XMLElement*> (scaleElem));
		obj->scale(scale.x, scale.y, scale.z);
	}

	m_scene->addSceneObject(obj);
}

void SceneLoader::processShaderProgram(tinyxml2::XMLNode* node)
{
	const tinyxml2::XMLElement* const selem = node->ToElement();

	bool validate = selem->BoolAttribute("validate");
	bool use = selem->BoolAttribute("use");

	const tinyxml2::XMLElement* vertexElem = node->FirstChildElement("vertex");
	const char* vertexName = vertexElem->FindAttribute("name")->Value();

	const tinyxml2::XMLElement* fragElem = node->FirstChildElement("fragment");
	const char* fragName = fragElem->FindAttribute("name")->Value();

	ShaderProgram* shader = new ShaderProgram();
	shader->compileShader(vertexName);
	shader->compileShader(fragName);
	shader->link();
	if (validate) shader->validate();
	if (use) shader->use();

	m_scene->addShaderProgram(shader);
}