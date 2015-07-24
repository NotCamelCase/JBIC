#pragma once

#include <Scene.h>

class App;
class SkyBox;
class SceneLoader;
class ShaderProgram;

class SceneEnvironmentMap : public Scene
{
public:
	SceneEnvironmentMap(App* app, const RenderParams& params);
	~SceneEnvironmentMap();

	void setup();

	void update(double delta);

private:
	SceneLoader* m_loader;

	ShaderProgram* m_program;

	SkyBox* m_skyBox;
};