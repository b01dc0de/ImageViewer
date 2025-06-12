#include "Common.h"
#include "DX11_Graphics.h"

// Globals
bool bRunning = false;
HWND hWindow = nullptr;
UINT WinResX = 0;
UINT WinResY = 0;
float WinAspectRatio = 0.0f;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, PSTR CmdLine, int WndShow)
{
	(void)hPrevInst;
	(void)CmdLine;
	(void)WndShow;

	ImageViewer::Init(hInst, CmdLine);
	ImageViewer::Run();
	ImageViewer::Term();

	Win32_Init();
	if (HWND hWnd = InitWindow(hInst, WinResX, WinResY))
	{
		hWindow = hWnd;

		QueryFilesInDirectory();

		HRESULT Result = Graphics::Init();
		if (Result != S_OK) { DebugBreak(); }

		bRunning = Result == S_OK;
		ShowWindow(hWindow, SW_SHOWMAXIMIZED);
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

