#pragma once
#include "defines.h"

#include "lights.h"
#include "mesh.h"

struct SHADER_CAMERA
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	float totalTime;
	XMFLOAT3 padding;
};
