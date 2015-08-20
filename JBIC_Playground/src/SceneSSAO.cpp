#include <SceneSSAO.h>

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

SceneSSAO::SceneSSAO(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_deferredShading(nullptr),
	m_fsq(0), m_gbuffer(nullptr), m_geoPass(nullptr)
{
}

SceneSSAO::~SceneSSAO()
{
	SAFE_DELETE(m_loader);
	SAFE_DELETE(m_gbuffer);
}

void SceneSSAO::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_ssao.scene");

	m_geoPass = m_programs[0];
	m_deferredShading = m_programs[1];

	m_deferredShading->use();
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

	m_gbuffer = new GBuffer(this, GL_TEXTURE0);

	m_fsq = Scene::createFullScreenQuad();
}

void SceneSSAO::update(double delta)
{
	renderScene(delta);
}

void SceneSSAO::renderScene(double delta)
{
	geometryPass(delta);
	deferredShadingPass(delta);
}

void SceneSSAO::createSSAO_FBO()
{
	// Generate and bind the framebuffer
	glGenFramebuffers(1, &m_fboSSAO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboSSAO);

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
	GLuint aoTex;
	glGenTextures(1, &aoTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, aoTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, m_params.width, m_params.height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, aoTex, 0);

	// Set the targets for the fragment output variables
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	bool res = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	// Unbind the framebuffer, and revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneSSAO::geometryPass(double delta)
{
	m_geoPass->use();

	glBindFramebuffer(GL_FRAMEBUFFER, m_gbuffer->getFBO());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.f, 0.5f, 23.f, 1.f);

	m_sceneCam->update();

	for (SceneObject* obj : m_sceneObjects)
	{
		//obj->rotate(2.5 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		// Pass to shader anyway though only needed for when non-uniform scales are used!
		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_geoPass->setUniform("MODELVIEW", mv);
		m_geoPass->setUniform("MVP", mvp);
		m_geoPass->setUniform("NORMAL", normal);
		m_geoPass->setUniform("KA", obj->getMesh()->getMaterial()->getAmbientColor());
		m_geoPass->setUniform("KD", obj->getMesh()->getMaterial()->getDiffuseColor());
		m_geoPass->setUniform("KS", obj->getMesh()->getMaterial()->getSpecularColor());
		obj->render();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glFlush();
	glFinish();
}

void SceneSSAO::deferredShadingPass(double delta)
{
	m_deferredShading->use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < m_lightList.size(); i++)
	{
		Light* l = m_lightList[i];
		//l->rotate(2.5 * delta, vec3(0, 1, -1));
		std::stringstream lightPos;
		lightPos << "LightList[" << i << "].lightPos_CAM";
		m_deferredShading->setUniform(lightPos.str().c_str(),
			(m_sceneCam->getViewMatrix() * l->getWorldTransform()) * l->getPosition());
	}

	m_deferredShading->setUniform("PosTex", 0);
	m_deferredShading->setUniform("NormalTex", 1);
	m_deferredShading->setUniform("MatKaTex", 2);
	m_deferredShading->setUniform("MatKdTex", 3);
	m_deferredShading->setUniform("MatKsTex", 4);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SceneSSAO::ssaoPass()
{
	m_ssao->use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_ssao->setUniform("AOTex", 0);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SceneSSAO::onKeyPressed(int key)
{
	Scene::onKeyPressed(key);

	switch (key)
	{
	default:
		break;
	}
}

void SceneSSAO::onKeyReleased(int key)
{
	Scene::onKeyReleased(key);

	switch (key)
	{
	default:
		break;
	}
}