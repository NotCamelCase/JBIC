#pragma once

#include <base.h>

class Scene;

class FrameBuffer
{
public:
	/** Frame Buffer Object to be used for off-screen rendering, RTT, post-process effects */
	FrameBuffer(Scene* scene, uint w, uint h, bool useDepth, GLuint rttFormat = GL_RGBA);
	~FrameBuffer();

	DECLARE_GETTER(bool, HasDepthAttachement, m_hasDepthAttachement);

	GLuint getRTTHandle() const { return m_rttHandle; }

	/** Bind FBO to render */
	void bind();

	/** Set back to default frame buffer */
	void unbind();

	static unsigned int s_activeTexID;

private:
	Scene* m_scene;

	GLuint m_handle;
	GLuint m_depthHandle;
	GLuint m_rttHandle;
	GLuint m_rttFormat;

	uint m_width;
	uint m_height;

	bool m_hasDepthAttachement; // If FBO has depth buffer attached
	bool m_fullScreen; // If FBO is covering full screen, avoid calling glViewport()

	/** Set up FBO, fill in RenderToTexture texture, create desired attachements */
	void setupFBO();
};