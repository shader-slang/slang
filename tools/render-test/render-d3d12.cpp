// render-d3d12.cpp
#include "render-d3d12.h"

#include "options.h"
#include "render.h"

// In order to use the Slang API, we need to include its header

#include <slang.h>

// We will be rendering with Direct3D 12, so we need to include
// the Windows and D3D12 headers

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <dxgi1_4.h>
#include <d3d12.h>
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

#define ENABLE_DEBUG_LAYER 1

namespace renderer_test {

// The Slang compiler currently generates HLSL source, so we'll need a utility
// routine (defined later) to translate that into D3D11 shader bytecode.
// Returns nullptr if compilation fails.
ID3DBlob* compileHLSLShader(
    char const* sourcePath,
    char const* source,
    char const* entryPointName,
    char const* dxProfileName);

//static char const* vertexProfileName   = "vs_4_0";
//static char const* fragmentProfileName = "ps_4_0";

//
class D3D12Renderer : public Renderer, public ShaderCompiler
{
public:
    // Renderer    implementation
    virtual SlangResult initialize(void* inWindowHandle) override;
    virtual void setClearColor(float const* color) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual SlangResult captureScreenShot(char const* outputPath) override;
    virtual void serializeOutput(BindingState* state, const char * fileName) override;
    virtual Buffer* createBuffer(BufferDesc const& desc) override;
    virtual InputLayout* createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) override;
    virtual BindingState * createBindingState(const ShaderInputLayout & layout) override;
    virtual ShaderCompiler* getShaderCompiler() override;
    virtual void* map(Buffer* buffer, MapFlavor flavor) override;
    virtual void unmap(Buffer* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState * state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* strides, UInt const* offsets) override;
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;

    // ShaderCompiler implementation
    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override;
    
    protected:
    PROC loadProc(HMODULE module, char const* name);
    static DXGI_FORMAT mapFormat(Format format);

    float clearColor[4] = { 0, 0, 0, 0 };
    IDXGISwapChain* dxSwapChain = nullptr;
    ID3D12Device* dxDevice = nullptr;
};

Renderer* createD3D12Renderer()
{
    return new D3D12Renderer;
}

PROC D3D12Renderer::loadProc(HMODULE module, char const* name)
{
    PROC proc = ::GetProcAddress(module, name);
    if (!proc)
    {
        fprintf(stderr, "error: failed load symbol '%s'\n", name);
        return nullptr;
    }
    return proc;
}

