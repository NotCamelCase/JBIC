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

#include <MovableObject.h>

#include <glm\gtc\matrix_transform.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;
using glm::mat4;
using glm::mat3;
using glm::quat;

MovableObject::MovableObject()
	: m_worldTransform(1.f)
{
}

MovableObject::~MovableObject()
{
}

void MovableObject::translate(const vec3& offset)
{
	m_worldTransform = glm::translate(m_worldTransform, offset);
}

void MovableObject::scale(float x, float y, float z)
{
	m_worldTransform = glm::scale(m_worldTransform, vec3(x, y, z));
}

void MovableObject::rotate(float angle, const vec3& about)
{
	m_worldTransform = glm::rotate(m_worldTransform, glm::radians(angle), about);
}