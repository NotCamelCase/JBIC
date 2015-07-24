#pragma once

class Scene;

namespace tinyxml2 { class XMLDocument; class XMLElement; class XMLAttribute; class XMLNode; }

#include <glm\fwd.hpp>

class SceneLoader
{
public:
	SceneLoader(Scene* parent);
	~SceneLoader();

	void import(const char* fileName);

private:
	Scene* m_scene;

	SceneLoader(const SceneLoader& self);

	tinyxml2::XMLDocument* m_xmlDoc;

	//const char* readContent(const char* filename);

	glm::vec3 parseVec3(tinyxml2::XMLElement* node);
	glm::vec4 parseVec4(tinyxml2::XMLElement* node);
	bool parseBool(tinyxml2::XMLAttribute* attr);

	void processCamera(tinyxml2::XMLNode* node);
	void processLight(tinyxml2::XMLNode* node);
	void processSceneObject(tinyxml2::XMLNode* node);
	void processShaderProgram(tinyxml2::XMLNode* node);

	const char* TAG_CAMERA = "camera";
	const char* TAG_SCENEOBJECT = "sceneObject";
	const char* TAG_SHADERPROGRAM = "shaderProgram";
	const char* TAG_LIGHT = "light";
};