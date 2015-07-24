#pragma once

#include <Scene.h>

class ShaderProgram;
class SceneObject;
class Light;

class SceneLighting : public Scene
{
public:
	SceneLighting(App* app);
	~SceneLighting();

	void setup();

	void update(double delta);

private:
	ShaderProgram* m_adsShader;

	SceneObject* m_plane;
	SceneObject* m_monkey;

	Light* m_light;
};