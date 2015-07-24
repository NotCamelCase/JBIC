#include <SceneImportDemo.h>

#include <SceneLoader.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

#include <TextureManager.h>
#include <ShaderProgram.h>
#include <SceneObject.h>
#include <Material.h>
#include <Texture.h>
#include <Camera.h>
#include <Light.h>
#include <Mesh.h>
#include <App.h>

SceneImportDemo::SceneImportDemo(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_program(nullptr), m_sceneLight(nullptr)
{
}

SceneImportDemo::~SceneImportDemo()
{
	SAFE_DELETE(m_loader);
}

void SceneImportDemo::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/test.scene");

	m_sceneLight = m_lightList.at(0);
	m_program = m_programs.at(0);

	m_program->setUniform("Light.lightColor", m_sceneLight->getLightColor());
	m_program->setUniform("Light.lightIntensity", m_sceneLight->getLightIntensity());

	glActiveTexture(GL_TEXTURE0);
	Texture* cubeTexture = TextureManager::createTexture("assets/cement.jpg", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, cubeTexture->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, cubeTexture->getWidth(), cubeTexture->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cubeTexture->getWidth(), cubeTexture->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)cubeTexture->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glActiveTexture(GL_TEXTURE1);
	Texture* multiTex = TextureManager::createTexture("assets/moss.png", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, multiTex->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, multiTex->getWidth(), multiTex->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, multiTex->getWidth(), multiTex->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)multiTex->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void SceneImportDemo::update(double delta)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_program->use();

	m_sceneCam->update();

	m_sceneLight->rotate(200 * delta, vec3(.5, 1, -1));
	m_program->setUniform("Light.lightPos_CAM",
		(m_sceneCam->getViewMatrix() * m_sceneLight->getWorldTransform()) * m_sceneLight->getPosition());

	{
		SceneObject* obj = m_sceneObjects.at(0);

		//obj->rotate(75 * delta, vec3(.75, .5, -1));
		//obj->rotate(100 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_program->setUniform("MODELVIEW", mv);
		m_program->setUniform("MVP", mvp);
		m_program->setUniform("NORMAL", normal);
		obj->getMesh()->uploadMaterialProperties(m_program);

		obj->render();
	}
}