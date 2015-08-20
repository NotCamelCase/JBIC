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

#include <common.h>

#include <string>
using std::string;

class Scene;

class App
{
public:
	/** Sets up window and glfw; fires up scene specified in app.config with assigned properties */
	App();
	~App();

	DECLARE_GETTER(Scene*, Scene, m_scene);
	DECLARE_SETTER_BY_VALUE(Scene*, Scene, m_scene);

	DECLARE_GETTER(GLFWwindow*, GLFWWindowHandle, m_appWindow);

	/** Returns true if key is currently pressed */
	bool isKeyDown(int key);

	/** Returns true if key is currently released */
	bool isKeyUp(int key);

	/** TODO: Write frame buffer content to disk */
	bool takeScreenshot();

	/** Set current window's title */
	void setTitle(const char* title);

	/** Start up App with specified showcase entry */
	void fire(Scene* scene);

	/** Callback to be called upon keyboard press*/
	void onKeyPress(int key, int scanCod, int mods);

	/** Callback to be called upon keyboard release*/
	void onKeyRelease(int key, int scanCod, int mods);

private:
	GLFWwindow* m_appWindow; // GLFW window handle
	Scene* m_scene; // Main scene to be rendered

	/** Start off app's continuous loop */
	void run();
};