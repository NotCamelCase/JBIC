#include <Texture.h>

#include <base.h>

#include "stb_image.h"

Texture::Texture(const char* name, GLuint format)
	: m_name(name), m_width(0), m_height(0), m_textureID(0), m_imageData(nullptr), m_pixelFormat(format)
{
	glGenTextures(1, &m_textureID);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_textureID);

	if (m_imageData)
	{
		stbi_image_free(m_imageData);
		m_imageData = nullptr;
	}
}