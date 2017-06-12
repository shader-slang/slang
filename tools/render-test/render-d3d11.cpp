// render-d3d11.cpp
#include "render-d3d11.h"

#include "options.h"
#include "render.h"

// In order to use the Slang API, we need to include its header

#include <Slang.h>

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

namespace renderer_test {

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

// Initialization when using HLSL for shaders
HRESULT initializeHLSLInner(ID3D11Device* dxDevice, char const* sourcePath, char const* sourceText)
{
    // Compile the generated HLSL code
    gVertexShaderBlob = compileHLSLShader(sourcePath, sourceText, vertexEntryPointName, vertexProfileName);
    if(!gVertexShaderBlob) return E_FAIL;

    gPixelShaderBlob = compileHLSLShader(sourcePath, sourceText, fragmentEntryPointName, fragmentProfileName);
    if(!gPixelShaderBlob) return E_FAIL;


    return S_OK;
}

// Initialization when using HLSL for shaders
HRESULT initializeHLSL(ID3D11Device* dxDevice, char const* sourceText)
{
    HRESULT hr = initializeHLSLInner(dxDevice, gOptions.sourcePath, sourceText);
    if(FAILED(hr))
        return hr;

    // TODO: any reflection stuff to do here?

    return S_OK;
}

// Initialization when using Slang for shaders
HRESULT initializeSlang(ID3D11Device* dxDevice, char const* sourceText)
{
    //
    // First, we will load and compile our Slang source code.
    //

    // The argument here is an optional directory where the Slang compiler
    // can cache files to speed up compilation of many kernels.
    SlangSession* slangSession = spCreateSession(NULL);

    // A compile request represents a single invocation of the compiler,
    // to process some inputs and produce outputs (or errors).
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    // Instruct Slang to generate code as HLSL
    spSetCodeGenTarget(slangRequest, SLANG_HLSL);

    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

    spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, gOptions.sourcePath, sourceText);

    spAddTranslationUnitEntryPoint(slangRequest, translationUnitIndex, vertexEntryPointName,   spFindProfile(slangSession, vertexProfileName));
    spAddTranslationUnitEntryPoint(slangRequest, translationUnitIndex, fragmentEntryPointName, spFindProfile(slangSession, fragmentProfileName));

    int compileErr = spCompile(slangRequest);
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        OutputDebugStringA(diagnostics);
        fprintf(stderr, "%s", diagnostics);
    }
    if(compileErr)
    {
        return E_FAIL;
    }

    char const* translatedCode = spGetTranslationUnitSource(slangRequest, translationUnitIndex);

    // Compile the generated HLSL code
    HRESULT hr = initializeHLSLInner(dxDevice, "slangGeneratedCode", translatedCode);
    if(FAILED(hr))
        return hr;

    // We clean up the Slang compilation context and result *after*
    // we have done the HLSL-to-bytecode compilation, because Slang
    // owns the memory allocation for the generated HLSL, and will
    // free it when we destroy the compilation result.
    spDestroyCompileRequest(slangRequest);
    spDestroySession(slangSession);

    return S_OK;
}

#if 0

