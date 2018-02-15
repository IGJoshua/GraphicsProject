//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>

using namespace std;

#include "defines.h"

#include "greendragon.h"
#include "StoneHenge_Texture.h"
#include "StoneHengeSpecular.h"
#include "StoneHenge.h"

#include "TrollFace.h"

#include "DDSTextureLoader.h"
#include "scene.h"
#include "camera.h"
#include "model.h"
#include "XTime.h"

#include "Trivial_VS.csh"
#include "Trivial_PS.csh"
#include "VSSineShader.csh"

#include "psBlank.csh"
#include "SkyboxPS.csh"

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

unsigned int colorTGAConversion(unsigned int initCol)
{
	color ret;
	color tga;
	tga.cint = initCol;
	ret.A = tga.B;
	ret.R = tga.G;
	ret.G = tga.R;
	ret.B = tga.A;
	return ret.cint;
}

struct D3D11Window
{
	HINSTANCE application;
	WNDPROC appWndProc;
	HWND window;

	UINT bufferCount;
	DXGI_FORMAT dxgiFormat;
	UINT swpFlags;

	ID3D11Device *device;
	ID3D11DeviceContext *context;
	ID3D11RenderTargetView *renderTargetView;

	ID3D11Texture2D *depthStencil;
	ID3D11DepthStencilState *depthStencilState;
	ID3D11DepthStencilView *depthStencilView;

	IDXGISwapChain *swapchain;
	ID3D11Resource *pBB;

	D3D11_VIEWPORT viewport;

	ID3D11InputLayout *inputLayout;
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

D3D11Window InitApp(HINSTANCE hinst, WNDPROC proc, unsigned int width, unsigned int height)
{
	D3D11Window wnd = {};

	wnd.application = hinst;
	wnd.appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = wnd.appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = wnd.application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, (long)width, (long)height };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	wnd.window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW, //& ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, wnd.application, &wnd);

	ShowWindow(wnd.window, SW_SHOW);

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	ZeroMemory(&swapchainDesc, sizeof(swapchainDesc));
	swapchainDesc.BufferCount = 1;
	wnd.bufferCount = 1;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	wnd.dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	wnd.swpFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapchainDesc.OutputWindow = wnd.window;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.Windowed = true;

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL,
#ifdef _DEBUG
		D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG,
#else
		NULL,
#endif
		NULL, NULL, D3D11_SDK_VERSION, &swapchainDesc, &wnd.swapchain, &wnd.device, NULL, &wnd.context);

	wnd.swapchain->GetBuffer(0, __uuidof(wnd.pBB), reinterpret_cast<void**>(&wnd.pBB));
	wnd.device->CreateRenderTargetView(wnd.pBB, NULL, &wnd.renderTargetView);

	wnd.viewport.TopLeftX = 0;
	wnd.viewport.TopLeftY = 0;
	wnd.viewport.MinDepth = 0;
	wnd.viewport.MaxDepth = 1;
	wnd.swapchain->GetDesc(&swapchainDesc);
	wnd.viewport.Width = (float)swapchainDesc.BufferDesc.Width;
	wnd.viewport.Height = (float)swapchainDesc.BufferDesc.Height;

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PADDING", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	wnd.device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), Trivial_VS, ARRAYSIZE(Trivial_VS), &wnd.inputLayout);

	D3D11_TEXTURE2D_DESC dsTexDesc;
	ZeroMemory(&dsTexDesc, sizeof(dsTexDesc));
	dsTexDesc.Width = swapchainDesc.BufferDesc.Width;
	dsTexDesc.Height = swapchainDesc.BufferDesc.Height;
	dsTexDesc.MipLevels = 1;
	dsTexDesc.ArraySize = 1;
	dsTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsTexDesc.SampleDesc.Count = 1;
	dsTexDesc.SampleDesc.Quality = 0;
	dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
	dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	wnd.device->CreateTexture2D(&dsTexDesc, NULL, &wnd.depthStencil);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	wnd.device->CreateDepthStencilState(&dsDesc, &wnd.depthStencilState);

	wnd.context->OMSetDepthStencilState(wnd.depthStencilState, 0);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	wnd.device->CreateDepthStencilView(wnd.depthStencil, &dsvDesc, &wnd.depthStencilView);

	return wnd;
}

