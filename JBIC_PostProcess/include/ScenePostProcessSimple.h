#pragma once

#include <Scene.h>

class App;
class Light;
class FrameBuffer;
class SceneLoader;
class ShaderProgram;

typedef unsigned int GLuint;

class ScenePostProcessSimple : public Scene
{
public:
	ScenePostProcessSimple(App* app, const RenderParams& params);
	~ScenePostProcessSimple();

	void setup();

	void update(double delta);

private:
	Light* m_light;
	FrameBuffer* m_fbo;
	SceneLoader* m_loader;
	ShaderProgram* m_shadingProg;
	ShaderProgram* m_postProcessProg;

	void renderScene(double);
	void renderPostProcess(double);

	GLuint m_fsq;
};