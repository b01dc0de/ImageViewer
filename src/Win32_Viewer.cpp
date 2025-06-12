#include "Common.h" // Includes Win32_Viewer.h
struct ViewerState
{
    bool bInit = false;

    Array<ImageT> LoadedImages{};
	Array<TextureStateT> LoadedTextures{};
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
			ViewerDrawParameters Params = {
				State.CurrIdx < State.LoadedTextures.Num ? &State.LoadedTextures[State.CurrIdx] : nullptr
			};
            Graphics::Draw(Params);
        }
    }
}

void ImageViewer::Init(HINSTANCE hInst, PSTR CmdLine)
{
	if (HWND hWnd = Win32_Init(hInst, WinResX, WinResY))
	{
		hWindow = hWnd;

		HRESULT Result = Graphics::Init();
		if (Result != S_OK) { DebugBreak(); }

		bRunning = Result == S_OK;
		ShowWindow(hWindow, SW_SHOWMAXIMIZED);

		LoadImagesInDirectory();

		State.bInit = true;
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
			else if (VK_LEFT == wParam)
			{
				State.CurrIdx = (State.CurrIdx + 1) % State.LoadedTextures.Num;
			}
			else if (VK_RIGHT == wParam)
			{
				State.CurrIdx = (State.CurrIdx + State.LoadedTextures.Num - 1) % State.LoadedTextures.Num;
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

HWND Win32_Init(HINSTANCE hInstance, int Width, int Height)
{
	RECT WorkArea{};
	SystemParametersInfoA(SPI_GETWORKAREA, 0, &WorkArea, 0);

	WinResX = WorkArea.right - WorkArea.left;
	WinResY = WorkArea.bottom - WorkArea.top;
	ASSERT(WinResX && WinResY);
	WinAspectRatio = (float)WinResX / (float)WinResY;

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

bool IsFileBMP(WIN32_FIND_DATAA& File)
{
	for (int Idx = 0; File.cFileName[Idx] && Idx < (MAX_PATH - 4); Idx++)
	{
		if (File.cFileName[Idx] == '.')
		{
			return File.cFileName[Idx + 1] == 'b' &&
				File.cFileName[Idx + 2] == 'm' &&
				File.cFileName[Idx + 3] == 'p' &&
				File.cFileName[Idx + 4] == '\0';
		}
	}
	return false;
}

void LoadImagesInDirectory()
{
	static constexpr const char* SearchQuery = "Assets\\Test\\*";
	Array<WIN32_FIND_DATAA> FileList;
	WIN32_FIND_DATAA FoundFile;
    HANDLE SearchHandle = FindFirstFileA(SearchQuery, &FoundFile);

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


	for (int Idx = 0; Idx < FileList.Num; Idx++)
	{
		WIN32_FIND_DATAA& CurrFile = FileList[Idx];
		if (IsFileBMP(CurrFile))
		{
			char FullFileName[MAX_PATH];
			sprintf_s(FullFileName, MAX_PATH, "Assets/Test/%s", CurrFile.cFileName);
			ImageT ImageBMP = {};
			ReadBMP(FullFileName, ImageBMP);
			State.LoadedImages.Add(ImageBMP);

			if (ImageBMP.PxBytes)
			{
				TextureStateT TextureBMP = TextureStateT::Init(ImageBMP);
				State.LoadedTextures.Add(TextureBMP);
			}
		}
	}

	static constexpr bool bDebugPrint = true;
	if (bDebugPrint)
	{
        Outf("LoadFilesInDirectory: Found %d files (%s)\n", FileList.Num, SearchQuery);
        for (int Idx = 0; Idx < FileList.Num; Idx++)
        {
            Outf("[%d]: %s\n", Idx, FileList[Idx].cFileName);
        }
	}
}

