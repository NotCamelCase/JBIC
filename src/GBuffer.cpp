#include <GBuffer.h>

#include <Scene.h>

GBuffer::GBuffer(Scene* scene, GLuint texUnit)
	: m_texUnitStart(texUnit), m_fbo(0), m_scene(scene)
{
	assert(fillGBuffer() && "Error creating GBuffer content!");
}

GBuffer::~GBuffer()
{
	glDeleteFramebuffers(1, &m_fbo);
}

bool GBuffer::fillGBuffer()
{
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	GLuint renderTex = 0, posTex = 0, normalTex = 0, uvTex = 0;

	glGenRenderbuffers(1, &renderTex);
	glBindRenderbuffer(GL_RENDERBUFFER, renderTex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
		m_scene->getRenderParams().width, m_scene->getRenderParams().height);

	posTex = createGBufferTexture(GBufferTexType::VERTEX_ATTRIB_POS);
	normalTex = createGBufferTexture(GBufferTexType::VERTEX_ATTRIB_NORMAL);
	uvTex = createGBufferTexture(GBufferTexType::VERTEX_ATTRIB_UV);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, uvTex, 0);

	GLenum db[] = { GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(5, db);

	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

GLuint GBuffer::createGBufferTexture(GBufferTexType texType)
{
	GLuint tex;
	glActiveTexture(m_texUnitStart++);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, getTextureFormat(texType), m_scene->getRenderParams().width, m_scene->getRenderParams().height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return tex;
}

uint GBuffer::getTextureFormat(GBufferTexType type)
{
	switch (type)
	{
	case GBufferTexType::VERTEX_ATTRIB_POS:
	case GBufferTexType::VERTEX_ATTRIB_NORMAL:
		return GL_RGB32F;
	case GBufferTexType::VERTEX_ATTRIB_UV:
		return GL_RGB8;
	default:
		return GL_RGB4;
	}
}