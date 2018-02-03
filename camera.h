#pragma once
#include "defines.h"

struct camera
{
	XMFLOAT4 position;
	float yaw;
	float pitch;
	float fov;
	float nearPlane;
	float farPlane;
};