#pragma once

#include <cstring>

typedef unsigned int GLuint;

class Texture
{
public:
	Texture(const char* name, GLuint format);
	~Texture();

	friend class TextureManager;

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

	const char* getName() const { return m_name; }

	unsigned char* getImageData() const { return m_imageData; }
	GLuint getTexId() const { return m_textureID; }

	friend bool operator==(const Texture& lhs, const Texture& rhs)
	{
		return (lhs.m_textureID == rhs.m_textureID && strcmp(lhs.m_name, rhs.m_name) > 0);
	}

	friend bool operator!=(const Texture& lhs, const Texture& rhs)
	{
		return !(lhs == rhs);
	}

private:
	Texture(const Texture& self);

	GLuint m_textureID;
	GLuint m_pixelFormat;

	unsigned char* m_imageData;

	int m_width, m_height;

	const char* m_name;
};