#ifndef COMMON_H
#define COMMON_H

// C++ Std Lib
#include <cstdio>
#include <vector>
// Win32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// DX11
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

// Types
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using s32 = int;

// Globals
extern bool bRunning;
extern HWND hWindow;
extern UINT WinResX;
extern UINT WinResY;
extern float WinAspectRatio;

#define CONFIG_DEBUG() (_DEBUG)
#define CONFIG_RELEASE() !CONFIG_DEBUG()

#define APPNAME() ("ImageViewer")
#define ARRAY_SIZE(Arr) (sizeof((Arr)) / sizeof((Arr)[0]))
#if CONFIG_DEBUG()
    #define ASSERT(Exp) { if (!(Exp)) { Outf("[error] ASSERT failed: %s\n", ##Exp); DebugBreak();} }
#else
    #define ASSERT(Exp) (void)0;
#endif // CONFIG_DEBUG()

// ImageViewer headers
#include "Image.h"
#include "Math.h"
#include "Utils.h"

#endif // COMMON_H

