#include "model.h"

void RenderModel(model *m, ID3D11DeviceContext *context)
{
	context->PSSetShader(m->pixelShader, NULL, NULL);
	if (m->shaderResourceView)
	{
		context->PSSetShaderResources(0, 1, &m->shaderResourceView);
		context->PSSetSamplers(0, 1, &m->textureSampler);
	}

	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(m->transformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &m->transform, sizeof(XMFLOAT4X4));
	context->Unmap(m->transformBuffer, 0);

	context->VSSetConstantBuffers(1, 1, &m->transformBuffer);

	RenderMesh(m->mesh, context);
}