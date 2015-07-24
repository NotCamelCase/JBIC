#pragma once

#include <Scene.h>

class App;
class SceneLoader;
class FrameBuffer;
class ShaderProgram;

typedef unsigned int GLuint;

class SceneBloom : public Scene
{
public:
	SceneBloom(App* app, const RenderParams& params);
	~SceneBloom();

	// Inherited via Scene
	virtual void setup() override;
	virtual void update(double delta) override;

	virtual void onKeyPressed(int) override;
	virtual void onKeyReleased(int) override;

	/** Render scene normally with Blinn-Phong shading */
	void renderScene(double delta);

	/*
	* Calculate overall average scene luminance using global Tone Mapping Operator.
	* Can be avoided CPU -> GPU using Compute shaders
	*/
	void calcToneMapping();

private:
	SceneLoader* m_loader;
	FrameBuffer* m_hdrFBO;
	FrameBuffer* m_brightPassFBO; // Downsampled bright pass FBO
	ShaderProgram* m_phongShading;
	ShaderProgram* m_bloomProg;

	GLuint m_fsq;
	GLuint m_horBlurPass, m_vertBlurPass, m_brightPass, m_toneMapPass;

	float* m_hdrBuffer;

	void renderPostProcess();
	void renderBrightPass();
	void renderHorizontalBlurPass();
	void renderVerticalBlurPass();
	void renderToneMapping();
};
