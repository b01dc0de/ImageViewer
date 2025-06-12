#include "Common.h" // Includes Win32_Viewer.h
struct ViewerState
{
    bool bInit = false;

    Array<ImageT> LoadedImages{};
    size_t CurrIdx = 0;
};

static ViewerState State;

void ImageViewer::Run()
{
    if (State.bInit)
    {
        while (bRunning)
        {
            WindowMsgLoop(hWindow);
            UpdateWindow(hWindow);
            Graphics::Draw();
        }
    }
}

void ImageViewer::Init(HINSTANCE hInst, PSTR CmdLine)
{
	Win32_Init();
	if (HWND hWnd = InitWindow(hInst, WinResX, WinResY))
	{
		hWindow = hWnd;

		QueryFilesInDirectory();

		HRESULT Result = Graphics::Init();
		if (Result != S_OK) { DebugBreak(); }

		bRunning = Result == S_OK;
		ShowWindow(hWindow, SW_SHOWMAXIMIZED);
	}

}

void ImageViewer::Term()
{
    Graphics::Term();
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch (uMsg)
	{
		case WM_SIZE:
		{
			UINT Width = LOWORD(lParam);
			UINT Height = HIWORD(lParam);
			ASSERT(Width && Height);
			WinResX = Width;
			WinResY = Height;
			WinAspectRatio = (float)WinResX / (float)WinResY;
		} break;
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
	RECT WorkArea{};
	SystemParametersInfoA(SPI_GETWORKAREA, 0, &WorkArea, 0);

	WinResX = WorkArea.right - WorkArea.left;
	WinResY = WorkArea.bottom - WorkArea.top;
	ASSERT(WinResX && WinResY);
	WinAspectRatio = (float)WinResX / (float)WinResY;
}

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
	UINT WndExStyle = 0;

	HWND NewWindow = CreateWindowExA(
		WndExStyle,
		APPNAME(),
		APPNAME(),
		WndStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WndRect.right - WndRect.left,
		WndRect.bottom - WndRect.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	return NewWindow;
}

void QueryFilesInDirectory()
{
	Array<WIN32_FIND_DATAA> FileList;
	WIN32_FIND_DATAA FoundFile;
    HANDLE SearchHandle = FindFirstFileA("Assets\\*", &FoundFile);

	bool bDone = !SearchHandle;
	while (!bDone)
	{
		if (!(FoundFile.cFileName[0] == '.' && FoundFile.cFileName[1] == '\0') &&
            !(FoundFile.cFileName[0] == '.' && FoundFile.cFileName[1] == '.' && FoundFile.cFileName[2] == '\0'))
		{
			FileList.Add(FoundFile);
		}
		bDone = !FindNextFileA(SearchHandle, &FoundFile);
	}
	FindClose(SearchHandle);

    Outf("QueryFilesInDirectory: Found %d files\n", FileList.Num);
    for (int Idx = 0; Idx < FileList.Num; Idx++)
    {
        Outf("[%d]: %s\n", Idx, FileList[Idx].cFileName);
    }
}

