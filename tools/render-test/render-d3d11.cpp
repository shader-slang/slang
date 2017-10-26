// render-d3d11.cpp
#include "render-d3d11.h"

#include "options.h"
#include "render.h"

// In order to use the Slang API, we need to include its header

#include <slang.h>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"

// We will be rendering with Direct3D 11, so we need to include
// the Windows and D3D11 headers

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <d3d11_2.h>
#include <d3dcompiler.h>

// We will use the C standard library just for printing error messages.
#include <stdio.h>

#ifdef _MSC_VER
#include <stddef.h>
#if (_MSC_VER < 1900)
#define snprintf sprintf_s
#endif
#endif
//
using namespace Slang;

namespace renderer_test {

struct D3DBinding
{
    ShaderInputType type;
    ID3D11ShaderResourceView * srv = nullptr;
    ID3D11UnorderedAccessView * uav = nullptr;
    ID3D11Buffer * buffer = nullptr;
    ID3D11SamplerState * samplerState = nullptr;
    int binding = 0;
    bool isOutput = false;
    int bufferLength = 0;
};
struct D3DBindingState
{
    List<D3DBinding> bindings;
};

//


//

// Global variabels for the various D3D11 API objects to be used for rendering
ID3D11Buffer*       dxConstantBuffer;
ID3D11InputLayout*  dxInputLayout;
ID3D11Buffer*       dxVertexBuffer;
ID3D11VertexShader* dxVertexShader;
ID3D11PixelShader*  dxPixelShader;

// The Slang compiler currently generates HLSL source, so we'll need a utility
// routine (defined later) to translate that into D3D11 shader bytecode.
ID3DBlob* compileHLSLShader(
    char const* sourcePath,
    char const* source,
    char const* entryPointName,
    char const* dxProfileName);

static char const* vertexEntryPointName    = "vertexMain";
static char const* fragmentEntryPointName  = "fragmentMain";

static char const* vertexProfileName   = "vs_4_0";
static char const* fragmentProfileName = "ps_4_0";

ID3DBlob* gVertexShaderBlob;
ID3DBlob* gPixelShaderBlob;

//
// Definition of the HLSL-to-bytecode compilation logic.
//
ID3DBlob* compileHLSLShader(
    char const* sourcePath,
    char const* source,
    char const* entryPointName,
    char const* dxProfileName )
{
    // Rather than statically link against the `d3dcompile` library, we
    // dynamically load it.
    //
    // Note: A more realistic application would compile from HLSL text to D3D
    // shader bytecode as part of an offline process, rather than doing it
    // on-the-fly like this
    //
    static pD3DCompile D3DCompile_ = nullptr;
    if( !D3DCompile_ )
    {
        // TODO(tfoley): maybe want to search for one of a few versions of the DLL
        HMODULE d3dcompiler = LoadLibraryA("d3dcompiler_47.dll");
        if(!d3dcompiler)
        {
            fprintf(stderr, "error: failed load 'd3dcompiler_47.dll'\n");
            exit(1);
        }

        D3DCompile_ = (pD3DCompile)GetProcAddress(d3dcompiler, "D3DCompile");
        if( !D3DCompile_ )
        {
            fprintf(stderr, "error: failed load symbol 'D3DCompile'\n");
            exit(1);
        }
    }

    // For this example, we turn on debug output, and turn off all
    // optimization. A real application would only use these flags
    // when shader debugging is needed.
    UINT flags = 0;
    flags |= D3DCOMPILE_DEBUG;
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_SKIP_OPTIMIZATION;

    // We will always define `__HLSL__` when compiling here, so that
    // input code can react differently to being compiled as pure HLSL.
    D3D_SHADER_MACRO defines[] = {
        { "__HLSL__", "1" },
        { nullptr, nullptr },
    };

    // The `D3DCompile` entry point takes a bunch of parameters, but we
    // don't really need most of them for Slang-generated code.
    ID3DBlob* dxShaderBlob = nullptr;
    ID3DBlob* dxErrorBlob = nullptr;
    HRESULT hr = D3DCompile_(
        source,
        strlen(source),
        sourcePath,
        &defines[0],
        nullptr,
        entryPointName,
        dxProfileName,
        flags,
        0,
        &dxShaderBlob,
        &dxErrorBlob);

    // If the HLSL-to-bytecode compilation produced any diagnostic messages
    // then we will print them out (whether or not the compilation failed).
    if( dxErrorBlob )
    {
        fputs(
            (char const*)dxErrorBlob->GetBufferPointer(),
            stderr);
        fflush(stderr);

        OutputDebugStringA(
            (char const*)dxErrorBlob->GetBufferPointer());

        dxErrorBlob->Release();
    }

    if( FAILED(hr) )
    {
        return nullptr;
    }

    return dxShaderBlob;
}




// Capture a texture to a file

static HRESULT captureTextureToFile(
    ID3D11Device*           dxDevice,
    ID3D11DeviceContext*    dxContext,
    ID3D11Texture2D*        dxTexture,
    char const*             outputPath)
{
    if(!dxContext) return E_INVALIDARG;
    if(!dxTexture) return E_INVALIDARG;

    D3D11_TEXTURE2D_DESC dxTextureDesc;
    dxTexture->GetDesc(&dxTextureDesc);

    // Don't bother supporing MSAA for right now
    if( dxTextureDesc.SampleDesc.Count > 1 )
    {
        fprintf(stderr, "ERROR: cannot capture multisample texture\n");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ID3D11Texture2D* dxStagingTexture = nullptr;

    if( dxTextureDesc.Usage == D3D11_USAGE_STAGING && (dxTextureDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ) )
    {
        dxStagingTexture = dxTexture;
        dxStagingTexture->AddRef();
    }
    else
    {
        // Modify the descriptor to give us a staging texture
        dxTextureDesc.BindFlags = 0;
        dxTextureDesc.MiscFlags &= ~D3D11_RESOURCE_MISC_TEXTURECUBE;
        dxTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        dxTextureDesc.Usage = D3D11_USAGE_STAGING;

        hr = dxDevice->CreateTexture2D(&dxTextureDesc, 0, &dxStagingTexture);
        if( FAILED(hr) )
        {
            fprintf(stderr, "ERROR: failed to create staging texture\n");
            return hr;
        }
    
        dxContext->CopyResource(dxStagingTexture, dxTexture);
    }

    // Now just read back texels from the staging textures

    D3D11_MAPPED_SUBRESOURCE dxMappedResource;
    hr = dxContext->Map(dxStagingTexture, 0, D3D11_MAP_READ, 0, &dxMappedResource);
    if( FAILED(hr) )
    {
        fprintf(stderr, "ERROR: failed to map texture for read\n");
        return hr;
    }

    int stbResult = stbi_write_png(
        outputPath,
        dxTextureDesc.Width,
        dxTextureDesc.Height,
        4,
        dxMappedResource.pData,
        dxMappedResource.RowPitch);
    if( !stbResult )
    {
        fprintf(stderr, "ERROR: failed to write texture to file\n");
        return E_UNEXPECTED;
    }

    dxContext->Unmap(dxStagingTexture, 0);

    dxStagingTexture->Release();

    return S_OK;
}

//

class D3D11Renderer : public Renderer, public ShaderCompiler
{
public:
    IDXGISwapChain* dxSwapChain = NULL;
    ID3D11Device* dxDevice = NULL;
    ID3D11DeviceContext* dxImmediateContext = NULL;
    ID3D11Texture2D* dxBackBufferTexture = NULL;
    List<ID3D11RenderTargetView*> dxRenderTargetViews;
    List<ID3D11Texture2D *> dxRenderTargetTextures;
    D3DBindingState * currentBindings = nullptr;
    virtual void initialize(void* inWindowHandle) override
    {
        auto windowHandle = (HWND) inWindowHandle;
        // Rather than statically link against D3D, we load it dynamically.

        HMODULE d3d11 = LoadLibraryA("d3d11.dll");
        if(!d3d11)
        {
            fprintf(stderr, "error: failed load 'd3d11.dll'\n");
            exit(1);
        }

        PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN D3D11CreateDeviceAndSwapChain_ =
            (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(
                d3d11,
                "D3D11CreateDeviceAndSwapChain");
        if(!D3D11CreateDeviceAndSwapChain_)
        {
            fprintf(stderr,
                "error: failed load symbol 'D3D11CreateDeviceAndSwapChain'\n");
            exit(1);
        }

        // We create our device in debug mode, just so that we can check that the
        // example doesn't trigger warnings.
        UINT deviceFlags = 0;
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

        // We will ask for the highest feature level that can be supported.

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
        };
        D3D_FEATURE_LEVEL dxFeatureLevel = D3D_FEATURE_LEVEL_9_1;

        // Our swap chain uses RGBA8 with sRGB, with double buffering.

        DXGI_SWAP_CHAIN_DESC dxSwapChainDesc = { 0 };
        dxSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

        // Note(tfoley): Disabling sRGB for DX back buffer for now, so that we
        // can get consistent output with OpenGL, where setting up sRGB will
        // probably be more involved.
//        dxSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        dxSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        dxSwapChainDesc.SampleDesc.Count = 1;
        dxSwapChainDesc.SampleDesc.Quality = 0;
        dxSwapChainDesc.BufferCount = 2;
        dxSwapChainDesc.OutputWindow = windowHandle;
        dxSwapChainDesc.Windowed = TRUE;
        dxSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        dxSwapChainDesc.Flags = 0;

        // On a machine that does not have an up-to-date version of D3D installed,
        // the `D3D11CreateDeviceAndSwapChain` call will fail with `E_INVALIDARG`
        // if you ask for featuer level 11_1. The workaround is to call
        // `D3D11CreateDeviceAndSwapChain` up to twice: the first time with 11_1
        // at the start of the list of requested feature levels, and the second
        // time without it.

        HRESULT hr = S_OK;
        for( int ii = 0; ii < 2; ++ii )
        {
            hr = D3D11CreateDeviceAndSwapChain_(
                NULL,                    // adapter (use default)
                D3D_DRIVER_TYPE_WARP,
              //D3D_DRIVER_TYPE_HARDWARE,
                NULL,                    // software
                deviceFlags,
                &featureLevels[ii],
                (sizeof(featureLevels) / sizeof(featureLevels[0])) - 1,
                D3D11_SDK_VERSION,
                &dxSwapChainDesc,
                &dxSwapChain,
                &dxDevice,
                &dxFeatureLevel,
                &dxImmediateContext);

            // Failures with `E_INVALIDARG` might be due to feature level 11_1
            // not being supported. Other failures are real, though.
            if( hr != E_INVALIDARG )
                break;
        }
        if( FAILED(hr) )
        {
            exit(1);
        }

        // After we've created the swap chain, we can request a pointer to the
        // back buffer as a D3D11 texture, and create a render-target view from it.

        static const IID kIID_ID3D11Texture2D = {
            0x6f15aaf2, 0xd208, 0x4e89, 0x9a, 0xb4, 0x48,
            0x95, 0x35, 0xd3, 0x4f, 0x9c };

        dxSwapChain->GetBuffer(
            0,
            kIID_ID3D11Texture2D,
            (void**)&dxBackBufferTexture);

        for (int i = 0; i < 8; i++)
        {
            ID3D11Texture2D* texture;
            D3D11_TEXTURE2D_DESC textureDesc;
            dxBackBufferTexture->GetDesc(&textureDesc);
            dxDevice->CreateTexture2D(&textureDesc, nullptr, &texture);
            ID3D11RenderTargetView * rtv;
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rtvDesc.Texture2D.MipSlice = 0;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            dxDevice->CreateRenderTargetView(
                texture,
                &rtvDesc,
                &rtv);
            dxRenderTargetViews.Add(rtv);
            dxRenderTargetTextures.Add(texture);
        }

        // We immediately bind the back-buffer render target view, and we aren't
        // going to switch. We don't bother with a depth buffer.
        dxImmediateContext->OMSetRenderTargets(
            dxRenderTargetViews.Count(),
            dxRenderTargetViews.Buffer(),
            NULL);

        // Similarly, we are going to set up a viewport once, and then never
        // switch, since this is a simple test app.
        D3D11_VIEWPORT dxViewport;
        dxViewport.TopLeftX = 0;
        dxViewport.TopLeftY = 0;
        dxViewport.Width = (float) gWindowWidth;
        dxViewport.Height = (float) gWindowHeight;
        dxViewport.MaxDepth = 1; // TODO(tfoley): use reversed depth
        dxViewport.MinDepth = 0;
        dxImmediateContext->RSSetViewports(1, &dxViewport);
    }

    float clearColor[4] = { 0, 0, 0, 0 };
    virtual void setClearColor(float const* color) override
    {
        memcpy(clearColor, color, sizeof(clearColor));
    }

    virtual void clearFrame() override
    {
        for (auto i = 0u; i < dxRenderTargetViews.Count(); i++)
            dxImmediateContext->ClearRenderTargetView(
                dxRenderTargetViews[i],
                clearColor);
    }

    virtual void presentFrame() override
    {
        dxImmediateContext->CopyResource(dxBackBufferTexture, dxRenderTargetTextures[0]);
        dxSwapChain->Present(0, 0);
    }

    virtual void captureScreenShot(char const* outputPath) override
    {
        HRESULT hr = captureTextureToFile(
            dxDevice,
            dxImmediateContext,
            dxRenderTargetTextures[0],
            outputPath);
        if( FAILED(hr) )
        {
            fprintf(stderr, "error: could not capture screenshot to '%s'\n", outputPath);
            exit(1);
        }
    }

    virtual ShaderCompiler* getShaderCompiler() override
    {
        return this;
    }

	struct D3DBuffer
	{
		ID3D11UnorderedAccessView * view = nullptr;
		ID3D11Buffer * buffer = nullptr;
	};

    virtual Buffer* createBuffer(BufferDesc const& desc) override
    {
        D3D11_BUFFER_DESC dxBufferDesc = { 0 };
        dxBufferDesc.ByteWidth = (UINT) desc.size;

        switch( desc.flavor )
        {
        case BufferFlavor::Constant:
            dxBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            dxBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            dxBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            break;

        case BufferFlavor::Vertex:
            dxBufferDesc.Usage = D3D11_USAGE_DEFAULT;
            dxBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            dxBufferDesc.CPUAccessFlags = 0;
            break;

		case BufferFlavor::Storage:
			dxBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			dxBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			dxBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			dxBufferDesc.StructureByteStride = sizeof(float);
			dxBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			break;

        default:
            return nullptr;
        }

        D3D11_SUBRESOURCE_DATA dxInitData = { 0 };
        dxInitData.pSysMem = desc.initData;


        ID3D11Buffer* dxBuffer = nullptr;
        HRESULT hr = dxDevice->CreateBuffer(
            &dxBufferDesc,
            desc.initData ? &dxInitData : nullptr,
            &dxBuffer);
        if(FAILED(hr)) return nullptr;

		D3DBuffer * rs = new D3DBuffer();
		rs->buffer = dxBuffer;
		if (desc.flavor == BufferFlavor::Storage)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;
			memset(&viewDesc, 0, sizeof(viewDesc));
			viewDesc.Buffer.FirstElement = 0;
			viewDesc.Buffer.NumElements = 512;
			viewDesc.Buffer.Flags = 0;
			viewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			dxDevice->CreateUnorderedAccessView(dxBuffer, &viewDesc, &rs->view);
		}
        return (Buffer*) rs;
    }

    static DXGI_FORMAT mapFormat(Format format)
    {
        switch( format )
        {
        case Format::RGB_Float32:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case Format::RG_Float32:
            return DXGI_FORMAT_R32G32_FLOAT;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    virtual InputLayout* createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) override
    {
        D3D11_INPUT_ELEMENT_DESC dxInputElements[16] = {};

        char hlslBuffer[1024];
        char* hlslCursor = &hlslBuffer[0];

        hlslCursor += sprintf(hlslCursor, "float4 main(\n");

        for( UInt ii = 0; ii < inputElementCount; ++ii )
        {
            dxInputElements[ii].SemanticName            = inputElements[ii].semanticName;
            dxInputElements[ii].SemanticIndex           = (UINT) inputElements[ii].semanticIndex;
            dxInputElements[ii].Format                  = mapFormat(inputElements[ii].format);
            dxInputElements[ii].InputSlot               = 0;
            dxInputElements[ii].AlignedByteOffset       = (UINT) inputElements[ii].offset;
            dxInputElements[ii].InputSlotClass          = D3D11_INPUT_PER_VERTEX_DATA;
            dxInputElements[ii].InstanceDataStepRate    = 0;

            if(ii != 0)
            {
                hlslCursor+= sprintf(hlslCursor, ",\n");
            }

            char const* typeName = "Uknown";
            switch(inputElements[ii].format)
            {
            case Format::RGB_Float32:
                typeName = "float3";
                break;
            case Format::RG_Float32:
                typeName = "float2";
                break;
            default:
                return nullptr;
            }

            hlslCursor+= sprintf(hlslCursor, "%s a%d : %s%d",
                typeName,
                (int) ii,
                inputElements[ii].semanticName,
                (int) inputElements[ii].semanticIndex);
        }

        hlslCursor += sprintf(hlslCursor, "\n) : SV_Position { return 0; }");

        auto dxVertexShaderBlob = compileHLSLShader("inputLayout", hlslBuffer, "main", "vs_5_0");
        if(!dxVertexShaderBlob)
            return nullptr;

        ID3D11InputLayout* dxInputLayout = nullptr;
        HRESULT hr = dxDevice->CreateInputLayout(
            &dxInputElements[0],
            (UINT) inputElementCount,
            dxVertexShaderBlob->GetBufferPointer(),
            dxVertexShaderBlob->GetBufferSize(),
            &dxInputLayout);

        dxVertexShaderBlob->Release();

        if(FAILED(hr))
            return nullptr;

        return (InputLayout*) dxInputLayout;
    }

    void* map(ID3D11Buffer * buffer, MapFlavor flavor)
    {
        auto dxContext = dxImmediateContext;

        auto dxBuffer = buffer;

        D3D11_MAP dxMapFlavor;
        switch (flavor)
        {
        case MapFlavor::WriteDiscard:
            dxMapFlavor = D3D11_MAP_WRITE_DISCARD;
            break;
        case MapFlavor::HostWrite:
            dxMapFlavor = D3D11_MAP_WRITE;
            break;
        case MapFlavor::HostRead:
            dxMapFlavor = D3D11_MAP_READ;
            break;
        default:
            return nullptr;
        }

        // We update our constant buffer per-frame, just for the purposes
        // of the example, but we don't actually load different data
        // per-frame (we always use an identity projection).
        D3D11_MAPPED_SUBRESOURCE dxMapped;
        HRESULT hr = dxContext->Map(dxBuffer, 0, dxMapFlavor, 0, &dxMapped);
        if (FAILED(hr))
            return nullptr;

        return dxMapped.pData;
    }

    virtual void* map(Buffer* buffer, MapFlavor flavor) override
    {
        return map(((D3DBuffer*)buffer)->buffer, flavor);
    }

    void unmap(ID3D11Buffer * buffer)
    {
        auto dxContext = dxImmediateContext;
        dxContext->Unmap(buffer, 0);
    }

    virtual void unmap(Buffer* buffer) override
    {
        auto dxBuffer = ((D3DBuffer*)buffer)->buffer;
        unmap(dxBuffer);
    }

    virtual void setInputLayout(InputLayout* inputLayout) override
    {
        auto dxContext = dxImmediateContext;
        auto dxInputLayout = (ID3D11InputLayout*) inputLayout;

        dxContext->IASetInputLayout(dxInputLayout);
    }

    virtual void setPrimitiveTopology(PrimitiveTopology topology) override
    {
        auto dxContext = dxImmediateContext;

        D3D11_PRIMITIVE_TOPOLOGY dxTopology;
        switch( topology )
        {
        case PrimitiveTopology::TriangleList:
            dxTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;

        default:
            return;
        }

        dxContext->IASetPrimitiveTopology(dxTopology);
    }

    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* strides, UInt const* offsets) override
    {
        auto dxContext = dxImmediateContext;

        static const int kMaxVertexBuffers = 16;

        UINT dxVertexStrides[kMaxVertexBuffers];
        UINT dxVertexOffsets[kMaxVertexBuffers];

        for( UInt ii = 0; ii < slotCount; ++ii )
        {
            dxVertexStrides[ii] = (UINT) strides[ii];
            dxVertexOffsets[ii] = (UINT) offsets[ii];
        }

        auto dxVertexBuffers = (D3DBuffer* const*) buffers;

        dxContext->IASetVertexBuffers(
            (UINT) startSlot,
            (UINT) slotCount, &(dxVertexBuffers[0])->buffer, &dxVertexStrides[0], &dxVertexOffsets[0]);
    }

    virtual void setShaderProgram(ShaderProgram* inProgram) override
    {
        auto dxContext = dxImmediateContext;
        
        auto program = (D3D11ShaderProgram*) inProgram;
		dxContext->CSSetShader(program->dxComputeShader, NULL, 0);
        dxContext->VSSetShader(program->dxVertexShader, NULL, 0);
        dxContext->PSSetShader(program->dxPixelShader,  NULL, 0);
    }

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) override
    {
        auto dxContext = dxImmediateContext;

        // TODO: actually use those offsets

        auto dxConstantBuffers = (D3DBuffer* const*) buffers;
        dxContext->VSSetConstantBuffers(
            (UINT) startSlot, (UINT) slotCount, &dxConstantBuffers[0]->buffer);
        dxContext->VSSetConstantBuffers(
            (UINT) startSlot, (UINT) slotCount, &dxConstantBuffers[0]->buffer);
    }

