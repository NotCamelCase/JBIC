#include <Scene.h>

#include <App.h>
#include <Light.h>
#include <Camera.h>
#include <SceneObject.h>
#include <ShaderProgram.h>

#define MAX_LIGHT_COUNT 8
#define CAM_MOVE_SPEED 1.0

Scene::Scene(App* app, const RenderParams& params)
	: m_app(app), m_sceneCam(nullptr), m_params(params)
{
}

Scene::~Scene()
{
	SAFE_DELETE(m_sceneCam);

	for (const SceneObject* obj : m_sceneObjects)
	{
		SAFE_DELETE(obj);
	}
	m_sceneObjects.clear();

	for (const Light* l : m_lightList)
	{
		SAFE_DELETE(l);
	}
	m_lightList.clear();

	for (const ShaderProgram* sp : m_programs)
	{
		SAFE_DELETE(sp);
	}
	m_programs.clear();
}

void Scene::addLight(Light* l)
{
	assert(m_lightList.size() < MAX_LIGHT_COUNT && "Exceeded max light count!");

	m_lightList.push_back(l);
}

void Scene::addSceneObject(SceneObject* obj)
{
	m_sceneObjects.push_back(obj);
}

void Scene::addShaderProgram(ShaderProgram* sp)
{
	m_programs.push_back(sp);
}

void Scene::onKeyPressed(int key)
{
	switch (key)
	{
	case GLFW_KEY_W:
		m_sceneCam->getPosition().x -= CAM_MOVE_SPEED;
		break;
	case GLFW_KEY_S:
		m_sceneCam->getPosition().x += CAM_MOVE_SPEED;
		break;
	case GLFW_KEY_A:
		m_sceneCam->getPosition().z += CAM_MOVE_SPEED;
		break;
	case GLFW_KEY_D:
		m_sceneCam->getPosition().z -= CAM_MOVE_SPEED;
		break;
	case GLFW_KEY_Q:
		m_sceneCam->getPosition().y -= CAM_MOVE_SPEED;
		break;
	case GLFW_KEY_E:
		m_sceneCam->getPosition().y += CAM_MOVE_SPEED;
		break;
	default:
		break;
	}
}

void Scene::onKeyReleased(int key)
{
	switch (key)
	{
	default:
		break;
	}
}

uint Scene::createFullScreenQuad()
{
	float verts[] =
	{
		-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
	};

	float texCoords[] =
	{
		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

	GLuint fullscreenquad;
	glGenVertexArrays(1, &fullscreenquad);
	glBindVertexArray(fullscreenquad);

	GLuint posHandle, uvHandle;
	glGenBuffers(1, &posHandle);
	glGenBuffers(1, &uvHandle);

	glBindBuffer(GL_ARRAY_BUFFER, posHandle);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), &verts[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, uvHandle);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), &texCoords[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, posHandle);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);  // Vertex position

	glBindBuffer(GL_ARRAY_BUFFER, uvHandle);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);  // Texture coords

	glBindVertexArray(0);

	return fullscreenquad;
}