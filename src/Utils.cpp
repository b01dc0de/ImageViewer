#include "Common.h"

void Outf(const char* Fmt, ...)
{
    constexpr size_t BufferSize = 1024;
    char MsgBuffer[BufferSize];
    va_list Args;
    va_start(Args, Fmt);
    vsprintf_s(MsgBuffer, BufferSize, Fmt, Args);
    va_end(Args);
    OutputDebugStringA(MsgBuffer);
}

bool MatchStr(const char* A, const char* B)
{
    for (size_t Idx = 0;; Idx++)
    {
        if (!A[Idx] && !B[Idx])
        {
            return true;
        }
        if (A[Idx] != B[Idx])
        {
            return false;
        }
    }
}

void Release(FileContentsT& FileContents)
{
    if (FileContents.Contents)
    {
        delete[] FileContents.Contents;
        FileContents.Size = 0;
        FileContents.Contents = nullptr;
    }
}

FileContentsT ReadFileContents(const char* InFileName)
{
    ASSERT(InFileName);
    FileContentsT Result = { InFileName };

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, InFileName, "rb");

    if (nullptr != FileHandle)
    {
        // Get file size
        fpos_t FileSizeBytes = 0;
        fseek(FileHandle, 0, SEEK_END);
        fgetpos(FileHandle, &FileSizeBytes);
        fseek(FileHandle, 0, SEEK_SET);

        // Read whole file directly into Result.Contents
        ASSERT(FileSizeBytes);
        if (FileSizeBytes)
        {
            Result.Size = FileSizeBytes;
            Result.Contents = new byte[FileSizeBytes];
            fread_s(Result.Contents, FileSizeBytes, FileSizeBytes, 1, FileHandle);
        }

        fclose(FileHandle);
    }

    return Result;
}

//void WriteFileContents(FileContentsT& InFileContents, const char* OutFileName)
//{
//    ASSERT(InFileContents.Size && InFileContents.Contents && OutFileName);
//
//    FILE* FileHandle = nullptr;
//    fopen_s(&FileHandle, OutFileName, "wb");
//
//    if (nullptr != FileHandle)
//    {
//        fwrite(InFileContents.Contents, InFileContents.Size, 1, FileHandle);
//
//        fclose(FileHandle);
//    }
//}
