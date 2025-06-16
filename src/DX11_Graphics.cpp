#include "Common.h" // Includes DX11_Graphics.h

#define DXCHECK(Result) if (FAILED(Result)) { return -1; }

IDXGISwapChain* DX_SwapChain = nullptr;
ID3D11Device* DX_Device = nullptr;
D3D_FEATURE_LEVEL UsedFeatureLevel;
ID3D11DeviceContext* DX_ImmediateContext = nullptr;

ID3D11Texture2D* DX_BackBuffer = nullptr;
ID3D11RenderTargetView* DX_RenderTargetView = nullptr;

IDXGIFactory1* DX_Factory = nullptr;

ID3D11RasterizerState* DX_RasterizerState = nullptr;
ID3D11Texture2D* DX_DepthStencil = nullptr;
ID3D11DepthStencilView* DX_DepthStencilView = nullptr;
ID3D11BlendState* DX_BlendState = nullptr;

ID3D11SamplerState* DX_DefaultSamplerState = nullptr;

ID3D11Buffer* DX_WVPBuffer = nullptr;

MeshStateT MeshQuad;

DrawStateT ShaderColor;
DrawStateT ShaderTexture;

TextureStateT DebugTexture;

MeshStateT MeshStateT::Init(size_t _VxSize, size_t _NumVx, void* VxData, size_t _NumIx, void* _IxData)
{
    ASSERT(_VxSize && _NumVx && VxData && (_NumIx == 0 || _IxData));

    MeshStateT Result{ _VxSize, _NumVx, _NumIx };

    D3D11_BUFFER_DESC VertexBufferDesc = { _VxSize * _NumVx, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0 };
    D3D11_SUBRESOURCE_DATA VertexBufferInitData = { VxData, 0, 0 };
    ASSERT(!FAILED(DX_Device->CreateBuffer(&VertexBufferDesc, &VertexBufferInitData, &Result.VxBuffer)));

    if (_NumIx && _IxData)
    {
        ASSERT(_NumIx % 3 == 0);
        D3D11_BUFFER_DESC IndexBufferDesc = { _NumIx * sizeof(UINT), D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0};
        D3D11_SUBRESOURCE_DATA IndexBufferInitData = { _IxData, 0, 0 };
        ASSERT(!FAILED(DX_Device->CreateBuffer(&IndexBufferDesc, &IndexBufferInitData, &Result.IxBuffer)));
    }

    return Result;
}

int CompileShaderHelper
(
    LPCWSTR SourceFileName,
    LPCSTR EntryPointFunction,
    LPCSTR Profile,
    ID3DBlob** ShaderBlob,
    const D3D_SHADER_MACRO* Defines
)
{
    HRESULT Result = S_OK;

    if (SourceFileName == nullptr || EntryPointFunction == nullptr || Profile == nullptr || ShaderBlob == nullptr)
    {
        return E_INVALIDARG;
    }

    *ShaderBlob = nullptr;

    UINT CompileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    CompileFlags |= D3DCOMPILE_DEBUG;
    CompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* OutBlob = nullptr;
    ID3DBlob* ErrorMsgBlob = nullptr;

    Result = D3DCompileFromFile
    (
        SourceFileName,
        Defines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPointFunction,
        Profile,
        CompileFlags,
        0, //UINT Flags2
        &OutBlob,
        &ErrorMsgBlob
    );

    if (FAILED(Result) && OutBlob)
    {
        OutBlob->Release();
        OutBlob = nullptr;
    }
    if (ErrorMsgBlob)
    {
        OutputDebugStringA((char*)ErrorMsgBlob->GetBufferPointer());
        ErrorMsgBlob->Release();
    }

    *ShaderBlob = OutBlob;

    return Result;
};

DrawStateT DrawStateT::Init(LPCWSTR ShaderFileName, const D3D_SHADER_MACRO* Defines, D3D11_INPUT_ELEMENT_DESC* InputLayoutDesc, UINT NumInputElements)
{
    DrawStateT Result{};

    ID3DBlob* VSCodeBlob = nullptr;
    ID3DBlob* PSCodeBlob = nullptr;

    ASSERT(!FAILED(CompileShaderHelper(ShaderFileName, "VSMain", "vs_5_0", &VSCodeBlob, Defines)));
    ASSERT(!FAILED(CompileShaderHelper(L"src/hlsl/BaseShader.hlsl", "PSMain", "ps_5_0", &PSCodeBlob, Defines)));

    if (VSCodeBlob && PSCodeBlob)
    {
        ASSERT(!FAILED(DX_Device->CreateVertexShader(VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), nullptr, &Result.VxShader)));
        ASSERT(!FAILED(DX_Device->CreatePixelShader(PSCodeBlob->GetBufferPointer(), PSCodeBlob->GetBufferSize(), nullptr, &Result.PxShader)));
        ASSERT(!FAILED(DX_Device->CreateInputLayout(InputLayoutDesc, NumInputElements, VSCodeBlob->GetBufferPointer(), VSCodeBlob->GetBufferSize(), &Result.VxInputLayout)));
    }
    SafeRelease(VSCodeBlob);
    SafeRelease(PSCodeBlob);

    return Result;
}

