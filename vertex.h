#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct SIMPLE_VERTEX
{
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 color;
	XMFLOAT2 uv;
	XMFLOAT2 padding;
};
