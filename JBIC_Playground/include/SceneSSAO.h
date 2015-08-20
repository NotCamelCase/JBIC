#pragma once

#include <Scene.h>

#include <glm\glm.hpp>

class App;
class GBuffer;
class SceneLoader;
class ShaderProgram;

typedef unsigned int GLuint;

class SceneSSAO : public Scene
{
public:
	SceneSSAO(App* app, const RenderParams& params);
	~SceneSSAO();

	// Inherited via Scene
	virtual void setup() override;
	virtual void update(double delta) override;

	virtual void onKeyPressed(int) override;
	virtual void onKeyReleased(int) override;

	/** Render scene normally with Blinn-Phong shading */
	void renderScene(double delta);
private:
	SceneLoader* m_loader;
	ShaderProgram* m_deferredShading;
	ShaderProgram* m_geoPass;
	ShaderProgram* m_ssao;

	GBuffer* m_gbuffer;

	GLuint m_fboSSAO;
	GLuint m_fsq;

	void createSSAO_FBO();

	/** Store geometry data of whole scene */
	void geometryPass(double delta);

	/** Shade deferred full-screen quad */
	void deferredShadingPass(double delta);

	/* Generate A*/
	void ssaoPass();
};