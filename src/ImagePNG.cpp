#include "Common.h" // Includes Image.h -> includes ImagePNG.h

struct PNGChunk
{
    u32 Length;
    u32 Type;
    u8* Data;
    u32 CRC;
};

struct PNG
{
    static constexpr const u8 Signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    static constexpr bool bEnableDebugPrint = true;

    bool bValid;
    Array<PNGChunk> Chunks;

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
    void Parse(FileContentsT& FileContents)
    {
        ASSERT(FileContents.Size && FileContents.Contents);

        FileReaderT FileReader{ FileContents };

        bool bError = !CheckSignature(FileReader.ReadU64());
        while (!bError && !FileReader.IsDone())
        {
            PNGChunk NextChunk = {};
            NextChunk.Length = SwapEndian(FileReader.ReadU32());
            NextChunk.Type = FileReader.ReadU32();
            if (NextChunk.Length)
            {
                NextChunk.Data = new u8[NextChunk.Length];
                FileReader.ReadData(NextChunk.Data, NextChunk.Length);
            }
            NextChunk.CRC = FileReader.ReadU32();
            Chunks.Add(NextChunk);
        }

        ASSERT(!bError);

        if (bEnableDebugPrint)
        {
            Outf("[png][debug]: Parsed %d chunks in %s\n", Chunks.Num, FileContents.Name);
            for (int ChunkIdx = 0; ChunkIdx < Chunks.Num; ChunkIdx++)
            {
                PNGChunk& Chunk = Chunks[ChunkIdx];
                u8 Type[] = {
                    (Chunk.Type & 0xFF) >> 0,
                    (Chunk.Type & 0xFF00) >> 8,
                    (Chunk.Type & 0xFF0000) >> 16,
                    (Chunk.Type & 0xFF000000) >> 24,
                };
                Outf("\t[%d]: Type: %c%c%c%c\n", ChunkIdx, Type[0], Type[1], Type[2], Type[3]);
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
    }
};

void ReadPNG(const char* InFileName, ImageT& OutImage)
{
    FileContentsT FilePNG = ReadFileContents(InFileName);
    if (FilePNG.Size && FilePNG.Contents)
    {
        PNG ThePNG = {};
        ThePNG.Parse(FilePNG);

        Release(FilePNG);
    }
}
