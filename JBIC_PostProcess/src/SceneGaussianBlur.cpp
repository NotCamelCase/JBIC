#include <SceneGaussianBlur.h>

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

SceneGaussianBlur::SceneGaussianBlur(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_postProcessProg(nullptr),
	m_shadingProg(nullptr), m_light(nullptr), m_fbo(nullptr), m_tempFbo(nullptr),
	m_fsq(0), m_verticalPass(0), m_horizontalPass(0)
{
}

SceneGaussianBlur::~SceneGaussianBlur()
{
	SAFE_DELETE(m_loader);
	SAFE_DELETE(m_fbo);
	SAFE_DELETE(m_tempFbo);

	glDeleteVertexArrays(1, &m_fsq);
}

void SceneGaussianBlur::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_postprocess.scene");

	m_shadingProg = m_programs[0];
	m_postProcessProg = m_programs[1];
	m_light = m_lightList[0];

	m_shadingProg->setUniform("Light.lightColor", m_light->getLightColor());
	m_shadingProg->setUniform("Light.lightIntensity", m_light->getLightIntensity());
	m_shadingProg->setUniform("Light.type", static_cast<uint> (m_light->getLightType()));

	// Get the subroutine handles of shader passess
	m_verticalPass = glGetSubroutineIndex(m_postProcessProg->getHandle(), GL_FRAGMENT_SHADER, "VerticalPass");
	m_horizontalPass = glGetSubroutineIndex(m_postProcessProg->getHandle(), GL_FRAGMENT_SHADER, "HorizontalPass");

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

	Texture* boxTexture2 = TextureManager::getInstance()->createTexture("assets/drkwood2.jpg", GL_RGBA);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, boxTexture2->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, boxTexture2->getWidth(), boxTexture2->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, boxTexture2->getWidth(), boxTexture2->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)boxTexture2->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_fbo = new FrameBuffer(this, m_params.width, m_params.height, true);
	m_tempFbo = new FrameBuffer(this, m_params.width, m_params.height, true);

	m_fsq = Scene::createFullScreenQuad();
}

void SceneGaussianBlur::update(double delta)
{
	// Render the scene with Blinn-Phong shading to first frame buffer
	m_fbo->bind();
	renderScene(delta);
	m_fbo->unbind();

	// Render the vertically blurred full-screen quad to second frame buffer
	m_tempFbo->bind();
	renderVerticalBlurPass();
	m_tempFbo->unbind();

	// Finally render full-screen quad to default frame buffer completely Gaussian-blurred
	renderHorizontalBlurPass();
}

void SceneGaussianBlur::renderScene(double delta)
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
		SceneObject* obj = m_sceneObjects[2];
		obj->rotate(10 * delta, vec3(0, 1, 0));
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

	// Render box 1
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

	// Render box 2
	{
		m_shadingProg->setUniform("Tex0", 2);
		SceneObject* obj = m_sceneObjects[1];
		obj->rotate(-25 * delta, vec3(0, 1, 0));
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

void SceneGaussianBlur::renderVerticalBlurPass()
{
	m_postProcessProg->use();
	m_postProcessProg->setUniform("RTT", 3);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_verticalPass);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SceneGaussianBlur::renderHorizontalBlurPass()
{
	m_postProcessProg->use();
	m_postProcessProg->setUniform("RTT", 4);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_horizontalPass);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SceneGaussianBlur::onKeyPressed(int key)
{
	switch (key)
	{
	default:
		break;
	}
}

void SceneGaussianBlur::onKeyReleased(int key)
{
	switch (key)
	{
	default:
		break;
	}
}