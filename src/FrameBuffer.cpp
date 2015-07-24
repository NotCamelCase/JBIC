#include "FrameBuffer.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <App.h>
#include <Scene.h>

uint FrameBuffer::s_activeTexID = GL_TEXTURE3;

FrameBuffer::FrameBuffer(Scene* scene, uint w, uint h, bool useDepth, GLuint rttFormat)
	: m_scene(scene), m_width(w), m_height(h), m_handle(useDepth), m_depthHandle(0),
	m_rttFormat(rttFormat), m_fullScreen(true)
{
	m_fullScreen = (m_width == m_scene->getRenderParams().width &&
		m_height == m_scene->getRenderParams().height);

	setupFBO();
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &m_handle);
	if (m_hasDepthAttachement) glDeleteRenderbuffers(1, &m_depthHandle);
}

void FrameBuffer::setupFBO()
{
	glGenFramebuffers(1, &m_handle);
	bind();

	// Create render texture target
	glGenTextures(1, &m_rttHandle);
	glActiveTexture(s_activeTexID);
	glBindTexture(GL_TEXTURE_2D, m_rttHandle);

	// Increment active texture id
	FrameBuffer::s_activeTexID++;

	// Data is NULL for current render texture
	glTexStorage2D(GL_TEXTURE_2D, 1, m_rttFormat, m_width, m_height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rttHandle, 0);

	if (m_hasDepthAttachement)
	{
		// Create render buffer object
		GLuint rbo;
		glGenRenderbuffers(1, &rbo);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ME("Frame buffer set up failed!");
	}

	unbind();
}

void FrameBuffer::bind()
{
	if (!m_fullScreen) // Re-set viewport if not full-screen
	{
		glViewport(0, 0, m_width, m_height);
	}

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Bound to render
	glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
}

void FrameBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}