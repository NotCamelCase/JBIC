#include <SceneTexturing.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

#include <TextureManager.h>
#include <ShaderProgram.h>
#include <SceneObject.h>
#include <Material.h>
#include <Texture.h>
#include <Camera.h>
#include <Light.h>
#include <Mesh.h>
#include <App.h>

#define TEXTURED_MODE
#undef TEXTURED_MODE

SceneTexturing::SceneTexturing(App* app, const RenderParams& params)
	: Scene(app, params), m_texturedLighting(nullptr), m_cube(nullptr)
{
}

SceneTexturing::~SceneTexturing()
{
	SAFE_DELETE(m_texturedLighting);
	SAFE_DELETE(m_sceneLight);

	TextureManager::destroyTexture(m_cubeTexture);
}

void SceneTexturing::setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	m_sceneCam = new Camera(getAspectRatio(), 2.5, 200, 45);
	m_sceneCam->setPosition(vec3(35, 45, -85));

	m_sceneLight = new Light(LightType::POINT, vec4(0, 50, 10, 1));
	m_sceneLight->setLightColor(vec3(0.85, 0.37, 0.34));
	m_sceneLight->setLightIntensity(1.5f);

	m_texturedLighting = new ShaderProgram();
	m_texturedLighting->compileShader("assets/textured_phong.vert");
	m_texturedLighting->compileShader("assets/textured_phong.frag");
	m_texturedLighting->link();
	m_texturedLighting->validate();

	m_texturedLighting->use();
	m_texturedLighting->setUniform("Light.lightColor", m_sceneLight->getLightColor());
	m_texturedLighting->setUniform("Light.lightIntensity", m_sceneLight->getLightIntensity());

	m_cube = new SceneObject(this);
	//m_cube->rotate(180, vec3(0, .25, 0));
	m_cube->addMesh("assets/house.obj", true, false);
	m_sceneObjects.push_back(m_cube);

	glActiveTexture(GL_TEXTURE0);
	m_cubeTexture = TextureManager::createTexture("assets/cement.jpg", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, m_cubeTexture->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_cubeTexture->getWidth(), m_cubeTexture->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_cubeTexture->getWidth(), m_cubeTexture->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)m_cubeTexture->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glActiveTexture(GL_TEXTURE1);
	Texture* multiTex = TextureManager::createTexture("assets/moss.png", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, multiTex->getTexId());
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, multiTex->getWidth(), multiTex->getHeight());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, multiTex->getWidth(), multiTex->getHeight(),
		GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte*)multiTex->getImageData());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void SceneTexturing::update(double delta)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_texturedLighting->use();

	m_sceneCam->update();

	m_sceneLight->rotate(200 * delta, vec3(.5, 1, -1));
	m_texturedLighting->setUniform("Light.lightPos_CAM",
		(m_sceneCam->getViewMatrix() * m_sceneLight->getWorldTransform()) * m_sceneLight->getPosition());

	{
		//m_cube->rotate(75 * delta, vec3(.75, .5, -1));
		//m_cube->rotate(100 * delta, vec3(0, 1, 0));
		m_cube->update(delta);

		mat4 mvp, mv;
		mat3 normal;
		mv = m_sceneCam->getViewMatrix() * m_cube->getWorldTransform();
		mvp = m_sceneCam->getMVP(m_cube->getWorldTransform());

		normal = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

		m_texturedLighting->setUniform("MODELVIEW", mv);
		m_texturedLighting->setUniform("MVP", mvp);
		m_texturedLighting->setUniform("NORMAL", normal);
		m_cube->getMesh()->uploadMaterialProperties(m_texturedLighting);

		m_cube->render();
	}
}