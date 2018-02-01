#include "mesh.h"

mesh CreateMesh(ID3D11Device *device, SIMPLE_VERTEX *verts, unsigned int vertCount, D3D11_PRIMITIVE_TOPOLOGY topology, UINT stride, UINT offset)
{
	mesh ret = {};

	ret.vertCount = vertCount;
	ret.indexCount = 0;
	ret.topology = topology;
	ret.stride = stride;
	ret.offset = offset;
	
	ret.indexBuffer = NULL;

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * vertCount;

	D3D11_SUBRESOURCE_DATA srd;
	ZeroMemory(&srd, sizeof(srd));
	srd.pSysMem = verts;

	device->CreateBuffer(&bufferDesc, &srd, &ret.vertexBuffer);

	return ret;
}

mesh CreateMeshIndexed(ID3D11Device *device, SIMPLE_VERTEX *verts, unsigned int vertCount, unsigned int *indices, unsigned int indexCount,
	D3D11_PRIMITIVE_TOPOLOGY topology, UINT stride, UINT offset)
{
	mesh ret = {};

	ret.vertCount = vertCount;
	ret.indexCount = indexCount;
	ret.topology = topology;
	ret.stride = stride;
	ret.offset = offset;

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * vertCount;

	D3D11_SUBRESOURCE_DATA srd;
	ZeroMemory(&srd, sizeof(srd));
	srd.pSysMem = verts;

	device->CreateBuffer(&bufferDesc, &srd, &ret.vertexBuffer);

	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(unsigned int) * indexCount;

	ZeroMemory(&srd, sizeof(srd));
	srd.pSysMem = indices;

	device->CreateBuffer(&bufferDesc, &srd, &ret.indexBuffer);

	return ret;
}

void FreeMesh(mesh *m)
{
	SAFE_RELEASE(m->indexBuffer);
	SAFE_RELEASE(m->vertexBuffer);
	m->indexCount = 0;
	m->vertCount = 0;
	m->topology = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;
	m->offset = 0;
	m->stride = 0;
}

void RenderMesh(mesh *m, ID3D11DeviceContext *context)
{
	UINT strides = m->stride;
	UINT offsets = m->offset;
	context->IASetVertexBuffers(0, 1, &m->vertexBuffer, &strides, &offsets);
	context->IASetPrimitiveTopology(m->topology);

	if (m->indexBuffer)
	{
		context->IASetIndexBuffer(m->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->DrawIndexed(m->indexCount, m->offset, 0);
	}
	else
	{
		context->Draw(m->vertCount, m->offset);
	}
}