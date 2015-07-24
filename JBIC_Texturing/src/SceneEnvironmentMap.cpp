#include <SceneEnvironmentMap.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <TextureManager.h>
#include <ShaderProgram.h>
#include <SceneLoader.h>
#include <SceneObject.h>
#include <Material.h>
#include <Texture.h>
#include <Camera.h>
#include <Skybox.h>
#include <Mesh.h>
#include <App.h>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneEnvironmentMap::SceneEnvironmentMap(App* app, const RenderParams& params)
	: Scene(app, params), m_loader(nullptr), m_program(nullptr), m_skyBox(nullptr)
{
}

SceneEnvironmentMap::~SceneEnvironmentMap()
{
	SAFE_DELETE(m_loader);
	SAFE_DELETE(m_skyBox);
}

void SceneEnvironmentMap::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_loader = new SceneLoader(this);
	m_loader->import("assets/env_map.scene");

	m_program = m_programs.at(0);

	m_skyBox = new SkyBox("yokohama");
}

void SceneEnvironmentMap::update(double delta)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_program->use();

	m_program->setUniform("drawSkyBox", true);
	m_skyBox->render();
	m_program->setUniform("drawSkyBox", false);

	m_sceneCam->update();
	m_program->setUniform("camPOS", m_sceneCam->getPosition());

	for (SceneObject* obj : m_sceneObjects)
	{
		obj->rotate(5 * delta, vec3(0, 1, 0));
		obj->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * obj->getWorldTransform();
		mvp = m_sceneCam->getMVP(obj->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_program->setUniform("MODELVIEW", mv);
		m_program->setUniform("MVP", mvp);
		m_program->setUniform("NORMAL", normal);

		obj->render();
	}
}