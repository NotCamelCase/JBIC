/*
The MIT License (MIT)
Copyright (c) 2015 Tayfun Kayhan
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <vector>

#include <glm\glm.hpp>

typedef unsigned int GLuint;

class ShaderProgram;
class Material;

class Mesh
{
public:
	Mesh(const char* meshName, bool hasUV = false, bool genTangents = false);
	~Mesh();

	void draw();

	void bindBuffers();
	void unbindBuffers();

	void generateTangents(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs, std::vector<glm::vec4>& tangents);

	bool importModel(const char* fileName);

	void createIBO(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs, const std::vector<glm::vec4>& tangents);

	void uploadMaterialProperties(ShaderProgram* program);

	Material* getMaterial() const { return m_material; }
	void setMaterial(Material* mat) { m_material = mat; }

	bool hasUV() const { return m_hasUV; }
	bool hasTangents() const { return m_genTangets; }

private:
	Mesh(const Mesh& self);

	Material* m_material;

	GLuint m_vaoHandle, m_posHandle, m_normalHandle, m_uvHandle, m_tanHandle;
	GLuint m_iboHandle;

	std::vector<unsigned short> m_indices;

	bool m_hasUV;
	bool m_genTangets;
};