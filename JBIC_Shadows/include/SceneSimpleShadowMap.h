#pragma once

#include <Scene.h>

#include <glm\glm.hpp>

#include <Light.h>

class App;
class SceneLoader;
class ShaderProgram;

typedef unsigned int GLuint;

class SceneSimpleShadowMap : public Scene
{
public:
	SceneSimpleShadowMap(App* app, const RenderParams& params);
	~SceneSimpleShadowMap();

	// Inherited via Scene
	virtual void setup() override;
	virtual void update(double delta) override;

	virtual void onKeyPressed(int) override;
	virtual void onKeyReleased(int) override;

	/** Render scene normally with Blinn-Phong shading */
	void renderScene(double delta);
private:
	SceneLoader* m_loader;
	ShaderProgram* m_phongShading;

	GLuint m_shadowFBO;

	/** Create FBO to render light pass */
	bool createShadowFBO();

	GLuint m_shadowPassHandle, m_shadingHandle;
};