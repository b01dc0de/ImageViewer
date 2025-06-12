#ifndef DX11_GRAPHICS_H
#define DX11_GRAPHICS_H

#include "Image.h"
#include "Math.h"

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

struct MeshStateT
{
    size_t VxSize;
    size_t NumVx;
    size_t NumIx;
    ID3D11Buffer* VxBuffer;
    ID3D11Buffer* IxBuffer;

    static MeshStateT Init(size_t _VxSize, size_t _NumVx, void* VxData, size_t _NumIx = 0, void* _IxData = nullptr);
};

struct DrawStateT
{
    ID3D11InputLayout* VxInputLayout;
    ID3D11VertexShader* VxShader;
    ID3D11PixelShader* PxShader;

    static DrawStateT Init(LPCWSTR ShaderFileName, const D3D_SHADER_MACRO* Defines, D3D11_INPUT_ELEMENT_DESC* InputElementDesc, UINT NumInputElements);
};

struct TextureStateT
{
    size_t Width;
    size_t Height;
    float AspectRatio;
    ID3D11Texture2D* Tex2D;
    ID3D11ShaderResourceView* SRV;

    static TextureStateT Init(ImageT& Image);
};

int CompileShaderHelper(LPCWSTR SourceFileName, LPCSTR EntryPointFunction, LPCSTR Profile, ID3DBlob** ShaderBlob, const D3D_SHADER_MACRO* Defines = nullptr);

namespace Graphics
{

void SubmitDraw(DrawStateT& InShaderState, MeshStateT& InVxState);
void Draw();
int Init();
void Term();

}

inline void SafeRelease(IUnknown* Ptr)
{
    if (Ptr) { Ptr->Release(); /*Ptr = nullptr;*/ }
}

inline void SafeRelease(MeshStateT& State)
{
    SafeRelease(State.VxBuffer);
    SafeRelease(State.IxBuffer);
}

inline void SafeRelease(DrawStateT& State)
{
    SafeRelease(State.VxInputLayout);
    SafeRelease(State.VxShader);
    SafeRelease(State.PxShader);
}

inline void SafeRelease(TextureStateT& State)
{
    SafeRelease(State.Tex2D);
    SafeRelease(State.SRV);
}

#endif // DX11_GRAPHICS_H

