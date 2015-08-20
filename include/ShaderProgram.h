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

#include "common.h"

#include <map>
#include <string>
using std::string;

#include <glm\glm.hpp>

enum ShaderType
{
	UNDEFINED = 0x0f,
	VERTEX = GL_VERTEX_SHADER,
	FRAGMENT = GL_FRAGMENT_SHADER,
	GEOMETRY = GL_GEOMETRY_SHADER,
	COMPUTE = GL_COMPUTE_SHADER,
	TESS_EVAL = GL_TESS_EVALUATION_SHADER,
	TESS_CONTROL = GL_TESS_CONTROL_SHADER
};

/* Encapsulates a GLSL program */
class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	/** Determine shader type from fileName, read it and compile it */
	bool compileShader(const string& fileName);

	/** Make program that's compiled and linked the current one */
	void use();

	/** Link program that's compiled */
	void link();

	/** Validate current shader program */
	void validate();

	// General-use wrappers
	void setUniform(const char* name, float x, float y, float z);
	void setUniform(const char* name, const glm::vec2 & v);
	void setUniform(const char* name, const glm::vec3 & v);
	void setUniform(const char* name, const glm::vec4 & v);
	void setUniform(const char* name, const glm::mat4 & m);
	void setUniform(const char* name, const glm::mat3 & m);
	void setUniform(const char* name, float val);
	void setUniform(const char* name, int val);
	void setUniform(const char* name, bool val);
	void setUniform(const char* name, GLuint val);

	void bindAttribute(const char* name, int index);
	void bindFragmentAttribute(const char* name, int index);

	GLuint getHandle() const { return m_programHandle; }

	GLint getUniformLocation(const char* uniformDataName);

	/** Returns if program is already linked */
	bool isLinked() const { return m_linked; }

private:
	ShaderProgram(const ShaderProgram& self);

	GLuint m_programHandle;

	// Uniform name : GL Uniform location map
	std::map<const char*, GLint> m_uniformLocations;

	bool m_linked;

	/** Compile shader of specified type from source */
	bool compileShader(const char* source, ShaderType type);
};