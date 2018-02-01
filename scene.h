#pragma once
#include "defines.h"

#include <vector>

#include "vertex.h"
#include "lights.h"
#include "mesh.h"

struct camera
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
};
