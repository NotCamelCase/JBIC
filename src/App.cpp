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

#include "App.h"

#include <fstream>

#include <Utils.h>
#include <Scene.h>
#include <TextureManager.h>

#include "stb_image_write.h"

#define GL_MAJOR 4
#define GL_MINOR 5

using std::string;

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

#if defined(_DEBUG)
void APIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	printf("GL Debug System: %s\n", message);
}
#endif

void key_callback(GLFWwindow* window, int key, int scanCod, int action, int mods)
{
	App* app = static_cast<App*> (glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS)
	{
		app->onKeyPress(key, scanCod, mods);
		app->getScene()->onKeyPressed(key);
	}
	else if (action == GLFW_RELEASE)
	{
		app->onKeyRelease(key, scanCod, mods);
		app->getScene()->onKeyReleased(key);
	}
}

App::App()
	: m_appWindow(nullptr), m_scene(nullptr)
{
}

void App::fire(Scene* scene)
{
	LOG_ME("App::App()");

	m_scene = scene;

	if (!glfwInit())
	{
		LOG_ME("Error initializing GLFW!");

		return;
	}

	const RenderParams& params = m_scene->getRenderParams();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
	if (params.MSAA > 0)
	{
		glfwWindowHint(GLFW_SAMPLES, params.MSAA);
		glEnable(GL_MULTISAMPLE);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
	}
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, nullptr, true);
#endif
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	m_appWindow = glfwCreateWindow(params.width, params.height, "GL Showcase", params.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	if (!m_appWindow)
	{
		LOG_ME("Error creating GLFW window!");
		exit(EXIT_FAILURE);
	}

	glfwSetWindowUserPointer(m_appWindow, this);
	glfwMakeContextCurrent(m_appWindow);
	glewExperimental = GL_TRUE;
	int glewRes = glewInit();
	assert(glewRes == GLEW_OK && "Error initializing GLEW!");

	glfwSetInputMode(m_appWindow, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(m_appWindow, key_callback);

#if _DEBUG
	const GLubyte* renderer = glGetString(GL_RENDER);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVer = glGetString(GL_SHADING_LANGUAGE_VERSION);

	printf("GL Renderer: %s\t\n", renderer);
	printf("GL Vendor: %s\t\n", vendor);
	printf("GL Version: %s\t\n", version);
	printf("GL Shading Language Version: %s\t\n", glslVer);

	int numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	LOG_ME("Listing supported GL extensions");
	for (int i = 0; i < numExtensions; i++)
	{
		printf("GL Extension: %s\t\n", glGetStringi(GL_EXTENSIONS, i));
	}

	glDebugMessageCallback(opengl_debug_callback, nullptr);
#endif

	LOG_ME("App initialized");

	const double inception = glfwGetTime();
	m_scene->setup();
	const double finish = glfwGetTime();
	printf("******* Scene setup time: %f sec *******\n", finish - inception);
	run();
}

App::~App()
{
	LOG_ME("App::~App()");

	SAFE_DELETE(m_scene);

	TextureManager* texMan = TextureManager::getInstance();
	SAFE_DELETE(texMan);

	glfwDestroyWindow(m_appWindow);
	m_appWindow = nullptr;

	glfwTerminate();

	LOG_ME("Disposed of App");
}

void App::run()
{
	assert(m_scene != nullptr && "Error invalid scene!");

	double frameTime = .0, currTime = .0, delta = .016;
	LOG_ME("App started rendering...");
	while (!glfwWindowShouldClose(m_appWindow))
	{
		currTime = glfwGetTime();
		delta = currTime - frameTime;
		m_scene->update(delta);
		frameTime = glfwGetTime();

		glfwSwapBuffers(m_appWindow);
		glfwPollEvents();
	}
}

bool App::isKeyDown(int key)
{
	return (glfwGetKey(m_appWindow, key) == GLFW_PRESS);
}

bool App::isKeyUp(int key)
{
	return (glfwGetKey(m_appWindow, key) == GLFW_RELEASE);
}

bool App::takeScreenshot()
{
	//TODO: Correct disoriented frame content!

	const RenderParams& params = m_scene->getRenderParams();
	unsigned char* pixels = new unsigned char[params.width * params.height * 4];
	glReadPixels(0, 0, params.width, params.height, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)pixels);
#if _DEBUG
	GLenum error = glGetError();
	if (error == GL_INVALID_OPERATION) LOG_ME("Invalid operation!");
	else if (error == GL_INVALID_VALUE) LOG_ME("Invalid value!");
	else if (error == GL_INVALID_ENUM) LOG_ME("Invalid enum!");
#endif

	int res = stbi_write_png("assets/shot.png", params.width, params.height, 4, pixels, 0);

	SAFE_DELETE_ARRAY(pixels);

	return res != 0;
}

void App::setTitle(const char* title)
{
	glfwSetWindowTitle(m_appWindow, title);
}

void App::onKeyPress(int key, int scanCod, int mods)
{
	switch (key)
	{
	default:
		break;
	}
}

void App::onKeyRelease(int key, int scanCod, int mods)
{
	switch (key)
	{
	case GLFW_KEY_SPACE:
		if (takeScreenshot())
		{
			LOG_ME("Frame screenshot saved");
		}
		break;
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(m_appWindow, 1);
		break;
	default:
		break;
	}
}