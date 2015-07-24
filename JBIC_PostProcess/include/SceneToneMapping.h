#pragma once

#include <Scene.h>

class App;
class SceneLoader;
class FrameBuffer;
class ShaderProgram;

typedef unsigned int GLuint;

class SceneToneMapping : public Scene
{
public:
	SceneToneMapping(App* app, const RenderParams& params);
	~SceneToneMapping();

	// Inherited via Scene
	virtual void setup() override;
	virtual void update(double delta) override;

	virtual void onKeyPressed(int) override;
	virtual void onKeyReleased(int) override;

	/** Render scene normally with Blinn-Phong shading */
	void renderScene(double delta);

	/*
	* Calculate overall average scene luminance using global Tone Mapping Operator.
	* Can be avoided CPU -> GPU using FBO & downsampling.
	*/
	void calcToneMapping();

private:
	SceneLoader* m_loader;
	FrameBuffer* m_fbo;
	ShaderProgram* m_phongShading;
	ShaderProgram* m_hdrProg;

	GLuint m_fsq;
	GLuint m_pass1, m_pass2;

	float* m_hdrBuffer;

	void renderPostProcess();
};
