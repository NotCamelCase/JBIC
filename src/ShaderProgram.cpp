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

#include "ShaderProgram.h"

#include <fstream>
#include <sstream>

#include <Utils.h>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

ShaderProgram::ShaderProgram()
	: m_programHandle(0), m_linked(nullptr)
{
}

ShaderProgram::~ShaderProgram()
{
	LOG_ME("ShaderProgram::~ShaderProgram()");

	GLsizei numShadersAttached;
	glGetProgramiv(m_programHandle, GL_ATTACHED_SHADERS, &numShadersAttached);

	GLuint* attachedShaderNames = new GLuint[numShadersAttached];
	glGetAttachedShaders(m_programHandle, numShadersAttached, nullptr, attachedShaderNames);
	for (int i = 0; i < numShadersAttached; i++)
	{
		glDeleteShader(attachedShaderNames[i]);
	}

	m_uniformLocations.clear();
	glDeleteProgram(m_programHandle);

	delete[] attachedShaderNames;

	LOG_ME("Disposed of ShaderProgram");
}

GLint ShaderProgram::getUniformLocation(const char* uniformDataName)
{
	std::map<const char*, GLint>::const_iterator it = m_uniformLocations.find(uniformDataName);
	if (it != m_uniformLocations.end())
	{
		return it->second;
	}
	else
	{
		const GLint loc = glGetUniformLocation(m_programHandle, uniformDataName);
		if (loc < 0)
		{
			printf("Uniform location lookup of %s failed!\n", uniformDataName);
		}

		m_uniformLocations[uniformDataName] = loc;

		return loc;
	}
}

bool ShaderProgram::compileShader(const string& fileName)
{
	ShaderType type = ShaderType::UNDEFINED;
	const string& ext = Utils::split(fileName, ".", true);
	if (ext == "vert" || ext == "vs")
	{
		type = ShaderType::VERTEX;
	}
	else if (ext == "frag" || ext == "fs")
	{
		type = ShaderType::FRAGMENT;
	}
	else if (ext == "geo" || ext == "gs")
	{
		type = ShaderType::GEOMETRY;
	}
	else if (ext == "comp" || ext == "cs")
	{
		type = ShaderType::COMPUTE;
	}
	else if (ext == "tes")
	{
		type = ShaderType::TESS_EVAL;
	}
	else if (ext == "tcs")
	{
		type = ShaderType::TESS_CONTROL;
	}
	assert(type != ShaderType::UNDEFINED && "Error finding shader source type!");

	std::ifstream sourceFile(fileName, std::ios::in);
	assert(sourceFile.good() && "Error reading shader source!");
	std::stringstream shaderContent;
	shaderContent << sourceFile.rdbuf();
	sourceFile.close();

	return compileShader(shaderContent.str().c_str(), type);
}

bool ShaderProgram::compileShader(const char* source, ShaderType type)
{
	LOG_ME("Compiling shader");

	bool retval = false;

	if (m_programHandle == 0)
	{
		m_programHandle = glCreateProgram();
		assert(m_programHandle != 0 && "Error creating shader program handle!");
	}

	GLuint handle = glCreateShader(static_cast<GLenum> (type));
	glShaderSource(handle, 1, &source, nullptr);
	glCompileShader(handle);

	GLint res;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length);
		if (length > 1)
		{
			char* compileLog = new char[length];
			glGetShaderInfoLog(handle, length, nullptr, compileLog);
			LOG_ME(compileLog);
			delete[] compileLog;
			assert("Check shader compile log!" && false);
		}
	}
	else
	{
		glAttachShader(m_programHandle, handle);

		retval = true;

		LOG_ME("Shader compiled and attached successfully");
	}

	return retval;
}

void ShaderProgram::use()
{
	assert(m_programHandle != 0 && "Invalid shader program handle!");

	glUseProgram(m_programHandle);
}

void ShaderProgram::link()
{
	// Dismiss if already linked
	if (m_linked) return;

	assert(m_programHandle != 0 && "ERROR: Shader program handle corrupted");

	glLinkProgram(m_programHandle);
	GLint res;
	glGetProgramiv(m_programHandle, GL_LINK_STATUS, &res);
	if (res == GL_FALSE)
	{
		LOG_ME("Error linking program!");
		GLint length;
		glGetProgramiv(m_programHandle, GL_INFO_LOG_LENGTH, &length);
		if (length > 1)
		{
			char* reasonLog = new char[length];
			glGetProgramInfoLog(m_programHandle, length, nullptr, reasonLog);
			printf("Linkage error: %s\n", reasonLog);
			delete[] reasonLog;
		}
	}
	else
	{
		LOG_ME("Shader program linked successfully!");

		m_linked = true;

		m_uniformLocations.clear();
	}
}

void ShaderProgram::validate()
{
	LOG_ME("Validating shader");

	assert(isLinked() && "ShaderProgram is not linked; first link it!");

	glValidateProgram(m_programHandle);
	GLint isValid;
	glGetShaderiv(m_programHandle, GL_VALIDATE_STATUS, &isValid);
	if (isValid == GL_FALSE)
	{
		GLint length = 0;
		glGetShaderiv(m_programHandle, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			char* logg = new char[length];
			glGetShaderInfoLog(m_programHandle, length, 0, logg);
			printf("Error validating shader: %s\n", logg);
			delete[] logg;
		}
		return;
	}

	LOG_ME("Shader validated");
}

void ShaderProgram::bindAttribute(const char* name, int index)
{
	glBindAttribLocation(m_programHandle, index, name);
}

void ShaderProgram::bindFragmentAttribute(const char* name, int index)
{
	glBindFragDataLocation(m_programHandle, index, name);
}

void ShaderProgram::setUniform(const char* name, float x, float y, float z)
{
	GLint loc = getUniformLocation(name);
	glUniform3f(loc, x, y, z);
}

void ShaderProgram::setUniform(const char* name, const vec3& v)
{
	this->setUniform(name, v.x, v.y, v.z);
}

void ShaderProgram::setUniform(const char* name, const vec4& v)
{
	GLint loc = getUniformLocation(name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void ShaderProgram::setUniform(const char* name, const vec2& v)
{
	GLint loc = getUniformLocation(name);
	glUniform2f(loc, v.x, v.y);
}

void ShaderProgram::setUniform(const char* name, const mat4& m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

void ShaderProgram::setUniform(const char* name, const mat3& m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

void ShaderProgram::setUniform(const char* name, float val)
{
	GLint loc = getUniformLocation(name);
	glUniform1f(loc, val);
}

void ShaderProgram::setUniform(const char* name, int val)
{
	GLint loc = getUniformLocation(name);
	glUniform1i(loc, val);
}

void ShaderProgram::setUniform(const char* name, GLuint val)
{
	GLint loc = getUniformLocation(name);
	glUniform1ui(loc, val);
}

void ShaderProgram::setUniform(const char* name, bool val)
{
	int loc = getUniformLocation(name);
	glUniform1i(loc, val);
}