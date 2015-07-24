#include <SceneRTT.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <TextureManager.h>
#include <ShaderProgram.h>
#include <SceneLoader.h>
#include <FrameBuffer.h>
#include <SceneObject.h>
#include <Material.h>
#include <Texture.h>
#include <Camera.h>
#include <Light.h>
#include <Mesh.h>
#include <App.h>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneRTT::SceneRTT(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_program(nullptr), m_light(nullptr), m_fbo(nullptr)
{
}

SceneRTT::~SceneRTT()
{
	SAFE_DELETE(m_loader);
	SAFE_DELETE(m_fbo);
}

void SceneRTT::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_rtt.scene");

	m_program = m_programs[0];
	m_light = m_lightList[0];

	m_program->setUniform("Light.lightColor", m_light->getLightColor());
	m_program->setUniform("Light.lightIntensity", m_light->getLightIntensity());

	m_fbo = new FrameBuffer(this, m_params.width, m_params.height, true);

	GLuint placeHolderHandle;
	GLubyte placeHolder[] = { 255, 255, 255, 255 };
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &placeHolderHandle);
	glBindTexture(GL_TEXTURE_2D, placeHolderHandle);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, placeHolder);
}

void SceneRTT::update(double delta)
{
	m_fbo->bind();
	renderToTexture(delta);
	m_fbo->unbind();
	renderScene(delta);
}

void SceneRTT::renderScene(double delta)
{
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, m_params.width, m_params.height);
	glClearColor(.75f, .5f, .5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_program->setUniform("Tex0", 0);

	m_sceneCam->update();

	m_light->rotate(200 * delta, vec3(.5, 1, -1));
	m_program->setUniform("Light.lightPos_CAM",
		(m_sceneCam->getViewMatrix() * m_light->getWorldTransform()) * m_light->getPosition());

	SceneObject* obj = m_sceneObjects[0];
	{
		//obj->rotate(25 * delta, vec3(0, 1, 0));
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

void SceneRTT::renderToTexture(double delta)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(.75f, .5f, .5f, 1.0f);
	glDisable(GL_DEPTH_TEST);

	m_program->setUniform("Tex0", 1);

	m_sceneCam->update();

	m_light->rotate(200 * delta, vec3(.5, 1, -1));
	m_program->setUniform("Light.lightPos_CAM",
		(m_sceneCam->getViewMatrix() * m_light->getWorldTransform()) * m_light->getPosition());

	SceneObject* obj = m_sceneObjects[1];
	{
		obj->rotate(25 * delta, vec3(0, 1, 0));
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