/* static */DXGI_FORMAT D3D12Renderer::mapFormat(Format format)
{
    switch (format)
    {
        case Format::RGB_Float32:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case Format::RG_Float32:
            return DXGI_FORMAT_R32G32_FLOAT;
        default:
            return DXGI_FORMAT_UNKNOWN;
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult D3D12Renderer::initialize(void* inWindowHandle)
{
    auto windowHandle = (HWND)inWindowHandle;
    // Rather than statically link against D3D, we load it dynamically.

    HMODULE d3d12 = LoadLibraryA("d3d12.dll");
    if (!d3d12)
    {
        fprintf(stderr, "error: failed load 'd3d12.dll'\n");
        return SLANG_FAIL;
    }

#define LOAD_PROC(TYPE, NAME) \
        TYPE NAME##_ = (TYPE) loadProc(d3d12, #NAME); \
        if (NAME##_ == nullptr) return SLANG_FAIL;

    UINT dxgiFactoryFlags = 0;

#if ENABLE_DEBUG_LAYER
    LOAD_PROC(PFN_D3D12_GET_DEBUG_INTERFACE, D3D12GetDebugInterface);

    ID3D12Debug* debugController;
    if (SUCCEEDED(D3D12GetDebugInterface_(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    typedef HRESULT(WINAPI *PFN_DXGI_CREATE_FACTORY_2)(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory);

    LOAD_PROC(PFN_DXGI_CREATE_FACTORY_2, CreateDXGIFactory2);

    IDXGIFactory4* dxgiFactory;
    SLANG_RETURN_ON_FAIL(CreateDXGIFactory2_(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    // Search for an adapter that meets our requirements
    IDXGIAdapter* adapter = nullptr;

    LOAD_PROC(PFN_D3D12_CREATE_DEVICE, D3D12CreateDevice);

    UINT adapterCounter = 0;
    for (;;)
    {
        UINT adapterIndex = adapterCounter++;
        IDXGIAdapter1* candidateAdapter = nullptr;
        if (dxgiFactory->EnumAdapters1(adapterIndex, &candidateAdapter) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC1 desc;
        candidateAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // TODO: may want to allow software driver as fallback
        }
        else if (SUCCEEDED(D3D12CreateDevice_(candidateAdapter, featureLevel, IID_PPV_ARGS(&dxDevice))))
        {
            // We found one!
            adapter = candidateAdapter;
            break;
        }

        candidateAdapter->Release();
    }

    if (!adapter)
    {
        // Couldn't find an adapter
        return SLANG_FAIL;
    }

    // Command Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ID3D12CommandQueue* commandQueue;
    SLANG_RETURN_ON_FAIL(dxDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    // Swap Chain
    UINT frameCount = 2; // TODO: configure

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = frameCount;
    swapChainDesc.Width = gWindowWidth;
    swapChainDesc.Height = gWindowHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    IDXGISwapChain1* swapChain;
    SLANG_RETURN_ON_FAIL(dxgiFactory->CreateSwapChainForHwnd(commandQueue, windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain));

    // Is this needed?
    dxgiFactory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

    IDXGISwapChain3* swapChainEx;
    SLANG_RETURN_ON_FAIL(swapChain->QueryInterface(IID_PPV_ARGS(&swapChainEx)));

    UINT frameIndex = swapChainEx->GetCurrentBackBufferIndex();

    // Descriptor heaps

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    ID3D12DescriptorHeap* rtvHeap;
    SLANG_RETURN_ON_FAIL(dxDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    UINT rtvDescriptorSize = dxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Create per-frame RTVs
    ID3D12Resource* backBufferResources[2];
    for (UINT ff = 0; ff < frameCount; ++ff)
    {
        SLANG_RETURN_ON_FAIL(swapChainEx->GetBuffer(ff, IID_PPV_ARGS(&backBufferResources[ff])));
        dxDevice->CreateRenderTargetView(backBufferResources[ff], nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    ID3D12CommandAllocator* commandAllocator;
    SLANG_RETURN_ON_FAIL(dxDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
    return SLANG_OK;
}

void D3D12Renderer::setClearColor(float const* color)
{
    memcpy(clearColor, color, sizeof(clearColor));
}

void D3D12Renderer::clearFrame()
{
}

void D3D12Renderer::presentFrame()
{
}

SlangResult D3D12Renderer::captureScreenShot(char const* outputPath)
{
    return SLANG_FAIL;
}

ShaderCompiler* D3D12Renderer::getShaderCompiler() 
{
    return this;
}

Buffer* D3D12Renderer::createBuffer(BufferDesc const& desc)
{
    return nullptr;
}


InputLayout* D3D12Renderer::createInputLayout(InputElementDesc const* inputElements, UInt inputElementCount) 
{
    return nullptr;
}

void* D3D12Renderer::map(Buffer* buffer, MapFlavor flavor) 
{
    return nullptr;
}

void D3D12Renderer::unmap(Buffer* buffer)
{
}

void D3D12Renderer::setInputLayout(InputLayout* inputLayout) 
{
}

void D3D12Renderer::setPrimitiveTopology(PrimitiveTopology topology) 
{
}

void D3D12Renderer::setVertexBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* strides, UInt const* offsets)
{
}

void D3D12Renderer::setShaderProgram(ShaderProgram* inProgram)
{
}

void D3D12Renderer::setConstantBuffers(UInt startSlot, UInt slotCount, Buffer* const* buffers, UInt const* offsets)
{
}

void D3D12Renderer::draw(UInt vertexCount, UInt startVertex)
{
}


void D3D12Renderer::dispatchCompute(int x, int y, int z)
{
}

BindingState* D3D12Renderer::createBindingState(const ShaderInputLayout & layout)
{
    return nullptr;
}

void D3D12Renderer::setBindingState(BindingState * state)
{
}

void D3D12Renderer::serializeOutput(BindingState* state, const char * fileName)
{
}

// ShaderCompiler interface

ShaderProgram* D3D12Renderer::compileProgram(ShaderCompileRequest const& request)
{
    return nullptr;
}

} // renderer_test
