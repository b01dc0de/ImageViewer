#ifndef IMAGE_H
#define IMAGE_H

// Common.h includes Image.h

#define NORM_RGB(R, G, B) (R)/255.0f, (G)/255.0f, (B)/255.0f

struct Pixel_RGBA32
{
    u8 R;
    u8 G;
    u8 B;
    u8 A;

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
void SafeRelease(ImageT& Image);

#include "ImageBitmap.h"
#include "ImagePNG.h"

#endif // IMAGE_H

