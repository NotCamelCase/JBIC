#pragma once

#include <Scene.h>

class App;
class Light;
class FrameBuffer;
class SceneLoader;
class ShaderProgram;

typedef unsigned int GLuint;

class SceneGaussianBlur : public Scene
{
public:
	SceneGaussianBlur(App* app, const RenderParams& params);
	~SceneGaussianBlur();

	void setup();

	void update(double delta);

	void onKeyPressed(int key);
	void onKeyReleased(int key);

private:
	Light* m_light;
	FrameBuffer* m_fbo;
	FrameBuffer* m_tempFbo;
	SceneLoader* m_loader;
	ShaderProgram* m_shadingProg;
	ShaderProgram* m_postProcessProg;

	void renderScene(double);
	void renderPostProcess();
	void renderVerticalBlurPass();
	void renderHorizontalBlurPass();

	GLuint m_fsq;
	GLuint m_verticalPass, m_horizontalPass;
};