TextureStateT TextureStateT::Init(ImageT& Image)
{
    ASSERT(Image.Width && Image.Height && Image.PxCount && Image.PxBytes && Image.PixelBuffer);

    TextureStateT Result{};
    Result.Width = Image.Width;
    Result.Height = Image.Height;
    Result.AspectRatio = (float)Image.Width / (float)Image.Height;

    D3D11_SUBRESOURCE_DATA TextureResourceData[] = { {} };
    TextureResourceData[0].pSysMem = Image.PixelBuffer;
    TextureResourceData[0].SysMemPitch = sizeof(u32) * Image.Width;
    TextureResourceData[0].SysMemSlicePitch = sizeof(u32) * Image.Width * Image.Height;
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Image.Width;
    TextureDesc.Height = Image.Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;
    ASSERT(!FAILED(DX_Device->CreateTexture2D(&TextureDesc, &TextureResourceData[0], &Result.Tex2D)));
    ASSERT(!FAILED(DX_Device->CreateShaderResourceView(Result.Tex2D, nullptr, &Result.SRV)));

    return Result;
}

void Graphics::SubmitDraw(DrawStateT& InShaderState, MeshStateT& InVxState)
{
    ASSERT(InShaderState.VxInputLayout && InShaderState.VxShader && InShaderState.PxShader);
    DX_ImmediateContext->IASetInputLayout(InShaderState.VxInputLayout);
    DX_ImmediateContext->VSSetShader(InShaderState.VxShader, nullptr, 0);
    DX_ImmediateContext->PSSetShader(InShaderState.PxShader, nullptr, 0);

    UINT Stride = InVxState.VxSize;
    UINT Offset = 0;
    DX_ImmediateContext->IASetVertexBuffers(0, 1, &InVxState.VxBuffer, &Stride, &Offset);
    DX_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    if (InVxState.IxBuffer)
    {
        DX_ImmediateContext->IASetIndexBuffer(InVxState.IxBuffer, DXGI_FORMAT_R32_UINT, 0);
        DX_ImmediateContext->DrawIndexed(InVxState.NumIx, 0u, 0u);
    }
    else
    {
        DX_ImmediateContext->IASetVertexBuffers(0, 1, &InVxState.VxBuffer, &Stride, &Offset);
        DX_ImmediateContext->Draw(InVxState.NumIx, 0u);
    }
}

