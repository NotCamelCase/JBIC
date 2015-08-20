#pragma once

#include <glm\vec3.hpp>

#include <string>

#include <common.h>

class Material
{
public:
	Material(const glm::vec3& ambient = glm::vec3(0.5, .5, .5), const glm::vec3& diffuse = glm::vec3(0.5, .5, .5), const glm::vec3& specular = glm::vec3(0.5, .5, .5), float sh = 200.f);
	~Material();

	static Material* create(const std::string& fileName);

	DECLARE_GETTER(const glm::vec3&, AmbientColor, m_ambientColor);
	DECLARE_GETTER(const glm::vec3&, DiffuseColor, m_diffuseColor);
	DECLARE_GETTER(const glm::vec3&, SpecularColor, m_specularColor);

	DECLARE_GETTER(float, Shininess, m_shininess);

	DECLARE_SETTER_BY_CONST_REF(glm::vec3, AmbientColor, m_ambientColor);
	DECLARE_SETTER_BY_CONST_REF(glm::vec3, DiffuseColor, m_diffuseColor);
	DECLARE_SETTER_BY_CONST_REF(glm::vec3, SpecularColor, m_specularColor);

	DECLARE_SETTER_BY_VALUE(float, Shininess, m_shininess);

private:
	glm::vec3 m_ambientColor;
	glm::vec3 m_diffuseColor;
	glm::vec3 m_specularColor;

	float m_shininess;
};