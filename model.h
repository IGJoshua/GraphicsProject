#pragma once

#include "defines.h"

#include "mesh.h"

struct model
{
	mesh *mesh;

	ID3D11PixelShader *pixelShader;

	ID3D11SamplerState *textureSampler;
	ID3D11ShaderResourceView *shaderResourceView;

	XMFLOAT4X4 transform;
	ID3D11Buffer *transformBuffer;
};

void RenderModel(model *m, ID3D11DeviceContext *context);
