#pragma once

#include <Scene.h>

class App;
class Light;
class SceneLoader;
class ShaderProgram;

class SceneNormalMapping : public Scene
{
public:
	SceneNormalMapping(App* app, const RenderParams& params);
	~SceneNormalMapping();

	void setup();

	void update(double delta);

private:
	SceneLoader* m_loader;

	ShaderProgram* m_program;
	Light* m_sceneLight;
};