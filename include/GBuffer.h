#pragma once

#include <base.h>

class Scene;

class GBuffer
{
public:
	GBuffer(Scene* scene, GLuint texUnit);
	~GBuffer();

	/** Fill in GBuffer content with necessary vertex attribs */
	bool fillGBuffer();

	enum GBufferTexType
	{
		// Vertex attributes
		VERTEX_ATTRIB_POS,
		VERTEX_ATTRIB_NORMAL,
		VERTEX_ATTRIB_UV,

		MAT_ATTRIB_KD,
		MAT_ATTRIB_KS,
		MAT_ATTRIB_KA
	};

	/** Create a GBuffer texture for each vertex attrib to be fed */
	GLuint createGBufferTexture(GBufferTexType texType);

	/** Returns the last activated texture slot */
	GLuint getTexUnit() const { return m_texUnitStart; }

	GLuint getFBO() const { return m_fbo; }

private:
	Scene* m_scene;

	GLuint m_fbo;

	GLuint m_texUnitStart;

	/** Get GBuffer texture format in which to store vertex attrib data */
	uint getTextureFormat(GBufferTexType type);
};