#pragma once

#include <d3d11.h>

#include "defines.h"

#include "vertex.h"

struct mesh
{
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;

	D3D11_PRIMITIVE_TOPOLOGY topology;

	unsigned int vertCount;
	unsigned int indexCount;

	UINT stride;
	UINT offset;
};

mesh CreateMesh(ID3D11Device *device, SIMPLE_VERTEX *verts, unsigned int vertCount, D3D11_PRIMITIVE_TOPOLOGY topology, UINT stride = sizeof(SIMPLE_VERTEX), UINT offset = 0);
mesh CreateMeshIndexed(ID3D11Device *device, SIMPLE_VERTEX *verts, unsigned int vertCount, unsigned int *indices, unsigned int indexCount,
	D3D11_PRIMITIVE_TOPOLOGY topology, UINT stride = sizeof(SIMPLE_VERTEX), UINT offset = 0);
void FreeMesh(mesh *m);

void RenderMesh(mesh *m, ID3D11DeviceContext *context);
