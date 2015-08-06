#include <iostream>

#include <App.h>
#include <SceneBloom.h>
#include <SceneGaussianBlur.h>
#include <SceneToneMapping.h>
#include <SceneMotionBlur.h>
#include <ScenePostProcessSimple.h>

int main(int argc, char* argv[])
{
	RenderParams params;
	params.width = 960;
	params.height = 640;
	params.MSAA = 8;
	params.fullscreen = false;

	App* app = new App();
	//Scene* scene = new SceneToneMapping(app, params);
	//Scene* scene = new SceneGaussianBlur(app, params);
	//Scene* scene = new SceneBloom(app, params);
	//Scene* scene = new SceneMotionBlur(app, params);
	Scene* scene = new ScenePostProcessSimple(app, params);
	app->fire(scene);

	SAFE_DELETE(app);

	return 0;
}