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

#include <Mesh.h>

#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

#include <base.h>

#include <Utils.h>
#include <ShaderProgram.h>
#include <Material.h>

using std::vector;
using std::map;
using std::string;
using std::atof;
using std::atoi;
using std::getline;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

typedef unsigned short ushort;

struct Vertex
{
	vec3 pos;
	vec3 normal;
	vec2 uv;
	vec4 tangent;

	bool operator<(const Vertex& other) const
	{
		return memcmp((void*) this, (void*)(&other), sizeof(Vertex)) > 0;
	}
};

Mesh::Mesh(const char* meshName, bool hasUV, bool genTangents)
	: m_hasUV(hasUV), m_genTangets(genTangents),
	m_vaoHandle(0), m_posHandle(0), m_normalHandle(0), m_uvHandle(0), m_tanHandle(0), m_iboHandle(0), m_material(nullptr)
{
	importModel(meshName);
}

Mesh::~Mesh()
{
	SAFE_DELETE(m_material);

	glDeleteBuffers(1, &m_posHandle);
	glDeleteBuffers(1, &m_normalHandle);
	glDeleteBuffers(1, &m_uvHandle);
	glDeleteBuffers(1, &m_tanHandle);
	glDeleteBuffers(1, &m_iboHandle);
	glDeleteVertexArrays(1, &m_vaoHandle);
}

bool Mesh::importModel(const char* fileName)
{
	std::ifstream modelFile(fileName, std::ios::in);
	if (!modelFile.good())
	{
		LOG_ME("Error reading Mesh .obj file!");

		return false;
	}

	vector<uint> vertexIndices, uvIndices, normalIndices;
	vector<vec3> tempVertices, vertices, tempNormals, normals;
	vector<vec2> tempUV, uvs;

	string line;
	getline(modelFile, line);
	while (!modelFile.eof())
	{
		const StringVector elems = Utils::split(line, " ");
		const string firstElem = elems[0];
		if (firstElem == "mtllib ")
		{
			setMaterial(Material::create(elems[1]));
		}
		else if (firstElem == "v ")
		{
			float x = (float)atof(elems[1].c_str());
			float y = (float)atof(elems[2].c_str());
			float z = (float)atof(elems[3].c_str());

			tempVertices.push_back(vec3(x, y, z));
		}
		else if (firstElem == "vn ")
		{
			float x = (float)atof(elems[1].c_str());
			float y = (float)atof(elems[2].c_str());
			float z = (float)atof(elems[3].c_str());

			tempNormals.push_back(vec3(x, y, z));
		}
		else if (m_hasUV && firstElem == "vt ")
		{
			float x = (float)atof(elems[1].c_str());
			float y = (float)atof(elems[2].c_str());

			tempUV.push_back(vec2(x, y));
		}
		else if (firstElem == "f ")
		{
			for (int i = 1; i < 4; i++)
			{
				uint vIndex[3], uvIndex[3], nIndex[3];

				string faceStr = elems[i];
				auto twoSlash = faceStr.find("//");
				uint vind, nind, uvind;
				if (twoSlash != string::npos)
				{
					// Format is missing UV so parse  accordingly

					vind = atoi(faceStr.substr(0, twoSlash).c_str());
					nind = atoi(faceStr.substr(twoSlash + 2, string::npos).c_str());
					vIndex[i - 1] = vind;
					nIndex[i - 1] = nind;
				}
				else
				{
					if (!m_hasUV) continue;

					const size_t slash = faceStr.find("/");
					const size_t secondSlash = faceStr.find("/", slash + 1);
					const size_t thirdSlash = faceStr.find("/", secondSlash + 1);

					vind = atoi(faceStr.substr(0, slash).c_str());
					uvind = atoi(faceStr.substr(slash + 1, secondSlash + 1).c_str());
					nind = atoi(faceStr.substr(secondSlash + 1, string::npos).c_str());
					vIndex[i - 1] = vind;
					nIndex[i - 1] = nind;
					uvIndex[i - 1] = uvind;

					uvIndices.push_back(uvind);
				}

				vertexIndices.push_back(vind);
				normalIndices.push_back(nind);
			}
		}

		getline(modelFile, line);
	}
	modelFile.close();

	for (int i = 0; i < vertexIndices.size(); i++)
	{
		uint vertexIndex = vertexIndices[i];
		uint normalIndex = normalIndices[i];

		vec3 vertex = tempVertices[vertexIndex - 1];
		vec3 normal = tempNormals[normalIndex - 1];

		if (m_hasUV)
		{
			uint uvIndex = uvIndices[i];
			vec2 uv = tempUV[uvIndex - 1];

			uvs.push_back(uv);
		}

		vertices.push_back(vertex);
		normals.push_back(normal);
	}

	vector<vec4> tangents;
	if (m_genTangets)
	{
		generateTangents(vertices, normals, uvs, tangents);
		assert(!tangents.empty() && "Failed to generate tangents!");
	}

	createIBO(vertices, normals, uvs, tangents);

	return true;
}

