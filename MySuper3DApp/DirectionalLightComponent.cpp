#include "DirectionalLightComponent.h"
#include "GameObject.h"
#include "Game.h"
#include "DisplayWin32.h"
#include "RenderSystem.h"

DirectionalLightComponent::DirectionalLightComponent(int shadowMapSize, float viewWidth, float viewHeight, float nearZ, float farZ)
{
	this->shadowMapSize = shadowMapSize;
	this->viewWidth     = viewWidth;
	this->viewHeight    = viewHeight;
	this->nearZ         = nearZ;
	this->farZ          = farZ;
}

void DirectionalLightComponent::Initialize()
{
	D3D11_TEXTURE2D_DESC textureDesc = {}; // ok
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = static_cast<float>(shadowMapSize);
	textureDesc.Height = static_cast<float>(shadowMapSize);
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 4; // shadowCascadeLevels.size()
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	auto result = Game::GetInstance()->GetRenderSystem()->device->CreateTexture2D(&textureDesc, nullptr, shadowMapTexture2D.GetAddressOf());
	assert(SUCCEEDED(result));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc; // ok
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;
	depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthStencilViewDesc.Texture2DArray.ArraySize = 4; // shadowCascadeLevels.size()
	result = Game::GetInstance()->GetRenderSystem()->device->CreateDepthStencilView(shadowMapTexture2D.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
	assert(SUCCEEDED(result));

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc; // ok
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.ArraySize = 4; // shadowCascadeLevels.size()
	result = Game::GetInstance()->GetRenderSystem()->device->CreateShaderResourceView(shadowMapTexture2D.Get(), &shaderResourceViewDesc, textureResourceView.GetAddressOf());
	assert(SUCCEEDED(result));

	viewport = std::make_shared<D3D11_VIEWPORT>(); // ok
	viewport->Width = static_cast<float>(shadowMapSize);
	viewport->Height = static_cast<float>(shadowMapSize);
	viewport->MinDepth = 0.0f;
	viewport->MaxDepth = 1.0f;
	viewport->TopLeftX = 0.0f;
	viewport->TopLeftY = 0.0f;
}

Matrix DirectionalLightComponent::GetViewMatrix()
{
	return gameObject->transformComponent->GetView();
}

Matrix DirectionalLightComponent::GetProjectionMatrix()
{
	return XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);
}


std::vector<Vector4> DirectionalLightComponent::GetFrustumCornerWorldSpace(const Matrix& view, const Matrix& proj)
{
	const auto viewProj = view * proj;
	const auto inv = viewProj.Invert();

	std::vector<Vector4> frustumCorners;
	frustumCorners.reserve(8);
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const Vector4 pt =
					Vector4::Transform(Vector4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						z,
						1.0f), inv);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}
	return frustumCorners;
}

Matrix DirectionalLightComponent::GetLightSpaceMatrix(const float nearZ, const float farZ)
{
	Matrix rotation = Matrix::CreateFromAxisAngle(Vector3::Up, DirectX::XM_PI);
	const Matrix perspective = rotation * Matrix::CreatePerspectiveFieldOfView(
		Game::GetInstance()->currentCamera->fovAngle,
		Game::GetInstance()->currentCamera->aspect,
		nearZ,
		farZ
	);
	const std::vector<Vector4> corners = GetFrustumCornerWorldSpace(Game::GetInstance()->currentCamera->gameObject->transformComponent->GetView(), perspective);

	Vector3 center = Vector3::Zero;
	for (const auto& v : corners)
	{
		center += Vector3(v);
	}
	center /= corners.size();
	const auto lightView = Matrix::CreateLookAt(
		center,
		center + direction,
		Vector3::Up
	);

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		const auto trf = Vector4::Transform(v, lightView);
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}
	constexpr float zMult = 10.0f;

	minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
	maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

	auto lightProjection = Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);
	return lightView * lightProjection;
}

std::vector<Matrix> DirectionalLightComponent::GetLightSpaceMatrices()
{
	std::vector<Matrix> ret;
	ret.push_back(GetLightSpaceMatrix(Game::GetInstance()->currentCamera->nearZ, shadowCascadeLevels[0]));
	ret.push_back(GetLightSpaceMatrix(shadowCascadeLevels[0], shadowCascadeLevels[1]));
	ret.push_back(GetLightSpaceMatrix(shadowCascadeLevels[1], shadowCascadeLevels[2]));
	ret.push_back(GetLightSpaceMatrix(shadowCascadeLevels[2], Game::GetInstance()->currentCamera->farZ));
	return ret;
}