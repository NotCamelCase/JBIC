/*
The MIT License (MIT)
Copyright (c) 2015 Tayfun Kayhan
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <vector>

class App;
class Light;
class Camera;
class SceneObject;
class ShaderProgram;

typedef struct RenderParams
{
	int width = 960;
	int height = 640;
	int MSAA = 0;
	bool fullscreen = false;
}RenderParams;

class Scene
{
public:
	Scene(App* app, const RenderParams& params);
	virtual ~Scene();

	/** Set up any VAO, UBO, VBO here */
	virtual void setup() = 0;

	/** Update internal objects such as uniforms and data and render scene */
	virtual void update(double delta) = 0;

	/** Callback function to call on key press */
	virtual void onKeyPressed(int keycode);

	/** Callback function to call on key release */
	virtual void onKeyReleased(int keycode);

	App* getApp() const { return m_app; }

	Camera* getCamera() const { return m_sceneCam; }
	void setCamera(Camera* cam) { m_sceneCam = cam; }

	std::vector<Light*>& getLightList() { return m_lightList; }
	std::vector<SceneObject*>& getSceneObjects() { return m_sceneObjects; }
	std::vector<ShaderProgram*>& getShaderPrograms() { return m_programs; }

	void addShaderProgram(ShaderProgram* sp);
	void addSceneObject(SceneObject* obj);
	void addLight(Light* l);

	const RenderParams& getRenderParams() const { return m_params; }

	float getAspectRatio() const { return ((float)m_params.width / m_params.height); }

	/** Utility function to create full-screen quad for RTT */
	static unsigned int createFullScreenQuad();

protected:
	Scene(const Scene&) {}

	App* m_app;
	Camera* m_sceneCam; // Main camera
	RenderParams m_params; // Scene render parameters

	std::vector<SceneObject*> m_sceneObjects;
	std::vector<ShaderProgram*> m_programs;
	std::vector<Light*> m_lightList;
};