#include <SceneDeferredPCF.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <sstream>

#include <TextureManager.h>
#include <ShaderProgram.h>
#include <SceneLoader.h>
#include <FrameBuffer.h>
#include <SceneObject.h>
#include <Material.h>
#include <GBuffer.h>
#include <Texture.h>
#include <Camera.h>
#include <Light.h>
#include <Mesh.h>
#include <App.h>

using namespace glm;

ShadowMapParams g_shadowMapParams;

SceneDeferredPCF::SceneDeferredPCF(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_deferredShading(nullptr),
	m_fsq(0), m_gbuffer(nullptr), m_geoPass(nullptr)
{
}

SceneDeferredPCF::~SceneDeferredPCF()
{
	SAFE_DELETE(m_loader);

	SAFE_DELETE(m_gbuffer);
}

void SceneDeferredPCF::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_deferred.scene");

	m_geoPass = m_programs[0];
	m_deferredShading = m_programs[1];

	for (int i = 0; i < m_lightList.size(); i++)
	{
		const Light* light = m_lightList[i];
		std::stringstream lightAttr;
		lightAttr << "LightList[" << i << "].lightColor";
		LOG_ME(lightAttr.str().c_str());
		m_deferredShading->setUniform(lightAttr.str().c_str(), light->getLightColor());
		std::stringstream lightAttr2;
		lightAttr2 << "LightList[" << i << "].lightIntensity";
		LOG_ME(lightAttr2.str().c_str());
		m_deferredShading->setUniform(lightAttr2.str().c_str(), light->getLightIntensity());
		std::stringstream lightAttr3;
		lightAttr3 << "LightList[" << i << "].type";
		LOG_ME(lightAttr3.str().c_str());
		m_deferredShading->setUniform(lightAttr3.str().c_str(), static_cast<unsigned int> (light->getLightType()));
		if (light->getLightType() == LightType::POINT) // Set attenuation values
		{
			std::stringstream constAtten, linearAtten, expAtten;
			constAtten << "LightList[" << i << "].atten.constant";
			linearAtten << "LightList[" << i << "].atten.linear";
			expAtten << "LightList[" << i << "].atten.exponent";
			m_deferredShading->setUniform(constAtten.str().c_str(), light->getConstantAttenuation());
			m_deferredShading->setUniform(linearAtten.str().c_str(), light->getLinearAttenuation());
			m_deferredShading->setUniform(expAtten.str().c_str(), light->getExponentAttenuation());
		}
	}

	/*Texture* planeTexture = TextureManager::getInstance()->createTexture("assets/wood.png", GL_RGBA);
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

	Texture* boxTexture2 = TextureManager::getInstance()->createTexture("assets/toy_box_diffuse.png", GL_RGBA);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, boxTexture2->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, boxTexture2->getWidth(), boxTexture2->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, boxTexture2->getWidth(), boxTexture2->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)boxTexture2->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

	m_gbuffer = new GBuffer(this, GL_TEXTURE0);

	m_fsq = Scene::createFullScreenQuad();
}

void SceneDeferredPCF::update(double delta)
{
	renderScene(delta);
}

void SceneDeferredPCF::renderScene(double delta)
{
	geometryPass(delta);
	deferredShadingPass(delta);
}

void SceneDeferredPCF::geometryPass(double delta)
{
	m_geoPass->use();

	glBindFramebuffer(GL_FRAMEBUFFER, m_gbuffer->getFBO());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_sceneCam->update();

	for (SceneObject* obj : m_sceneObjects)
	{
		obj->rotate(2.5 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_geoPass->setUniform("MODELVIEW", mv);
		m_geoPass->setUniform("MVP", mvp);
		m_geoPass->setUniform("NORMAL", normal);

		m_geoPass->setUniform("in_KD", vec4(obj->getMesh()->getMaterial()->getDiffuseColor(),
			obj->getMesh()->getMaterial()->getShininess()));
		m_geoPass->setUniform("in_KS", obj->getMesh()->getMaterial()->getSpecularColor());
		m_geoPass->setUniform("in_KA", obj->getMesh()->getMaterial()->getAmbientColor());
		obj->render();
	}

	glFinish();
}

void SceneDeferredPCF::deferredShadingPass(double delta)
{
	m_deferredShading->use();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(.5f, .5f, .5f, 1.f);

	for (int i = 0; i < m_lightList.size(); i++)
	{
		Light* l = m_lightList[i];
		l->rotate(1.25 * delta, vec3(0, 0, 1));
		std::stringstream lightPos;
		lightPos << "LightList[" << i << "].lightPos_CAM";
		m_deferredShading->setUniform(lightPos.str().c_str(),
			(m_sceneCam->getViewMatrix() * l->getWorldTransform()) * l->getPosition());
	}

	m_deferredShading->setUniform("PosTex", 0);
	m_deferredShading->setUniform("NormalTex", 1);
	m_deferredShading->setUniform("MatKDTex", 2);
	m_deferredShading->setUniform("MatKSTex", 3);
	m_deferredShading->setUniform("MatKATex", 4);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SceneDeferredPCF::onKeyPressed(int key)
{
	Scene::onKeyPressed(key);

	switch (key)
	{
	default:
		break;
	}
}

void SceneDeferredPCF::onKeyReleased(int key)
{
	Scene::onKeyReleased(key);

	switch (key)
	{
	default:
		break;
	}
}