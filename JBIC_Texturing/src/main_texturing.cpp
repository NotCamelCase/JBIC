#include <iostream>

#include <App.h>
#include <SceneEnvironmentMap.h>
#include <SceneNormalMapping.h>
#include <SceneImportDemo.h>
#include <SceneRTT.h>

int main(int argc, char* argv[])
{
	RenderParams params;
	params.width = 960;
	params.height = 640;
	params.MSAA = 8;
	params.fullscreen = false;

	App* test = new App();
	Scene* scene = new SceneRTT(test, params);
	test->fire(scene);

	SAFE_DELETE(test);

	return 0;
}