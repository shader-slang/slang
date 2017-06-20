// hello.cpp

// In order to use the Slang API, we need to include its header

#define SLANG_INCLUDE_IMPLEMENTATION
#include <slang.h>

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

static int gWindowWidth = 1024;
static int gWindowHeight = 768;

//

//
// For the purposes of a small example, we will define the vertex data for a
// single triangle directly in the source file. It should be easy to extend
// this example to load data from an external source, if desired.
//

struct Vertex
{
    float position[3];
    float color[3];
};

static const int kVertexCount = 3;
static const Vertex kVertexData[kVertexCount] = {
    { { 0,  0, 0.5 }, {1, 0, 0} },
    { { 0,  1, 0.5 }, {0, 0, 1} },
    { { 1,  0, 0.5 }, {0, 1, 0} },
};

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
    char const* source,
    char const* entryPointName,
    char const* dxProfileName);

//
// At initialization time, we are going to load and compile our Slang shader
// code, and then create the D3D11 API objects we need for rendering.
//
HRESULT initialize( ID3D11Device* dxDevice )
{
#if 1
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

    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, "hello.slang");

    char const* vertexEntryPointName    = "vertexMain";
    char const* fragmentEntryPointName  = "fragmentMain";

    char const* vertexProfileName   = "vs_4_0";
    char const* fragmentProfileName = "ps_4_0";

    spAddEntryPoint(slangRequest, translationUnitIndex, vertexEntryPointName,   spFindProfile(slangSession, vertexProfileName));
    spAddEntryPoint(slangRequest, translationUnitIndex, fragmentEntryPointName, spFindProfile(slangSession, fragmentProfileName));

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


    // TODO(tfoley): Query the required constant-buffer size
    int constantBufferSize = 16 * sizeof(float);

    // Compile the generated HLSL code
    ID3DBlob* dxVertexShaderBlob = compileHLSLShader(translatedCode, vertexEntryPointName, vertexProfileName);
    if(!dxVertexShaderBlob) return E_FAIL;

    ID3DBlob* dxPixelShaderBlob = compileHLSLShader(translatedCode, fragmentEntryPointName, fragmentProfileName);
    if(!dxPixelShaderBlob) return E_FAIL;

    HRESULT hr = S_OK;


    D3D11_BUFFER_DESC dxConstantBufferDesc = { 0 };
    dxConstantBufferDesc.ByteWidth = constantBufferSize;
    dxConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    dxConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    dxConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = dxDevice->CreateBuffer(
        &dxConstantBufferDesc,
        NULL,
        &dxConstantBuffer);
    if(FAILED(hr)) return hr;

    // We clean up the Slang compilation context and result *after*
    // we have done the HLSL-to-bytecode compilation, because Slang
    // owns the memory allocation for the generated HLSL, and will
    // free it when we destroy the compilation result.
    spDestroyCompileRequest(slangRequest);
    spDestroySession(slangSession);

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
        dxVertexShaderBlob->GetBufferPointer(),
        dxVertexShaderBlob->GetBufferSize(),
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
        dxVertexShaderBlob->GetBufferPointer(),
        dxVertexShaderBlob->GetBufferSize(),
        NULL,
        &dxVertexShader);
    dxVertexShaderBlob->Release();
    if(FAILED(hr)) return hr;

    // Pixel Shader (PS)

    hr = dxDevice->CreatePixelShader(
        dxPixelShaderBlob->GetBufferPointer(),
        dxPixelShaderBlob->GetBufferSize(),
        NULL,
        &dxPixelShader);
    dxPixelShaderBlob->Release();
    if(FAILED(hr)) return hr;
#endif

    return S_OK;
}

void renderFrame(ID3D11DeviceContext* dxContext)
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

