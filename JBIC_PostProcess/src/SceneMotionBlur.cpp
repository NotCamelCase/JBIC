#include <SceneMotionBlur.h>

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

#define TRANSLATE_VELOCITY -2.5
#define ROTATE_VELOCITY 100

SceneMotionBlur::SceneMotionBlur(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_motionShader(nullptr),
	m_shadingProg(nullptr), m_light(nullptr), m_fbo(0), m_fsq(0)
{
}

SceneMotionBlur::~SceneMotionBlur()
{
	SAFE_DELETE(m_loader);

	glDeleteFramebuffers(1, &m_fbo);
	glDeleteVertexArrays(1, &m_fsq);
}

void SceneMotionBlur::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_motion.scene");

	m_shadingProg = m_programs[0];
	m_motionShader = m_programs[1];
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

	assert(createFBO() && "Failed to create FBO for post-processing!");

	m_fsq = Scene::createFullScreenQuad();
}

void SceneMotionBlur::update(double delta)
{
	// Render the scene to frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	renderScene(delta);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render full-screen quad
	renderBlurred(delta);
}

void SceneMotionBlur::renderScene(double delta)
{
	m_shadingProg->use();

	glClearColor(.5f, .5f, .5f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_sceneCam->update();

	m_shadingProg->setUniform("Light.lightPos_CAM",
		(m_sceneCam->getViewMatrix() * m_light->getWorldTransform()) * m_light->getPosition());

	// Render plane
	{
		m_shadingProg->setUniform("MainTex", 0);
		SceneObject* obj = m_sceneObjects[1];
		mat4 prevMVP = m_sceneCam->getMVP(obj->getWorldTransform());
		obj->rotate(-ROTATE_VELOCITY * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_shadingProg->setUniform("prev_MVP", prevMVP);

		m_shadingProg->setUniform("MODELVIEW", mv);
		m_shadingProg->setUniform("MVP", mvp);
		m_shadingProg->setUniform("NORMAL", normal);
		obj->getMesh()->uploadMaterialProperties(m_shadingProg);

		obj->render();
	}

	// Render box
	{
		m_shadingProg->setUniform("MainTex", 1);
		SceneObject* obj = m_sceneObjects[0];
		mat4 prevMVP = m_sceneCam->getMVP(obj->getWorldTransform());
		obj->translate(vec3(TRANSLATE_VELOCITY * delta, 0, 0));
		obj->rotate(ROTATE_VELOCITY * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_shadingProg->setUniform("prev_MVP", prevMVP);

		m_shadingProg->setUniform("MODELVIEW", mv);
		m_shadingProg->setUniform("MVP", mvp);
		m_shadingProg->setUniform("NORMAL", normal);
		obj->getMesh()->uploadMaterialProperties(m_shadingProg);

		obj->render();
	}
}

void SceneMotionBlur::renderBlurred(double delta)
{
	m_motionShader->use();

	m_motionShader->setUniform("RTT", 2);
	m_motionShader->setUniform("VelocityTex", 3);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

bool SceneMotionBlur::createFBO()
{
	// Generate and bind the framebuffer
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Create render texture
	GLuint renderTex;
	glGenTextures(1, &renderTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_params.width, m_params.height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Bind the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

	// Create velocity buffer
	GLuint velocityTex;
	glGenTextures(1, &velocityTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, velocityTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, m_params.width, m_params.height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, velocityTex, 0);

	// Set the targets for the fragment output variables
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	bool res = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	// Unbind the framebuffer, and revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return res;
}