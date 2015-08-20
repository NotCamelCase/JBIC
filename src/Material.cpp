#include <Material.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <common.h>
#include <Utils.h>

using std::string;
using glm::vec3;

Material::Material(const vec3& ambient, const vec3& diffuse, const vec3& specular, float shininess)
	: m_ambientColor(ambient), m_diffuseColor(diffuse), m_specularColor(specular), m_shininess(shininess)
{
}

Material::~Material()
{
}

Material* Material::create(const std::string& fileName)
{
	std::ifstream matFile("assets/" + fileName, std::ios::in);
	if (!matFile.good())
	{
		LOG_ME("Invalid material file!");

		return nullptr;
	}

	Material* mat = new Material();

	string line;
	std::getline(matFile, line);
	while (!matFile.eof())
	{
		// TODO: Check if it's empty line!
		StringVector elems = Utils::split(line, " ");
		const string firstToken = elems[0];
		if (firstToken == "Ns ")
		{
			mat->setShininess((float)(std::atof(elems[1].c_str())));
		}
		else if (firstToken == "Ka ")
		{
			glm::vec3 ambient;
			ambient.x = (float)(std::atof(elems[1].c_str()));
			ambient.y = (float)(std::atof(elems[2].c_str()));
			ambient.z = (float)(std::atof(elems[3].c_str()));
			mat->setAmbientColor(ambient);
		}
		else if (firstToken == "Kd ")
		{
			glm::vec3 diffuse;
			diffuse.x = (float)(std::atof(elems[1].c_str()));
			diffuse.y = (float)(std::atof(elems[2].c_str()));
			diffuse.z = (float)(std::atof(elems[3].c_str()));
			mat->setDiffuseColor(diffuse);
		}
		else if (firstToken == "Ks ")
		{
			glm::vec3 spec;
			spec.x = (float)(std::atof(elems[1].c_str()));
			spec.y = (float)(std::atof(elems[2].c_str()));
			spec.z = (float)(std::atof(elems[3].c_str()));
			mat->setSpecularColor(spec);
		}

		std::getline(matFile, line);
	}
	matFile.close();

	return mat;
}