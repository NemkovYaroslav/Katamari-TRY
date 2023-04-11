#pragma once

#include "TransformComponent.h"
#include "Component.h"

using namespace DirectX;

class PointLightComponent : public Component
{
public:

	PointLightComponent(float constant, float linear, float quadratic, float fovAngle, float nearZ, float farZ);
	PointLightComponent() = delete;

	virtual void Initialize() override;

	float constant  = 1.0f;
	float linear    = 0.09f;
	float quadratic = 0.032f;

	float fovAngle = DirectX::XM_PIDIV2;
	float aspect;
	float nearZ;
	float farZ;

	Matrix GetViewMatrix();
	Matrix GetProjectionMatrix();
};
