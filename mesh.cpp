#include "mesh.h"
#include <fstream>
#include <vector>

mesh LoadMesh(ID3D11Device *device, std::string fileName)
{
	std::ifstream file(fileName.c_str());

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT2> UVs;
	std::vector<XMFLOAT3> normals;

	struct vert
	{
		unsigned int posIndex;
		unsigned int uvIndex;
		unsigned int normIndex;
		bool operator==(vert other)
		{
			return this->posIndex == other.posIndex && this->uvIndex == other.posIndex && this->normIndex == other.normIndex;
		}
	};

	std::vector<vert> verts;

	std::vector<unsigned int> faceIndices;

	enum supported_ops
	{
		SUPPORTED_POS,
		SUPPORTED_UV,
		SUPPORTED_NORM,
		SUPPORTED_FACE,
		UNSUPPORTED_OP
	};

	void (*getIndices)(std::string, unsigned int[3]) = [](std::string vert, unsigned int out[3])
	{
		unsigned int firstSlash = vert.find('/');
		unsigned int secondSlash = vert.find('/', firstSlash + 1);
		std::string indexes[3];
		indexes[0] = vert.substr(0, firstSlash);
		indexes[1] = vert.substr(firstSlash + 1, secondSlash - firstSlash - 1);
		indexes[2] = vert.substr(secondSlash + 1);

		for (unsigned int i = 0; i < 3; ++i)
			out[i] = std::stoi(indexes[i]) - 1;
	};

	while (file.good())
	{
		std::string opType;
		file >> opType;

		file.get();

		char c_line[1024] = { 0 };
		file.getline(c_line, 1023);
		std::string line = c_line;
		
		supported_ops inputType = opType == "v"
			? SUPPORTED_POS
			: opType == "vt"
			? SUPPORTED_UV
			: opType == "vn"
			? SUPPORTED_NORM
			: opType == "f"
			? SUPPORTED_FACE
			: UNSUPPORTED_OP;

		switch (inputType)
		{
			case SUPPORTED_POS:
			{
				std::string posStrings[3];

				unsigned int firstSpace = line.find(' ');
				unsigned int secondSpace = line.find(' ', firstSpace + 1);

				posStrings[0] = line.substr(0, firstSpace);
				posStrings[1] = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
				posStrings[2] = line.substr(secondSpace + 1);

				positions.push_back(XMFLOAT3(std::stof(posStrings[0]), std::stof(posStrings[1]), std::stof(posStrings[2])));
			} break;
			case SUPPORTED_UV:
			{
				std::string uvStrings[2];

				unsigned int space = line.find(' ');

				uvStrings[0] = line.substr(0, space);
				uvStrings[1] = line.substr(space + 1);

				UVs.push_back(XMFLOAT2(std::stof(uvStrings[0]), std::stof(uvStrings[1])));
			} break;
			case SUPPORTED_NORM:
			{
				std::string normStrings[3];

				unsigned int firstSpace = line.find(' ');
				unsigned int secondSpace = line.find(' ', firstSpace + 1);

				normStrings[0] = line.substr(0, firstSpace);
				normStrings[1] = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
				normStrings[2] = line.substr(secondSpace + 1);

				normals.push_back(XMFLOAT3(std::stof(normStrings[0]), std::stof(normStrings[1]), std::stof(normStrings[2])));
			} break;
			case SUPPORTED_FACE:
			{
				std::string vertStrings[3];

				unsigned int firstSpace = line.find(' ');
				unsigned int secondSpace = line.find(' ', firstSpace + 1);
				vertStrings[0] = line.substr(0, firstSpace);
				vertStrings[1] = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
				vertStrings[2] = line.substr(secondSpace + 1);

				unsigned int vert1[3];
				getIndices(vertStrings[0], vert1);

				unsigned int vert2[3];
				getIndices(vertStrings[1], vert2);

				unsigned int vert3[3];
				getIndices(vertStrings[2], vert3);

				vert v[3] = { 0 };

				v[0].posIndex = vert1[0];
				v[0].uvIndex = vert1[1];
				v[0].normIndex = vert1[2];

				v[1].posIndex = vert2[0];
				v[1].uvIndex = vert2[1];
				v[1].normIndex = vert2[2];

				v[2].posIndex = vert3[0];
				v[2].uvIndex = vert3[1];
				v[2].normIndex = vert3[2];

				for (unsigned int i = 0; i < 3; ++i)
				{
					std::vector<vert>::iterator vertPos = std::find(verts.begin(), verts.end(), v[i]);
					if (vertPos != verts.end())
					{
						// Push something onto the index buffer
						faceIndices.push_back(std::distance(verts.begin(), vertPos));
					}
					else
					{
						// Push something onto the vertex buffer
						verts.push_back(v[i]);
						// Push something onto the index buffer
						faceIndices.push_back(verts.size() - 1);
					}
				}

			} break;
			default:
			{

			} break;
		}

	}

	SIMPLE_VERTEX *sVerts = new SIMPLE_VERTEX[verts.size()];
	unsigned int index = 0;

	// Convert from vertex indices into actual SIMPLE_VERTEX structures
	for (std::vector<vert>::iterator itr = verts.begin();
		itr != verts.end();
		++itr)
	{
		SIMPLE_VERTEX v;
		v.position = XMFLOAT4(positions[itr->posIndex].x, positions[itr->posIndex].y, positions[itr->posIndex].z, 1);
		v.uv = UVs[itr->uvIndex];
		v.normal = XMFLOAT4(normals[itr->normIndex].x, normals[itr->normIndex].y, normals[itr->normIndex].z, 0);
		v.color = XMFLOAT4(0, 0, 0, 1);
		sVerts[index++] = v;
	}

	mesh ret = CreateMeshIndexed(device, sVerts, verts.size(), faceIndices.data(), faceIndices.size(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	delete[] sVerts;

	return ret;
}

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