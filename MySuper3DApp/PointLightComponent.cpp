#include "PointLightComponent.h"
#include "GameObject.h"
#include "Game.h"
#include "DisplayWin32.h"

PointLightComponent::PointLightComponent(float constant, float linear, float quadratic, float fovAngle, float nearZ, float farZ)
{
	this->constant  = constant;
	this->linear    = linear;
	this->quadratic = quadratic;
	this->fovAngle  = fovAngle;
	this->aspect = 1.0f;
	this->nearZ = 0.1f;
	this->farZ = 100.0f;
}

void PointLightComponent::Initialize()
{
	aspect = Game::GetInstance()->GetDisplay()->GetClientWidth() / Game::GetInstance()->GetDisplay()->GetClientHeight();
}

Matrix PointLightComponent::GetViewMatrix()
{
	return gameObject->transformComponent->GetView();
}

Matrix PointLightComponent::GetProjectionMatrix()
{
	return Matrix::CreatePerspectiveFieldOfView(
		fovAngle,
		aspect,
		nearZ,
		farZ
	);
}