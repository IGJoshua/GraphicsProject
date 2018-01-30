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

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500

#define SAFE_RELEASE(resource) if(resource) { resource->Release(); resource = NULL; }

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	// TODO: PART 1 STEP 2
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	ID3D11RenderTargetView *renderTargetView;

	IDXGISwapChain *swapchain;
	ID3D11Resource * pBB;

	D3D11_VIEWPORT viewport;

	ID3D11InputLayout *inputLayout;

	ID3D11VertexShader *vertShader;
	ID3D11PixelShader *pixelShader;

public:
	struct SIMPLE_VERTEX
	{
		XMFLOAT2 position;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"CGS Hardware Project", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	ZeroMemory(&swapchainDesc, sizeof(swapchainDesc));
	swapchainDesc.BufferCount = 1;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapchainDesc.OutputWindow = window;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.Windowed = true;

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL,
#ifdef _DEBUG
		D3D11_CREATE_DEVICE_FLAG::D3D11_CREATE_DEVICE_DEBUG,
#else
		NULL,
#endif
		NULL, NULL, D3D11_SDK_VERSION, &swapchainDesc, &swapchain, &device, NULL, &context);

	swapchain->GetBuffer(0, __uuidof(pBB), reinterpret_cast<void**>(&pBB));
	device->CreateRenderTargetView(pBB, NULL, &renderTargetView);

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	swapchain->GetDesc(&swapchainDesc);
	viewport.Width = (float)swapchainDesc.BufferDesc.Width;
	viewport.Height = (float)swapchainDesc.BufferDesc.Height;

	device->CreateVertexShader(&Trivial_VS, ARRAYSIZE(Trivial_VS), NULL, &vertShader);
	device->CreatePixelShader(&Trivial_PS, ARRAYSIZE(Trivial_PS), NULL, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device->CreateInputLayout(layoutDesc, 1, Trivial_VS, ARRAYSIZE(Trivial_VS), &inputLayout);
}

bool DEMO_APP::Run()
{
	context->OMSetRenderTargets(1, &renderTargetView, 0);
	context->RSSetViewports(1, &viewport);

	const float darkBlue[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, darkBlue);

	context->IASetInputLayout(inputLayout);

	context->PSSetShader(pixelShader, NULL, NULL);
	context->VSSetShader(vertShader, NULL, NULL);

	swapchain->Present(1, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	// TODO: PART 1 STEP 6
	context->ClearState();

	SAFE_RELEASE(pBB);
	SAFE_RELEASE(swapchain);
	SAFE_RELEASE(inputLayout);
	SAFE_RELEASE(vertShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(renderTargetView);
	SAFE_RELEASE(context);
	SAFE_RELEASE(device);

	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY): { PostQuitMessage(0); }
					   break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//