//
// Definition of the HLSL-to-bytecode compilation logic.
//
ID3DBlob* compileHLSLShader(
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

    // The `D3DCompile` entry point takes a bunch of parameters, but we
    // don't really need most of them for Slang-generated code.
    ID3DBlob* dxShaderBlob = nullptr;
    ID3DBlob* dxErrorBlob = nullptr;
    HRESULT hr = D3DCompile_(
        source,
        strlen(source),
        "slangGeneratedCode", // TODO: proper path for error messages
        nullptr,
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


//
// We use a bare-minimum window procedure to get things up and running.
//

static LRESULT CALLBACK windowProc(
    HWND    windowHandle,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(windowHandle, message, wParam, lParam);
}

//
// Our `WinMain` handles the basic task of getting a window and rendering
// context up and running. There should be nothing suprising or interesting
// here.
//

int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE /* prevInstance */,
    LPSTR     /* commandLine */,
    int       showCommand)
{
    // First we register a window class.

    WNDCLASSEXW windowClassDesc;
    windowClassDesc.cbSize = sizeof(windowClassDesc);
    windowClassDesc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClassDesc.lpfnWndProc = &windowProc;
    windowClassDesc.cbClsExtra = 0;
    windowClassDesc.cbWndExtra = 0;
    windowClassDesc.hInstance = instance;
    windowClassDesc.hIcon = 0;
    windowClassDesc.hCursor = 0;
    windowClassDesc.hbrBackground = 0;
    windowClassDesc.lpszMenuName = 0;
    windowClassDesc.lpszClassName = L"HelloWorld";
    windowClassDesc.hIconSm = 0;
    ATOM windowClassAtom = RegisterClassExW(&windowClassDesc);
    if(!windowClassAtom)
    {
        fprintf(stderr, "error: failed to register window class\n");
        return 1;
    }

    // Next, we create a window using that window class.

    DWORD windowExtendedStyle = 0;
    DWORD windowStyle = 0;
    LPWSTR windowName = L"Slang Hello World";
    HWND windowHandle = CreateWindowExW(
        windowExtendedStyle,
        (LPWSTR)windowClassAtom,
        windowName,
        windowStyle,
        0, 0, // x, y
        gWindowWidth, gWindowHeight,
        NULL, // parent
        NULL, // menu
        instance,
        NULL);
    if(!windowHandle)
    {
        fprintf(stderr, "error: failed to create window\n");
        return 1;
    }


    // Rather than statically link against D3D, we load it dynamically.

    HMODULE d3d11 = LoadLibraryA("d3d11.dll");
    if(!d3d11)
    {
        fprintf(stderr, "error: failed load 'd3d11.dll'\n");
        return 1;
    }

    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN D3D11CreateDeviceAndSwapChain_ =
        (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(
            d3d11,
            "D3D11CreateDeviceAndSwapChain");
    if(!D3D11CreateDeviceAndSwapChain_)
    {
        fprintf(stderr,
            "error: failed load symbol 'D3D11CreateDeviceAndSwapChain'\n");
        return 1;
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
    dxSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
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

    IDXGISwapChain* dxSwapChain = NULL;
    ID3D11Device* dxDevice = NULL;
    ID3D11DeviceContext* dxImmediateContext = NULL;
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
        return 1;
    }

    // After we've created the swap chain, we can request a pointer to the
    // back buffer as a D3D11 texture, and create a render-target view from it.

    ID3D11Texture2D* dxBackBufferTexture = NULL;
    static const IID kIID_ID3D11Texture2D = {
        0x6f15aaf2, 0xd208, 0x4e89, 0x9a, 0xb4, 0x48,
        0x95, 0x35, 0xd3, 0x4f, 0x9c };
    dxSwapChain->GetBuffer(
        0,
        kIID_ID3D11Texture2D,
        (void**)&dxBackBufferTexture);

    ID3D11RenderTargetView* dxBackBufferRTV = NULL;
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

    // Once we've done the general-purpose initialization, we
    // initialize anything specific to the "hello world" application
    hr = initialize( dxDevice );
    if( FAILED(hr) )
    {
        exit(1);
    }

    // Once initialization is all complete, we show the window...
    ShowWindow(windowHandle, showCommand);

    // ... and enter the event loop:
    for(;;)
    {
        MSG message;

        int result = PeekMessageW(&message, NULL, 0, 0, PM_REMOVE);
        if (result != 0)
        {
            if (message.message == WM_QUIT)
            {
                return (int)message.wParam;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        else
        {
            // Whenver we don't have Windows events to process,
            // we render a frame.

            static const float kClearColor[] = { 0.25, 0.25, 0.25, 1.0 };
            dxImmediateContext->ClearRenderTargetView(
                dxBackBufferRTV,
                kClearColor);

            renderFrame( dxImmediateContext );

            dxSwapChain->Present(0, 0);
        }
    }

    return 0;
}


//
// In order to actually use Slang in our application, we need to link in its
// implementation. The easiest way to accomplish this is by directly inlcuding
// the (concatenated) Slang source code into our app.
//

#define SLANG_INCLUDE_IMPLEMENTATION
#include <slang.h>
