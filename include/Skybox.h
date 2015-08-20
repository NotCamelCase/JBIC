#pragma once

#include <string>

#include <common.h>

class SkyBox
{
public:
	/** Load cube map texture, initialize cube map and create vertices & indexes to be used when rendering */
	SkyBox(const std::string& baseName);
	~SkyBox();

	/* Render SkyBox using baseName cubemap texture. SkyBox renders disabling depth check */
	void render();

private:
	std::string m_baseName;

	GLuint m_vaoHandle;
};