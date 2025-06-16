#include "Common.h" // Includes Image.h -> includes ImagePNG.h

struct PNGChunk
{

};

struct PNG
{
    Array<PNGChunk> Chunks;
};

void ReadPNG(const char* InFilename, ImageT& OutImage)
{
}
