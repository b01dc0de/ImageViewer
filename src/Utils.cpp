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

