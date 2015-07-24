#include "Skybox.h"

#include <TextureManager.h>
#include <Texture.h>

SkyBox::SkyBox(const std::string& baseName)
	: m_vaoHandle(0), m_baseName(baseName)
{
	LOG_ME("Creating cube map texture resource for SkyBox based on name " + m_baseName);
	GLuint cubeMapID;
	glGenTextures(1, &cubeMapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapID);
	glActiveTexture(GL_TEXTURE0);
	const std::string names[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	const GLuint targets[] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, 2048, 2048);
	for (uint i = 0; i < 6; i++)
	{
		std::string textureName = "assets/" + m_baseName + "_" + names[i] + ".jpg";
		Texture* faceTex = TextureManager::getInstance()->createTexture(textureName.c_str(), GL_RGBA);
		glTexSubImage2D(targets[i], 0, 0, 0, faceTex->getWidth(), faceTex->getHeight(),
			GL_RGBA, GL_UNSIGNED_BYTE, faceTex->getImageData());
		TextureManager::getInstance()->destroyTexture(faceTex); // Uploaded to VRAM, no need to keep on RAM, too
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	const float side = 50.0f;
	const float side2 = side / 2.0f;
	float v[24 * 3] = {
		// Front
		-side2, -side2, side2,
		side2, -side2, side2,
		side2,  side2, side2,
		-side2,  side2, side2,
		// Right
		side2, -side2, side2,
		side2, -side2, -side2,
		side2,  side2, -side2,
		side2,  side2, side2,
		// Back
		-side2, -side2, -side2,
		-side2,  side2, -side2,
		side2,  side2, -side2,
		side2, -side2, -side2,
		// Left
		-side2, -side2, side2,
		side2,  side2, side2,
		-side2,  side2, -side2,
		-side2, -side2, -side2,
		// Bottom
		-side2, -side2, side2,
		-side2, -side2, -side2,
		side2, -side2, -side2,
		side2, -side2, side2,
		// Top
		-side2,  side2, side2,
		side2,  side2, side2,
		side2,  side2, -side2,
		-side2,  side2, -side2
	};

	GLuint el[] = {
		0,2,1,0,3,2,
		4,6,5,4,7,6,
		8,10,9,8,11,10,
		12,14,13,12,15,14,
		16,18,17,16,19,18,
		20,22,21,20,23,22
	};

	GLuint handle[2];
	glGenBuffers(2, handle);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]); // Vertex buffer
	glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), v, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[1]); // Index buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), el, GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[1]);

	glBindVertexArray(0);
}

SkyBox::~SkyBox()
{
}

void SkyBox::render()
{
	LOG_ME("Rendering SkyBox...");
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(m_vaoHandle);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
	glEnable(GL_DEPTH_TEST);
	LOG_ME("SkyBox rendered");
}