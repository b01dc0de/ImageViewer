#ifndef IMAGEBMP_H
#define IMAGEBMP_H

#pragma pack(push, 1)
struct BMPFileHeader
{
    u16 Type; // Always Ascii BM
    u32 SizeInBytes; // Size (bytes) of file
    u16 Res1; // 0
    u16 Res2; // 0
    u32 OffsetBytes; // Offset (bytes) to actual pixel data
};

struct BMPInfoHeader
{
    u32 StructSize; // Size (bytes) of InfoHeader
    s32 Width;
    s32 Height; // NOTE(chris): If positive, pixel data is bottom to top
    u16 Planes; // Must be 1
    u16 BitsPerPixel; // Bits-per-pixel (0, 1, 4, 8, 16, 24, 32)
    u32 Compression; // *Should* be 0
    u32 Unused_ImgSize; // Only used if Compression is weird (not 0)
    s32 HRes; // Horizontal resolution
    s32 VRes; // Vertical resolution
    u32 ColorsUsed; // 0 for our purposes
    u32 ColorsImportant; // 0 for our purposes
};
#pragma pack(pop)

void ReadBMP(const char* InFileName, ImageT& OutImage);
void WriteBMP(const char* OutFileName, const ImageT& InImage);

#endif // IMAGEBMP_H

