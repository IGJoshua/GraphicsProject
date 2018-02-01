#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct point_light
{
	XMFLOAT4 lightPosition;
	XMFLOAT3 lightColor;
	float lightRadius;
};

struct spot_light
{
	XMFLOAT4 lightPosition;
	XMFLOAT4 lightNormal;
	XMFLOAT4 lightColor;
	float outerRadius;
	float innerRadius;
	XMFLOAT2 padding;
};

struct lights
{
	point_light pointLights[16];
	spot_light spotLights[4];

	XMFLOAT4 directionalLightNormal;
	XMFLOAT4 directionalLightColor;

	XMFLOAT4 ambientLightColor;
};