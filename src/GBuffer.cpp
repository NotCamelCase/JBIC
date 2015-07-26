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

	GLuint renderTex, posTex, normalTex, matKA, matKD, matKS;

	glGenRenderbuffers(1, &renderTex);
	glBindRenderbuffer(GL_RENDERBUFFER, renderTex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
		m_scene->getRenderParams().width, m_scene->getRenderParams().height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	posTex = createGBufferTexture(GBufferTexType::VERTEX_ATTRIB_POS);
	normalTex = createGBufferTexture(GBufferTexType::VERTEX_ATTRIB_NORMAL);
	matKA = createGBufferTexture(GBufferTexType::MAT_ATTRIB_KA);
	matKD = createGBufferTexture(GBufferTexType::MAT_ATTRIB_KD);
	matKS = createGBufferTexture(GBufferTexType::MAT_ATTRIB_KS);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, matKA, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, matKD, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, matKS, 0);

	// Depth - POS - NORMAL - MAT_KA - MAT_KD - MAT_KS
	GLenum db[] = { GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(6, db);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
	case GBufferTexType::MAT_ATTRIB_KS:
		return GL_RGB8;
	default: // Ka, Kd, Ks
		return GL_RGB8;
	}
}