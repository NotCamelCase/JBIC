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

#include <glm\glm.hpp>

class Camera
{
public:
	Camera(float aspect, float near = 0.1f, float far = 100.f, float fov = 45);
	~Camera();

	const glm::mat4 getMVP(const glm::mat4& modelMatrix) { return m_projMatrix * m_viewMatrix * modelMatrix; }
	const glm::mat4 getViewProjMatrix() const { return m_projMatrix * m_viewMatrix; }

	DECLARE_GETTER(const glm::mat4&, ViewMatrix, m_viewMatrix);
	DECLARE_SETTER_BY_CONST_REF(glm::mat4, ViewMatrix, m_viewMatrix);

	DECLARE_GETTER(const glm::mat4&, ProjMatrix, m_viewMatrix);
	DECLARE_SETTER_BY_CONST_REF(glm::mat4, ProjMatrix, m_projMatrix);

	DECLARE_GETTER_NONCONST(glm::vec3&, Position, m_position);
	DECLARE_SETTER_BY_CONST_REF(glm::vec3, Position, m_position);

	DECLARE_GETTER_NONCONST(glm::vec3&, LookAt, m_lookAtPos);
	DECLARE_SETTER_BY_CONST_REF(glm::vec3, LookAt, m_lookAtPos);

	DECLARE_GETTER_NONCONST(glm::vec3&, LookUp, m_lookUpPos);
	DECLARE_SETTER_BY_CONST_REF(glm::vec3, LookUp, m_lookUpPos);

	DECLARE_GETTER(float, FOV, m_fov);
	DECLARE_SETTER_BY_VALUE(float, FOV, m_fov);

	DECLARE_GETTER(float, AspectRatio, m_aspectRatio);
	DECLARE_SETTER_BY_VALUE(float, AspectRatio, m_aspectRatio);

	DECLARE_GETTER(float, NearDistance, m_nearDist);
	DECLARE_SETTER_BY_VALUE(float, NearDistance, m_nearDist);

	DECLARE_GETTER(float, FarDistance, m_aspectRatio);
	DECLARE_SETTER_BY_VALUE(float, FarDistance, m_farDist);

	void update();

private:
	Camera(const Camera& self);

	float m_fov;
	float m_nearDist;
	float m_farDist;
	float m_aspectRatio;

	glm::vec3 m_position;
	glm::vec3 m_lookAtPos;
	glm::vec3 m_lookUpPos;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projMatrix;

	void updateView();
	void updateProj();
};