void InitRender(D3D11Window *wnd)
{
	wnd->context->OMSetDepthStencilState(wnd->depthStencilState, 0);
	wnd->context->OMSetRenderTargets(1, &wnd->renderTargetView, wnd->depthStencilView);

	wnd->context->ClearDepthStencilView(wnd->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	wnd->context->RSSetViewports(1, &wnd->viewport);

	const float black[4] = { 0.125f, 0.3f, 0.5f, 1.0f };
	wnd->context->ClearRenderTargetView(wnd->renderTargetView, black);

	wnd->context->IASetInputLayout(wnd->inputLayout);
}

void EndRender(D3D11Window *wnd)
{
	wnd->swapchain->Present(1, 0);
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool ShutDown(D3D11Window *wnd)
{
	wnd->context->ClearState();

	SAFE_RELEASE(wnd->depthStencil);
	SAFE_RELEASE(wnd->depthStencilState);
	SAFE_RELEASE(wnd->depthStencilView);

	SAFE_RELEASE(wnd->pBB);
	SAFE_RELEASE(wnd->swapchain);
	SAFE_RELEASE(wnd->inputLayout);
	SAFE_RELEASE(wnd->renderTargetView);
	SAFE_RELEASE(wnd->context);
	SAFE_RELEASE(wnd->device);

	UnregisterClass(L"DirectXApplication", wnd->application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

global_variable D3D11Window wnd;
global_variable XMFLOAT4X4 projectionMatrix;
global_variable SHADER_CAMERA defaultCamera;
global_variable unsigned int width = 640;
global_variable unsigned int height = 480;
global_variable XTime timer;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	timer.Restart();


	wnd = InitApp(hInstance, (WNDPROC)WndProc, width, height);

	XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT4 green = { 0.0f, 1.0f, 0.0f, 1.0f };

	// Define a spiral
	unsigned int spiralVertCount = 360 * 16;
	SIMPLE_VERTEX *spiralVerts = new SIMPLE_VERTEX[spiralVertCount];

	for (unsigned int i = 0; i < spiralVertCount; ++i)
	{
		spiralVerts[i].position.x = (float)cos(XMConvertToRadians((float)i)) * 0.5f;
		spiralVerts[i].position.y = (float)sin(XMConvertToRadians((float)i)) * 0.5f;
		spiralVerts[i].position.z = (float)i / (float)spiralVertCount * 10.0f;
		spiralVerts[i].position.w = 1.0f;

		spiralVerts[i].uv = { 0.0f, 0.0f };
		spiralVerts[i].color = white;
		spiralVerts[i].normal = { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	mesh spiralMesh = CreateMesh(wnd.device, spiralVerts, spiralVertCount, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	XMFLOAT4X4 spiralWorldMatrix;
	XMStoreFloat4x4(&spiralWorldMatrix, XMMatrixIdentity());

	ID3D11PixelShader *pixelShaderBlank;
	wnd.device->CreatePixelShader(psBlank, ARRAYSIZE(psBlank), NULL, &pixelShaderBlank);

	ID3D11VertexShader *vertShader;
	wnd.device->CreateVertexShader(&Trivial_VS, ARRAYSIZE(Trivial_VS), NULL, &vertShader);

	model spiral = {};
	spiral.mesh = &spiralMesh;
	spiral.pixelShader = pixelShaderBlank;
	spiral.vertexShader = vertShader;
	spiral.transform = spiralWorldMatrix;

	delete[] spiralVerts;

	// Define a cube
	SIMPLE_VERTEX cubeVerts[] = {
		{ { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, green, { 0.0f, 1.0f }, {} }, // 0
		{ { -0.5f,  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, green, { 0.0f, 0.0f }, {} },
		{ {  0.5f,  0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, green, { 1.0f, 0.0f }, {} },
		{ {  0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, green, { 1.0f, 1.0f }, {} },

		{ {  0.5f, -0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, green,  { 0.0f, 1.0f }, {} }, // 4
		{ {  0.5f,  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, green,  { 0.0f, 0.0f }, {} },
		{ { -0.5f,  0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, green,  { 1.0f, 0.0f }, {} },
		{ { -0.5f, -0.5f,  0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, green,  { 1.0f, 1.0f }, {} },

		{ { -0.5f, -0.5f,  0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, green, { 0.0f, 1.0f }, {} }, // 8
		{ { -0.5f,  0.5f,  0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, green, { 0.0f, 0.0f }, {} },
		{ { -0.5f,  0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, green, { 1.0f, 0.0f }, {} },
		{ { -0.5f, -0.5f, -0.5f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, green, { 1.0f, 1.0f }, {} },

		{ {  0.5f, -0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, green,  { 0.0f, 1.0f }, {} }, // 12
		{ {  0.5f,  0.5f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, green,  { 0.0f, 0.0f }, {} },
		{ {  0.5f,  0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, green,  { 1.0f, 0.0f }, {} },
		{ {  0.5f, -0.5f,  0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, green,  { 1.0f, 1.0f }, {} },

		{ { -0.5f,  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, green,  { 0.0f, 1.0f }, {} }, // 16
		{ { -0.5f,  0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, green,  { 0.0f, 0.0f }, {} },
		{ {  0.5f,  0.5f,  0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, green,  { 1.0f, 0.0f }, {} },
		{ {  0.5f,  0.5f, -0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, green,  { 1.0f, 1.0f }, {} },

		{ { -0.5f, -0.5f,  0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, green,  { 0.0f, 1.0f }, {} }, // 20
		{ { -0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, green,  { 0.0f, 0.0f }, {} },
		{ {  0.5f, -0.5f, -0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, green,  { 1.0f, 0.0f }, {} },
		{ {  0.5f, -0.5f,  0.5f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, green,  { 1.0f, 1.0f }, {} },
	};

	unsigned int cubeIndices[] = {
		0, 1, 2,  2, 3, 0, // Front face
		4, 5, 6,  6, 7, 4, // Back face
		8, 9, 10, 10, 11, 8, // Left face
		12, 13, 14, 14, 15, 12, // Right face
		16, 17, 18, 18, 19, 16, // Top face
		20, 21, 22, 22, 23, 20 // Bottom face
	};

	mesh cubeMesh = CreateMeshIndexed(wnd.device, cubeVerts, 24, cubeIndices, 36, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	unsigned int skyboxIndices[] = {
		2, 1, 0,  0, 3, 2, // Front face
		6, 5, 4,  4, 7, 6, // Back face
		10, 9, 8, 8, 11, 10, // Left face
		14, 13, 12, 12, 15, 14, // Right face
		18, 17, 16, 16, 19, 18, // Top face
		22, 21, 20, 20, 23, 22 // Bottom face
	};

	mesh skyboxMesh = CreateMeshIndexed(wnd.device, cubeVerts, 24, skyboxIndices, 36, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = greendragon_numlevels;

	ID3D11SamplerState *dragonSamplerState;
	wnd.device->CreateSamplerState(&sampDesc, &dragonSamplerState);

	// Load the cube texture
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = greendragon_width;
	texDesc.Height = greendragon_height;
	texDesc.MipLevels = greendragon_numlevels;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Convert the texture
	unsigned int *greendragon_converted = new unsigned int[greendragon_numpixels];
	for (unsigned int i = 0; i < greendragon_numpixels; ++i)
	{
		greendragon_converted[i] = colorTGAConversion(greendragon_pixels[i]);
	}

	D3D11_SUBRESOURCE_DATA srd[10];
	for (int i = 0; i < 10; ++i)
	{
		ZeroMemory(&srd[i], sizeof(srd[i]));
		srd[i].pSysMem = greendragon_converted + greendragon_leveloffsets[i];
		srd[i].SysMemPitch = (UINT)(greendragon_width >> i) * sizeof(unsigned int);
	}

	ID3D11Texture2D *dragonTexture;
	wnd.device->CreateTexture2D(&texDesc, srd, &dragonTexture);

	delete[] greendragon_converted;

	// Load the trollface texture
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = TrollFace_width;
	texDesc.Height = TrollFace_height;
	texDesc.MipLevels = TrollFace_numlevels;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Convert the texture
	unsigned int *TrollFace_converted = new unsigned int[TrollFace_numpixels];
	for (unsigned int i = 0; i < TrollFace_numpixels; ++i)
	{
		TrollFace_converted[i] = colorTGAConversion(TrollFace_pixels[i]);
	}

	for (int i = 0; i < 10; ++i)
	{
		ZeroMemory(&srd[i], sizeof(srd[i]));
		srd[i].pSysMem = TrollFace_converted + TrollFace_leveloffsets[i];
		srd[i].SysMemPitch = (UINT)(TrollFace_width >> i) * sizeof(unsigned int);
	}

	ID3D11Texture2D *trollfaceTexture;
	wnd.device->CreateTexture2D(&texDesc, srd, &trollfaceTexture);

	delete[] TrollFace_converted;

	ID3D11PixelShader *pixelShader;
	wnd.device->CreatePixelShader(Trivial_PS, ARRAYSIZE(Trivial_PS), NULL, &pixelShader);

	model cube = {};
	cube.mesh = &cubeMesh;
	cube.vertexShader = vertShader;
	cube.pixelShader = pixelShader;
	cube.textureSampler = dragonSamplerState;

	ID3D11Texture2D *textures[5] = {};
	textures[0] = dragonTexture;
	textures[1] = trollfaceTexture;
	CreateTextureResourceViews(wnd.device, &cube, textures);

	XMFLOAT4X4 cubeWorldMatrix;
	XMStoreFloat4x4(&cubeWorldMatrix, XMMatrixTranspose(XMMatrixTranslation(-2, 0, 1)));

	cube.transform = cubeWorldMatrix;

	// Define the camera
	camera viewCamera;
	viewCamera.position = { 0, 5, -5, 1 };
	viewCamera.pitch = 0.7f;
	viewCamera.yaw = -0.2f;
	viewCamera.fov = 90;
	viewCamera.nearPlane = 0.1f;
	viewCamera.farPlane = 1000;

	XMFLOAT4X4 viewMatrix;

	// Do the whole constant buffer dance
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.ByteWidth = sizeof(SHADER_CAMERA);

	ID3D11Buffer *cameraConstBuffer;
	wnd.device->CreateBuffer(&constBufferDesc, NULL, &cameraConstBuffer);

	ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.ByteWidth = sizeof(XMFLOAT4X4);

	ID3D11Buffer *objectConstBuffer;
	wnd.device->CreateBuffer(&constBufferDesc, NULL, &objectConstBuffer);

	ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.ByteWidth = sizeof(XMFLOAT4);

	ID3D11Buffer *timeConstBuffer;
	XMFLOAT4 timeFloats = { (float)timer.TotalTimeExact(), 0, 0, 0 };
	D3D11_SUBRESOURCE_DATA timeSRD = {};
	timeSRD.pSysMem = &timeFloats;
	wnd.device->CreateBuffer(&constBufferDesc, &timeSRD, &timeConstBuffer);

	// =========================
	// Define lights

	lights lightsToVRAM = {};
	lightsToVRAM.ambientLightColor = { 0.025f, 0.01f, 0.1f, 1.0f };
	lightsToVRAM.directionalLightColor = { 0.4f, 0.4f, 0.39f, 1.0f };
	lightsToVRAM.directionalLightNormal = { 1.0f, -1.0f, 0.5f, 0.0f };

	lightsToVRAM.pointLights[0].lightColor = { 1.0f, 0.0f, 0.0f };
	lightsToVRAM.pointLights[0].lightPosition = { -3.0f, 2.0f, -3.0f, 1.0f };
	lightsToVRAM.pointLights[0].lightRadius = 20.0f;

	lightsToVRAM.spotLights[0].lightPosition = { -5.0f, 2.0f, 2.0f, 1.0f };
	lightsToVRAM.spotLights[0].lightNormal = { 2.0f, -1.0f, 0.5f, 0.0f };
	lightsToVRAM.spotLights[0].lightColor = { 0.2f, 1.0f, 1.0f, 1.0f };
	lightsToVRAM.spotLights[0].innerRadius = 0.9f;
	lightsToVRAM.spotLights[0].outerRadius = 0.8f;

	ZeroMemory(&constBufferDesc, sizeof(constBufferDesc));
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.ByteWidth = sizeof(lights);

	D3D11_SUBRESOURCE_DATA srd2;
	srd2.pSysMem = &lightsToVRAM;
	ID3D11Buffer *lightsConstBuffer;
	wnd.device->CreateBuffer(&constBufferDesc, &srd2, &lightsConstBuffer);

	int stonehengeVertCount = 1457;
	SIMPLE_VERTEX *stonehengeVerts = new SIMPLE_VERTEX[stonehengeVertCount];

	XMFLOAT4 red = { 1.0, 0.0, 0.0, 1.0 };

	for (int i = 0; i < stonehengeVertCount; ++i)
	{
		SIMPLE_VERTEX temp;
		temp.position = { StoneHenge_data[i].pos[0] / 2, StoneHenge_data[i].pos[1] / 2, StoneHenge_data[i].pos[2] / 2, 1.0f };
		temp.color = red;
		temp.normal = { StoneHenge_data[i].nrm[0], StoneHenge_data[i].nrm[1], StoneHenge_data[i].nrm[2], 0.0f };
		temp.uv = { StoneHenge_data[i].uvw[0], StoneHenge_data[i].uvw[1] };
		temp.padding = { 0.0f, 0.0f };
		stonehengeVerts[i] = temp;
	}

	int stonehengeIndexCount = 2532;
	unsigned int *stonehengeIndices = new unsigned int[stonehengeIndexCount];

	memcpy(stonehengeIndices, StoneHenge_indicies, stonehengeIndexCount * sizeof(unsigned int));

	mesh stonehengeMesh = CreateMeshIndexed(wnd.device, stonehengeVerts, stonehengeVertCount, stonehengeIndices, stonehengeIndexCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	delete[] stonehengeVerts;
	delete[] stonehengeIndices;

	// Texture for stonehenge
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = StoneHenge_numlevels;

	ID3D11SamplerState *stonehengeSamplerState;
	wnd.device->CreateSamplerState(&sampDesc, &stonehengeSamplerState);

	// Load the stonehenge texture
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = StoneHenge_width;
	texDesc.Height = StoneHenge_height;
	texDesc.MipLevels = StoneHenge_numlevels;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Convert the texture
	unsigned int *stonehenge_converted = new unsigned int[StoneHenge_numpixels];
	for (unsigned int i = 0; i < StoneHenge_numpixels; ++i)
	{
		stonehenge_converted[i] = colorTGAConversion(StoneHenge_pixels[i]);
	}

	for (int i = 0; i < 10; ++i)
	{
		ZeroMemory(&srd[i], sizeof(srd[i]));
		srd[i].pSysMem = stonehenge_converted + StoneHenge_leveloffsets[i];
		srd[i].SysMemPitch = (UINT)(StoneHenge_width >> i) * sizeof(unsigned int);
	}

	ID3D11Texture2D *stonehengeTexture;
	wnd.device->CreateTexture2D(&texDesc, srd, &stonehengeTexture);

	delete[] stonehenge_converted;

	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = StoneHengeSpecular_width;
	texDesc.Height = StoneHengeSpecular_height;
	texDesc.MipLevels = StoneHengeSpecular_numlevels;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// Convert the texture
	unsigned int *stonehengespecular_converted = new unsigned int[StoneHengeSpecular_numpixels];
	for (unsigned int i = 0; i < StoneHengeSpecular_numpixels; ++i)
	{
		stonehengespecular_converted[i] = colorTGAConversion(StoneHengeSpecular_pixels[i]);
	}

	for (int i = 0; i < 10; ++i)
	{
		ZeroMemory(&srd[i], sizeof(srd[i]));
		srd[i].pSysMem = stonehengespecular_converted + StoneHengeSpecular_leveloffsets[i];
		srd[i].SysMemPitch = (UINT)(StoneHengeSpecular_width >> i) * sizeof(unsigned int);
	}

	ID3D11Texture2D *stonehengeSpecularMap;
	wnd.device->CreateTexture2D(&texDesc, srd, &stonehengeSpecularMap);

	delete[] stonehengespecular_converted;

	// ===============================
	// Shader Resource View
	ID3D11Texture2D *skyboxTexture;
	ID3D11ShaderResourceView *skyboxSRV;
	CreateDDSTextureFromFile(wnd.device, L"Skybox.dds", (ID3D11Resource**)&skyboxTexture, &skyboxSRV);

	for (int i = 0; i < 4; ++i)
		textures[i] = NULL;

	textures[0] = stonehengeTexture;
	textures[3] = stonehengeSpecularMap;
	textures[4] = skyboxTexture;

	model stonehenge = {};
	stonehenge.mesh = &stonehengeMesh;
	stonehenge.vertexShader = vertShader;
	stonehenge.pixelShader = pixelShader;
	XMStoreFloat4x4(&stonehenge.transform, XMMatrixTranspose(XMMatrixTranslation(-2.3f, -1.7f, 0.9f)));
	stonehenge.textureSampler = stonehengeSamplerState;

	CreateTextureResourceViews(wnd.device, &stonehenge, textures);

	SAFE_RELEASE(stonehenge.shaderResourceViews[4]);
	stonehenge.shaderResourceViews[4] = skyboxSRV;

	// =======================================
	// Set standard const buffer locations

	spiral.transformBuffer = objectConstBuffer;
	cube.transformBuffer = objectConstBuffer;
	stonehenge.transformBuffer = objectConstBuffer;

	// ========================================
	// Set up skybox

	model skybox = {};
	skybox.mesh = &skyboxMesh;

	ID3D11PixelShader *skyboxPixelShader;
	wnd.device->CreatePixelShader(SkyboxPS, ARRAYSIZE(SkyboxPS), NULL, &skyboxPixelShader);

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MinLOD = 0;
	skyboxTexture->GetDesc(&texDesc);
	sampDesc.MaxLOD = (float)texDesc.MipLevels;

	ID3D11SamplerState *skyboxSamplerState;
	wnd.device->CreateSamplerState(&sampDesc, &skyboxSamplerState);

	skybox.pixelShader = skyboxPixelShader;
	skybox.shaderResourceViews[0] = skyboxSRV;
	skybox.vertexShader = vertShader;
	skybox.textureSampler = skyboxSamplerState;
	XMStoreFloat4x4(&skybox.transform, XMMatrixTranspose(XMMatrixTranslationFromVector(XMLoadFloat4(&viewCamera.position))));
	skybox.transformBuffer = objectConstBuffer;

	// =======================================
	// Grid to undulate
	unsigned int gridWidth = 5;
	unsigned int gridHeight = 3;
	unsigned int gridVertCount = (gridWidth + 1) * (gridHeight + 1);
	unsigned int gridIndexCount = gridWidth * gridHeight * 6;
	SIMPLE_VERTEX gridVerts[] = {
		{ { 0.0f, 0, 0.0f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.0f, 0.0f }, {} },
		{ { 0.2f, 0, 0.0f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.2f, 0.0f }, {} },
		{ { 0.4f, 0, 0.0f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.4f, 0.0f }, {} },
		{ { 0.6f, 0, 0.0f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.6f, 0.0f }, {} },
		{ { 0.8f, 0, 0.0f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.8f, 0.0f }, {} },
		{ { 1.0f, 0, 0.0f, 1.0f }, { 0, 1, 0, 0 }, white, { 1.0f, 0.0f }, {} },

		{ { 0.0f, 0, 0.2f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.0f, 0.2f }, {} },
		{ { 0.2f, 0, 0.2f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.2f, 0.2f }, {} },
		{ { 0.4f, 0, 0.2f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.4f, 0.2f }, {} },
		{ { 0.6f, 0, 0.2f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.6f, 0.2f }, {} },
		{ { 0.8f, 0, 0.2f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.8f, 0.2f }, {} },
		{ { 1.0f, 0, 0.2f, 1.0f }, { 0, 1, 0, 0 }, white, { 1.0f, 0.2f }, {} },

		{ { 0.0f, 0, 0.4f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.0f, 0.4f }, {} },
		{ { 0.2f, 0, 0.4f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.2f, 0.4f }, {} },
		{ { 0.4f, 0, 0.4f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.4f, 0.4f }, {} },
		{ { 0.6f, 0, 0.4f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.6f, 0.4f }, {} },
		{ { 0.8f, 0, 0.4f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.8f, 0.4f }, {} },
		{ { 1.0f, 0, 0.4f, 1.0f }, { 0, 1, 0, 0 }, white, { 1.0f, 0.4f }, {} },

		{ { 0.0f, 0, 0.6f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.0f, 0.6f }, {} },
		{ { 0.2f, 0, 0.6f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.2f, 0.6f }, {} },
		{ { 0.4f, 0, 0.6f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.4f, 0.6f }, {} },
		{ { 0.6f, 0, 0.6f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.6f, 0.6f }, {} },
		{ { 0.8f, 0, 0.6f, 1.0f }, { 0, 1, 0, 0 }, white, { 0.8f, 0.6f }, {} },
		{ { 1.0f, 0, 0.6f, 1.0f }, { 0, 1, 0, 0 }, white, { 1.0f, 0.6f }, {} },
	};
	unsigned int gridIndices[216] = {
		7, 1, 0, 6, 7, 0,
		8, 2, 1, 7, 8, 1,
		9, 3, 2, 8, 9, 2,
		10, 4, 3, 9, 10, 3,
		11, 5, 4, 10, 11, 4,

		13, 7, 6, 12, 13, 6,
		14, 8, 7, 13, 14, 7,
		15, 9, 8, 14, 15, 8,
		16, 10, 9, 15, 16, 9,
		17, 11, 10, 13, 17, 10,

		19, 13, 12, 18, 19, 12,
		20, 14, 13, 19, 20, 13,
		21, 15, 14, 20, 21, 14,
		22, 16, 15, 21, 22, 15,
		23, 17, 16, 22, 23, 16,
	};

	mesh gridMesh = CreateMeshIndexed(wnd.device, gridVerts, gridVertCount, gridIndices, gridIndexCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11VertexShader *sineShader;
	wnd.device->CreateVertexShader(VSSineShader, ARRAYSIZE(VSSineShader), NULL, &sineShader);

	model grid = {};
	grid.mesh = &gridMesh;
	grid.pixelShader = pixelShader;
	grid.vertexShader = sineShader;
	XMStoreFloat4x4(&grid.transform, XMMatrixTranspose(XMMatrixScaling(10.0f, 1.0f, 10.0f)));
	grid.transformBuffer = objectConstBuffer;

	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		timer.Signal();

		double delta = timer.Delta();

		// Input (and message handling)

		// Update
		if (GetAsyncKeyState('A'))
		{
			viewCamera.position.x -= (float)(cos(viewCamera.yaw) * timer.Delta() * 3);
			viewCamera.position.z += (float)(sin(viewCamera.yaw) * timer.Delta() * 3);
		}
		if (GetAsyncKeyState('D'))
		{
			viewCamera.position.x += (float)(cos(viewCamera.yaw) * timer.Delta() * 3);
			viewCamera.position.z -= (float)(sin(viewCamera.yaw) * timer.Delta() * 3);
		}
		if (GetAsyncKeyState('W'))
		{
			viewCamera.position.x += (float)(sin(viewCamera.yaw) * timer.Delta() * 3);
			viewCamera.position.z += (float)(cos(viewCamera.yaw) * timer.Delta() * 3);
		}
		if (GetAsyncKeyState('S'))
		{
			viewCamera.position.x -= (float)(sin(viewCamera.yaw) * timer.Delta() * 3);
			viewCamera.position.z -= (float)(cos(viewCamera.yaw) * timer.Delta() * 3);
		}
		if (GetAsyncKeyState(VK_SPACE))
			viewCamera.position.y += (float)(timer.Delta() * 3);
		if (GetAsyncKeyState(VK_LSHIFT))
			viewCamera.position.y -= (float)(timer.Delta() * 3);
		if (GetAsyncKeyState('Q'))
			viewCamera.fov += (float)(timer.Delta() * 100);
		if (GetAsyncKeyState('E'))
			viewCamera.fov -= (float)(timer.Delta() * 100);
		if (GetAsyncKeyState(VK_LEFT))
			viewCamera.yaw -= (float)(timer.Delta() * 1);
		if (GetAsyncKeyState(VK_RIGHT))
			viewCamera.yaw += (float)(timer.Delta() * 1);
		if (GetAsyncKeyState(VK_UP))
			viewCamera.pitch -= (float)(timer.Delta() * 1);
		if (GetAsyncKeyState(VK_DOWN))
			viewCamera.pitch += (float)(timer.Delta() * 1);
		if (GetAsyncKeyState(VK_RSHIFT) != 0 && GetAsyncKeyState(VK_OEM_4) != 0)
		{
			viewCamera.farPlane -= (float)(timer.Delta() * 100);
			if (viewCamera.farPlane < viewCamera.nearPlane + 0.1f) viewCamera.farPlane = viewCamera.nearPlane + 0.1f;
		}
		else if (GetAsyncKeyState(VK_OEM_4))
		{
			viewCamera.nearPlane -= (float)(timer.Delta() * 1);
			if (viewCamera.nearPlane < 0.1f) viewCamera.nearPlane = 0.1f;
		}
		if (GetAsyncKeyState(VK_RSHIFT) != 0 && GetAsyncKeyState(VK_OEM_6) != 0)
		{
			viewCamera.farPlane += (float)(timer.Delta() * 100);
			if (viewCamera.farPlane > 1000) viewCamera.farPlane = 1000;
		}
		else if (GetAsyncKeyState(VK_OEM_6))
		{
			viewCamera.nearPlane += (float)(timer.Delta() * 1);
			if (viewCamera.nearPlane > viewCamera.farPlane - 0.1f) viewCamera.nearPlane = viewCamera.farPlane - 0.1f;
		}

		float s = (float)sin(timer.TotalTime());
		float c = (float)cos(timer.TotalTime());

		lightsToVRAM.pointLights[0].lightPosition.x = s * -5;
		lightsToVRAM.pointLights[0].lightPosition.z = c * -5;

		lightsToVRAM.spotLights[0].lightPosition.x = s * 5;
		lightsToVRAM.spotLights[0].lightPosition.z = c * 5;
		lightsToVRAM.spotLights[0].lightNormal.x = c;
		lightsToVRAM.spotLights[0].lightNormal.z = s;

		lightsToVRAM.directionalLightNormal.x = s;
		lightsToVRAM.directionalLightNormal.z = c;

		lightsToVRAM.cameraWorldPosition = viewCamera.position;

		// Render
		InitRender(&wnd);

		// Setup the camera
		XMMATRIX cameraMatrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixMultiply(
			XMMatrixRotationX(viewCamera.pitch),
			XMMatrixRotationY(viewCamera.yaw)),
			XMMatrixTranslation(viewCamera.position.x, viewCamera.position.y, viewCamera.position.z)));

		XMStoreFloat4x4(&viewMatrix, XMMatrixInverse(NULL, cameraMatrix));
		XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(viewCamera.fov), (float)width / (float)height, viewCamera.nearPlane, viewCamera.farPlane)));

		defaultCamera.viewMatrix = viewMatrix;
		defaultCamera.projectionMatrix = projectionMatrix;
		defaultCamera.totalTime = (float)timer.TotalTime();

		D3D11_MAPPED_SUBRESOURCE msr;
		wnd.context->Map(cameraConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &msr);
		memcpy(msr.pData, &defaultCamera, sizeof(SHADER_CAMERA));
		wnd.context->Unmap(cameraConstBuffer, 0);

		wnd.context->VSSetConstantBuffers(0, 1, &cameraConstBuffer);

		wnd.context->Map(lightsConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &msr);
		memcpy(msr.pData, &lightsToVRAM, sizeof(lights));
		wnd.context->Unmap(lightsConstBuffer, 0);

		wnd.context->PSSetConstantBuffers(0, 1, &lightsConstBuffer);

		// Render skybox
		XMStoreFloat4x4(&skybox.transform, XMMatrixTranspose(XMMatrixTranslationFromVector(XMLoadFloat4(&viewCamera.position))));
		RenderModel(&skybox, wnd.context);

		wnd.context->ClearDepthStencilView(wnd.depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);

		// Render each mesh
		RenderModel(&cube, wnd.context);
		RenderModel(&stonehenge, wnd.context);
		RenderModel(&grid, wnd.context);

		timeFloats.x = (float)timer.TotalTime();
		wnd.context->Map(timeConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &msr);
		memcpy(msr.pData, &timeFloats, sizeof(timeFloats));
		wnd.context->Unmap(timeConstBuffer, 0);
		wnd.context->PSSetConstantBuffers(2, 1, &timeConstBuffer);
		
		RenderModel(&spiral, wnd.context);

		EndRender(&wnd);

		// Timestep
		// Fixed framerate?
	}

	FreeModel(&cube);
	FreeModel(&spiral);
	FreeModel(&stonehenge);

	FreeMesh(&gridMesh);
	FreeMesh(&cubeMesh);
	FreeMesh(&skyboxMesh);
	FreeMesh(&spiralMesh);
	FreeMesh(&stonehengeMesh);

	SAFE_RELEASE(lightsConstBuffer);
	SAFE_RELEASE(cameraConstBuffer);
	SAFE_RELEASE(objectConstBuffer);
	SAFE_RELEASE(timeConstBuffer);

	SAFE_RELEASE(cube.textureSampler);
	SAFE_RELEASE(stonehenge.textureSampler);
	SAFE_RELEASE(skybox.textureSampler);

	SAFE_RELEASE(skyboxTexture);

	SAFE_RELEASE(stonehengeTexture);
	SAFE_RELEASE(stonehengeSpecularMap);
	SAFE_RELEASE(dragonTexture);
	SAFE_RELEASE(trollfaceTexture);

	SAFE_RELEASE(vertShader);
	SAFE_RELEASE(pixelShaderBlank);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(skyboxPixelShader);
	SAFE_RELEASE(sineShader);

	ShutDown(&wnd);

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY):
	{
		PostQuitMessage(0);
	} break;
	case (WM_SIZE):
	{
		if (wnd.swapchain == nullptr) break;
		wnd.context->OMSetRenderTargets(0, 0, 0);
		SAFE_RELEASE(wnd.renderTargetView);
		SAFE_RELEASE(wnd.pBB);
		SAFE_RELEASE(wnd.depthStencilView);
		SAFE_RELEASE(wnd.depthStencilState);
		SAFE_RELEASE(wnd.depthStencil);

		HRESULT hr;
		hr = wnd.swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		// TODO: Error handling

		hr = wnd.swapchain->GetBuffer(0, __uuidof(wnd.pBB), reinterpret_cast<void**>(&wnd.pBB));

		// TODO: Error handling

		hr = wnd.device->CreateRenderTargetView(wnd.pBB, NULL, &wnd.renderTargetView);

		// TODO: Error handling

		wnd.context->OMSetRenderTargets(1, &wnd.renderTargetView, NULL);

		DXGI_SWAP_CHAIN_DESC swapchainDesc;

		wnd.viewport.TopLeftX = 0;
		wnd.viewport.TopLeftY = 0;
		wnd.viewport.MinDepth = 0;
		wnd.viewport.MaxDepth = 1;
		wnd.swapchain->GetDesc(&swapchainDesc);
		wnd.viewport.Width = (float)swapchainDesc.BufferDesc.Width;
		wnd.viewport.Height = (float)swapchainDesc.BufferDesc.Height;

		D3D11_TEXTURE2D_DESC dsTexDesc;
		ZeroMemory(&dsTexDesc, sizeof(dsTexDesc));
		dsTexDesc.Width = swapchainDesc.BufferDesc.Width;
		dsTexDesc.Height = swapchainDesc.BufferDesc.Height;
		dsTexDesc.MipLevels = 1;
		dsTexDesc.ArraySize = 1;
		dsTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsTexDesc.SampleDesc.Count = 1;
		dsTexDesc.SampleDesc.Quality = 0;
		dsTexDesc.Usage = D3D11_USAGE_DEFAULT;
		dsTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		wnd.device->CreateTexture2D(&dsTexDesc, NULL, &wnd.depthStencil);

		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(dsDesc));
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		wnd.device->CreateDepthStencilState(&dsDesc, &wnd.depthStencilState);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(dsvDesc));
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		wnd.device->CreateDepthStencilView(wnd.depthStencil, &dsvDesc, &wnd.depthStencilView);

		width = swapchainDesc.BufferDesc.Width;
		height = swapchainDesc.BufferDesc.Height;
	} break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}