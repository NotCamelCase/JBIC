#include <TextureManager.h>

#include <base.h>

#include <Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
	if (!m_textureList.empty())
	{
		for (std::unordered_map<const char*, Texture*>::iterator it = m_textureList.begin(); it != m_textureList.end();)
		{
			if (it->second)
			{
				SAFE_DELETE(it->second);
				it = m_textureList.erase(it);
			}
			else
			{
				it++;
			}
		}
		m_textureList.clear();
	}
}

Texture* TextureManager::createTexture(const char* name, GLuint format)
{
	Texture* newTexture = new Texture(name, format);

	int numComp = getNumComponent(format);
	newTexture->m_imageData = stbi_load(name, &newTexture->m_width, &newTexture->m_height, &numComp, numComp);
	if (newTexture->m_imageData == nullptr)
	{
		SAFE_DELETE(newTexture);
		assert(0 && "ERROR: Reading image pixel data!");
	}

	m_textureList.insert(std::make_pair(name, newTexture));

	return newTexture;
}

int TextureManager::getNumComponent(GLuint format)
{
	switch (format)
	{
	case GL_RGBA:
		return 4;
	case GL_RGB:
		return 3;
	case GL_RG:
		return 2;
	case GL_R:
		return 1;
	default:
		return 3;
	}
}

void TextureManager::destroyTexture(const char* name)
{
	Texture* textureToDelete = getTexture(name);
	SAFE_DELETE(textureToDelete);
}

void TextureManager::destroyTexture(Texture* texture)
{
	SAFE_DELETE(texture);
}

Texture* TextureManager::getTexture(const char* name)
{
	Texture* retval = nullptr;
	const std::unordered_map<const char*, Texture*>::iterator it = m_textureList.find(name);
	if (it != m_textureList.end())
	{
		retval = it->second;
	}

	return retval;
}

bool TextureManager::saveTexture(const Texture* texture)
{
	assert(texture->getImageData() && "Invalid texture data!");

	int pf = getNumComponent(texture->m_pixelFormat);

	return stbi_write_png(strcat(strcat("assets/", texture->getName()), ".png"), texture->getWidth(), texture->getHeight(),
		pf, texture->getImageData(), 0) == 1 ? true : false;
}