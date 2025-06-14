#include "Common.h" // Includes Image.h -> includes ImageBitmap.h

struct BMP
{
    BMPFileHeader FileHeader;
    BMPInfoHeader InfoHeader;
};

constexpr u16 BitmapFileTypeValue = 0x4D42;

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

void ReadBMP(const char* InFilename, ImageT& OutImage)
{
    FILE* BMP_File = nullptr;
    fopen_s(&BMP_File, InFilename, "rb");

    if (nullptr != BMP_File)
    {
        // Get file size in bytes
        fpos_t FileSizeBytes = 0;
        {
            int Result = fseek(BMP_File, 0, SEEK_END);
            // CKA_TODO: Assert Result == 0
            fgetpos(BMP_File, &FileSizeBytes);

            // Set curr pos to beginning of file
            Result = fseek(BMP_File, 0, SEEK_SET);
        }

        // Parse .BMP file into BMP struct
        BMP ReadBMP = {};
        fread_s(&ReadBMP, sizeof(BMP), sizeof(BMP), 1, BMP_File);

        size_t BytesRemaining = FileSizeBytes - sizeof(BMP);
        if (BitmapFileTypeValue == ReadBMP.FileHeader.Type)
        {
            OutImage.Width = ReadBMP.InfoHeader.Width;
            OutImage.Height = ReadBMP.InfoHeader.Height > 0 ? ReadBMP.InfoHeader.Height : -ReadBMP.InfoHeader.Height;
            OutImage.PxCount = OutImage.Width * OutImage.Height;
            OutImage.PxBytes = BytesRemaining;
            OutImage.PixelBuffer = new Pixel_RGBA32[OutImage.PxCount];

            bool bUpsideDown = ReadBMP.InfoHeader.Height > 0;

            // Extract pixel data into OutImage struct
            switch (ReadBMP.InfoHeader.BitsPerPixel)
            {
                case 24:
                {
                    size_t Stride = OutImage.Width * 3 % 4 == 0 ? OutImage.Width * 3 : (OutImage.Width + 1) * 3;
                    u8* TmpPxData = new u8[BytesRemaining];
                    fread_s(TmpPxData, BytesRemaining, BytesRemaining, 1, BMP_File);

                    int PxIdx = 0;
                    if (bUpsideDown)
                    {
                        for (int Row = OutImage.Height - 1; Row >= 0; Row--)
                        {
                            for (int Col = 0; Col < OutImage.Width; Col++)
                            {
                                u8* ReadPx = TmpPxData + (Row * Stride) + (Col * 3);
                                Pixel_RGBA32* WritePx = OutImage.PixelBuffer + PxIdx;
                                *WritePx = { ReadPx[2], ReadPx[1], ReadPx[0], 255 };
                                PxIdx++;
                            }
                        }
                    }
                    else
                    {
                        for (int Row = 0; Row < OutImage.Height; Row++)
                        {
                            for (int Col = 0; Col < OutImage.Width; Col++)
                            {
                                u8* ReadPx = TmpPxData + (Row * Stride) + (Col * 3);
                                OutImage.PixelBuffer[PxIdx] = { ReadPx[2], ReadPx[1], ReadPx[0], 255 };
                                PxIdx++;
                            }
                        }
                    }

                    delete[] TmpPxData;
                } break;
                case 32:
                {
                    u8* TmpPxData = new u8[BytesRemaining];
                    fread_s(TmpPxData, BytesRemaining, BytesRemaining, 1, BMP_File);

                    int PxIdx = 0;
                    if (bUpsideDown)
                    {
                        for (int Row = OutImage.Height - 1; Row >= 0; Row--)
                        {
                            for (int Col = 0; Col < OutImage.Width; Col++)
                            {
                                u8* ReadPx = TmpPxData + ((Row * OutImage.Width) + Col) * 4;
                                Pixel_RGBA32* WritePx = OutImage.PixelBuffer + PxIdx;
                                *WritePx = Pixel_RGBA32::Swizzle(*(Pixel_RGBA32*)ReadPx);
                                PxIdx++;
                            }
                        }
                    }
                    else
                    {
                        for (int Row = 0; Row < OutImage.Height; Row++)
                        {
                            for (int Col = 0; Col < OutImage.Width; Col++)
                            {
                                u8* ReadPx = TmpPxData + ((Row * OutImage.Width) + Col) * 4;
                                Pixel_RGBA32* WritePx = OutImage.PixelBuffer + PxIdx;
                                *WritePx = Pixel_RGBA32::Swizzle(*(Pixel_RGBA32*)ReadPx);
                                PxIdx++;
                            }
                        }
                    }
                } break;
            }
        }

        fclose(BMP_File);
    }
}

