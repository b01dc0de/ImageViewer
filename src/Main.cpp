#include "Common.h"
#include "Graphics.h"

// Globals
bool bRunning = false;
HWND hWindow = nullptr;
UINT WinResX = 1600U;
UINT WinResY = 900U;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND InitWindow(HINSTANCE hInstance, int Width, int Height)
{
	WNDCLASSEXA WndClass = {};
	WndClass.cbSize = sizeof(WNDCLASSEXA);
	WndClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.hInstance = hInstance;
	WndClass.lpszClassName = APPNAME();

	RegisterClassExA(&WndClass);

	RECT WndRect = { 0, 0, (LONG)Width, (LONG)Height};
	UINT WndStyle = WS_CAPTION | WS_OVERLAPPEDWINDOW;
	UINT WndExStyle = WS_EX_OVERLAPPEDWINDOW;
	AdjustWindowRectEx(&WndRect, WndStyle, FALSE, WndExStyle);

	HWND NewWindow = CreateWindowExA(
		WndExStyle,
		APPNAME(),
		APPNAME(),
		WndStyle,
		0,
		0,
		WndRect.right - WndRect.left,
		WndRect.bottom - WndRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	return NewWindow;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch (uMsg)
	{
		case WM_KEYUP:
		{
			if (VK_ESCAPE == wParam)
			{
				bRunning = false;
			}
		} break;
		case WM_CLOSE:
		{
			bRunning = false;
		} break;
		default:
		{
			Result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
		} break;
	}
	
	return Result;
}

int WindowMsgLoop(HWND hWindow)
{
	MSG Msg = {};
	int MsgCount = 0;
    while (PeekMessageA(&Msg, hWindow, 0, 0, PM_REMOVE) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessageA(&Msg);
		MsgCount++;
    }
	return MsgCount;
}

void Win32_Init()
{
	//SetProcessDPIAware();

	RECT WorkArea{};
	SystemParametersInfoA(SPI_GETWORKAREA, 0, &WorkArea, 0);

	WinResX = WorkArea.right - WorkArea.left;
	WinResY = WorkArea.bottom - WorkArea.top;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	(void)hPrevInst;
	(void)CmdLine;
	Win32_Init();
	if (HWND hWnd = InitWindow(hInst, WinResX, WinResY))
	{
		hWindow = hWnd;

		HRESULT Result = Graphics::Init();
		if (Result != S_OK) { DebugBreak(); }

		bRunning = Result == S_OK;
		ShowWindow(hWindow, WndShow);
		while (bRunning)
		{
			WindowMsgLoop(hWindow);
			UpdateWindow(hWindow);
			Graphics::Draw();
		}

		Graphics::Term();
	}

	return 0;
}

