#include <ScenePostProcessSimple.h>

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

ScenePostProcessSimple::ScenePostProcessSimple(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_postProcessProg(nullptr),
	m_shadingProg(nullptr), m_light(nullptr), m_fbo(nullptr), m_fsq(0)
{
}

ScenePostProcessSimple::~ScenePostProcessSimple()
{
	SAFE_DELETE(m_loader);
	SAFE_DELETE(m_fbo);

	glDeleteVertexArrays(1, &m_fsq);
}

void ScenePostProcessSimple::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_simplefxs.scene");

	m_shadingProg = m_programs[0];
	m_postProcessProg = m_programs[1];
	m_light = m_lightList[0];

	m_shadingProg->setUniform("Light.lightColor", m_light->getLightColor());
	m_shadingProg->setUniform("Light.lightIntensity", m_light->getLightIntensity());
	m_shadingProg->setUniform("Light.type", static_cast<int> (m_light->getLightType()));

	Texture* planeTexture = TextureManager::getInstance()->createTexture("assets/plane_uv.png", GL_RGBA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planeTexture->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, planeTexture->getWidth(), planeTexture->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, planeTexture->getWidth(), planeTexture->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)planeTexture->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	Texture* boxTexture = TextureManager::getInstance()->createTexture("assets/fieldstone-rgba.tga", GL_RGBA);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, boxTexture->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, boxTexture->getWidth(), boxTexture->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, boxTexture->getWidth(), boxTexture->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)boxTexture->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	m_fbo = new FrameBuffer(this, m_params.width, m_params.height, false);

	m_fsq = Scene::createFullScreenQuad();
}

void ScenePostProcessSimple::update(double delta)
{
	// Render the scene to frame buffer
	m_fbo->bind();
	renderScene(delta);
	m_fbo->unbind();

	// Render full-screen quad
	renderPostProcess(delta);
}

void ScenePostProcessSimple::renderScene(double delta)
{
	m_shadingProg->use();

	glClearColor(.5f, .5f, .5f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_sceneCam->update();

	m_shadingProg->setUniform("Light.lightPos_CAM",
		(m_sceneCam->getViewMatrix() * m_light->getWorldTransform()) * m_light->getPosition());

	// Render plane
	{
		m_shadingProg->setUniform("Tex0", 0);
		SceneObject* obj = m_sceneObjects[1];
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_shadingProg->setUniform("MODELVIEW", mv);
		m_shadingProg->setUniform("MVP", mvp);
		m_shadingProg->setUniform("NORMAL", normal);
		obj->getMesh()->uploadMaterialProperties(m_shadingProg);

		obj->render();
	}

	// Render box
	{
		m_shadingProg->setUniform("Tex0", 1);
		SceneObject* obj = m_sceneObjects[0];
		obj->rotate(25 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_shadingProg->setUniform("MODELVIEW", mv);
		m_shadingProg->setUniform("MVP", mvp);
		m_shadingProg->setUniform("NORMAL", normal);
		obj->getMesh()->uploadMaterialProperties(m_shadingProg);

		obj->render();
	}
}

void ScenePostProcessSimple::renderPostProcess(double delta)
{
	m_postProcessProg->use();
	m_postProcessProg->setUniform("RTT", 2);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}