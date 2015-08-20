#include <SceneLighting.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

#include <common.h>

#include <ShaderProgram.h>
#include <SceneObject.h>
#include <Material.h>
#include <Camera.h>
#include <Light.h>
#include <Mesh.h>
#include <App.h>

SceneLighting::SceneLighting(App* app)
	: Scene(app), m_adsShader(nullptr), m_plane(nullptr), m_light(nullptr), m_monkey(nullptr)
{
	setup();
}

SceneLighting::~SceneLighting()
{
	SAFE_DELETE(m_adsShader);
	SAFE_DELETE(m_monkey);
	SAFE_DELETE(m_light);
	SAFE_DELETE(m_plane);
}

void SceneLighting::setup()
{
	m_sceneCam = new Camera((float)m_app->getWidth() / m_app->getHeight(), 0.1f, 100.f, 45);
	m_sceneCam->setPosition(vec3(10, 10, 10));

	m_plane = new SceneObject(this);
	m_plane->addMesh("assets/walls.obj", false, false, false);

	m_monkey = new SceneObject(this);
	m_monkey->translate(vec3(-2.5, 1.2, -1));
	Mesh* monkeyMesh = new Mesh("assets/monkey.obj", false, false, false);
	monkeyMesh->setMaterial(new Material());
	m_monkey->setMesh(monkeyMesh);

	m_adsShader = new ShaderProgram();
	m_adsShader->compileShader("assets/phong.vert");
	m_adsShader->compileShader("assets/phong.frag");
	m_adsShader->link();
	m_adsShader->use();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_light = new Light(LightType::SPOT, vec4(0, 50, 10, 1));
	m_light->setLightIntensity(vec3(0.79, 0.69, 0.9));

	m_adsShader->setUniform("u_spotLight.exponent", 10.f);
	m_adsShader->setUniform("u_spotLight.cutoff", 14.0f);
	m_adsShader->setUniform("u_light.intensity", m_light->getLightIntensity());
	m_adsShader->setUniform("u_light.lightPos_camSpace", (m_sceneCam->getViewMatrix() * m_light->getWorldTransform()) * m_light->getPosition());

	m_adsShader->setUniform("u_fog.color", vec3(0.35));
	m_adsShader->setUniform("u_fog.minDist", 5.f);
	m_adsShader->setUniform("u_fog.maxDist", 35.f);
}

void SceneLighting::update(double delta)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_sceneCam->update();

	// Set up light properties
	{
		mat4 lmv;
		mat3 lnormal;
		lmv = m_sceneCam->getViewMatrix() * m_light->getWorldTransform();
		lnormal = mat3(vec3(lmv[0]), vec3(lmv[1]), vec3(lmv[2]));
		m_adsShader->setUniform("u_spotLight.direction_camSpace", lnormal * vec3(-m_light->getPosition()));

		mat4 mv = m_sceneCam->getViewMatrix() * m_light->getWorldTransform();
		m_adsShader->setUniform("u_light.lightPos_camSpace", (mv * m_light->getPosition()));
	}

	// Render monkey
	{
		m_monkey->rotate(5 * delta, vec3(0, 1, 0));
		m_monkey->update(delta);

		mat4 model, mv, mvp;
		mat3 normal;

		model = m_monkey->getWorldTransform();
		mv = m_sceneCam->getViewMatrix() * model;
		mvp = m_sceneCam->getMVP(model);

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])); // Uniform scaling allows that!

		m_adsShader->setUniform("MVP", mvp);
		m_adsShader->setUniform("MODELVIEW", mv);
		m_adsShader->setUniform("NORMAL", normal);

		m_monkey->getMesh()->uploadMaterialProperties(m_adsShader);

		m_monkey->render();
	}

	// Render plane
	{
		m_plane->update(delta);

		mat4 model, mv, mvp;
		mat3 normal;

		model = m_plane->getWorldTransform();
		mv = m_sceneCam->getViewMatrix() * model;
		mvp = m_sceneCam->getMVP(model);

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])); // Uniform scaling allows that!

		m_adsShader->setUniform("MVP", mvp);
		m_adsShader->setUniform("MODELVIEW", mv);
		m_adsShader->setUniform("NORMAL", normal);

		m_adsShader->setUniform("u_material.Ka", vec3(.2, .6, .3));
		m_adsShader->setUniform("u_material.Kd", vec3(.2, .6, .56));
		m_adsShader->setUniform("u_material.Ks", vec3(.25, .75, .25));
		m_adsShader->setUniform("u_material.shininess", 100.f);

		m_plane->render();
	}
}