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

#include <Camera.h>

#include <glm/gtc/matrix_transform.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

Camera::Camera(float aspect, float near, float far, float fov)
	: m_aspectRatio(aspect), m_fov(fov), m_viewMatrix(mat4(1.0f)), m_projMatrix(mat4(1.0f)), m_nearDist(near), m_farDist(far),
	m_position(vec3(5.0, 2.5, 5.0)), m_lookAtPos(vec3(0.0, 0.0, 0.0)), m_lookUpPos(vec3(0.0, 1.0, 0.0))
{
}

Camera::~Camera()
{
}

void Camera::update()
{
	updateView();
	updateProj();
}

void Camera::updateView()
{
	m_viewMatrix = glm::lookAt(m_position, m_lookAtPos, m_lookUpPos);
}

void Camera::updateProj()
{
	m_projMatrix = glm::perspective(m_fov, m_aspectRatio, m_nearDist, m_farDist);
}