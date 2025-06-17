#include "Common.h" // Includes Image.h -> includes ImageBitmap.h

struct BMP
{
    BMPFileHeader FileHeader;
    BMPInfoHeader InfoHeader;
};

constexpr u16 BitmapFileTypeValue = 0x4D42;

void ReadBMP(const char* InFileName, ImageT& OutImage)
{
    FileContentsT FileBMP = {};
    FileBMP = ReadFileContents(InFileName);

    if (FileBMP.Size && FileBMP.Contents)
    {
        // Parse .BMP file into BMP struct
        BMP ReadBMP = *(BMP*)FileBMP.Contents;
        size_t BytesRemaining = FileBMP.Size - sizeof(BMP);

        byte* BeginImageData = FileBMP.Contents + sizeof(BMP);
        if (BitmapFileTypeValue == ReadBMP.FileHeader.Type)
        {
            bool bUpsideDown = ReadBMP.InfoHeader.Height > 0;

            OutImage.Width = ReadBMP.InfoHeader.Width;
            OutImage.Height = bUpsideDown ? ReadBMP.InfoHeader.Height : -ReadBMP.InfoHeader.Height;
            OutImage.PxCount = OutImage.Width * OutImage.Height;
            OutImage.PxBytes = BytesRemaining;
            OutImage.PixelBuffer = new Pixel_RGBA32[OutImage.PxCount];

            // Extract pixel data into OutImage struct
            int PxIdx = 0;
            switch (ReadBMP.InfoHeader.BitsPerPixel)
            {
                case 24:
                {
                    size_t Padding = OutImage.Width * 3 % 4 == 0 ? 0 : 4 - (OutImage.Width * 3 % 4);
                    size_t Stride = (OutImage.Width * 3) + Padding;

                    for (int Row = 0; Row < OutImage.Height; Row++)
                    {
                        int AdjRow = bUpsideDown ? OutImage.Height - Row - 1 : Row;
                        for (int Col = 0; Col < OutImage.Width; Col++)
                        {
                            u8* ReadPx = BeginImageData + (AdjRow * Stride) + (Col * 3);
                            OutImage.PixelBuffer[PxIdx++] = { ReadPx[2], ReadPx[1], ReadPx[0], 255 };
                        }
                    }

                } break;
                case 32:
                {
                    for (int Row = 0; Row < OutImage.Height; Row++)
                    {
                        int AdjRow = bUpsideDown ? OutImage.Height - Row - 1 : Row;
                        for (int Col = 0; Col < OutImage.Width; Col++)
                        {
                            u8* ReadPx = BeginImageData + (AdjRow * OutImage.Width + Col) * 4;
                            OutImage.PixelBuffer[PxIdx++] = { ReadPx[2], ReadPx[1], ReadPx[0], ReadPx[3] };
                        }
                    }
                } break;
                default:
                {
                    DebugBreak();
                } break;
            }
        }

        Release(FileBMP);
    }
}

void WriteBMP(const char* OutFileName, const ImageT& InImage)
{
    Pixel_RGBA32* SwizzledImage = new Pixel_RGBA32[InImage.PxCount];
    for (unsigned int PxIdx = 0; PxIdx < InImage.PxCount; PxIdx++)
    {
        SwizzledImage[PxIdx] = Pixel_RGBA32::Swizzle(InImage.PixelBuffer[PxIdx]);
    }

    u32 PxBytes = sizeof(Pixel_RGBA32) * InImage.PxCount;

    BMP BMPHeaders = {};

    BMPHeaders.FileHeader.Type = BitmapFileTypeValue;
    BMPHeaders.FileHeader.SizeInBytes = sizeof(BMPHeaders) + PxBytes;
    BMPHeaders.FileHeader.Res1 = 0;
    BMPHeaders.FileHeader.Res2 = 0;
    BMPHeaders.FileHeader.OffsetBytes = sizeof(BMPHeaders);

    BMPHeaders.InfoHeader.StructSize = sizeof(BMPInfoHeader);
    BMPHeaders.InfoHeader.Width = (s32)InImage.Width;
    BMPHeaders.InfoHeader.Height = -(s32)InImage.Height;
    BMPHeaders.InfoHeader.Planes = 1;
    BMPHeaders.InfoHeader.BitsPerPixel = 32;
    BMPHeaders.InfoHeader.Compression = 0;
    BMPHeaders.InfoHeader.Unused_ImgSize = PxBytes;
    BMPHeaders.InfoHeader.HRes = 0;
    BMPHeaders.InfoHeader.VRes = 0;
    BMPHeaders.InfoHeader.ColorsUsed = 0;
    BMPHeaders.InfoHeader.ColorsImportant = 0;

    FILE* BMP_File = nullptr;
    fopen_s(&BMP_File, OutFileName, "wb");
    if (BMP_File != nullptr)
    {
        fwrite(&BMPHeaders.FileHeader, sizeof(BMPHeaders.FileHeader), 1, BMP_File);
        fwrite(&BMPHeaders.InfoHeader, sizeof(BMPHeaders.InfoHeader), 1, BMP_File);
        fwrite(SwizzledImage, InImage.PxBytes, 1, BMP_File);
        fclose(BMP_File);
    }
    else
    {
        Outf("[error]: Cannot open file \"%s\" for write\n", OutFileName);
    }

    delete[] SwizzledImage;
}

