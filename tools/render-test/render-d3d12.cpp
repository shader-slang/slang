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

#include "../../source/core/slang-com-ptr.h"

// We will use the C standard library just for printing error messages.
#include <stdio.h>

#ifdef _MSC_VER
#include <stddef.h>
#if (_MSC_VER < 1900)
#define snprintf sprintf_s
#endif
#endif
//

#define ENABLE_DEBUG_LAYER 1

namespace renderer_test {
using namespace Slang;

// The Slang compiler currently generates HLSL source, so we'll need a utility
// routine (defined later) to translate that into D3D11 shader bytecode.
// Returns nullptr if compilation fails.
/* ID3DBlob* compileHLSLShader(
    char const* sourcePath,
    char const* source,
    char const* entryPointName,
    char const* dxProfileName); */

//static char const* vertexProfileName   = "vs_4_0";
//static char const* fragmentProfileName = "ps_4_0";

//
class D3D12Renderer : public Renderer, public ShaderCompiler
{
public:
    // Renderer    implementation
    virtual SlangResult initialize(void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual SlangResult captureScreenShot(const char* outputPath) override;
    virtual void serializeOutput(BindingState* state, const char* fileName) override;
    virtual Buffer* createBuffer(const BufferDesc& desc) override;
    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) override;
    virtual BindingState * createBindingState(const ShaderInputLayout& layout) override;
    virtual ShaderCompiler* getShaderCompiler() override;
    virtual void* map(Buffer* buffer, MapFlavor flavor) override;
    virtual void unmap(Buffer* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState* state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* offsets) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;

    // ShaderCompiler implementation
    virtual ShaderProgram* compileProgram(const ShaderCompileRequest& request) override;
    
    protected:
    PROC loadProc(HMODULE module, char const* name);
    static DXGI_FORMAT mapFormat(Format format);

    float m_clearColor[4] = { 0, 0, 0, 0 };

    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

    ComPtr<ID3D12Resource> m_backBufferResources[2];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
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

    HMODULE d3dModule = LoadLibraryA("d3d12.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d12.dll'\n");
        return SLANG_FAIL;
    }

#define LOAD_PROC(TYPE, NAME) \
        TYPE NAME##_ = (TYPE) loadProc(d3dModule, #NAME); \
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
    ComPtr<IDXGIAdapter> adapter;
    
    LOAD_PROC(PFN_D3D12_CREATE_DEVICE, D3D12CreateDevice);

    UINT adapterCounter = 0;
    for (;;)
    {
        UINT adapterIndex = adapterCounter++;

        ComPtr<IDXGIAdapter1> candidateAdapter;
        if (dxgiFactory->EnumAdapters1(adapterIndex, candidateAdapter.writeRef()) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC1 desc;
        candidateAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // TODO: may want to allow software driver as fallback
        }
        else if (SUCCEEDED(D3D12CreateDevice_(candidateAdapter, featureLevel, IID_PPV_ARGS(m_device.writeRef()))))
        {
            // We found one!
            adapter = candidateAdapter;
            break;
        }
    }

    if (!adapter)
    {
        // Couldn't find an adapter
        return SLANG_FAIL;
    }

    // Command Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    SLANG_RETURN_ON_FAIL(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.writeRef())));

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

    SLANG_RETURN_ON_FAIL(dxgiFactory->CreateSwapChainForHwnd(m_commandQueue, windowHandle, &swapChainDesc, nullptr, nullptr, m_swapChain.writeRef()));

    // Is this needed?
    dxgiFactory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

    ComPtr<IDXGISwapChain3> swapChainEx;
    SLANG_RETURN_ON_FAIL(m_swapChain->QueryInterface(IID_PPV_ARGS(swapChainEx.writeRef())));

    UINT frameIndex = swapChainEx->GetCurrentBackBufferIndex();

    // Descriptor heaps

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    SLANG_RETURN_ON_FAIL(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.writeRef())));

    UINT rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Create per-frame RTVs
    ComPtr<ID3D12Resource> backBufferResources[2];
    for (UINT ff = 0; ff < frameCount; ++ff)
    {
        SLANG_RETURN_ON_FAIL(swapChainEx->GetBuffer(ff, IID_PPV_ARGS(m_backBufferResources[ff].writeRef())));
        m_device->CreateRenderTargetView(backBufferResources[ff], nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    SLANG_RETURN_ON_FAIL(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.writeRef())));
    return SLANG_OK;
}

void D3D12Renderer::setClearColor(const float color[4])
{
    memcpy(m_clearColor, color, sizeof(m_clearColor));
}

void D3D12Renderer::clearFrame()
{
}

void D3D12Renderer::presentFrame()
{
}

SlangResult D3D12Renderer::captureScreenShot(const char* outputPath)
{
    return SLANG_FAIL;
}

ShaderCompiler* D3D12Renderer::getShaderCompiler() 
{
    return this;
}

Buffer* D3D12Renderer::createBuffer(const BufferDesc& desc)
{
    return nullptr;
}


InputLayout* D3D12Renderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) 
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

void D3D12Renderer::setVertexBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt * strides, const UInt* offsets)
{
}

void D3D12Renderer::setShaderProgram(ShaderProgram* inProgram)
{
}

void D3D12Renderer::setConstantBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* offsets)
{
}

void D3D12Renderer::draw(UInt vertexCount, UInt startVertex)
{
}


void D3D12Renderer::dispatchCompute(int x, int y, int z)
{
}

BindingState* D3D12Renderer::createBindingState(const ShaderInputLayout& layout)
{
    return nullptr;
}

void D3D12Renderer::setBindingState(BindingState* state)
{
}

void D3D12Renderer::serializeOutput(BindingState* state, const char* fileName)
{
}

// ShaderCompiler interface

ShaderProgram* D3D12Renderer::compileProgram(const ShaderCompileRequest& request)
{
    return nullptr;
}

} // renderer_test
