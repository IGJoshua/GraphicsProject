#pragma once

#include "defines.h"

#include "mesh.h"

struct model
{
	mesh *mesh;

	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;

	ID3D11SamplerState *textureSampler;
	ID3D11ShaderResourceView *shaderResourceViews[4];

	XMFLOAT4X4 transform;
	ID3D11Buffer *transformBuffer;
};

void CreateTextureResourceViews(ID3D11Device *device, model *m, ID3D11Texture2D *textureMaps[4]);
void FreeModel(model *m);

void RenderModel(model *m, ID3D11DeviceContext *context);