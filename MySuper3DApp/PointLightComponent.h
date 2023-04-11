#pragma once

#include "TransformComponent.h"
#include "Component.h"

using namespace DirectX;

class PointLightComponent : public Component
{
public:

	PointLightComponent(float constant, float linear, float quadratic);
	PointLightComponent() = delete;

	float constant  = 1.0f;
	float linear    = 0.09f;
	float quadratic = 0.032f;
};
