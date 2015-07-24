#include <Light.h>

#include <Scene.h>

#include <glm\gtc\matrix_transform.hpp>

using namespace glm;

Light::Light(Scene* scene, LightType type, const glm::vec4& pos, LightFrustumParams* params)
	: m_parentScene(scene), m_type(type), m_lightIntensity(.25), m_lightColor(.5), m_position(pos),
	m_frustumParams(params), m_MVP(0.f)
{
	calcLightFrustum();
}

Light::~Light()
{
	SAFE_DELETE(m_frustumParams);
}

void Light::update()
{
	calcLightFrustum();
}

void Light::calcLightFrustum()
{
	mat4 projMat, viewMat; // Light-space view & projection matrices
	if (m_frustumParams->projType == LightFrustumParams::PERSPECTIVE)
	{
		projMat = glm::perspectiveFov<float>(m_frustumParams->fovY,
			(float)m_parentScene->getRenderParams().width, (float)m_parentScene->getRenderParams().height,
			m_frustumParams->nearPlane, m_frustumParams->farPlane);
	}
	else if (m_frustumParams->projType == LightFrustumParams::ORTHO)
	{
		projMat = glm::ortho<float>((float)-m_parentScene->getRenderParams().width,
			(float)m_parentScene->getRenderParams().width, (float)-m_parentScene->getRenderParams().height,
			(float)m_parentScene->getRenderParams().height);
	}

	vec3 center(0.f, -1.f, 1.f), up(0.f, 1.f, 0.f);
	viewMat = glm::lookAt((vec3)m_position, center, up);

	m_MVP = projMat * viewMat;
}