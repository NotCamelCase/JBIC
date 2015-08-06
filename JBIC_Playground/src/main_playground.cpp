#include <iostream>

#include <App.h>
#include <SceneDeferred.h>
#include <SceneSSAO.h>

int main(int, char* argv[])
{
	RenderParams params;
	params.width = 1024;
	params.height = 768;
	params.MSAA = 8;
	params.fullscreen = false;

	App* app = new App();
	Scene* scene = nullptr;
	//scene = new SceneDeferred(app, params);
	scene = new SceneSSAO(app, params);
	app->fire(scene);

	SAFE_DELETE(app);

	return 0;
}