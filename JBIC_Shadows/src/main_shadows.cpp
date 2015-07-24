#include <iostream>

#include <App.h>
#include <SceneSimpleShadowMap.h>
#include <SceneDeferredPCF.h>

int main(int, char* argv[])
{
	RenderParams params;
	params.width = 1024;
	params.height = 768;
	params.MSAA = 8;
	params.fullscreen = false;

	App* app = new App();
	Scene* scene = nullptr;
	//scene = new SceneSimpleShadowMap(app, params);
	scene = new SceneDeferredPCF(app, params);
	app->fire(scene);

	SAFE_DELETE(app);

	return 0;
}