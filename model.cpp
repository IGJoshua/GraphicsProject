#include "model.h"

void CreateTextureResourceViews(ID3D11Device *device, model *m, ID3D11Texture2D *textureMaps[5])
{
	D3D11_TEXTURE2D_DESC texDesc;

	for (int i = 0; i < 5; ++i)
	{
		SAFE_RELEASE(m->shaderResourceViews[i]);

		if (textureMaps[i])
		{
			textureMaps[i]->GetDesc(&texDesc);
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = texDesc.Format;
			srvDesc.Buffer.ElementOffset = 0;
			srvDesc.Buffer.ElementWidth = sizeof(unsigned int);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;

			device->CreateShaderResourceView(textureMaps[i], &srvDesc, &m->shaderResourceViews[i]);
		}
	}
}

void FreeModel(model *m)
{
	for (int i = 0; i < 5; ++i)
	{
		SAFE_RELEASE(m->shaderResourceViews[i]);
	}
}

void RenderModel(model *m, ID3D11DeviceContext *context)
{
	context->VSSetShader(m->vertexShader, NULL, NULL);

	context->PSSetShader(m->pixelShader, NULL, NULL);
	for (int i = 0; i < 5; ++i)
	{
		if (m->shaderResourceViews[i])
			context->PSSetShaderResources(i, 1, &m->shaderResourceViews[i]);
	}
	if (m->textureSampler)
		context->PSSetSamplers(0, 1, &m->textureSampler);

	D3D11_MAPPED_SUBRESOURCE msr;
	context->Map(m->transformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	memcpy(msr.pData, &m->transform, sizeof(XMFLOAT4X4));
	context->Unmap(m->transformBuffer, 0);

	context->VSSetConstantBuffers(1, 1, &m->transformBuffer);

	RenderMesh(m->mesh, context);

	ID3D11ShaderResourceView *nullView[5] = {};
	context->PSSetShaderResources(0, 4, nullView);

	ID3D11Buffer *cb = NULL;
	context->VSSetConstantBuffers(1, 1, &cb);
	ID3D11SamplerState *ss = NULL;
	context->PSSetSamplers(0, 1, &ss);

	context->VSSetShader(NULL, NULL, NULL);
	context->PSSetShader(NULL, NULL, NULL);
}