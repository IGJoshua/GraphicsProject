//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>

using namespace std;

// BEGIN PART 1
// TODO: PART 1 STEP 1a
#include <d3d11.h>

// TODO: PART 1 STEP 1b
#include <DirectXMath.h>

using namespace DirectX;

// TODO: PART 2 STEP 6
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"

#define internal_function static
#define global_variable static
#define persistent_variable static

#define SAFE_RELEASE(resource) if((resource)) { (resource)->Release(); (resource) = NULL; }

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

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

	IDXGISwapChain *swapchain;
	ID3D11Resource * pBB;

	D3D11_VIEWPORT viewport;

	ID3D11InputLayout *inputLayout;

	ID3D11VertexShader *vertShader;
	ID3D11PixelShader *pixelShader;
};

struct SIMPLE_VERTEX
{
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 color;
	XMFLOAT2 uv;
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

D3D11Window InitApp(HINSTANCE hinst, WNDPROC proc, unsigned int width, unsigned int height)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
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
	//********************* END WARNING ************************//

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

	wnd.device->CreateVertexShader(&Trivial_VS, ARRAYSIZE(Trivial_VS), NULL, &wnd.vertShader);
	wnd.device->CreatePixelShader(&Trivial_PS, ARRAYSIZE(Trivial_PS), NULL, &wnd.pixelShader);

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	wnd.device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), Trivial_VS, ARRAYSIZE(Trivial_VS), &wnd.inputLayout);

	return wnd;
}

bool Run(D3D11Window wnd)
{
	wnd.context->OMSetRenderTargets(1, &wnd.renderTargetView, 0);
	wnd.context->RSSetViewports(1, &wnd.viewport);

	const float darkBlue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	wnd.context->ClearRenderTargetView(wnd.renderTargetView, darkBlue);

	wnd.context->IASetInputLayout(wnd.inputLayout);

	wnd.context->PSSetShader(wnd.pixelShader, NULL, NULL);
	wnd.context->VSSetShader(wnd.vertShader, NULL, NULL);

	wnd.swapchain->Present(1, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool ShutDown(D3D11Window wnd)
{
	wnd.context->ClearState();

	SAFE_RELEASE(wnd.pBB);
	SAFE_RELEASE(wnd.swapchain);
	SAFE_RELEASE(wnd.inputLayout);
	SAFE_RELEASE(wnd.vertShader);
	SAFE_RELEASE(wnd.pixelShader);
	SAFE_RELEASE(wnd.renderTargetView);
	SAFE_RELEASE(wnd.context);
	SAFE_RELEASE(wnd.device);

	UnregisterClass(L"DirectXApplication", wnd.application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

global_variable D3D11Window wnd;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));

	unsigned int width = 1280;
	unsigned int height = 720;

	wnd = InitApp(hInstance, (WNDPROC)WndProc, width, height);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && Run(wnd))
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	ShutDown(wnd);
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
		INT nHeight = HIWORD(lParam);
		INT nWidth = LOWORD(lParam);
		wnd.swapchain->ResizeBuffers(wnd.bufferCount, nWidth, nHeight, wnd.dxgiFormat, wnd.swpFlags);
	} break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}