#include "Common.h" // Includes Win32_Viewer.h

struct ViewerState
{
    bool bInit = false;

    Array<ImageT> LoadedImages{};
	Array<TextureStateT> LoadedTextures{};
    size_t CurrIdx = 0;

	void Release()
	{
		for (int Idx = 0; Idx < LoadedImages.Num; Idx++)
		{
			SafeRelease(LoadedImages[Idx]);
		}

		for (int Idx = 0; Idx < LoadedTextures.Num; Idx++)
		{
			SafeRelease(LoadedTextures[Idx]);
		}
	}
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

void ImageViewer::Init(HINSTANCE hInst, LPSTR CmdLine)
{
	if (HWND hWnd = Win32_Init(hInst))
	{
		hWindow = hWnd;

        ASSERT(Graphics::Init() == S_OK);

		ShowWindow(hWindow, SW_SHOWNORMAL);

		LoadImagesInDirectory();

		State.bInit = true;
	}
    bRunning = State.bInit;
}

void ImageViewer::Term()
{
	State.Release();
    Graphics::Term();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = 0;
	switch (uMsg)
	{
        case WM_GETMINMAXINFO:
        {
			MINMAXINFO* OutWinInfo = (MINMAXINFO*)lParam;
			ASSERT(OutWinInfo);
			OutWinInfo->ptMinTrackSize.x = 100;
			OutWinInfo->ptMinTrackSize.y = 100;
        } break;
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

HWND Win32_Init(HINSTANCE hInstance)
{
#if CONFIG_DEBUG()
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif // CONFIG_DEBUG()

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

	RECT WndRect = { 0, 0, (LONG)WinResX, (LONG)WinResY};
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

void LoadImagesInDirectory()
{
	//static constexpr const char* DirectoryToLoad = "Assets/Test";
	static constexpr const char* DirectoryToLoad = "Assets/Test/PNG";
	char SearchQuery[MAX_PATH] = {};
	sprintf_s(SearchQuery, MAX_PATH, "%s\\*", DirectoryToLoad);
	for (int Idx = 0; Idx < MAX_PATH && SearchQuery[Idx]; Idx++)
	{
		if (SearchQuery[Idx] == '/') { SearchQuery[Idx] = '\\'; }
	}

	Array<WIN32_FIND_DATAA> FileList;
	WIN32_FIND_DATAA FoundFile;
    HANDLE SearchHandle = FindFirstFileA(SearchQuery, &FoundFile);

	bool bDone = !SearchHandle;
	while (!bDone)
	{
		if (!MatchStr(".", FoundFile.cFileName) && !MatchStr("..", FoundFile.cFileName))
		{
			FileList.Add(FoundFile);
		}
		bDone = !FindNextFileA(SearchHandle, &FoundFile);
	}
	FindClose(SearchHandle);

	for (int Idx = 0; Idx < FileList.Num; Idx++)
	{
		WIN32_FIND_DATAA& CurrFile = FileList[Idx];
		ImageFileType Type = GetImageFileType(CurrFile.cFileName);
		ImageT NewImage = {};
        char FullFileName[MAX_PATH];
        sprintf_s(FullFileName, MAX_PATH, "%s/%s", DirectoryToLoad, CurrFile.cFileName);
		switch (Type)
		{
			case ImageFileType::Invalid:
			{
			} break;
			case ImageFileType::BMP:
			{
                ReadBMP(FullFileName, NewImage);
            } break;
			case ImageFileType::PNG:
			{
				ReadPNG(FullFileName, NewImage);
			} break;
		}

        if (NewImage.PxBytes)
        {
            TextureStateT NewTexture = TextureStateT::Init(NewImage);
            State.LoadedImages.Add(NewImage);
            State.LoadedTextures.Add(NewTexture);
        }
	}

	static constexpr bool bDebugPrint = false;
	if (bDebugPrint)
	{
        Outf("LoadFilesInDirectory: Found %d files (%s)\n", FileList.Num, SearchQuery);
        for (int Idx = 0; Idx < FileList.Num; Idx++)
        {
            Outf("\t[%d]: %s\n", Idx, FileList[Idx].cFileName);
        }
	}
}

