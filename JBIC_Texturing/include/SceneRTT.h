#pragma once

#include <Scene.h>

class App;
class Light;
class FrameBuffer;
class SceneLoader;
class ShaderProgram;

class SceneRTT : public Scene
{
public:
	SceneRTT(App* app, const RenderParams& params);
	~SceneRTT();

	void setup();

	void update(double delta);

private:
	Light* m_light;
	FrameBuffer* m_fbo;
	SceneLoader* m_loader;
	ShaderProgram* m_program;

	void renderScene(double);
	void renderToTexture(double);
};