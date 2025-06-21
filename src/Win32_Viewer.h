#ifndef WIN32_VIEWER_H
#define WIN32_VIEWER_H

struct ImageViewer
{
    static void Run();
    static void Init(HINSTANCE hInst, PSTR CmdLine);
    static void Term();
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int WindowMsgLoop(HWND hWindow);
HWND Win32_Init(HINSTANCE hInstance);
void LoadImagesInDirectory();

#endif // WIN32_VIEWER_H