	virtual void setStorageBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) override
	{
		auto dxContext = dxImmediateContext;

		// TODO: actually use those offsets

		auto dxStorageBuffers = (D3DBuffer* const*)buffers;
		dxContext->CSSetUnorderedAccessViews(
			(UINT)startSlot, (UINT)slotCount, &dxStorageBuffers[0]->view, 0);
	}


    virtual void draw(UInt vertexCount, UInt startVertex) override
    {
        auto dxContext = dxImmediateContext;
        applyBindingState(false);
        dxContext->Draw((UINT) vertexCount, (UINT) startVertex);
    }


    // ShaderCompiler interface

    struct D3D11ShaderProgram
    {
        ID3D11VertexShader* dxVertexShader = nullptr;
        ID3D11PixelShader*  dxPixelShader = nullptr;
		ID3D11ComputeShader* dxComputeShader = nullptr;
    };

    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override
    {
		if (request.computeShader.name)
		{
			auto dxComputeShaderBlob = compileHLSLShader(request.computeShader.source.path, request.computeShader.source.text, request.computeShader.name, request.computeShader.profile);
			if (!dxComputeShaderBlob)     return nullptr;

			ID3D11ComputeShader* dxComputeShader;

			HRESULT csResult = dxDevice->CreateComputeShader(dxComputeShaderBlob->GetBufferPointer(), dxComputeShaderBlob->GetBufferSize(), nullptr, &dxComputeShader);

			dxComputeShaderBlob->Release();

			if (FAILED(csResult)) return nullptr;

			D3D11ShaderProgram* shaderProgram = new D3D11ShaderProgram();
			shaderProgram->dxComputeShader = dxComputeShader;
			return (ShaderProgram*)shaderProgram;
		}
		else
		{
			auto dxVertexShaderBlob = compileHLSLShader(request.vertexShader.source.path, request.vertexShader.source.text, request.vertexShader.name, request.vertexShader.profile);
			if (!dxVertexShaderBlob)     return nullptr;

			auto dxFragmentShaderBlob = compileHLSLShader(request.fragmentShader.source.path, request.fragmentShader.source.text, request.fragmentShader.name, request.fragmentShader.profile);
			if (!dxFragmentShaderBlob)   return nullptr;

			ID3D11VertexShader* dxVertexShader;
			ID3D11PixelShader*  dxPixelShader;

			HRESULT vsResult = dxDevice->CreateVertexShader(dxVertexShaderBlob->GetBufferPointer(), dxVertexShaderBlob->GetBufferSize(), nullptr, &dxVertexShader);
			HRESULT psResult = dxDevice->CreatePixelShader(dxFragmentShaderBlob->GetBufferPointer(), dxFragmentShaderBlob->GetBufferSize(), nullptr, &dxPixelShader);

			dxVertexShaderBlob->Release();
			dxFragmentShaderBlob->Release();

			if (FAILED(vsResult)) return nullptr;
			if (FAILED(psResult)) return nullptr;

			D3D11ShaderProgram* shaderProgram = new D3D11ShaderProgram();
			shaderProgram->dxVertexShader = dxVertexShader;
			shaderProgram->dxPixelShader = dxPixelShader;
			return (ShaderProgram*)shaderProgram;
		}
    }

	virtual void dispatchCompute(int x, int y, int z) override
	{
		auto dxContext = dxImmediateContext;
        applyBindingState(true);
		dxContext->Dispatch(x, y, z);
	}

    void createInputBuffer(
        InputBufferDesc & bufferDesc, 
        List<unsigned int> & bufferData, 
        ID3D11Buffer * &bufferOut, 
        ID3D11UnorderedAccessView * &viewOut,
        ID3D11ShaderResourceView * &srvOut)
    {
        auto dxContext = dxImmediateContext;
        D3D11_BUFFER_DESC desc = {0};
        desc.ByteWidth = bufferData.Count() * sizeof(unsigned int);
        if (bufferDesc.type == InputBufferType::ConstantBuffer)
        {
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        }
        else
        {
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            if (bufferDesc.stride != 0)
            {
                desc.StructureByteStride = bufferDesc.stride;
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            }
            else
            {
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            }
        }
        D3D11_SUBRESOURCE_DATA data = {0};
        data.pSysMem = bufferData.Buffer();
        dxDevice->CreateBuffer(&desc, &data, &bufferOut);
        int elemSize = bufferDesc.stride <= 0 ? 1 : bufferDesc.stride;
        if (bufferDesc.type == InputBufferType::StorageBuffer)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;
            memset(&viewDesc, 0, sizeof(viewDesc));
            viewDesc.Buffer.FirstElement = 0;
            viewDesc.Buffer.NumElements = (UINT)(bufferData.Count() * sizeof(unsigned int) / elemSize);
            viewDesc.Buffer.Flags = 0;
            viewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            viewDesc.Format = DXGI_FORMAT_UNKNOWN;
            dxDevice->CreateUnorderedAccessView(bufferOut, &viewDesc, &viewOut);
        }
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        memset(&srvDesc, 0, sizeof(srvDesc));
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.ElementWidth = elemSize;
        srvDesc.Buffer.NumElements = (UINT)(bufferData.Count() * sizeof(unsigned int) / elemSize);
        srvDesc.Buffer.ElementOffset = 0;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        dxDevice->CreateShaderResourceView(bufferOut, &srvDesc, &srvOut);
    }

    void createInputTexture(const InputTextureDesc & inputDesc, ID3D11ShaderResourceView * &viewOut)
    {
        TextureData texData;
        generateTextureData(texData, inputDesc);
        List<D3D11_SUBRESOURCE_DATA> subRes;
        for (int i = 0; i < texData.arraySize; i++)
        {
            int slice = 0;
            for (int j = 0; j < texData.mipLevels; j++)
            {
                int size = texData.textureSize >> j;
                D3D11_SUBRESOURCE_DATA res;
                res.pSysMem = texData.dataBuffer[slice].Buffer();
                res.SysMemPitch = sizeof(unsigned int) * size;
                res.SysMemSlicePitch = sizeof(unsigned int) * size * size;
                subRes.Add(res);
                slice++;
            }
        }
        if (inputDesc.dimension == 1)
        {
            D3D11_TEXTURE1D_DESC desc = { 0 };
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.MiscFlags = 0;
            desc.MipLevels = texData.mipLevels;
            desc.ArraySize = texData.arraySize;
            desc.Width = texData.textureSize;
            desc.Usage = D3D11_USAGE_DEFAULT;
            
            ID3D11Texture1D * texture;
            dxDevice->CreateTexture1D(&desc, subRes.Buffer(), &texture);
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
            if (inputDesc.arrayLength != 0)
                viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
            viewDesc.Texture1D.MipLevels = texData.mipLevels;
            viewDesc.Texture1D.MostDetailedMip = 0;
            viewDesc.Texture1DArray.ArraySize = texData.arraySize;
            viewDesc.Texture1DArray.FirstArraySlice = 0;
            viewDesc.Texture1DArray.MipLevels = texData.mipLevels;
            viewDesc.Texture1DArray.MostDetailedMip = 0;
            viewDesc.Format = desc.Format;
            dxDevice->CreateShaderResourceView(texture, &viewDesc, &viewOut);
        }
        else if (inputDesc.dimension == 2)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            D3D11_TEXTURE2D_DESC desc = { 0 };
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.MiscFlags = 0;
            desc.MipLevels = texData.mipLevels;
            desc.ArraySize = texData.arraySize;
            if (inputDesc.isCube)
            {
                desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
                viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
                viewDesc.TextureCube.MipLevels = texData.mipLevels;
                viewDesc.TextureCube.MostDetailedMip = 0;
                viewDesc.TextureCubeArray.MipLevels = texData.mipLevels;
                viewDesc.TextureCubeArray.MostDetailedMip = 0;
                viewDesc.TextureCubeArray.First2DArrayFace = 0;
                viewDesc.TextureCubeArray.NumCubes = inputDesc.arrayLength;
            }
            else
            {
                viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                viewDesc.Texture2D.MipLevels = texData.mipLevels;
                viewDesc.Texture2D.MostDetailedMip = 0;
                viewDesc.Texture2DArray.ArraySize = texData.arraySize;
                viewDesc.Texture2DArray.FirstArraySlice = 0;
                viewDesc.Texture2DArray.MipLevels = texData.mipLevels;
                viewDesc.Texture2DArray.MostDetailedMip = 0;
            }
            if (inputDesc.arrayLength != 0)
                viewDesc.ViewDimension = (D3D11_SRV_DIMENSION)(int)(viewDesc.ViewDimension + 1);
            desc.Width = texData.textureSize;
            desc.Height = texData.textureSize;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            viewDesc.Format = desc.Format;
            ID3D11Texture2D * texture;
            dxDevice->CreateTexture2D(&desc, subRes.Buffer(), &texture);
            dxDevice->CreateShaderResourceView(texture, &viewDesc, &viewOut);
        }
        else if (inputDesc.dimension == 3)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
            D3D11_TEXTURE3D_DESC desc = { 0 };
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.MiscFlags = 0;
            desc.MipLevels = 1;
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            desc.Width = texData.textureSize;
            desc.Height = texData.textureSize;
            desc.Depth = texData.textureSize;
            desc.Usage = D3D11_USAGE_DEFAULT;
            ID3D11Texture3D * texture;
            dxDevice->CreateTexture3D(&desc, subRes.Buffer(), &texture);
            viewDesc.Texture3D.MipLevels = 1;
            viewDesc.Texture3D.MostDetailedMip = 0;
            viewDesc.Format = desc.Format;
            dxDevice->CreateShaderResourceView(texture, &viewDesc, &viewOut);
        }
    }

    void createInputSampler(const InputSamplerDesc & inputDesc, ID3D11SamplerState * & stateOut)
    {
        D3D11_SAMPLER_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        if (inputDesc.isCompareSampler)
        {
            desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
            desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            desc.MinLOD = desc.MaxLOD = 0.0f;
        }
        else
        {
            desc.Filter = D3D11_FILTER_ANISOTROPIC;
            desc.MaxAnisotropy = 8;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = 100.0f;
        }
        dxDevice->CreateSamplerState(&desc, &stateOut);
    }
    virtual BindingState * createBindingState(const ShaderInputLayout & layout)
    {
        D3DBindingState * rs = new D3DBindingState();
        for (auto & entry : layout.entries)
        {
            D3DBinding rsEntry;
            rsEntry.type = entry.type;
            rsEntry.binding = entry.hlslBinding;
            rsEntry.isOutput = entry.isOutput;
            switch (entry.type)
            {
            case ShaderInputType::Buffer:
            {
                createInputBuffer(entry.bufferDesc, entry.bufferData, rsEntry.buffer, rsEntry.uav, rsEntry.srv);
                rsEntry.bufferLength = entry.bufferData.Count() * sizeof(unsigned int);
            }
            break;
            case ShaderInputType::Texture:
            {
                createInputTexture(entry.textureDesc, rsEntry.srv);
            }
            break;
            case ShaderInputType::Sampler:
            {
                createInputSampler(entry.samplerDesc, rsEntry.samplerState);
            }
            break;
            case ShaderInputType::CombinedTextureSampler:
            {
                throw "not implemented";
            }
            break;
            }
            rs->bindings.Add(rsEntry);
        }

        return (BindingState*)rs;
    }

    void applyBindingState(bool isCompute)
    {
        auto dxContext = dxImmediateContext;
        for (auto & binding : currentBindings->bindings)
        {
            if (binding.type == ShaderInputType::Buffer)
            {
                if (binding.uav)
                {
                    if (isCompute)
                        dxContext->CSSetUnorderedAccessViews(binding.binding, 1, &binding.uav, nullptr);
                    else
                        dxContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
                            nullptr, nullptr, binding.binding, 1, &binding.uav, nullptr);
                }
                else
                {
                    if (isCompute)
                        dxContext->CSSetShaderResources(binding.binding, 1, &binding.srv);
                    else
                    {
                        dxContext->PSSetShaderResources(binding.binding, 1, &binding.srv);
                        dxContext->VSSetShaderResources(binding.binding, 1, &binding.srv);
                    }
                }
            }
            else if (binding.type == ShaderInputType::Texture)
            {
                if (binding.uav)
                {
                    if (isCompute)
                        dxContext->CSSetUnorderedAccessViews(binding.binding, 1, &binding.uav, nullptr);
                    else
                        dxContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
                            nullptr, nullptr, binding.binding, 1, &binding.uav, nullptr);
                }
                else
                {
                    if (isCompute)
                        dxContext->CSSetShaderResources(binding.binding, 1, &binding.srv);
                    else
                    {
                        dxContext->PSSetShaderResources(binding.binding, 1, &binding.srv);
                        dxContext->VSSetShaderResources(binding.binding, 1, &binding.srv);
                    }
                }
            }
            else if (binding.type == ShaderInputType::Sampler)
            {
                if (isCompute)
                    dxContext->CSSetSamplers(binding.binding, 1, &binding.samplerState);
                else
                {
                    dxContext->PSSetSamplers(binding.binding, 1, &binding.samplerState);
                    dxContext->VSSetSamplers(binding.binding, 1, &binding.samplerState);
                }
            }
            else
                throw "not implemented";
        }
    }

    virtual void setBindingState(BindingState * state)
    {
        auto dxBindingState = (D3DBindingState*) state;
        currentBindings = dxBindingState;
    }

    virtual void serializeOutput(BindingState* state, const char * fileName)
    {
        auto dxContext = dxImmediateContext;
        auto dxBindingState = (D3DBindingState*)state;
        FILE * f = fopen(fileName, "wb");
        int id = 0;
        for (auto & binding : dxBindingState->bindings)
        {
            if (binding.isOutput)
            {
                if (binding.buffer)
                {
                    auto ptr = (unsigned int *)map(binding.buffer, MapFlavor::HostRead);
                    for (auto i = 0u; i < binding.bufferLength / sizeof(unsigned int); i++)
                        fprintf(f, "%X\n", ptr[i]);
                    unmap(binding.buffer);
                }
                else
                {
                    printf("invalid output type at %d.\n");
                }
            }
            id++;
        }
        fprintf(f, "test result. \n");
        fclose(f);
    }
};



Renderer* createD3D11Renderer()
{
    return new D3D11Renderer();
}

} // renderer_test