void Mesh::createIBO(const vector<vec3>& vertices, const vector<vec3>& normals, const vector<vec2>& uvs, const vector<vec4>& tangents)
{
	// Create IBO and fill with correct vertex data

	vector<vec3> finalVertices;
	vector<vec3> finalNormals;
	vector<vec2> finalUVs;
	vector<vec4> finalTangents;
	map<Vertex, ushort> indices;

	// Zero-init finalTangents
	for (uint i = 0; i < tangents.size(); i++) { finalTangents.push_back(vec4(0.0)); }

	for (size_t i = 0; i < vertices.size(); i++)
	{
		Vertex next;
		ushort index;

		next.pos = vertices[i];
		next.normal = normals[i];
		if (m_hasUV && !uvs.empty())
		{
			next.uv = uvs[i];
			if (!tangents.empty())
			{
				next.tangent = tangents[i];
			}
		}

		map<Vertex, ushort>::iterator it = indices.find(next);
		if (it != indices.end())
		{
			// Vertex is already indexed, add index only to the m_indices
			index = it->second;
			m_indices.push_back(index);
		}
		else
		{
			finalVertices.push_back(next.pos);
			finalNormals.push_back(next.normal);
			if (m_hasUV && !uvs.empty())
			{
				finalUVs.push_back(next.uv);
				if (!tangents.empty())
				{
					finalTangents[i] += tangents[i];
					//finalBitangents[i] += bitangents;
				}
			}

			index = (ushort)(finalVertices.size() - 1);
			m_indices.push_back(index);
			indices[next] = index;
		}
	}

	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);

	GLuint vboHandles[2];
	glGenBuffers(2, vboHandles);
	m_posHandle = vboHandles[0];
	m_normalHandle = vboHandles[1];

	glBindBuffer(GL_ARRAY_BUFFER, m_posHandle);
	glBufferData(GL_ARRAY_BUFFER, finalVertices.size() * sizeof(vec3), &finalVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_normalHandle);
	glBufferData(GL_ARRAY_BUFFER, finalNormals.size() * sizeof(vec3), &finalNormals[0], GL_STATIC_DRAW);

	if (m_hasUV)
	{
		glGenBuffers(1, &m_uvHandle);
		glBindBuffer(GL_ARRAY_BUFFER, m_uvHandle);
		glBufferData(GL_ARRAY_BUFFER, finalUVs.size() * sizeof(vec2), &finalUVs[0], GL_STATIC_DRAW);

		if (!finalTangents.empty())
		{
			glGenBuffers(1, &m_tanHandle);
			glBindBuffer(GL_ARRAY_BUFFER, m_tanHandle);
			glBufferData(GL_ARRAY_BUFFER, finalTangents.size() * sizeof(vec4), &finalTangents[0], GL_STATIC_DRAW);
		}
	}

	glGenBuffers(1, &m_iboHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(ushort), &m_indices[0], GL_STATIC_DRAW);
}

void Mesh::bindBuffers()
{
	glBindVertexArray(m_vaoHandle);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, m_posHandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_normalHandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_hasUV)
	{
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, m_uvHandle);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	if (m_genTangets)
	{
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, m_tanHandle);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	}
}

void Mesh::unbindBuffers()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::draw()
{
	bindBuffers();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboHandle);
	glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_SHORT, (void*)0);
	unbindBuffers();
}

void Mesh::generateTangents(vector<vec3>& vertices, vector<vec3>& normals, vector<vec2>& uvs, vector<vec4>& tangents)
{
	assert(m_hasUV && "Error: Mesh has to have UV to approximate tangent!");

	vector<vec3> tans;
	vector<vec3> bitangents;

	// TODO: Implement generating tangents: http://www.terathon.com/code/tangent.html

	for (int i = 0; i < vertices.size(); i += 3)
	{
		vec3 v0 = vertices[i];
		vec3 v1 = vertices[i + 1];
		vec3 v2 = vertices[i + 2];

		vec2 uv0 = uvs[i];
		vec2 uv1 = uvs[i + 1];
		vec2 uv2 = uvs[i + 2];

		vec3 deltaPos1 = v1 - v0;
		vec3 deltaPos2 = v2 - v0;

		vec2 deltaUV1 = uv1 - uv0;
		vec2 deltaUV2 = uv2 - uv0;

		float r = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		tans.push_back(tangent);
		tans.push_back(tangent);
		tans.push_back(tangent);

		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
	}

	for (uint i = 0; i < vertices.size(); i += 1)
	{
		vec3& n = normals[i];
		vec3& t = tans[i];
		vec3& b = bitangents[i];
		vec4 tangent(0.0, 0.0, 0.0, 1.0);

		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));

		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f)
		{
			tangent.w = tangent.w * -1.0f;
		}

		tangent.x = t.x;
		tangent.y = t.y;
		tangent.z = t.z;

		tangents.push_back(tangent);
	}
}

void Mesh::uploadMaterialProperties(ShaderProgram* program)
{
	assert(m_material && "NULL Material !!!");

	program->setUniform("Material.Ka", m_material->getAmbientColor());
	program->setUniform("Material.Kd", m_material->getDiffuseColor());
	program->setUniform("Material.Ks", m_material->getSpecularColor());
	program->setUniform("Material.shininess", m_material->getShininess());
}