//
// At initialization time, we are going to load and compile our Slang shader
// code, and then create the D3D11 API objects we need for rendering.
//
HRESULT initializeInner( ID3D11Device* dxDevice )
{
    HRESULT hr = S_OK;

    // Read in the source code
    char const* sourcePath = gOptions.sourcePath;
    FILE* sourceFile = fopen(sourcePath, "rb");
    if( !sourceFile )
    {
        fprintf(stderr, "error: failed to open '%s' for reading\n", sourcePath);
        exit(1);
    }
    fseek(sourceFile, 0, SEEK_END);
    size_t sourceSize = ftell(sourceFile);
    fseek(sourceFile, 0, SEEK_SET);
    char* sourceText = (char*) malloc(sourceSize + 1);
    if( !sourceText )
    {
        fprintf(stderr, "error: out of memory");
        exit(1);
    }
    fread(sourceText, sourceSize, 1, sourceFile);
    fclose(sourceFile);
    sourceText[sourceSize] = 0;

    switch( gOptions.mode )
    {
    case Mode::HLSL:
        hr = initializeHLSL(dxDevice, sourceText);
        break;

    case Mode::Slang:
        hr = initializeSlang(dxDevice, sourceText);
        break;

    default:
        hr = E_FAIL;
        break;
    }
    if( FAILED(hr) )
    {
        return hr;
    }

    // Do other initialization that doesn't depend on the source language.

    // TODO(tfoley): use each API's reflection interface to query the constant-buffer size needed
    gConstantBufferSize = 16 * sizeof(float);


    D3D11_BUFFER_DESC dxConstantBufferDesc = { 0 };
    dxConstantBufferDesc.ByteWidth = gConstantBufferSize;
    dxConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    dxConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    dxConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = dxDevice->CreateBuffer(
        &dxConstantBufferDesc,
        NULL,
        &dxConstantBuffer);
    if(FAILED(hr)) return hr;


    // Input Assembler (IA)

    // In Slang-generated HLSL, all vertex shader inputs have a semantic
    // like: `A0`, `A1`, `A2`, etc., rather than trying to do by-name
    // matching. The user is thus responsibile for ensuring that the
    // order of their "input element descs" here matches the order
    // in which inputs are declared in the shader code.
    D3D11_INPUT_ELEMENT_DESC dxInputElements[] = {
        {"A", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        {"A", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = dxDevice->CreateInputLayout(
        &dxInputElements[0],
        2,
        gVertexShaderBlob->GetBufferPointer(),
        gVertexShaderBlob->GetBufferSize(),
        &dxInputLayout);
    if(FAILED(hr)) return hr;

    D3D11_BUFFER_DESC dxVertexBufferDesc = { 0 };
    dxVertexBufferDesc.ByteWidth = kVertexCount * sizeof(Vertex);
    dxVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    dxVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA dxVertexBufferInitData = { 0 };
    dxVertexBufferInitData.pSysMem = &kVertexData[0];

    hr = dxDevice->CreateBuffer(
        &dxVertexBufferDesc,
        &dxVertexBufferInitData,
        &dxVertexBuffer);
    if(FAILED(hr)) return hr;

    // Vertex Shader (VS)

    hr = dxDevice->CreateVertexShader(
        gVertexShaderBlob->GetBufferPointer(),
        gVertexShaderBlob->GetBufferSize(),
        NULL,
        &dxVertexShader);
    gVertexShaderBlob->Release();
    if(FAILED(hr)) return hr;

    // Pixel Shader (PS)

    hr = dxDevice->CreatePixelShader(
        gPixelShaderBlob->GetBufferPointer(),
        gPixelShaderBlob->GetBufferSize(),
        NULL,
        &dxPixelShader);
    gPixelShaderBlob->Release();
    if(FAILED(hr)) return hr;

    return S_OK;
}

void renderFrameInner(ID3D11DeviceContext* dxContext)
{
    // We update our constant buffer per-frame, just for the purposes
    // of the example, but we don't actually load different data
    // per-frame (we always use an identity projection).
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = dxContext->Map(dxConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if(!FAILED(hr))
    {
        float* data = (float*) mapped.pData;

        static const float kIdentity[] =
        { 1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1 };
        memcpy(data, kIdentity, sizeof(kIdentity));

        dxContext->Unmap(dxConstantBuffer, 0);
    }

    // Input Assembler (IA)

    dxContext->IASetInputLayout(dxInputLayout);
    dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT dxVertexStride = sizeof(Vertex);
    UINT dxVertexBufferOffset = 0;
    dxContext->IASetVertexBuffers(0, 1, &dxVertexBuffer, &dxVertexStride, &dxVertexBufferOffset);

    // Vertex Shader (VS)

    dxContext->VSSetShader(dxVertexShader, NULL, 0);
    dxContext->VSSetConstantBuffers(0, 1, &dxConstantBuffer);

    // Pixel Shader (PS)

    dxContext->PSSetShader(dxPixelShader, NULL, 0);
    dxContext->VSSetConstantBuffers(0, 1, &dxConstantBuffer);

    //

    dxContext->Draw(3, 0);
}

void finalize()
{
}

#endif

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
    if(dxTextureDesc.SampleDesc.Count > 1)
        return E_INVALIDARG;

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
        dxTextureDesc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        dxTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        dxTextureDesc.Usage = D3D11_USAGE_STAGING;

        hr = dxDevice->CreateTexture2D(&dxTextureDesc, 0, &dxStagingTexture);
        if(FAILED(hr))
            return hr;
    
        dxContext->CopyResource(dxStagingTexture, dxTexture);
    }

    // Now just read back texels from the staging textures

    D3D11_MAPPED_SUBRESOURCE dxMappedResource;
    hr = dxContext->Map(dxStagingTexture, 0, D3D11_MAP_READ, 0, &dxMappedResource);
    if(FAILED(hr))
        return hr;

    int stbResult = stbi_write_png(
        outputPath,
        dxTextureDesc.Width,
        dxTextureDesc.Height,
        4,
        dxMappedResource.pData,
        dxMappedResource.RowPitch);
    if( !stbResult )
    {
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
    ID3D11RenderTargetView* dxBackBufferRTV = NULL;

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
                D3D_DRIVER_TYPE_HARDWARE,
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

        dxDevice->CreateRenderTargetView(
            dxBackBufferTexture,
            NULL,
            &dxBackBufferRTV);

        // We immediately bind the back-buffer render target view, and we aren't
        // going to switch. We don't bother with a depth buffer.
        dxImmediateContext->OMSetRenderTargets(
            1,
            &dxBackBufferRTV,
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
        dxImmediateContext->ClearRenderTargetView(
            dxBackBufferRTV,
            clearColor);
    }

    virtual void presentFrame() override
    {
        dxSwapChain->Present(0, 0);
    }

    virtual void captureScreenShot(char const* outputPath) override
    {
        HRESULT hr = captureTextureToFile(
            dxDevice,
            dxImmediateContext,
            dxBackBufferTexture,
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

    virtual Buffer* createBuffer(BufferDesc const& desc) override
    {
        D3D11_BUFFER_DESC dxBufferDesc = { 0 };
        dxBufferDesc.ByteWidth = desc.size;

        switch( desc.flavor )
        {
        case BufferFlavor::Constant:
            dxBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            dxBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            dxBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            break;

        case BufferFlavor::Vertex:
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

        return (Buffer*) dxBuffer;
    }

    static DXGI_FORMAT mapFormat(Format format)
    {
        switch( format )
        {
        case Format::RGB_Float32:
            return DXGI_FORMAT_R32G32B32_FLOAT;

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
            dxInputElements[ii].SemanticIndex           = inputElements[ii].semanticIndex;
            dxInputElements[ii].Format                  = mapFormat(inputElements[ii].format);
            dxInputElements[ii].InputSlot               = 0;
            dxInputElements[ii].AlignedByteOffset       = inputElements[ii].offset;
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

            default:
                return nullptr;
            }

            hlslCursor+= sprintf(hlslCursor, "%s a%d : %s%d", typeName, ii,
                inputElements[ii].semanticName,
                inputElements[ii].semanticIndex);
        }

        hlslCursor += sprintf(hlslCursor, "\n) : SV_Position { return 0; }");

        auto dxVertexShaderBlob = compileHLSLShader("inputLayout", hlslBuffer, "main", "vs_4_0");
        if(!dxVertexShaderBlob)
            return nullptr;

        ID3D11InputLayout* dxInputLayout = nullptr;
        HRESULT hr = dxDevice->CreateInputLayout(
            &dxInputElements[0],
            inputElementCount,
            dxVertexShaderBlob->GetBufferPointer(),
            dxVertexShaderBlob->GetBufferSize(),
            &dxInputLayout);

        dxVertexShaderBlob->Release();

        if(FAILED(hr))
            return nullptr;

        return (InputLayout*) dxInputLayout;
    }

    virtual void* map(Buffer* buffer, MapFlavor flavor) override
    {
        auto dxContext = dxImmediateContext;

        auto dxBuffer = (ID3D11Buffer*) buffer;

        D3D11_MAP dxMapFlavor;
        switch( flavor )
        {
        case MapFlavor::WriteDiscard:
            dxMapFlavor = D3D11_MAP_WRITE_DISCARD;
            break;

        default:
            return nullptr;
        }

        // We update our constant buffer per-frame, just for the purposes
        // of the example, but we don't actually load different data
        // per-frame (we always use an identity projection).
        D3D11_MAPPED_SUBRESOURCE dxMapped;
        HRESULT hr = dxContext->Map(dxBuffer, 0, dxMapFlavor, 0, &dxMapped);
        if(FAILED(hr))
            return nullptr;

        return dxMapped.pData;
    }

    virtual void unmap(Buffer* buffer) override
    {
        auto dxContext = dxImmediateContext;

        auto dxBuffer = (ID3D11Buffer*) buffer;

        dxContext->Unmap(dxBuffer, 0);
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
            dxVertexStrides[ii] = strides[ii];
            dxVertexOffsets[ii] = offsets[ii];
        }

        auto dxVertexBuffers = (ID3D11Buffer* const*) buffers;

        dxContext->IASetVertexBuffers(startSlot, slotCount, &dxVertexBuffers[0], &dxVertexStrides[0], &dxVertexOffsets[0]);
    }

    virtual void setShaderProgram(ShaderProgram* inProgram) override
    {
        auto dxContext = dxImmediateContext;
        
        auto program = (D3D11ShaderProgram*) inProgram;

        dxContext->VSSetShader(program->dxVertexShader, NULL, 0);
        dxContext->PSSetShader(program->dxPixelShader,  NULL, 0);
    }

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) override
    {
        auto dxContext = dxImmediateContext;

        // TODO: actually use those offsets

        auto dxConstantBuffers = (ID3D11Buffer* const*) buffers;

        dxContext->VSSetConstantBuffers(startSlot, slotCount, &dxConstantBuffers[0]);
        dxContext->VSSetConstantBuffers(startSlot, slotCount, &dxConstantBuffers[0]);
    }


    virtual void draw(UInt vertexCount, UInt startVertex) override
    {
        auto dxContext = dxImmediateContext;

        dxContext->Draw(vertexCount, startVertex);
    }


    // ShaderCompiler interface

    struct D3D11ShaderProgram
    {
        ID3D11VertexShader* dxVertexShader;
        ID3D11PixelShader*  dxPixelShader;
    };

    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override
    {
        auto dxVertexShaderBlob     = compileHLSLShader(request.source.path, request.source.text, request.vertexShader    .name,  request.vertexShader    .profile);
        if(!dxVertexShaderBlob)     return nullptr;

        auto dxFragmentShaderBlob   = compileHLSLShader(request.source.path, request.source.text, request.fragmentShader  .name,  request.fragmentShader  .profile);
        if(!dxFragmentShaderBlob)   return nullptr;

        ID3D11VertexShader* dxVertexShader;
        ID3D11PixelShader*  dxPixelShader;

        HRESULT vsResult = dxDevice->CreateVertexShader(  dxVertexShaderBlob  ->GetBufferPointer(),   dxVertexShaderBlob  ->GetBufferSize(), nullptr, &dxVertexShader);
        HRESULT psResult = dxDevice->CreatePixelShader(   dxFragmentShaderBlob->GetBufferPointer(),   dxFragmentShaderBlob->GetBufferSize(), nullptr, &dxPixelShader);

        dxVertexShaderBlob  ->Release();
        dxFragmentShaderBlob->Release();

        if(FAILED(vsResult)) return nullptr;
        if(FAILED(psResult)) return nullptr;

        D3D11ShaderProgram* shaderProgram = new D3D11ShaderProgram();
        shaderProgram->dxVertexShader   = dxVertexShader;
        shaderProgram->dxPixelShader    = dxPixelShader;
        return (ShaderProgram*) shaderProgram;
    }
};



Renderer* createD3D11Renderer()
{
    return new D3D11Renderer();
}

} // renderer_test
