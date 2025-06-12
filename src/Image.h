#ifndef IMAGE_H
#define IMAGE_H

// Common.h includes Image.h

#define NORM_RGB(R, G, B) (R)/255.0f, (G)/255.0f, (B)/255.0f

enum struct PixelFormat
{
    RGBA32
};

struct Pixel_RGBA32
{
    u8 R;
    u8 G;
    u8 B;
    u8 A;

    u32 SwizzleAsU32()
    {
        return u32((A << 24) | (B << 16) | (G << 8) | (R << 0));
    }
    static Pixel_RGBA32 Swizzle(Pixel_RGBA32 InVal)
    {
        Pixel_RGBA32 Result = {};
        Result.R = InVal.B;
        Result.G = InVal.G;
        Result.B = InVal.R;
        Result.A = InVal.A;
        return Result;
    }
};

struct ImageT
{
    u32 Width;
    u32 Height;
    u32 PxCount;
    u32 PxBytes;
    Pixel_RGBA32* PixelBuffer;
};

void GetDebugImage(ImageT& OutImage);

#include "ImageBitmap.h"

#endif // IMAGE_H

