#include <Scene.h>

class App;
class Light;
class Texture;
class SceneObject;
class ShaderProgram;

class SceneTexturing : public Scene
{
public:
	SceneTexturing(App* app, const RenderParams& params);
	~SceneTexturing();

	void setup();

	void update(double delta);

private:
	ShaderProgram* m_texturedLighting;

	SceneObject* m_cube;
	Texture* m_cubeTexture;

	Light* m_sceneLight;
};