void Graphics::Draw(ViewerDrawParameters& Params)
{
    DX_ImmediateContext->OMSetRenderTargets(1, &DX_RenderTargetView, DX_DepthStencilView);

    constexpr float ClearColor[4] = { NORM_RGB(30, 30, 46), 1.0f};
    constexpr float ClearDepth = 1.0f;
    DX_ImmediateContext->ClearRenderTargetView(DX_RenderTargetView, ClearColor);
    DX_ImmediateContext->ClearDepthStencilView(DX_DepthStencilView, D3D11_CLEAR_DEPTH, ClearDepth, 0);

    TextureStateT& ActiveTexture = Params.ActiveImage ? *Params.ActiveImage : DebugTexture;
    float ScaleX = 1.0f;
    float ScaleY = 1.0f;
    if (ActiveTexture.AspectRatio < WinAspectRatio)
    {
        ScaleX = ActiveTexture.AspectRatio / WinAspectRatio;
    }
    else if (ActiveTexture.AspectRatio > WinAspectRatio)
    {
        ScaleY = WinAspectRatio / ActiveTexture.AspectRatio;
    }

    m4f QuadWorld = {
        { ScaleX, 0.0f, 0.0f, 0.0f },
        { 0.0f, ScaleY, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
    };
    WVPData WVP_Trans = { QuadWorld, m4f::Identity(), m4f::Identity() };
    constexpr int WVPBufferSlot = 0;
    DX_ImmediateContext->UpdateSubresource(DX_WVPBuffer, 0, nullptr, &WVP_Trans, sizeof(WVPData), 0);

    {
        DX_ImmediateContext->PSSetShaderResources(0, 1, &ActiveTexture.SRV);
        DX_ImmediateContext->PSSetSamplers(0, 1, &DX_DefaultSamplerState);

        DX_ImmediateContext->VSSetConstantBuffers(WVPBufferSlot, 1, &DX_WVPBuffer);

        Graphics::SubmitDraw(ShaderTexture, MeshQuad);
    }

    DX_SwapChain->Present(0, 0);
}

int Graphics::Init()
{
    HRESULT Result = S_OK;

    D3D_FEATURE_LEVEL SupportedFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT NumSupportedFeatureLevels = ARRAYSIZE(SupportedFeatureLevels);
    D3D_FEATURE_LEVEL D3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;
    (void)D3DFeatureLevel;

    CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&DX_Factory);

    UINT FrameRefreshRate = 60;
    DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
    swapchain_desc.BufferCount = 2;
    swapchain_desc.BufferDesc.Width = WinResX;
    swapchain_desc.BufferDesc.Height = WinResY;
    swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.BufferDesc.RefreshRate.Numerator = FrameRefreshRate;
    swapchain_desc.BufferDesc.RefreshRate.Denominator = 1;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.OutputWindow = hWindow;
    swapchain_desc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchain_desc.Windowed = true;

    UINT CreateDeviceFlags = 0;
#ifdef _DEBUG
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    Result = D3D11CreateDeviceAndSwapChain(
        nullptr,					//IDXGIAdapter* pAdapter
        D3D_DRIVER_TYPE_HARDWARE,	//D3D_DRIVER_TYPE DriverType
        nullptr,					//HMODULE Software
        CreateDeviceFlags,			//UINT Flags
        SupportedFeatureLevels,		//const D3D_FEATURE_LEVEL* pFeatureLevels
        NumSupportedFeatureLevels,	//UINT FeatureLevels
        D3D11_SDK_VERSION,			//UINT SDKVersion
        &swapchain_desc,			//const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc
        &DX_SwapChain,
        &DX_Device,
        &UsedFeatureLevel,
        &DX_ImmediateContext
    );
    DXCHECK(Result);

    Result = DX_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&DX_BackBuffer);
    DXCHECK(Result);

    Result = DX_Device->CreateRenderTargetView(DX_BackBuffer, nullptr, &DX_RenderTargetView);
    DXCHECK(Result);

    D3D11_RASTERIZER_DESC RasterDesc = {};
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_BACK;
    RasterDesc.FrontCounterClockwise = true;
    RasterDesc.DepthClipEnable = true;
    RasterDesc.ScissorEnable = false;
    RasterDesc.MultisampleEnable = true;
    RasterDesc.AntialiasedLineEnable = true;

    Result = DX_Device->CreateRasterizerState(&RasterDesc, &DX_RasterizerState);
    DXCHECK(Result);

    DX_ImmediateContext->RSSetState(DX_RasterizerState);

    D3D11_TEXTURE2D_DESC DepthDesc = {};
    DepthDesc.Width = WinResX;
    DepthDesc.Height = WinResY;
    DepthDesc.MipLevels = 1;
    DepthDesc.ArraySize = 1;
    DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
    DepthDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthDesc.CPUAccessFlags = 0;
    DepthDesc.MiscFlags = 0;

    Result = DX_Device->CreateTexture2D(&DepthDesc, nullptr, &DX_DepthStencil);
    DXCHECK(Result);

    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DepthStencilViewDesc.Format;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    DepthStencilViewDesc.Texture2D.MipSlice = 0;

    Result = DX_Device->CreateDepthStencilView(DX_DepthStencil, &DepthStencilViewDesc, &DX_DepthStencilView);
    DXCHECK(Result);

    DX_ImmediateContext->OMSetRenderTargets(1, &DX_RenderTargetView, DX_DepthStencilView);

    D3D11_RENDER_TARGET_BLEND_DESC RTVBlendDesc = {};
    RTVBlendDesc.BlendEnable = true;
    RTVBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    RTVBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    RTVBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
    RTVBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    RTVBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
    RTVBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    RTVBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALPHA;

    D3D11_BLEND_DESC BlendDesc = {};
    BlendDesc.RenderTarget[0] = RTVBlendDesc;

    Result = DX_Device->CreateBlendState(&BlendDesc, &DX_BlendState);
    DXCHECK(Result);

    D3D11_VIEWPORT Viewport_Desc = {};
    Viewport_Desc.Width = (FLOAT)WinResX;
    Viewport_Desc.Height = (FLOAT)WinResY;
    Viewport_Desc.MinDepth = 0.0f;
    Viewport_Desc.MaxDepth = 1.0f;
    Viewport_Desc.TopLeftX = 0;
    Viewport_Desc.TopLeftY = 0;
    DX_ImmediateContext->RSSetViewports(1, &Viewport_Desc);

    // WorldViewProj CBuffer
    {
        D3D11_BUFFER_DESC WVPBufferDesc = {};
        WVPBufferDesc.ByteWidth = sizeof(WVPData);
        WVPBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        WVPBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        WVPBufferDesc.CPUAccessFlags = 0;
        DXCHECK(DX_Device->CreateBuffer(&WVPBufferDesc, nullptr, &DX_WVPBuffer));
    }

    // Default sampler state
    {
        D3D11_TEXTURE_ADDRESS_MODE AddressMode = D3D11_TEXTURE_ADDRESS_WRAP;
        D3D11_SAMPLER_DESC DefaultSamplerDesc = {};
        DefaultSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        DefaultSamplerDesc.AddressU = AddressMode;
        DefaultSamplerDesc.AddressV = AddressMode;
        DefaultSamplerDesc.AddressW = AddressMode;
        DefaultSamplerDesc.MipLODBias = 0.0f;
        DefaultSamplerDesc.MaxAnisotropy = 0;
        DefaultSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        DefaultSamplerDesc.MinLOD = 0;
        DefaultSamplerDesc.MaxLOD = 0;
        DXCHECK(DX_Device->CreateSamplerState(&DefaultSamplerDesc, &DX_DefaultSamplerState));
    }

    // Shader pipeline state
    {
        LPCWSTR ShaderFileName = L"src/hlsl/BaseShader.hlsl";

        // ShaderColor
        {
            const D3D_SHADER_MACRO Defines[] =
            {
                "ENABLE_VERTEX_COLOR", "1",
                "ENABLE_VERTEX_TEXTURE", "0",
                NULL, NULL
            };
            D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            UINT NumInputElements = ARRAYSIZE(InputLayoutDesc);

            ShaderColor = DrawStateT::Init(ShaderFileName, Defines, InputLayoutDesc, NumInputElements);
        }

        // ShaderTexture
        {
            const D3D_SHADER_MACRO Defines[] =
            {
                "ENABLE_VERTEX_COLOR", "0",
                "ENABLE_VERTEX_TEXTURE", "1",
                NULL, NULL
            };
            D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            UINT NumInputElements = ARRAYSIZE(InputLayoutDesc);

            ShaderTexture = DrawStateT::Init(ShaderFileName, Defines, InputLayoutDesc, NumInputElements);
        }
    }

    // Mesh Data
    {
        VertexTexture Vertices_Quad[]
        {
            {{-1.0f, +1.0f, +0.5f, +1.0f}, {+0.0f, +0.0f}},
            {{+1.0f, +1.0f, +0.5f, +1.0f}, {+1.0f, +0.0f}},
            {{-1.0f, -1.0f, +0.5f, +1.0f}, {+0.0f, +1.0f}},
            {{+1.0f, -1.0f, +0.5f, +1.0f}, {+1.0f, +1.0f}},
        };
        UINT Indices_Quad[] =
        {
            0, 2, 1,
            1, 2, 3
        };
        MeshQuad = MeshStateT::Init
        (
            sizeof(VertexTexture),
            ARRAY_SIZE(Vertices_Quad),
            Vertices_Quad,
            ARRAY_SIZE(Indices_Quad),
            Indices_Quad
        );
    }

    // DebugTexture
    {
        ImageT BMPImage = {};
        GetDebugImage(BMPImage);

        DebugTexture = TextureStateT::Init(BMPImage);

        delete[] BMPImage.PixelBuffer;
    }

    return Result;
}

void Graphics::Term()
{
    SafeRelease(ShaderColor);
    SafeRelease(ShaderTexture);
    SafeRelease(MeshQuad);
    SafeRelease(DebugTexture);

    SafeRelease(DX_WVPBuffer);
    SafeRelease(DX_DefaultSamplerState);

    SafeRelease(DX_RasterizerState);
    SafeRelease(DX_BlendState);
    SafeRelease(DX_DepthStencilView);
    SafeRelease(DX_DepthStencil);
    SafeRelease(DX_RenderTargetView);
    SafeRelease(DX_BackBuffer);

    SafeRelease(DX_SwapChain);
    SafeRelease(DX_ImmediateContext);
    SafeRelease(DX_Factory);
    SafeRelease(DX_Device);
}
