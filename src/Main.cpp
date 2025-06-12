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

	return 0;
}

