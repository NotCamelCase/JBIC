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

#include <glm\glm.hpp>

#include <base.h>

#include <MovableObject.h>

class Scene;

enum class LightType
{
	DIRECTIONAL = 0,
	POINT,
	SPOT
};

struct LightFrustumParams
{
	enum ProjectionType { PERSPECTIVE, ORTHO };

	LightFrustumParams(ProjectionType type) : projType(type) {}

	float nearPlane = 10.f, farPlane = 500.f, fovY = 60.f;
	ProjectionType projType;
};

struct ShadowMapParams
{
	// Shadow map texture resolution
	int resWidth = 4096;
	int resHeight = 4096;
};

class Light : public MovableObject
{
public:
	Light(Scene* scene, LightType type, const glm::vec4& position = glm::vec4(1.f), LightFrustumParams* params = nullptr);
	~Light();

	DECLARE_GETTER(const glm::vec4&, Position, m_position);

	DECLARE_GETTER(LightType, LightType, m_type);

	DECLARE_GETTER(const glm::vec3&, LightColor, m_lightColor)
	DECLARE_SETTER_BY_CONST_REF(glm::vec3, LightColor, m_lightColor);

	DECLARE_GETTER(float, LightIntensity, m_lightIntensity);
	DECLARE_SETTER_BY_VALUE(float, LightIntensity, m_lightIntensity);

	DECLARE_GETTER(float, LinearAttenuation, m_linearAtten);
	DECLARE_SETTER_BY_VALUE(float, LinearAttenuation, m_linearAtten);

	DECLARE_GETTER(float, ConstantAttenuation, m_constantAtten);
	DECLARE_SETTER_BY_VALUE(float, ConstantAttenuation, m_constantAtten);

	DECLARE_GETTER(float, ExponentAttenuation, m_expAtten);
	DECLARE_SETTER_BY_VALUE(float, ExponentAttenuation, m_expAtten);

	DECLARE_GETTER(LightFrustumParams*, LightFrustumParams, m_frustumParams);
	DECLARE_SETTER_BY_VALUE(LightFrustumParams*, LightFrustumParams, m_frustumParams);

	const glm::mat4& getMVP() const { return m_MVP; }

	void update();

private:
	Scene* m_parentScene;

	LightType m_type;

	/** Calculate light-space matrices for WORLD -> SHADOW_COORDS */
	void calcLightFrustum();

	glm::vec4 m_position;

	glm::vec3 m_lightColor;
	float m_lightIntensity;

	LightFrustumParams* m_frustumParams;
	glm::mat4 m_MVP; // ModelViewProjection matrix for light's frustum enclosing the scene

	float m_constantAtten;
	float m_linearAtten;
	float m_expAtten;
};