#ifndef GRAPHICS_DX11_H
#define GRAPHICS_DX11_H

#include "Common.h"

struct VertexColor
{
    v4f Position;
    v4f Color;
};

struct VertexTexture
{
    v4f Position;
    v2f TexUV;
};

struct WVPData
{
    m4f World;
    m4f View;
    m4f Proj;
};

struct VxState
{
    size_t VxSize;
    size_t NumVx;
    size_t NumIx;
    ID3D11Buffer* VxBuffer;
    ID3D11Buffer* IxBuffer;
    static VxState Init(size_t _VxSize, size_t _NumVx, void* VxData, size_t _NumIx = 0, void* IxData = nullptr);
};

struct ShaderState
{
    ID3D11InputLayout* VxInputLayout;
    ID3D11VertexShader* VxShader;
    ID3D11PixelShader* PxShader;
    static ShaderState Init(LPCWSTR ShaderFileName, const D3D_SHADER_MACRO* Defines, D3D11_INPUT_ELEMENT_DESC* InputElementDesc, UINT NumInputElements);
};

int CompileShaderHelper(LPCWSTR SourceFileName, LPCSTR EntryPointFunction, LPCSTR Profile, ID3DBlob** ShaderBlob, const D3D_SHADER_MACRO* Defines = nullptr);
int InitGraphics();
void UpdateAndDraw();
void Draw(ShaderState& InShaderState, VxState& InVxState);
void Draw();

inline void SafeRelease(IUnknown*& Ptr)
{
    if (Ptr) { Ptr->Release(); Ptr = nullptr; }
}

#endif // GRAPHICS_DX11_H

