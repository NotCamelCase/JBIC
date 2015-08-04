#pragma once

#include <Scene.h>

#include <glm\mat4x4.hpp>

class App;
class Light;
class SceneLoader;
class ShaderProgram;

typedef unsigned int GLuint;

class SceneMotionBlur : public Scene
{
public:
	SceneMotionBlur(App* app, const RenderParams& params);
	~SceneMotionBlur();

	void setup();

	void update(double delta);

private:
	Light* m_light;
	SceneLoader* m_loader;
	ShaderProgram* m_shadingProg;
	ShaderProgram* m_motionShader;

	void renderScene(double);
	void renderBlurred(double);

	bool createFBO();

	GLuint m_fbo;
	GLuint m_fsq;
};