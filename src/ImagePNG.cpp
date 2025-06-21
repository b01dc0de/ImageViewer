#include "Common.h" // Includes Image.h -> includes ImagePNG.h


#pragma pack(push, 1)
struct PNGChunk
{
    u32 Length;
    u32 Type;
    u8* Data;
    u32 CRC;
};

struct PNGHeader
{
    u32 Width;
    u32 Height;
    u8 BitDepth;
    u8 ColorType;
    u8 CompressionMethod;
    u8 FilterMethod;
    u8 InterlaceMethod;
};

struct PNGPalette
{
    struct Entry
    {
        u8 R;
        u8 G;
        u8 B;
    };

    size_t Num;
    Entry* Entries;
};
#pragma pack(pop)

struct PNG
{
    static constexpr const u8 Signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    static constexpr bool bDebugPrint = true;

    bool bValid = false;
    bool bError = false;
    bool bIHDR = false;
    bool bIEND = false;
    size_t NumChunks = 0;
    PNGHeader Header = {};
    PNGPalette Palette = {};
    Array<u8> ImageData = {};
    Array<PNGChunk> Chunks = {};

    static u32 SwapEndian(u32 Value)
    {
        u8 B0 = Value >> 0;
        u8 B1 = Value >> 8;
        u8 B2 = Value >> 16;
        u8 B3 = Value >> 24;
        return (B0 << 24) | (B1 << 16) | (B2 << 8) | (B3 << 0);
    }
    static bool CheckSignature(u64 InSignature)
    {
        u64 SignatureU64 = *(u64*)Signature;
        return SignatureU64 == InSignature;
    }
    void Cleanup()
    {
        if (Palette.Entries) { delete[] Palette.Entries; }
        for (int ChunkIdx = 0; ChunkIdx < Chunks.Num; ChunkIdx++)
        {
            if (Chunks[ChunkIdx].Data)
            {
                delete[] Chunks[ChunkIdx].Data;
            }
        }
    }
    void ParseChunk(FileReaderT& Reader)
    {
        PNGChunk NextChunk = {};
        NextChunk.Length = SwapEndian(Reader.ReadU32());
        NextChunk.Type = Reader.ReadU32();
        ASSERT(!bIEND);
        switch (SwapEndian(NextChunk.Type))
        {
            case 'IHDR':
            {
                ASSERT(Header.Width == 0 && Header.Height == 0);
                ASSERT(NextChunk.Length == sizeof(PNGHeader));
                ASSERT(!bIHDR);
                Header.Width = SwapEndian(Reader.ReadU32());
                Header.Height = SwapEndian(Reader.ReadU32());
                Header.BitDepth = Reader.ReadU8();
                Header.ColorType = Reader.ReadU8();
                Header.CompressionMethod = Reader.ReadU8();
                Header.FilterMethod = Reader.ReadU8();
                Header.InterlaceMethod = Reader.ReadU8();
                NextChunk.CRC = SwapEndian(Reader.ReadU32());
                bIHDR = true;
            } break;
            case 'PLTE':
            {
                ASSERT(bIHDR);
                ASSERT(Palette.Entries == nullptr && Palette.Num == 0);
                ASSERT(NextChunk.Length % sizeof(PNGPalette::Entry) == 0);
                ASSERT(ImageData.Num == 0); // PLTE must come before IDAT chunks
                Palette.Num = NextChunk.Length / 3;
                Palette.Entries = new PNGPalette::Entry[Palette.Num];
                Reader.ReadData(Palette.Entries, NextChunk.Length);
                NextChunk.CRC = SwapEndian(Reader.ReadU32());
            } break;
            case 'IDAT':
            {
                ASSERT(bIHDR);
                ASSERT(NextChunk.Length);
                if (NextChunk.Length)
                {
                    NextChunk.Data = new u8[NextChunk.Length];
                    Reader.ReadData(NextChunk.Data, NextChunk.Length);
                    ImageData.Add(NextChunk.Data, NextChunk.Length);
                    delete[] NextChunk.Data;
                }
                NextChunk.CRC = SwapEndian(Reader.ReadU32());
            } break;
            case 'IEND':
            {
                ASSERT(bIHDR);
                ASSERT(ImageData.Num > 0);
                ASSERT(NextChunk.Length == 0);
                NextChunk.CRC = SwapEndian(Reader.ReadU32());
                bIEND = true;
            } break;
            default:
            {
                ASSERT(bIHDR);
                if (NextChunk.Length)
                {
                    NextChunk.Data = new u8[NextChunk.Length];
                    Reader.ReadData(NextChunk.Data, NextChunk.Length);
                }
                NextChunk.CRC = SwapEndian(Reader.ReadU32());
                Chunks.Add(NextChunk);
            } break;
        }
        NumChunks++;
    }
    void Parse(FileContentsT& FileContents, ImageT& OutImage)
    {
        ASSERT(FileContents.Size && FileContents.Contents);

        FileReaderT FileReader{ FileContents };

        bError = !CheckSignature(FileReader.ReadU64());
        while (!bError && !FileReader.IsDone())
        {
            ParseChunk(FileReader);
        }
        ASSERT(!bError);

        if (bDebugPrint)
        {
            Outf("[png][debug]: Parsed %d chunks in %s\n", NumChunks, FileContents.Name);
            Outf("\tHeader: Width: %d\tHeight: %d\n", Header.Width, Header.Height);
            Outf("\t\tBitDepth: %d, ColorType: %d\n", Header.BitDepth, Header.ColorType);
            Outf("\t\tCompression: %d, Filter: %d, Interlace: %d\n", Header.CompressionMethod, Header.FilterMethod, Header.InterlaceMethod);
            Outf("\tTotal IDAT bytes: %d\n", ImageData.Num);
            for (int ChunkIdx = 0; ChunkIdx < Chunks.Num; ChunkIdx++)
            {
                PNGChunk& Chunk = Chunks[ChunkIdx];
                u8 Type[] = {
                    (Chunk.Type >> 0) & 0xFF,
                    (Chunk.Type >> 8) & 0xFF,
                    (Chunk.Type >> 16) & 0xFF,
                    (Chunk.Type >> 24) & 0xFF,
                };
                Outf("\t[%d]: %c%c%c%c\n", ChunkIdx, Type[0], Type[1], Type[2], Type[3]);
                Outf("\t\tLength: %d\t", Chunk.Length);
                if (Chunk.Data)
                {
                    static constexpr int MaxNumPrintedBytes = 4;
                    Outf("Data -> [ ");
                    for (int ByteIdx = 0; ByteIdx < MaxNumPrintedBytes && ByteIdx < Chunk.Length; ByteIdx++)
                    {
                        Outf("%02X ", Chunk.Data[ByteIdx]);
                    }
                    if (MaxNumPrintedBytes < Chunk.Length) { Outf("... "); }
                    Outf("]\n");
                }
                else
                {
                    Outf("Data -> ____\n");
                }
                u8* CRCAsU8 = (u8*)&Chunk.CRC;
                Outf("\t\tCRC: %02X %02X %02X %02X\n", CRCAsU8[0], CRCAsU8[1], CRCAsU8[2], CRCAsU8[3]);
            }
        }

        Cleanup();
    }
};

void ReadPNG(const char* InFileName, ImageT& OutImage)
{
    FileContentsT FilePNG = ReadFileContents(InFileName);
    if (FilePNG.Size && FilePNG.Contents)
    {
        PNG ThePNG = {};
        ThePNG.Parse(FilePNG, OutImage);

        Release(FilePNG);
    }
}
