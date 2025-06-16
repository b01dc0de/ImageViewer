#include "Common.h"

void GetDebugImage(ImageT& OutImage)
{
    u32 DebugImgLength = 16;
    OutImage.Width = DebugImgLength;
    OutImage.Height = DebugImgLength;
    OutImage.PxCount = OutImage.Width * OutImage.Height;
    OutImage.PxBytes = sizeof(Pixel_RGBA32) * OutImage.PxCount;
    OutImage.PixelBuffer = new Pixel_RGBA32[OutImage.PxCount];

    constexpr Pixel_RGBA32 Pink{ 255u, 73u, 173u, 255u };
    constexpr Pixel_RGBA32 Black{ 0u, 0u, 0u, 255u };
    constexpr Pixel_RGBA32 Red{ 255u, 0u, 0u, 255u };
    constexpr Pixel_RGBA32 Green{ 0u, 255u, 0u, 255u };
    constexpr Pixel_RGBA32 Blue{ 0u, 0u, 255u, 255u };
    constexpr Pixel_RGBA32 White{ 255u, 255u, 255u, 255u };

    Pixel_RGBA32* Pixels32 = (Pixel_RGBA32*)OutImage.PixelBuffer;
    for (int PxIdx = 0; PxIdx < OutImage.PxCount; PxIdx++)
    {
        int PxRow = PxIdx / OutImage.Width;
        int PxCol = PxIdx % OutImage.Width;
        if (PxRow == 0 && PxCol == 0)
        {
            Pixels32[PxIdx] = Red;
        }
        else if (PxRow == 0 && PxCol == OutImage.Width - 1)
        {
            Pixels32[PxIdx] = Green;
        }
        else if (PxRow == OutImage.Height - 1 && PxCol == 0)
        {
            Pixels32[PxIdx] = Blue;
        }
        else if (PxRow == OutImage.Height - 1 && PxCol == OutImage.Width - 1)
        {
            Pixels32[PxIdx] = White;
        }
        else
        {
            bool bEvenCell = (PxRow + PxCol) % 2 == 0;
            Pixels32[PxIdx] = bEvenCell ? Black : Pink;
        }
    }
}

void SafeRelease(ImageT& Image)
{
    if (Image.PixelBuffer)
    {
        delete[] Image.PixelBuffer;
    }
}
