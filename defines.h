#pragma once

#define internal_function static
#define global_variable static
#define persistent_variable static

#define SAFE_RELEASE(resource) if((resource)) { (resource)->Release(); (resource) = NULL; }

#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

struct SIMPLE_VERTEX
{
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 color;
	XMFLOAT2 uv;
	XMFLOAT2 padding;
};

union color
{
	unsigned int cint;
	struct
	{
		unsigned char B;
		unsigned char G;
		unsigned char R;
		unsigned char A;
	};
};
