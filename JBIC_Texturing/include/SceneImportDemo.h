#pragma once

#include <Scene.h>

class App;
class Light;
class SceneLoader;
class ShaderProgram;

class SceneImportDemo : public Scene
{
public:
	SceneImportDemo(App* app, const RenderParams& params);
	~SceneImportDemo();

	void setup();

	void update(double delta);

private:
	SceneLoader* m_loader;

	ShaderProgram* m_program;
	Light* m_sceneLight;
};