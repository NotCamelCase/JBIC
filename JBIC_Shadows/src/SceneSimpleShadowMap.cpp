#include <SceneSimpleShadowMap.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <sstream>

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

ShadowMapParams g_shadowParams;

#define PCF_ENABLED 1

SceneSimpleShadowMap::SceneSimpleShadowMap(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_phongShading(nullptr), m_shadowFBO(0),
	m_shadowPassHandle(0), m_shadingHandle(0)
{
}

SceneSimpleShadowMap::~SceneSimpleShadowMap()
{
	SAFE_DELETE(m_loader);

	glDeleteFramebuffers(1, &m_shadowFBO);
}

void SceneSimpleShadowMap::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_simpleshadowmap.scene");

	m_phongShading = m_programs[0];

	m_shadingHandle = glGetSubroutineIndex(m_phongShading->getHandle(), GL_FRAGMENT_SHADER, "BRDF_BlinnPhong");
	m_shadowPassHandle = glGetSubroutineIndex(m_phongShading->getHandle(), GL_FRAGMENT_SHADER, "ShadowPass");

	for (int i = 0; i < m_lightList.size(); i++)
	{
		const Light* light = m_lightList[i];
		std::stringstream lightAttr;
		lightAttr << "LightList[" << i << "].lightColor";
		LOG_ME(lightAttr.str().c_str());
		m_phongShading->setUniform(lightAttr.str().c_str(), light->getLightColor());
		std::stringstream lightAttr2;
		lightAttr2 << "LightList[" << i << "].lightIntensity";
		LOG_ME(lightAttr2.str().c_str());
		m_phongShading->setUniform(lightAttr2.str().c_str(), light->getLightIntensity());
		std::stringstream lightAttr3;
		lightAttr3 << "LightList[" << i << "].type";
		LOG_ME(lightAttr3.str().c_str());
		m_phongShading->setUniform(lightAttr3.str().c_str(), static_cast<uint> (light->getLightType()));
		if (light->getLightType() == LightType::POINT) // Set attenuation values
		{
			std::stringstream constAtten, linearAtten, expAtten;
			constAtten << "LightList[" << i << "].atten.constant";
			linearAtten << "LightList[" << i << "].atten.linear";
			expAtten << "LightList[" << i << "].atten.exponent";
			m_phongShading->setUniform(constAtten.str().c_str(), light->getConstantAttenuation());
			m_phongShading->setUniform(linearAtten.str().c_str(), light->getLinearAttenuation());
			m_phongShading->setUniform(expAtten.str().c_str(), light->getExponentAttenuation());
		}
	}

	Texture* planeTexture = TextureManager::getInstance()->createTexture("assets/wood.png", GL_RGBA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planeTexture->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, planeTexture->getWidth(), planeTexture->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, planeTexture->getWidth(), planeTexture->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)planeTexture->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	Texture* boxTexture = TextureManager::getInstance()->createTexture("assets/container2.png", GL_RGBA);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, boxTexture->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, boxTexture->getWidth(), boxTexture->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, boxTexture->getWidth(), boxTexture->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)boxTexture->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	Texture* boxTexture2 = TextureManager::getInstance()->createTexture("assets/container.jpg", GL_RGBA);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, boxTexture2->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, boxTexture2->getWidth(), boxTexture2->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, boxTexture2->getWidth(), boxTexture2->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)boxTexture2->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	assert(createShadowFBO() && "Failed to create FBO for shadow mapping!");
}

void SceneSimpleShadowMap::update(double delta)
{
	m_phongShading->use();

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_shadowPassHandle);

	// Render to shadow FBO
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, g_shadowParams.resWidth, g_shadowParams.resHeight);

	glCullFace(GL_FRONT);
	renderScene(delta);
	glFlush();
	glFinish();

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_shadingHandle);

	// Render to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_params.width, m_params.height);
	glCullFace(GL_BACK);
	renderScene(delta);
}

void SceneSimpleShadowMap::renderScene(double delta)
{
	glClearColor(.5f, .5f, .5f, 1.f);

	m_sceneCam->update();

	m_phongShading->setUniform("ShadowMap", 3);

	for (int i = 0; i < m_lightList.size(); i++)
	{
		Light* l = m_lightList[i];
		std::stringstream lightPos;
		lightPos << "LightList[" << i << "].lightPos_CAM";
		m_phongShading->setUniform(lightPos.str().c_str(),
			(m_sceneCam->getViewMatrix() * l->getWorldTransform()) * l->getPosition());
	}

	// Render plane
	{
		SceneObject* obj = m_sceneObjects[2];
		//obj->rotate(10 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_phongShading->setUniform("MainTex", 0);
		m_phongShading->setUniform("MODELVIEW", mv);
		m_phongShading->setUniform("MVP", mvp);
		m_phongShading->setUniform("NORMAL", normal);
		m_phongShading->setUniform("SHADOW", m_lightList[0]->getMVP());
		obj->getMesh()->uploadMaterialProperties(m_phongShading);

		obj->render();
	}

	// Render box 1
	{
		SceneObject* obj = m_sceneObjects[0];
		//obj->rotate(2.5 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_phongShading->setUniform("MainTex", 1);
		m_phongShading->setUniform("MODELVIEW", mv);
		m_phongShading->setUniform("MVP", mvp);
		m_phongShading->setUniform("NORMAL", normal);
		m_phongShading->setUniform("SHADOW", m_lightList[0]->getMVP());
		obj->getMesh()->uploadMaterialProperties(m_phongShading);

		obj->render();
	}

	// Render box 2
	{
		SceneObject* obj = m_sceneObjects[1];
		//obj->rotate(-2.5 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_phongShading->setUniform("MainTex", 2);
		m_phongShading->setUniform("MODELVIEW", mv);
		m_phongShading->setUniform("MVP", mvp);
		m_phongShading->setUniform("NORMAL", normal);
		m_phongShading->setUniform("SHADOW", m_lightList[0]->getMVP());
		obj->getMesh()->uploadMaterialProperties(m_phongShading);

		obj->render();
	}
}

bool SceneSimpleShadowMap::createShadowFBO()
{
	GLuint shadowTex;
	glGenTextures(1, &shadowTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, g_shadowParams.resWidth, g_shadowParams.resHeight);

	// Set texture filtering state
#if PCF_ENABLED // Bilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif

	// Set texture sampling state
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Set texture look-up compare mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

	// Set depth map border color
	GLfloat depthMapBorder[] = { 1.f, 0.f, 0.f, 0.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, depthMapBorder);

	glGenFramebuffers(1, &m_shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);

	GLenum drawBuff[] = { GL_NONE };
	glDrawBuffers(1, drawBuff);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind back to default FBO

	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void SceneSimpleShadowMap::onKeyPressed(int key)
{
	Scene::onKeyPressed(key);

	switch (key)
	{
	default:
		break;
	}
}

void SceneSimpleShadowMap::onKeyReleased(int key)
{
	Scene::onKeyReleased(key);

	switch (key)
	{
	default:
		break;
	}
}