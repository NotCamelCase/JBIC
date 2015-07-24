#include <SceneToneMapping.h>

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

SceneToneMapping::SceneToneMapping(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_phongShading(nullptr), m_hdrProg(nullptr),
	m_fbo(nullptr), m_fsq(0), m_hdrBuffer(nullptr)
{
}

SceneToneMapping::~SceneToneMapping()
{
	SAFE_DELETE(m_fbo);
	SAFE_DELETE(m_loader);

	SAFE_DELETE_ARRAY(m_hdrBuffer);

	glDeleteVertexArrays(1, &m_fsq);
}

void SceneToneMapping::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_hdrBuffer = new float[m_params.width * m_params.height * sizeof(float)];

	m_loader = new SceneLoader(this);
	m_loader->import("assets/scene_hdr.scene");

	m_phongShading = m_programs[0];
	m_hdrProg = m_programs[1];

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

	Texture* planeTexture = TextureManager::getInstance()->createTexture("assets/plane_uv.png", GL_RGBA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, planeTexture->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, planeTexture->getWidth(), planeTexture->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, planeTexture->getWidth(), planeTexture->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)planeTexture->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

	m_fbo = new FrameBuffer(this, m_params.width, m_params.height, true, GL_RGB16F);

	m_fsq = Scene::createFullScreenQuad();
}

void SceneToneMapping::update(double delta)
{
	m_fbo->bind();
	renderScene(delta);
	m_fbo->unbind();

	calcToneMapping();
	renderPostProcess();
}

void SceneToneMapping::renderScene(double delta)
{
	glEnable(GL_DEPTH_TEST);
	m_phongShading->use();

	glClearColor(.5f, .5f, .5f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_sceneCam->update();

	for (int i = 0; i < m_lightList.size(); i++)
	{
		std::stringstream lightPos;
		lightPos << "LightList[" << i << "].lightPos_CAM";
		m_lightList[i]->rotate(90 * delta, vec3(0, 1, 0));
		m_phongShading->setUniform(lightPos.str().c_str(),
			(m_sceneCam->getViewMatrix() * m_lightList[i]->getWorldTransform()) * m_lightList[i]->getPosition());
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
		obj->getMesh()->uploadMaterialProperties(m_phongShading);

		obj->render();
	}

	// Render box 1
	{
		SceneObject* obj = m_sceneObjects[0];
		obj->rotate(25 * delta, vec3(0, 1, 0));
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
		obj->getMesh()->uploadMaterialProperties(m_phongShading);

		obj->render();
	}

	// Render box 2
	{
		SceneObject* obj = m_sceneObjects[1];
		obj->rotate(-25 * delta, vec3(0, 1, 0));
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
		obj->getMesh()->uploadMaterialProperties(m_phongShading);

		obj->render();
	}

	glFinish();
}

void SceneToneMapping::calcToneMapping()
{
	static const vec3 luminance = vec3(0.2126, 0.7152, 0.0722);
	glActiveTexture(FrameBuffer::s_activeTexID);
	glBindTexture(GL_TEXTURE_2D, m_fbo->getRTTHandle());
	glReadPixels(0, 0, m_params.width, m_params.height, GL_RGB, GL_FLOAT, m_hdrBuffer);
	GLenum err = glGetError();
	if (err == GL_INVALID_ENUM) { LOG_ME("Invalid enum!"); }
	else if (err == GL_INVALID_OPERATION) { LOG_ME("Invalid operation!"); }
	else if (err == GL_INVALID_VALUE) { LOG_ME("Invalid value!"); }
	float sum = 0.f;
	for (int i = 0; i < m_params.width * m_params.height; i++)
	{
		const vec3 pix = vec3(m_hdrBuffer[i * 3], m_hdrBuffer[i * 3 + 1], m_hdrBuffer[i * 3 + 2]);
		float lum = glm::dot(luminance, pix);
		sum += logf(lum + glm::epsilon<float>());
	}

	m_hdrProg->use();
	m_hdrProg->setUniform("averagedLuminance", expf(sum / (m_params.width * m_params.height)));
}

void SceneToneMapping::renderPostProcess()
{
	glDisable(GL_DEPTH_TEST);
	m_hdrProg->setUniform("RTT", 3);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(m_fsq);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SceneToneMapping::onKeyPressed(int key)
{
	Scene::onKeyPressed(key);

	switch (key)
	{
	default:
		break;
	}
}

void SceneToneMapping::onKeyReleased(int key)
{
	Scene::onKeyReleased(key);

	switch (key)
	{
	default:
		break;
	}
}