#ifndef COMMON_H
#define COMMON_H

// C/C++ std lib 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Win32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// DX11
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

// Types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;

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
// Platform
#include "DX11_Graphics.h"
// Common
#include "Image.h"
#include "Math.h"
#include "Utils.h"
// Main
#include "Win32_Viewer.h"

#endif // COMMON_H

