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

        byte* ReadPtr = FileBMP.Contents + sizeof(BMP);
        if (BitmapFileTypeValue == ReadBMP.FileHeader.Type)
        {
            bool bUpsideDown = ReadBMP.InfoHeader.Height > 0;

            OutImage.Width = ReadBMP.InfoHeader.Width;
            OutImage.Height = bUpsideDown ? ReadBMP.InfoHeader.Height : -ReadBMP.InfoHeader.Height;
            OutImage.PxCount = OutImage.Width * OutImage.Height;
            OutImage.PxBytes = BytesRemaining;
            OutImage.PixelBuffer = new Pixel_RGBA32[OutImage.PxCount];

            u8* TmpPxData = new u8[BytesRemaining];
            memcpy_s(TmpPxData, BytesRemaining, ReadPtr, BytesRemaining);

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
                            u8* ReadPx = TmpPxData + (AdjRow * Stride) + (Col * 3);
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
                            u8* ReadPx = TmpPxData + (AdjRow * OutImage.Width + Col) * 4;
                            OutImage.PixelBuffer[PxIdx++] = { ReadPx[2], ReadPx[1], ReadPx[0], ReadPx[3] };
                        }
                    }
                } break;
                default:
                {
                    DebugBreak();
                } break;
            }

            delete[] TmpPxData;
        }

        Release(FileBMP);
    }
}

void WriteBMP(const char* OutFilename, const ImageT& InImage)
{
    Pixel_RGBA32* SwizzledImage = new Pixel_RGBA32[InImage.PxCount];
    for (unsigned int PxIdx = 0; PxIdx < InImage.PxCount; PxIdx++)
    {
        SwizzledImage[PxIdx] = Pixel_RGBA32::Swizzle(InImage.PixelBuffer[PxIdx]);
    }

    u32 PxBytes = sizeof(Pixel_RGBA32) * InImage.PxCount;

    BMP BMP_Data = {};

    BMP_Data.FileHeader.Type = BitmapFileTypeValue;
    BMP_Data.FileHeader.SizeInBytes = sizeof(BMP_Data) + PxBytes;
    BMP_Data.FileHeader.Res1 = 0;
    BMP_Data.FileHeader.Res2 = 0;
    BMP_Data.FileHeader.OffsetBytes = sizeof(BMP_Data);

    BMP_Data.InfoHeader.StructSize = sizeof(BMPInfoHeader);
    BMP_Data.InfoHeader.Width = (s32)InImage.Width;
    BMP_Data.InfoHeader.Height = -(s32)InImage.Height;
    BMP_Data.InfoHeader.Planes = 1;
    BMP_Data.InfoHeader.BitsPerPixel = 32;
    BMP_Data.InfoHeader.Compression = 0;
    BMP_Data.InfoHeader.Unused_ImgSize = PxBytes;
    BMP_Data.InfoHeader.HRes = 0;
    BMP_Data.InfoHeader.VRes = 0;
    BMP_Data.InfoHeader.ColorsUsed = 0;
    BMP_Data.InfoHeader.ColorsImportant = 0;

    FILE* BMP_File = nullptr;
    fopen_s(&BMP_File, OutFilename, "wb");
    if (BMP_File != nullptr)
    {
        fwrite(&BMP_Data.FileHeader, sizeof(BMP_Data.FileHeader), 1, BMP_File);
        fwrite(&BMP_Data.InfoHeader, sizeof(BMP_Data.InfoHeader), 1, BMP_File);
        fwrite(SwizzledImage, InImage.PxBytes, 1, BMP_File);
        fclose(BMP_File);
    }
    else
    {
        printf("ERROR: Cannot open file \"%s\" for write\n", OutFilename);
    }

    delete[] SwizzledImage;
}

