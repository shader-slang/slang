// render-d3d12.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "render-d3d12.h"

//WORKING:#include "options.h"
#include "render.h"

#include "surface.h"

// In order to use the Slang API, we need to include its header

//WORKING:#include <slang.h>

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

#include "../../slang-com-ptr.h"

#include "resource-d3d12.h"
#include "descriptor-heap-d3d12.h"
#include "circular-resource-heap-d3d12.h"

#include "d3d-util.h"

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

namespace slang_graphics {
using namespace Slang;

class D3D12Renderer : public Renderer
{
public:
    // Renderer    implementation
    virtual SlangResult initialize(const Desc& desc, void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual TextureResource* createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData) override;
    virtual BufferResource* createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& bufferDesc, const void* initData) override;
    virtual SlangResult captureScreenSurface(Surface& surfaceOut) override;
    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) override;
    virtual BindingState* createBindingState(const BindingState::Desc& bindingStateDesc) override;
    virtual ShaderProgram* createProgram(const ShaderProgram::Desc& desc) override;
    virtual void* map(BufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(BufferResource* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState* state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override;
    virtual void waitForGpu() override;
    virtual RendererType getRendererType() const override { return RendererType::DirectX12; }

    ~D3D12Renderer();

protected:
    static const Int kMaxNumRenderFrames = 4;
    static const Int kMaxNumRenderTargets = 3;

    struct Submitter
    {
        virtual void setRootConstantBufferView(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) = 0;
        virtual void setRootDescriptorTable(int index, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
        virtual void setRootSignature(ID3D12RootSignature* rootSignature) = 0;
    };

    struct FrameInfo
    {
        FrameInfo() :m_fenceValue(0) {}
        void reset()
        {
            m_commandAllocator.setNull();
        }
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;            ///< The command allocator for this frame
        UINT64 m_fenceValue;                                        ///< The fence value when rendering this Frame is complete
    };

    class ShaderProgramImpl: public ShaderProgram
    {
        public:
        PipelineType m_pipelineType;
        List<uint8_t> m_vertexShader;
        List<uint8_t> m_pixelShader;
        List<uint8_t> m_computeShader;
    };

    class BufferResourceImpl: public BufferResource
    {
        public:
        typedef BufferResource Parent;

        enum class BackingStyle
        {
            Unknown,
            ResourceBacked,             ///< The contents is only held within the resource
            MemoryBacked,               ///< The current contents is held in m_memory and copied to GPU every time it's used (typically used for constant buffers)
        };

        void bindConstantBufferView(D3D12CircularResourceHeap& circularHeap, int index, Submitter* submitter) const
        {
            switch (m_backingStyle)
            {
                case BackingStyle::MemoryBacked:
                {
                    const size_t bufferSize = m_memory.Count();
                    D3D12CircularResourceHeap::Cursor cursor = circularHeap.allocateConstantBuffer(bufferSize);
                    ::memcpy(cursor.m_position, m_memory.Buffer(), bufferSize);
                    // Set the constant buffer
                    submitter->setRootConstantBufferView(index, circularHeap.getGpuHandle(cursor));
                    break;
                }
                case BackingStyle::ResourceBacked:
                {
                    // Set the constant buffer
                    submitter->setRootConstantBufferView(index, m_resource.getResource()->GetGPUVirtualAddress());
                    break;
                }
                default: break;
            }
        }

        BufferResourceImpl(Resource::Usage initialUsage, const Desc& desc):
            Parent(desc),
            m_mapFlavor(MapFlavor::HostRead),
            m_initialUsage(initialUsage)
        {
        }

        static BackingStyle _calcResourceBackingStyle(Usage usage)
        {
            switch (usage)
            {
                case Usage::ConstantBuffer:     return BackingStyle::MemoryBacked;
                default:                        return BackingStyle::ResourceBacked;
            }
        }

        BackingStyle m_backingStyle;        ///< How the resource is 'backed' - either as a resource or cpu memory. Cpu memory is typically used for constant buffers.
        D3D12Resource m_resource;           ///< The resource typically in gpu memory
        D3D12Resource m_uploadResource;     ///< If the resource can be written to, and is in gpu memory (ie not Memory backed), will have upload resource

        Usage m_initialUsage;

        List<uint8_t> m_memory;             ///< Cpu memory buffer, used if the m_backingStyle is MemoryBacked
        MapFlavor m_mapFlavor;              ///< If the resource is mapped holds the current mapping flavor
    };

    class TextureResourceImpl: public TextureResource
    {
        public:
        typedef TextureResource Parent;

        TextureResourceImpl(const Desc& desc):
            Parent(desc)
        {
        }

        D3D12Resource m_resource;
    };

    class InputLayoutImpl: public InputLayout
    {
        public:
        List<D3D12_INPUT_ELEMENT_DESC> m_elements;
        List<char> m_text;                              ///< Holds all strings to keep in scope
    };

    struct BindingDetail
    {
        int m_srvIndex = -1;
        int m_uavIndex = -1;
        int m_samplerIndex = -1;
    };

    class BindingStateImpl: public BindingState
    {
        public:
        typedef BindingState Parent;

        Result init(ID3D12Device* device)
        {
            // Set up descriptor heaps
            SLANG_RETURN_ON_FAIL(m_viewHeap.init(device, 256, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
            SLANG_RETURN_ON_FAIL(m_samplerHeap.init(device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
            return SLANG_OK;
        }

            /// Ctor
        BindingStateImpl(const Desc& desc) :
            Parent(desc)
        {}

        List<BindingDetail> m_bindingDetails;       ///< These match 1-1 to the bindings in the m_desc

        D3D12DescriptorHeap m_viewHeap;            ///< Cbv, Srv, Uav
        D3D12DescriptorHeap m_samplerHeap;        ///< Heap for samplers
    };

    class RenderState: public RefObject
    {
        public:
        D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveTopologyType;
        RefPtr<BindingStateImpl> m_bindingState;
        RefPtr<InputLayoutImpl> m_inputLayout;
        RefPtr<ShaderProgramImpl> m_shaderProgram;

        ComPtr<ID3D12RootSignature> m_rootSignature;
        ComPtr<ID3D12PipelineState> m_pipelineState;
    };

    struct BoundVertexBuffer
    {
        RefPtr<BufferResourceImpl> m_buffer;
        int m_stride;
        int m_offset;
    };

    struct BindParameters
    {
        enum
        {
            kMaxRanges = 16,
            kMaxParameters = 32
        };

        D3D12_DESCRIPTOR_RANGE& nextRange() { return m_ranges[m_rangeIndex++]; }
        D3D12_ROOT_PARAMETER& nextParameter() { return m_parameters[m_paramIndex++]; }

        BindParameters():
            m_rangeIndex(0),
            m_paramIndex(0)
        {}

        D3D12_DESCRIPTOR_RANGE m_ranges[kMaxRanges];
        int m_rangeIndex;
        D3D12_ROOT_PARAMETER m_parameters[kMaxParameters];
        int m_paramIndex;
    };

    struct GraphicsSubmitter : public Submitter
    {
        virtual void setRootConstantBufferView(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
        {
            m_commandList->SetGraphicsRootConstantBufferView(index, gpuBufferLocation);
        }
        virtual void setRootDescriptorTable(int index, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor) override
        {
            m_commandList->SetGraphicsRootDescriptorTable(index, baseDescriptor);
        }
        void setRootSignature(ID3D12RootSignature* rootSignature)
        {
            m_commandList->SetGraphicsRootSignature(rootSignature);
        }

        GraphicsSubmitter(ID3D12GraphicsCommandList* commandList):
            m_commandList(commandList)
        {
        }

        ID3D12GraphicsCommandList* m_commandList;
    };

    struct ComputeSubmitter : public Submitter
    {
        virtual void setRootConstantBufferView(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
        {
            m_commandList->SetComputeRootConstantBufferView(index, gpuBufferLocation);
        }
        virtual void setRootDescriptorTable(int index, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor) override
        {
            m_commandList->SetComputeRootDescriptorTable(index, baseDescriptor);
        }
        void setRootSignature(ID3D12RootSignature* rootSignature)
        {
            m_commandList->SetComputeRootSignature(rootSignature);
        }

        ComputeSubmitter(ID3D12GraphicsCommandList* commandList) :
            m_commandList(commandList)
        {
        }

        ID3D12GraphicsCommandList* m_commandList;
    };

    static PROC loadProc(HMODULE module, char const* name);
    Result createFrameResources();
        /// Blocks until gpu has completed all work
    void releaseFrameResources();

    Result createBuffer(const D3D12_RESOURCE_DESC& resourceDesc, const void* srcData, D3D12Resource& uploadResource, D3D12_RESOURCE_STATES finalState, D3D12Resource& resourceOut);

    void beginRender();

    void endRender();

    void submitGpuWorkAndWait();
    void _resetCommandList();

    Result captureTextureToSurface(D3D12Resource& resource, Surface& surfaceOut);

    FrameInfo& getFrame() { return m_frameInfos[m_frameIndex]; }
    const FrameInfo& getFrame() const { return m_frameInfos[m_frameIndex]; }

    ID3D12GraphicsCommandList* getCommandList() const { return m_commandList; }

    RenderState* calcRenderState();
        /// From current bindings calculate the root signature and pipeline state
    Result calcGraphicsPipelineState(ComPtr<ID3D12RootSignature>& sigOut, ComPtr<ID3D12PipelineState>& pipelineStateOut);
    Result calcComputePipelineState(ComPtr<ID3D12RootSignature>& signatureOut, ComPtr<ID3D12PipelineState>& pipelineStateOut);

    Result _bindRenderState(RenderState* renderState, ID3D12GraphicsCommandList* commandList, Submitter* submitter);

    Result _calcBindParameters(BindParameters& params);
    RenderState* findRenderState(PipelineType pipelineType);

    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE m_D3D12SerializeRootSignature = nullptr;

    D3D12CircularResourceHeap m_circularResourceHeap;

    int m_commandListOpenCount = 0;            ///< If >0 the command list should be open

    List<BoundVertexBuffer> m_boundVertexBuffers;

    RefPtr<ShaderProgramImpl> m_boundShaderProgram;
    RefPtr<InputLayoutImpl> m_boundInputLayout;
    RefPtr<BindingStateImpl> m_boundBindingState;

    DXGI_FORMAT m_targetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    bool m_hasVsync = true;
    bool m_isFullSpeed = false;
    bool m_allowFullScreen = false;
    bool m_isMultiSampled = false;
    int m_numTargetSamples = 1;                                ///< The number of multi sample samples
    int m_targetSampleQuality = 0;                            ///< The multi sample quality

    Desc m_desc;

    bool m_isInitialized = false;

    D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    float m_clearColor[4] = { 0, 0, 0, 0 };

    D3D12_VIEWPORT m_viewport = {};

    ComPtr<ID3D12Debug> m_dxDebug;

    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    D3D12_RECT m_scissorRect = {};

    List<RefPtr<RenderState> > m_renderStates;                ///< Holds list of all render state combinations
    RenderState* m_currentRenderState = nullptr;            ///< The current combination

    UINT m_rtvDescriptorSize = 0;

    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    UINT m_dsvDescriptorSize = 0;

    // Synchronization objects.
    D3D12CounterFence m_fence;

    HANDLE m_swapChainWaitableObject;

    // Frame specific data
    int m_numRenderFrames = 0;
    UINT m_frameIndex = 0;
    FrameInfo m_frameInfos[kMaxNumRenderFrames];

    int m_numRenderTargets = 2;
    int m_renderTargetIndex = 0;

    D3D12Resource* m_backBuffers[kMaxNumRenderTargets];
    D3D12Resource* m_renderTargets[kMaxNumRenderTargets];

    D3D12Resource m_backBufferResources[kMaxNumRenderTargets];
    D3D12Resource m_renderTargetResources[kMaxNumRenderTargets];

    D3D12Resource m_depthStencil;
    D3D12_CPU_DESCRIPTOR_HANDLE m_depthStencilView = {};

    int32_t m_depthStencilUsageFlags = 0;    ///< D3DUtil::UsageFlag combination for depth stencil
    int32_t m_targetUsageFlags = 0;            ///< D3DUtil::UsageFlag combination for target

    HWND m_hwnd = nullptr;
};

Renderer* createD3D12Renderer()
{
    return new D3D12Renderer;
}

/* static */PROC D3D12Renderer::loadProc(HMODULE module, char const* name)
{
    PROC proc = ::GetProcAddress(module, name);
    if (!proc)
    {
        fprintf(stderr, "error: failed load symbol '%s'\n", name);
        return nullptr;
    }
    return proc;
}

void D3D12Renderer::releaseFrameResources()
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb174577%28v=vs.85%29.aspx

    // Release the resources holding references to the swap chain (requirement of
    // IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
    // current fence value.
    for (int i = 0; i < m_numRenderFrames; i++)
    {
        FrameInfo& info = m_frameInfos[i];
        info.reset();
        info.m_fenceValue = m_fence.getCurrentValue();
    }
    for (int i = 0; i < m_numRenderTargets; i++)
    {
        m_backBuffers[i]->setResourceNull();
        m_renderTargets[i]->setResourceNull();
    }
}

void D3D12Renderer::waitForGpu()
{
    m_fence.nextSignalAndWait(m_commandQueue);
}

D3D12Renderer::~D3D12Renderer()
{
    if (m_isInitialized)
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        waitForGpu();
    }
}

static void _initSrvDesc(Resource::Type resourceType, const TextureResource::Desc& textureDesc, const D3D12_RESOURCE_DESC& desc, DXGI_FORMAT pixelFormat, D3D12_SHADER_RESOURCE_VIEW_DESC& descOut)
{
    // create SRV
    descOut = D3D12_SHADER_RESOURCE_VIEW_DESC();

    descOut.Format = (pixelFormat == DXGI_FORMAT_UNKNOWN) ? D3DUtil::calcFormat(D3DUtil::USAGE_SRV, desc.Format) : pixelFormat;
    descOut.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    if (desc.DepthOrArraySize == 1)
    {
        switch (desc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:  descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D; break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:  descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:  descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D; break;
            default: assert(!"Unknown dimension");
        }

        descOut.Texture2D.MipLevels = desc.MipLevels;
        descOut.Texture2D.MostDetailedMip = 0;
        descOut.Texture2D.PlaneSlice = 0;
        descOut.Texture2D.ResourceMinLODClamp = 0.0f;
    }
    else if (resourceType == Resource::Type::TextureCube)
    {
        if (textureDesc.arraySize > 1)
        {
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;

            descOut.TextureCubeArray.NumCubes = textureDesc.arraySize;
            descOut.TextureCubeArray.First2DArrayFace = 0;
            descOut.TextureCubeArray.MipLevels = desc.MipLevels;
            descOut.TextureCubeArray.MostDetailedMip = 0;
            descOut.TextureCubeArray.ResourceMinLODClamp = 0;
        }
        else
        {
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

            descOut.TextureCube.MipLevels = desc.MipLevels;
            descOut.TextureCube.MostDetailedMip = 0;
            descOut.TextureCube.ResourceMinLODClamp = 0;
        }
    }
    else
    {
        assert(desc.DepthOrArraySize > 1);

        switch (desc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:  descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY; break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:  descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY; break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:  descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D; break;

            default: assert(!"Unknown dimension");
        }

        descOut.Texture2DArray.ArraySize = desc.DepthOrArraySize;
        descOut.Texture2DArray.MostDetailedMip = 0;
        descOut.Texture2DArray.MipLevels = desc.MipLevels;
        descOut.Texture2DArray.FirstArraySlice = 0;
        descOut.Texture2DArray.PlaneSlice = 0;
        descOut.Texture2DArray.ResourceMinLODClamp = 0;
    }
}

static void _initBufferResourceDesc(size_t bufferSize, D3D12_RESOURCE_DESC& out)
{
    out = {};

    out.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    out.Alignment = 0;
    out.Width = bufferSize;
    out.Height = 1;
    out.DepthOrArraySize = 1;
    out.MipLevels = 1;
    out.Format = DXGI_FORMAT_UNKNOWN;
    out.SampleDesc.Count = 1;
    out.SampleDesc.Quality = 0;
    out.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    out.Flags = D3D12_RESOURCE_FLAG_NONE;
}

Result D3D12Renderer::createBuffer(const D3D12_RESOURCE_DESC& resourceDesc, const void* srcData, D3D12Resource& uploadResource, D3D12_RESOURCE_STATES finalState, D3D12Resource& resourceOut)
{
   const  size_t bufferSize = size_t(resourceDesc.Width);

    {
        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        const D3D12_RESOURCE_STATES initialState = srcData ? D3D12_RESOURCE_STATE_COPY_DEST : finalState;

        SLANG_RETURN_ON_FAIL(resourceOut.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, initialState, nullptr));
    }

    {
        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC uploadResourceDesc(resourceDesc);
        uploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        SLANG_RETURN_ON_FAIL(uploadResource.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, uploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr));
    }

    if (srcData)
    {
        // Copy data to the intermediate upload heap and then schedule a copy
        // from the upload heap to the vertex buffer.
        UINT8* dstData;
        D3D12_RANGE readRange = {};         // We do not intend to read from this resource on the CPU.

        ID3D12Resource* dxUploadResource = uploadResource.getResource();

        SLANG_RETURN_ON_FAIL(dxUploadResource->Map(0, &readRange, reinterpret_cast<void**>(&dstData)));
        ::memcpy(dstData, srcData, bufferSize);
        dxUploadResource->Unmap(0, nullptr);

        m_commandList->CopyBufferRegion(resourceOut, 0, uploadResource, 0, bufferSize);

        // Make sure it's in the right state
        {
            D3D12BarrierSubmitter submitter(m_commandList);
            resourceOut.transition(finalState, submitter);
        }

        submitGpuWorkAndWait();
    }

    return SLANG_OK;
}

void D3D12Renderer::_resetCommandList()
{
    const FrameInfo& frame = getFrame();

    ID3D12GraphicsCommandList* commandList = getCommandList();
    commandList->Reset(frame.m_commandAllocator, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_renderTargetIndex * m_rtvDescriptorSize };
    if (m_depthStencil)
    {
        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &m_depthStencilView);
    }
    else
    {
        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    }

    // Set necessary state.
    commandList->RSSetViewports(1, &m_viewport);
    commandList->RSSetScissorRects(1, &m_scissorRect);
}

void D3D12Renderer::beginRender()
{
    // Should currently not be open!
    assert(m_commandListOpenCount == 0);

    m_circularResourceHeap.updateCompleted();

    getFrame().m_commandAllocator->Reset();

    _resetCommandList();

    // Indicate that the render target needs to be writable
    {
        D3D12BarrierSubmitter submitter(m_commandList);
        m_renderTargets[m_renderTargetIndex]->transition(D3D12_RESOURCE_STATE_RENDER_TARGET, submitter);
    }

    m_commandListOpenCount = 1;
}

void D3D12Renderer::endRender()
{
    assert(m_commandListOpenCount == 1);

    {
        const UInt64 signalValue = m_fence.nextSignal(m_commandQueue);
        m_circularResourceHeap.addSync(signalValue);
    }

    D3D12Resource& backBuffer = *m_backBuffers[m_renderTargetIndex];
    if (m_isMultiSampled)
    {
        // MSAA resolve
        D3D12Resource& renderTarget = *m_renderTargets[m_renderTargetIndex];
        assert(&renderTarget != &backBuffer);
        // Barriers to wait for the render target, and the backbuffer to be in correct state
        {
            D3D12BarrierSubmitter submitter(m_commandList);
            renderTarget.transition(D3D12_RESOURCE_STATE_RESOLVE_SOURCE, submitter);
            backBuffer.transition(D3D12_RESOURCE_STATE_RESOLVE_DEST, submitter);
        }

        // Do the resolve...
        m_commandList->ResolveSubresource(backBuffer, 0, renderTarget, 0, m_targetFormat);
    }

    // Make the back buffer presentable
    {
        D3D12BarrierSubmitter submitter(m_commandList);
        backBuffer.transition(D3D12_RESOURCE_STATE_PRESENT, submitter);
    }

    SLANG_ASSERT_VOID_ON_FAIL(m_commandList->Close());

    {
        // Execute the command list.
        ID3D12CommandList* commandLists[] = { m_commandList };
        m_commandQueue->ExecuteCommandLists(SLANG_COUNT_OF(commandLists), commandLists);
    }

    assert(m_commandListOpenCount == 1);
    // Must be 0
    m_commandListOpenCount = 0;
}

void D3D12Renderer::submitGpuWork()
{
    assert(m_commandListOpenCount);
    ID3D12GraphicsCommandList* commandList = getCommandList();

    SLANG_ASSERT_VOID_ON_FAIL(commandList->Close());
    {
        // Execute the command list.
        ID3D12CommandList* commandLists[] = { commandList };
        m_commandQueue->ExecuteCommandLists(SLANG_COUNT_OF(commandLists), commandLists);
    }

    // Reset the render target
    _resetCommandList();
}

void D3D12Renderer::submitGpuWorkAndWait()
{
    submitGpuWork();
    waitForGpu();
}

Result D3D12Renderer::captureTextureToSurface(D3D12Resource& resource, Surface& surfaceOut)
{
    const D3D12_RESOURCE_STATES initialState = resource.getState();

    const D3D12_RESOURCE_DESC desc = resource.getResource()->GetDesc();

    // Don't bother supporting MSAA for right now
    if (desc.SampleDesc.Count > 1)
    {
        fprintf(stderr, "ERROR: cannot capture multi-sample texture\n");
        return SLANG_FAIL;
    }

    size_t bytesPerPixel = sizeof(uint32_t);
    size_t rowPitch = int(desc.Width) * bytesPerPixel;
    size_t bufferSize = rowPitch * int(desc.Height);

    D3D12Resource stagingResource;
    {
        D3D12_RESOURCE_DESC stagingDesc;
        _initBufferResourceDesc(bufferSize, stagingDesc);

        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        SLANG_RETURN_ON_FAIL(stagingResource.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, stagingDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr));
    }

    {
        D3D12BarrierSubmitter submitter(m_commandList);
        resource.transition(D3D12_RESOURCE_STATE_COPY_SOURCE, submitter);
    }

    // Do the copy
    {
        D3D12_TEXTURE_COPY_LOCATION srcLoc;
        srcLoc.pResource = resource;
        srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLoc.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION dstLoc;
        dstLoc.pResource = stagingResource;
        dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dstLoc.PlacedFootprint.Offset = 0;
        dstLoc.PlacedFootprint.Footprint.Format = desc.Format;
        dstLoc.PlacedFootprint.Footprint.Width = UINT(desc.Width);
        dstLoc.PlacedFootprint.Footprint.Height = UINT(desc.Height);
        dstLoc.PlacedFootprint.Footprint.Depth = 1;
        dstLoc.PlacedFootprint.Footprint.RowPitch = UINT(rowPitch);

        m_commandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
    }

    {
        D3D12BarrierSubmitter submitter(m_commandList);
        resource.transition(initialState, submitter);
    }

    // Submit the copy, and wait for copy to complete
    submitGpuWorkAndWait();

    {
        ID3D12Resource* dxResource = stagingResource;

        UINT8* data;
        D3D12_RANGE readRange = {0, bufferSize};

        SLANG_RETURN_ON_FAIL(dxResource->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        Result res = surfaceOut.set(int(desc.Width), int(desc.Height), Format::RGBA_Unorm_UInt8, int(rowPitch), data, SurfaceAllocator::getMallocAllocator());

        dxResource->Unmap(0, nullptr);
        return res;
    }
}

Result D3D12Renderer::calcComputePipelineState(ComPtr<ID3D12RootSignature>& signatureOut, ComPtr<ID3D12PipelineState>& pipelineStateOut)
{
    BindParameters bindParameters;
    _calcBindParameters(bindParameters);

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;

    {
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.NumParameters = bindParameters.m_paramIndex;
        rootSignatureDesc.pParameters = bindParameters.m_parameters;
        rootSignatureDesc.NumStaticSamplers = 0;
        rootSignatureDesc.pStaticSamplers = nullptr;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        SLANG_RETURN_ON_FAIL(m_D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef()));
        SLANG_RETURN_ON_FAIL(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature.writeRef())));
    }

    {
        // Describe and create the compute pipeline state object
        D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
        computeDesc.pRootSignature = rootSignature;
        computeDesc.CS = { m_boundShaderProgram->m_computeShader.Buffer(), m_boundShaderProgram->m_computeShader.Count() };
        SLANG_RETURN_ON_FAIL(m_device->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(pipelineState.writeRef())));
    }

    signatureOut.swap(rootSignature);
    pipelineStateOut.swap(pipelineState);

    return SLANG_OK;
}

Result D3D12Renderer::calcGraphicsPipelineState(ComPtr<ID3D12RootSignature>& signatureOut, ComPtr<ID3D12PipelineState>& pipelineStateOut)
{
    BindParameters bindParameters;
    _calcBindParameters(bindParameters);

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;

    {
        // Deny unnecessary access to certain pipeline stages
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.NumParameters = bindParameters.m_paramIndex;
        rootSignatureDesc.pParameters = bindParameters.m_parameters;
        rootSignatureDesc.NumStaticSamplers = 0;
        rootSignatureDesc.pStaticSamplers = nullptr;
        rootSignatureDesc.Flags = m_boundInputLayout ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE;

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        SLANG_RETURN_ON_FAIL(m_D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef()));
        SLANG_RETURN_ON_FAIL(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature.writeRef())));
    }

    {
        // Describe and create the graphics pipeline state object (PSO)
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

        psoDesc.pRootSignature = rootSignature;

        psoDesc.VS = { m_boundShaderProgram->m_vertexShader.Buffer(), m_boundShaderProgram->m_vertexShader.Count() };
        psoDesc.PS = { m_boundShaderProgram->m_pixelShader.Buffer(), m_boundShaderProgram->m_pixelShader.Count() };

        {
            psoDesc.InputLayout = { m_boundInputLayout->m_elements.Buffer(), UINT(m_boundInputLayout->m_elements.Count()) };
            psoDesc.PrimitiveTopologyType = m_primitiveTopologyType;

            {
                const int numRenderTargets = m_boundBindingState ? m_boundBindingState->getDesc().m_numRenderTargets : 1;

                psoDesc.DSVFormat = m_depthStencilFormat;
                psoDesc.NumRenderTargets = numRenderTargets;
                for (Int i = 0; i < numRenderTargets; i++)
                {
                    psoDesc.RTVFormats[i] = m_targetFormat;
                }

                psoDesc.SampleDesc.Count = 1;
                psoDesc.SampleDesc.Quality = 0;

                psoDesc.SampleMask = UINT_MAX;
            }

            {
                auto& rs = psoDesc.RasterizerState;
                rs.FillMode = D3D12_FILL_MODE_SOLID;
                rs.CullMode = D3D12_CULL_MODE_NONE;
                rs.FrontCounterClockwise = FALSE;
                rs.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
                rs.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
                rs.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
                rs.DepthClipEnable = TRUE;
                rs.MultisampleEnable = FALSE;
                rs.AntialiasedLineEnable = FALSE;
                rs.ForcedSampleCount = 0;
                rs.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            }

            {
                D3D12_BLEND_DESC& blend = psoDesc.BlendState;

                blend.AlphaToCoverageEnable = FALSE;
                blend.IndependentBlendEnable = FALSE;
                const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
                {
                    FALSE,FALSE,
                    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                    D3D12_LOGIC_OP_NOOP,
                    D3D12_COLOR_WRITE_ENABLE_ALL,
                };
                for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                {
                    blend.RenderTarget[i] = defaultRenderTargetBlendDesc;
                }
            }

            {
                auto& ds = psoDesc.DepthStencilState;

                ds.DepthEnable = FALSE;
                ds.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
                ds.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                //ds.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
                ds.StencilEnable = FALSE;
                ds.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
                ds.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
                const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
                {
                    D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS
                };
                ds.FrontFace = defaultStencilOp;
                ds.BackFace = defaultStencilOp;
            }
        }

        psoDesc.PrimitiveTopologyType = m_primitiveTopologyType;

        SLANG_RETURN_ON_FAIL(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.writeRef())));
    }

    signatureOut.swap(rootSignature);
    pipelineStateOut.swap(pipelineState);

    return SLANG_OK;
}

D3D12Renderer::RenderState* D3D12Renderer::findRenderState(PipelineType pipelineType)
{
    switch (pipelineType)
    {
        case PipelineType::Compute:
        {
            // Check if current state is a match
            if (m_currentRenderState)
            {
                if (m_currentRenderState->m_bindingState == m_boundBindingState &&
                    m_currentRenderState->m_shaderProgram == m_boundShaderProgram)
                {
                    return m_currentRenderState;
                }
            }

            const int num = int(m_renderStates.Count());
            for (int i = 0; i < num; i++)
            {
                RenderState* renderState = m_renderStates[i];
                if (renderState->m_bindingState == m_boundBindingState &&
                    renderState->m_shaderProgram == m_boundShaderProgram)
                {
                    return renderState;
                }
            }
            break;
        }
        case PipelineType::Graphics:
        {
            if (m_currentRenderState)
            {
                if (m_currentRenderState->m_bindingState == m_boundBindingState &&
                    m_currentRenderState->m_inputLayout == m_boundInputLayout &&
                    m_currentRenderState->m_shaderProgram == m_boundShaderProgram &&
                    m_currentRenderState->m_primitiveTopologyType == m_primitiveTopologyType)
                {
                    return m_currentRenderState;
                }
            }
            // See if matches one in the list
            {
                const int num = int(m_renderStates.Count());
                for (int i = 0; i < num; i++)
                {
                    RenderState* renderState = m_renderStates[i];
                    if (renderState->m_bindingState == m_boundBindingState &&
                        renderState->m_inputLayout == m_boundInputLayout &&
                        renderState->m_shaderProgram == m_boundShaderProgram &&
                        renderState->m_primitiveTopologyType == m_primitiveTopologyType)
                    {
                        // Okay we have a match
                        return renderState;
                    }
                }
            }
            break;
        }
        default: break;
    }
    return nullptr;
}

D3D12Renderer::RenderState* D3D12Renderer::calcRenderState()
{
    if (!m_boundShaderProgram)
    {
        return nullptr;
    }
    m_currentRenderState = findRenderState(m_boundShaderProgram->m_pipelineType);
    if (m_currentRenderState)
    {
        return m_currentRenderState;
    }

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;

    switch (m_boundShaderProgram->m_pipelineType)
    {
        case PipelineType::Compute:
        {
            if (SLANG_FAILED(calcComputePipelineState(rootSignature, pipelineState)))
            {
                return nullptr;
            }
            break;
        }
        case PipelineType::Graphics:
        {
            if (SLANG_FAILED(calcGraphicsPipelineState(rootSignature, pipelineState)))
            {
                return nullptr;
            }
            break;
        }
        default: return nullptr;
    }

    RenderState* renderState = new RenderState;

    renderState->m_primitiveTopologyType = m_primitiveTopologyType;
    renderState->m_bindingState = m_boundBindingState;
    renderState->m_inputLayout = m_boundInputLayout;
    renderState->m_shaderProgram = m_boundShaderProgram;

    renderState->m_rootSignature.swap(rootSignature);
    renderState->m_pipelineState.swap(pipelineState);

    m_renderStates.Add(renderState);

    m_currentRenderState = renderState;

    return renderState;
}

Result D3D12Renderer::_calcBindParameters(BindParameters& params)
{
    int numConstantBuffers = 0;
    {
        if (m_boundBindingState)
        {
            const int numBoundConstantBuffers = numConstantBuffers;

            const BindingState::Desc& bindingStateDesc = m_boundBindingState->getDesc();

            const auto& bindings = bindingStateDesc.m_bindings;
            const auto& details = m_boundBindingState->m_bindingDetails;

            const int numBindings = int(bindings.Count());

            for (int i = 0; i < numBindings; i++)
            {
                const auto& binding = bindings[i];
                const auto& detail = details[i];

                const int bindingIndex = binding.registerRange.getSingleIndex();

                if (binding.bindingType == BindingType::Buffer)
                {
                    assert(binding.resource && binding.resource->isBuffer());
                    if (binding.resource->canBind(Resource::BindFlag::ConstantBuffer))
                    {
                        // Make sure it's not overlapping the ones we just statically defined
                        //assert(binding.m_binding < numBoundConstantBuffers);

                        D3D12_ROOT_PARAMETER& param = params.nextParameter();
                        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                        D3D12_ROOT_DESCRIPTOR& descriptor = param.Descriptor;
                        descriptor.ShaderRegister = bindingIndex;
                        descriptor.RegisterSpace = 0;

                        numConstantBuffers++;
                    }
                }

                if (detail.m_srvIndex >= 0)
                {
                    D3D12_DESCRIPTOR_RANGE& range = params.nextRange();

                    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    range.NumDescriptors = 1;
                    range.BaseShaderRegister = bindingIndex;
                    range.RegisterSpace = 0;
                    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                    D3D12_ROOT_PARAMETER& param = params.nextParameter();

                    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                    param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                    D3D12_ROOT_DESCRIPTOR_TABLE& table = param.DescriptorTable;
                    table.NumDescriptorRanges = 1;
                    table.pDescriptorRanges = &range;
                }

                if (detail.m_uavIndex >= 0)
                {
                    D3D12_DESCRIPTOR_RANGE& range = params.nextRange();

                    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    range.NumDescriptors = 1;
                    range.BaseShaderRegister = bindingIndex;
                    range.RegisterSpace = 0;
                    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                    D3D12_ROOT_PARAMETER& param = params.nextParameter();

                    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                    param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

                    D3D12_ROOT_DESCRIPTOR_TABLE& table = param.DescriptorTable;
                    table.NumDescriptorRanges = 1;
                    table.pDescriptorRanges = &range;
                }
            }
        }
    }

    // All the samplers are in one continuous section of the sampler heap
    if (m_boundBindingState && m_boundBindingState->m_samplerHeap.getUsedSize() > 0)
    {
        D3D12_DESCRIPTOR_RANGE& range = params.nextRange();

        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        range.NumDescriptors = m_boundBindingState->m_samplerHeap.getUsedSize();
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER& param = params.nextParameter();

        param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_DESCRIPTOR_TABLE& table = param.DescriptorTable;
        table.NumDescriptorRanges = 1;
        table.pDescriptorRanges = &range;
    }
    return SLANG_OK;
}

Result D3D12Renderer::_bindRenderState(RenderState* renderState, ID3D12GraphicsCommandList* commandList, Submitter* submitter)
{
    BindingStateImpl* bindingState = m_boundBindingState;

    submitter->setRootSignature(renderState->m_rootSignature);
    commandList->SetPipelineState(renderState->m_pipelineState);

    if (bindingState)
    {
        ID3D12DescriptorHeap* heaps[] =
        {
            bindingState->m_viewHeap.getHeap(),
            bindingState->m_samplerHeap.getHeap(),
        };
        commandList->SetDescriptorHeaps(SLANG_COUNT_OF(heaps), heaps);
    }
    else
    {
        commandList->SetDescriptorHeaps(0, nullptr);
    }

    {
        int index = 0;

        int numConstantBuffers = 0;
        {
            if (bindingState)
            {
                D3D12DescriptorHeap& heap = bindingState->m_viewHeap;
                const auto& details = bindingState->m_bindingDetails;
                const auto& bindings = bindingState->getDesc().m_bindings;
                const int numBindings = int(details.Count());

                for (int i = 0; i < numBindings; i++)
                {
                    const auto& detail = details[i];
                    const auto& binding = bindings[i];

                    if (binding.bindingType == BindingType::Buffer)
                    {
                        assert(binding.resource && binding.resource->isBuffer());
                        if (binding.resource->canBind(Resource::BindFlag::ConstantBuffer))
                        {
                            BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(binding.resource.Ptr());
                            buffer->bindConstantBufferView(m_circularResourceHeap, index++, submitter);
                            numConstantBuffers++;
                        }
                    }

                    if (detail.m_srvIndex >= 0)
                    {
                        submitter->setRootDescriptorTable(index++, heap.getGpuHandle(detail.m_srvIndex));
                    }

                    if (detail.m_uavIndex >= 0)
                    {
                        submitter->setRootDescriptorTable(index++, heap.getGpuHandle(detail.m_uavIndex));
                    }
                }
            }
        }

        if (bindingState && bindingState->m_samplerHeap.getUsedSize() > 0)
        {
            submitter->setRootDescriptorTable(index, bindingState->m_samplerHeap.getGpuStart());
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

Result D3D12Renderer::initialize(const Desc& desc, void* inWindowHandle)
{
    m_hwnd = (HWND)inWindowHandle;
    // Rather than statically link against D3D, we load it dynamically.

    HMODULE d3dModule = LoadLibraryA("d3d12.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d12.dll'\n");
        return SLANG_FAIL;
    }

    HMODULE dxgiModule = LoadLibraryA("Dxgi.dll");
    if (!dxgiModule)
    {
        fprintf(stderr, "error: failed load 'dxgi.dll'\n");
        return SLANG_FAIL;
    }


#define LOAD_D3D_PROC(TYPE, NAME) \
        TYPE NAME##_ = (TYPE) loadProc(d3dModule, #NAME);
#define LOAD_DXGI_PROC(TYPE, NAME) \
        TYPE NAME##_ = (TYPE) loadProc(dxgiModule, #NAME);

    UINT dxgiFactoryFlags = 0;

#if ENABLE_DEBUG_LAYER
    {
        LOAD_D3D_PROC(PFN_D3D12_GET_DEBUG_INTERFACE, D3D12GetDebugInterface);
        if (D3D12GetDebugInterface_)
        {
            if (SUCCEEDED(D3D12GetDebugInterface_(IID_PPV_ARGS(m_dxDebug.writeRef()))))
            {
                m_dxDebug->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
    }
#endif

    m_D3D12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)loadProc(d3dModule, "D3D12SerializeRootSignature");
    if (!m_D3D12SerializeRootSignature)
    {
        return SLANG_FAIL;
    }

    // Try and create DXGIFactory
    ComPtr<IDXGIFactory4> dxgiFactory;
    {
        typedef HRESULT(WINAPI *PFN_DXGI_CREATE_FACTORY_2)(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory);
        LOAD_DXGI_PROC(PFN_DXGI_CREATE_FACTORY_2, CreateDXGIFactory2);
        if (!CreateDXGIFactory2_)
        {
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(CreateDXGIFactory2_(dxgiFactoryFlags, IID_PPV_ARGS(dxgiFactory.writeRef())));
    }

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    // Search for an adapter that meets our requirements
    ComPtr<IDXGIAdapter> adapter;

    LOAD_D3D_PROC(PFN_D3D12_CREATE_DEVICE, D3D12CreateDevice);
    if (!D3D12CreateDevice_)
    {
        return SLANG_FAIL;
    }

    const bool useWarp = false;

    if (useWarp)
    {
        SLANG_RETURN_ON_FAIL(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.writeRef())));
        SLANG_RETURN_ON_FAIL(D3D12CreateDevice_(adapter, featureLevel, IID_PPV_ARGS(m_device.writeRef())));
    }
    else
    {
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
    }

    if (!adapter)
    {
        // Couldn't find an adapter
        return SLANG_FAIL;
    }

    m_numRenderFrames = 3;
    m_numRenderTargets = 2;

    m_desc = desc;

    // set viewport
    {
        m_viewport.Width = float(m_desc.width);
        m_viewport.Height = float(m_desc.height);
        m_viewport.MinDepth = 0;
        m_viewport.MaxDepth = 1;
        m_viewport.TopLeftX = 0;
        m_viewport.TopLeftY = 0;
    }

    {
        m_scissorRect.left = 0;
        m_scissorRect.top = 0;
        m_scissorRect.right = m_desc.width;
        m_scissorRect.bottom = m_desc.height;
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    SLANG_RETURN_ON_FAIL(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.writeRef())));

    // Describe the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = m_numRenderTargets;
    swapChainDesc.BufferDesc.Width = m_desc.width;
    swapChainDesc.BufferDesc.Height = m_desc.height;
    swapChainDesc.BufferDesc.Format = m_targetFormat;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = m_hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    if (m_isFullSpeed)
    {
        m_hasVsync = false;
        m_allowFullScreen = false;
    }

    if (!m_hasVsync)
    {
        swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    }

    // Swap chain needs the queue so that it can force a flush on it.
    ComPtr<IDXGISwapChain> swapChain;
    SLANG_RETURN_ON_FAIL(dxgiFactory->CreateSwapChain(m_commandQueue, &swapChainDesc, swapChain.writeRef()));
    SLANG_RETURN_ON_FAIL(swapChain->QueryInterface(m_swapChain.writeRef()));

    if (!m_hasVsync)
    {
        m_swapChainWaitableObject = m_swapChain->GetFrameLatencyWaitableObject();

        int maxLatency = m_numRenderTargets - 2;

        // Make sure the maximum latency is in the range required by dx12 runtime
        maxLatency = (maxLatency < 1) ? 1 : maxLatency;
        maxLatency = (maxLatency > DXGI_MAX_SWAP_CHAIN_BUFFERS) ? DXGI_MAX_SWAP_CHAIN_BUFFERS : maxLatency;

        m_swapChain->SetMaximumFrameLatency(maxLatency);
    }

    // This sample does not support fullscreen transitions.
    SLANG_RETURN_ON_FAIL(dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

    m_renderTargetIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

        rtvHeapDesc.NumDescriptors = m_numRenderTargets;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        SLANG_RETURN_ON_FAIL(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.writeRef())));
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    {
        // Describe and create a depth stencil view (DSV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        SLANG_RETURN_ON_FAIL(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.writeRef())));

        m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // Setup frame resources
    {
        SLANG_RETURN_ON_FAIL(createFrameResources());
    }

    // Setup fence, and close the command list (as default state without begin/endRender is closed)
    {
        SLANG_RETURN_ON_FAIL(m_fence.init(m_device));
        // Create the command list. When command lists are created they are open, so close it.
        FrameInfo& frame = m_frameInfos[m_frameIndex];
        SLANG_RETURN_ON_FAIL(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.m_commandAllocator, nullptr, IID_PPV_ARGS(m_commandList.writeRef())));
        m_commandList->Close();
    }

    {
        D3D12CircularResourceHeap::Desc desc;
        desc.init();
        // Define size
        desc.m_blockSize = 65536;
        // Set up the heap
        m_circularResourceHeap.init(m_device, desc, &m_fence);
    }

    // Setup for rendering
    beginRender();

    m_isInitialized = true;
    return SLANG_OK;
}

Result D3D12Renderer::createFrameResources()
{
    // Create back buffers
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvStart(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Work out target format
        D3D12_RESOURCE_DESC resourceDesc;
        {
            ComPtr<ID3D12Resource> backBuffer;
            SLANG_RETURN_ON_FAIL(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.writeRef())));
            resourceDesc = backBuffer->GetDesc();
        }
        const DXGI_FORMAT resourceFormat = D3DUtil::calcResourceFormat(D3DUtil::USAGE_TARGET, m_targetUsageFlags, resourceDesc.Format);
        const DXGI_FORMAT targetFormat = D3DUtil::calcFormat(D3DUtil::USAGE_TARGET, resourceFormat);

        // Set the target format
        m_targetFormat = targetFormat;

        // Create a RTV, and a command allocator for each frame.
        for (int i = 0; i < m_numRenderTargets; i++)
        {
            // Get the back buffer
            ComPtr<ID3D12Resource> backBuffer;
            SLANG_RETURN_ON_FAIL(m_swapChain->GetBuffer(UINT(i), IID_PPV_ARGS(backBuffer.writeRef())));

            // Set up resource for back buffer
            m_backBufferResources[i].setResource(backBuffer, D3D12_RESOURCE_STATE_COMMON);
            m_backBuffers[i] = &m_backBufferResources[i];
            // Assume they are the same thing for now...
            m_renderTargets[i] = &m_backBufferResources[i];

            // If we are multi-sampling - create a render target separate from the back buffer
            if (m_isMultiSampled)
            {
                D3D12_HEAP_PROPERTIES heapProps;
                heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
                heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                heapProps.CreationNodeMask = 1;
                heapProps.VisibleNodeMask = 1;
                D3D12_CLEAR_VALUE clearValue = {};
                clearValue.Format = m_targetFormat;

                // Don't know targets alignment, so just memory copy
                ::memcpy(clearValue.Color, m_clearColor, sizeof(m_clearColor));

                D3D12_RESOURCE_DESC desc(resourceDesc);

                desc.Format = resourceFormat;
                desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                desc.SampleDesc.Count = m_numTargetSamples;
                desc.SampleDesc.Quality = m_targetSampleQuality;
                desc.Alignment = 0;

                SLANG_RETURN_ON_FAIL(m_renderTargetResources[i].initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue));
                m_renderTargets[i] = &m_renderTargetResources[i];
            }

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { rtvStart.ptr + i * m_rtvDescriptorSize };
            m_device->CreateRenderTargetView(*m_renderTargets[i], nullptr, rtvHandle);
        }
    }

    // Set up frames
    for (int i = 0; i < m_numRenderFrames; i++)
    {
        FrameInfo& frame = m_frameInfos[i];
        SLANG_RETURN_ON_FAIL(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(frame.m_commandAllocator.writeRef())));
    }

    {
        D3D12_RESOURCE_DESC desc = m_backBuffers[0]->getResource()->GetDesc();
        assert(desc.Width == UINT64(m_desc.width) && desc.Height == UINT64(m_desc.height));
    }

    // Create the depth stencil view.
    {
        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        DXGI_FORMAT resourceFormat = D3DUtil::calcResourceFormat(D3DUtil::USAGE_DEPTH_STENCIL, m_depthStencilUsageFlags, m_depthStencilFormat);
        DXGI_FORMAT depthStencilFormat = D3DUtil::calcFormat(D3DUtil::USAGE_DEPTH_STENCIL, resourceFormat);

        // Set the depth stencil format
        m_depthStencilFormat = depthStencilFormat;

        // Setup default clear
        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = depthStencilFormat;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDesc.Format = resourceFormat;
        resourceDesc.Width = m_desc.width;
        resourceDesc.Height = m_desc.height;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.SampleDesc.Count = m_numTargetSamples;
        resourceDesc.SampleDesc.Quality = m_targetSampleQuality;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        resourceDesc.Alignment = 0;

        SLANG_RETURN_ON_FAIL(m_depthStencil.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue));

        // Set the depth stencil
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = depthStencilFormat;
        depthStencilDesc.ViewDimension = m_isMultiSampled ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        // Set up as the depth stencil view
        m_device->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        m_depthStencilView = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    m_viewport.Width = static_cast<float>(m_desc.width);
    m_viewport.Height = static_cast<float>(m_desc.height);
    m_viewport.MaxDepth = 1.0f;

    m_scissorRect.right = static_cast<LONG>(m_desc.width);
    m_scissorRect.bottom = static_cast<LONG>(m_desc.height);

    return SLANG_OK;
}

void D3D12Renderer::setClearColor(const float color[4])
{
    memcpy(m_clearColor, color, sizeof(m_clearColor));
}

void D3D12Renderer::clearFrame()
{
    // Record commands
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_renderTargetIndex * m_rtvDescriptorSize };
    m_commandList->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
    if (m_depthStencil)
    {
        m_commandList->ClearDepthStencilView(m_depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }
}

void D3D12Renderer::presentFrame()
{
    endRender();

    if (m_swapChainWaitableObject)
    {
        // check if now is good time to present
        // This doesn't wait - because the wait time is 0. If it returns WAIT_TIMEOUT it means that no frame is waiting to be be displayed
        // so there is no point doing a present.
        const bool shouldPresent = (WaitForSingleObjectEx(m_swapChainWaitableObject, 0, TRUE) != WAIT_TIMEOUT);
        if (shouldPresent)
        {
            m_swapChain->Present(0, 0);
        }
    }
    else
    {
        if (SLANG_FAILED(m_swapChain->Present(1, 0)))
        {
            assert(!"Problem presenting");
            beginRender();
            return;
        }
    }

    // Increment the fence value. Save on the frame - we'll know that frame is done when the fence value >=
    m_frameInfos[m_frameIndex].m_fenceValue = m_fence.nextSignal(m_commandQueue);

    // increment frame index after signal
    m_frameIndex = (m_frameIndex + 1) % m_numRenderFrames;
    // Update the render target index.
    m_renderTargetIndex = m_swapChain->GetCurrentBackBufferIndex();

    // On the current frame wait until it is completed
    {
        FrameInfo& frame = m_frameInfos[m_frameIndex];
        // If the next frame is not ready to be rendered yet, wait until it is ready.
        m_fence.waitUntilCompleted(frame.m_fenceValue);
    }

    // Setup such that rendering can restart
    beginRender();
}

SlangResult D3D12Renderer::captureScreenSurface(Surface& surfaceOut)
{
    return captureTextureToSurface(*m_renderTargets[m_renderTargetIndex], surfaceOut);
}

static D3D12_RESOURCE_STATES _calcResourceState(Resource::Usage usage)
{
    typedef Resource::Usage Usage;
    switch (usage)
    {
        case Usage::VertexBuffer:           return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        case Usage::IndexBuffer:            return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        case Usage::ConstantBuffer:         return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        case Usage::StreamOutput:           return D3D12_RESOURCE_STATE_STREAM_OUT;
        case Usage::RenderTarget:           return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case Usage::DepthWrite:             return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case Usage::DepthRead:              return D3D12_RESOURCE_STATE_DEPTH_READ;
        case Usage::UnorderedAccess:        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        case Usage::PixelShaderResource:    return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        case Usage::NonPixelShaderResource: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        case Usage::GenericRead:            return D3D12_RESOURCE_STATE_GENERIC_READ;
        default: return D3D12_RESOURCE_STATES(0);
    }
}

static D3D12_RESOURCE_FLAGS _calcResourceFlag(Resource::BindFlag::Enum bindFlag)
{
    typedef Resource::BindFlag BindFlag;
    switch (bindFlag)
    {
        case BindFlag::RenderTarget:        return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        case BindFlag::DepthStencil:        return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        case BindFlag::UnorderedAccess:     return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        default:                            return D3D12_RESOURCE_FLAG_NONE;
    }
}

static D3D12_RESOURCE_FLAGS _calcResourceBindFlags(Resource::Usage initialUsage, int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;

        dstFlags |= _calcResourceFlag(Resource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return D3D12_RESOURCE_FLAGS(dstFlags);
}

static D3D12_RESOURCE_DIMENSION _calcResourceDimension(Resource::Type type)
{
    switch (type)
    {
        case Resource::Type::Buffer:        return D3D12_RESOURCE_DIMENSION_BUFFER;
        case Resource::Type::Texture1D:     return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case Resource::Type::TextureCube:
        case Resource::Type::Texture2D:
        {
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        }
        case Resource::Type::Texture3D:     return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:                            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}

TextureResource* D3D12Renderer::createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& descIn, const TextureResource::Data* initData)
{
    // Description of uploading on Dx12
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215%28v=vs.85%29.aspx

    TextureResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    const DXGI_FORMAT pixelFormat = D3DUtil::getMapFormat(srcDesc.format);
    if (pixelFormat == DXGI_FORMAT_UNKNOWN)
    {
        return nullptr;
    }

    const int arraySize = srcDesc.calcEffectiveArraySize();

    const D3D12_RESOURCE_DIMENSION dimension = _calcResourceDimension(srcDesc.type);
    if (dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN)
    {
        return nullptr;
    }

    const int numMipMaps = srcDesc.numMipLevels;

    // Setup desc
    D3D12_RESOURCE_DESC resourceDesc;

    resourceDesc.Dimension = dimension;
    resourceDesc.Format = pixelFormat;
    resourceDesc.Width = srcDesc.size.width;
    resourceDesc.Height = srcDesc.size.height;
    resourceDesc.DepthOrArraySize = (srcDesc.size.depth > 1) ? srcDesc.size.depth : arraySize;

    resourceDesc.MipLevels = numMipMaps;
    resourceDesc.SampleDesc.Count = srcDesc.sampleDesc.numSamples;
    resourceDesc.SampleDesc.Quality = srcDesc.sampleDesc.quality;

    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Alignment = 0;

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(srcDesc));

    // Create the target resource
    {
        D3D12_HEAP_PROPERTIES heapProps;

        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        SLANG_RETURN_NULL_ON_FAIL(texture->m_resource.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr));

        texture->m_resource.setDebugName(L"Texture");
    }

    // Calculate the layout
    List<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
    layouts.SetSize(numMipMaps);
    List<UInt64> mipRowSizeInBytes;
    mipRowSizeInBytes.SetSize(numMipMaps);
    List<UInt32> mipNumRows;
    mipNumRows.SetSize(numMipMaps);

    // Since textures are effectively immutable currently initData must be set
    assert(initData);
    // We should have this many sub resources
    assert(initData->numSubResources == numMipMaps * srcDesc.size.depth * arraySize);

    // This is just the size for one array upload -> not for the whole texure
    UInt64 requiredSize = 0;
    m_device->GetCopyableFootprints(&resourceDesc, 0, numMipMaps, 0, layouts.begin(), mipNumRows.begin(), mipRowSizeInBytes.begin(), &requiredSize);

    // Sub resource indexing
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx#subresource_indexing

    int subResourceIndex = 0;
    for (int i = 0; i < arraySize; i++)
    {
        // Create the upload texture
        D3D12Resource uploadTexture;
        {
            D3D12_HEAP_PROPERTIES heapProps;

            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CreationNodeMask = 1;
            heapProps.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC uploadResourceDesc;

            uploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            uploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            uploadResourceDesc.Width = requiredSize;
            uploadResourceDesc.Height = 1;
            uploadResourceDesc.DepthOrArraySize = 1;
            uploadResourceDesc.MipLevels = 1;
            uploadResourceDesc.SampleDesc.Count = 1;
            uploadResourceDesc.SampleDesc.Quality = 0;
            uploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            uploadResourceDesc.Alignment = 0;

            SLANG_RETURN_NULL_ON_FAIL(uploadTexture.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, uploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr));

            uploadTexture.setDebugName(L"TextureUpload");
        }

        ID3D12Resource* uploadResource = uploadTexture;

        uint8_t* p;
        uploadResource->Map(0, nullptr, reinterpret_cast<void**>(&p));

        for (int j = 0; j < numMipMaps; ++j)
        {
            const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[j];
            const D3D12_SUBRESOURCE_FOOTPRINT& footprint = layout.Footprint;

            const TextureResource::Size mipSize = srcDesc.size.calcMipSize(j);

            assert(footprint.Width == mipSize.width && footprint.Height == mipSize.height && footprint.Depth == mipSize.depth);

            const ptrdiff_t dstMipRowPitch = ptrdiff_t(layouts[j].Footprint.RowPitch);
            const ptrdiff_t srcMipRowPitch = ptrdiff_t(initData->mipRowStrides[j]);

            assert(dstMipRowPitch >= srcMipRowPitch);

            const uint8_t* srcRow = (const uint8_t*)initData->subResources[subResourceIndex];
            uint8_t* dstRow = p + layouts[j].Offset;

            // Copy the depth each mip
            for (int l = 0; l < mipSize.depth; l++)
            {
                // Copy rows
                for (int k = 0; k < mipSize.height; ++k)
                {
                    ::memcpy(dstRow, srcRow, srcMipRowPitch);

                    srcRow += srcMipRowPitch;
                    dstRow += dstMipRowPitch;
                }
            }

            //assert(srcRow == (const uint8_t*)(srcMip.Buffer() + srcMip.Count()));
        }
        uploadResource->Unmap(0, nullptr);

        for (int mipIndex = 0; mipIndex < numMipMaps; ++mipIndex)
        {
            // https://msdn.microsoft.com/en-us/library/windows/desktop/dn903862(v=vs.85).aspx

            D3D12_TEXTURE_COPY_LOCATION src;
            src.pResource = uploadTexture;
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint = layouts[mipIndex];

            D3D12_TEXTURE_COPY_LOCATION dst;
            dst.pResource = texture->m_resource;
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = subResourceIndex;
            m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

            subResourceIndex++;
        }

        {
            // const D3D12_RESOURCE_STATES finalState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            const D3D12_RESOURCE_STATES finalState = _calcResourceState(initialUsage);

            D3D12BarrierSubmitter submitter(m_commandList);
            texture->m_resource.transition(finalState, submitter);
        }

        // Block - waiting for copy to complete (so can drop upload texture)
        submitGpuWorkAndWait();
    }

    return texture.detach();
}

BufferResource* D3D12Renderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData)
{
    typedef BufferResourceImpl::BackingStyle Style;

    BufferResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(initialUsage, srcDesc));

    // Save the style
    buffer->m_backingStyle = BufferResourceImpl::_calcResourceBackingStyle(initialUsage);

    D3D12_RESOURCE_DESC bufferDesc;
    _initBufferResourceDesc(srcDesc.sizeInBytes, bufferDesc);

    bufferDesc.Flags = _calcResourceBindFlags(initialUsage, srcDesc.bindFlags);

    switch (buffer->m_backingStyle)
    {
        case Style::MemoryBacked:
        {
            // Assume the constant buffer will change every frame. We'll just keep a copy of the contents
            // in regular memory until it needed
            buffer->m_memory.SetSize(UInt(srcDesc.sizeInBytes));
            // Initialize
            if (initData)
            {
                ::memcpy(buffer->m_memory.Buffer(), initData, srcDesc.sizeInBytes);
            }
            break;
        }
        case Style::ResourceBacked:
        {
            const D3D12_RESOURCE_STATES initialState = _calcResourceState(initialUsage);
            SLANG_RETURN_NULL_ON_FAIL(createBuffer(bufferDesc, initData, buffer->m_uploadResource, initialState, buffer->m_resource));
            break;
        }
        default: return nullptr;
    }

    return buffer.detach();
}

InputLayout* D3D12Renderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    // Work out a buffer size to hold all text
    size_t textSize = 0;
    for (int i = 0; i < Int(inputElementCount); ++i)
    {
        const char* text = inputElements[i].semanticName;
        textSize += text ? (::strlen(text) + 1) : 0;
    }
    layout->m_text.SetSize(textSize);
    char* textPos = layout->m_text.Buffer();

    //
    List<D3D12_INPUT_ELEMENT_DESC>& elements = layout->m_elements;
    elements.SetSize(inputElementCount);


    for (UInt i = 0; i < inputElementCount; ++i)
    {
        const InputElementDesc& srcEle = inputElements[i];
        D3D12_INPUT_ELEMENT_DESC& dstEle = elements[i];

        // Add text to the buffer
        const char* semanticName = srcEle.semanticName;
        if (semanticName)
        {
            const int len = int(::strlen(semanticName));
            ::memcpy(textPos, semanticName, len + 1);
            semanticName = textPos;
            textPos += len + 1;
        }

        dstEle.SemanticName = semanticName;
         dstEle.SemanticIndex = (UINT)srcEle.semanticIndex;
        dstEle.Format = D3DUtil::getMapFormat(srcEle.format);
        dstEle.InputSlot = 0;
        dstEle.AlignedByteOffset = (UINT)srcEle.offset;
        dstEle.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        dstEle.InstanceDataStepRate = 0;
    }

    return layout.detach();
}

void* D3D12Renderer::map(BufferResource* bufferIn, MapFlavor flavor)
{
    typedef BufferResourceImpl::BackingStyle Style;

    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    buffer->m_mapFlavor = flavor;

    const size_t bufferSize = buffer->getDesc().sizeInBytes;

    switch (buffer->m_backingStyle)
    {
        case Style::ResourceBacked:
        {
            // We need this in a state so we can upload
            switch (flavor)
            {
                case MapFlavor::HostWrite:
                case MapFlavor::WriteDiscard:
                {
                    D3D12BarrierSubmitter submitter(m_commandList);
                    buffer->m_uploadResource.transition(D3D12_RESOURCE_STATE_GENERIC_READ, submitter);
                    buffer->m_resource.transition(D3D12_RESOURCE_STATE_COPY_DEST, submitter);

                    const D3D12_RANGE readRange = {};

                    void* uploadData;
                    SLANG_RETURN_NULL_ON_FAIL(buffer->m_uploadResource.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&uploadData)));
                    return uploadData;

                    break;
                }
                case MapFlavor::HostRead:
                {
                    // This will be slow!!! - it blocks CPU on GPU completion
                    D3D12Resource& resource = buffer->m_resource;

                    // Readback heap
                    D3D12_HEAP_PROPERTIES heapProps;
                    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
                    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                    heapProps.CreationNodeMask = 1;
                    heapProps.VisibleNodeMask = 1;

                    // Resource to readback to
                    D3D12_RESOURCE_DESC stagingDesc;
                    _initBufferResourceDesc(bufferSize, stagingDesc);

                    D3D12Resource stageBuf;
                    SLANG_RETURN_NULL_ON_FAIL(stageBuf.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, stagingDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr));

                    const D3D12_RESOURCE_STATES initialState = resource.getState();

                    // Make it a source
                    {
                        D3D12BarrierSubmitter submitter(m_commandList);
                        resource.transition(D3D12_RESOURCE_STATE_COPY_SOURCE, submitter);
                    }
                    // Do the copy
                    m_commandList->CopyBufferRegion(stageBuf, 0, resource, 0, bufferSize);
                    // Switch it back
                    {
                        D3D12BarrierSubmitter submitter(m_commandList);
                        resource.transition(initialState, submitter);
                    }

                    // Wait until complete
                    submitGpuWorkAndWait();

                    // Map and copy
                    {
                        UINT8* data;
                        D3D12_RANGE readRange = { 0, bufferSize };

                        SLANG_RETURN_NULL_ON_FAIL(stageBuf.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&data)));

                        // Copy to memory buffer
                        buffer->m_memory.SetSize(bufferSize);
                        ::memcpy(buffer->m_memory.Buffer(), data, bufferSize);

                        stageBuf.getResource()->Unmap(0, nullptr);
                    }

                    return buffer->m_memory.Buffer();
                }
            }
            break;
        }
        case Style::MemoryBacked:
        {
            return buffer->m_memory.Buffer();
        }
        default: return nullptr;
    }

    return nullptr;
}

void D3D12Renderer::unmap(BufferResource* bufferIn)
{
    typedef BufferResourceImpl::BackingStyle Style;
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);

    switch (buffer->m_backingStyle)
    {
        case Style::MemoryBacked:
        {
            // Don't need to do anything, as will be uploaded automatically when used
            break;
        }
        case Style::ResourceBacked:
        {
            // We need this in a state so we can upload
            switch (buffer->m_mapFlavor)
            {
                case MapFlavor::HostWrite:
                case MapFlavor::WriteDiscard:
                {
                    // Unmap
                    ID3D12Resource* uploadResource = buffer->m_uploadResource;
                    ID3D12Resource* resource = buffer->m_resource;

                    uploadResource->Unmap(0, nullptr);

                    const D3D12_RESOURCE_STATES initialState = buffer->m_resource.getState();

                    {
                        D3D12BarrierSubmitter submitter(m_commandList);
                        buffer->m_uploadResource.transition(D3D12_RESOURCE_STATE_GENERIC_READ, submitter);
                        buffer->m_resource.transition(D3D12_RESOURCE_STATE_COPY_DEST, submitter);
                    }

                    m_commandList->CopyBufferRegion(resource, 0, uploadResource, 0, buffer->getDesc().sizeInBytes);

                    {
                        D3D12BarrierSubmitter submitter(m_commandList);
                        buffer->m_resource.transition(initialState, submitter);
                    }
                    break;
                }
                case MapFlavor::HostRead:
                {
                    break;
                }
            }
        }
    }
}

void D3D12Renderer::setInputLayout(InputLayout* inputLayout)
{
    m_boundInputLayout = static_cast<InputLayoutImpl*>(inputLayout);
}

void D3D12Renderer::setPrimitiveTopology(PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::TriangleList:
        {
            m_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            m_primitiveTopology = D3DUtil::getPrimitiveTopology(topology);
            break;
        }
        default:
        {
            assert(!"Unhandled type");
        }
    }
}

void D3D12Renderer::setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets)
{
    {
        const UInt num = startSlot + slotCount;
        if (num > m_boundVertexBuffers.Count())
        {
            m_boundVertexBuffers.SetSize(num);
        }
    }

    for (UInt i = 0; i < slotCount; i++)
    {
        BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[i]);
        if (buffer)
        {
            assert(buffer->m_initialUsage == Resource::Usage::VertexBuffer);
        }

        BoundVertexBuffer& boundBuffer = m_boundVertexBuffers[startSlot + i];
        boundBuffer.m_buffer = buffer;
        boundBuffer.m_stride = int(strides[i]);
        boundBuffer.m_offset = int(offsets[i]);
    }
}

void D3D12Renderer::setShaderProgram(ShaderProgram* inProgram)
{
    m_boundShaderProgram = static_cast<ShaderProgramImpl*>(inProgram);
}

void D3D12Renderer::draw(UInt vertexCount, UInt startVertex)
{
    ID3D12GraphicsCommandList* commandList = m_commandList;

    RenderState* renderState = calcRenderState();
    if (!renderState)
    {
        assert(!"Couldn't create render state");
        return;
    }

    BindingStateImpl* bindingState = m_boundBindingState;

    // Submit - setting for graphics
    {
        GraphicsSubmitter submitter(commandList);
        _bindRenderState(renderState, commandList, &submitter);
    }

    commandList->IASetPrimitiveTopology(m_primitiveTopology);

    // Set up vertex buffer views
    {
        int numVertexViews = 0;
        D3D12_VERTEX_BUFFER_VIEW vertexViews[16];
        for (int i = 0; i < int(m_boundVertexBuffers.Count()); i++)
        {
            const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[i];
            BufferResourceImpl* buffer = boundVertexBuffer.m_buffer;
            if (buffer)
            {
                D3D12_VERTEX_BUFFER_VIEW& vertexView = vertexViews[numVertexViews++];
                vertexView.BufferLocation = buffer->m_resource.getResource()->GetGPUVirtualAddress();
                vertexView.SizeInBytes = int(buffer->getDesc().sizeInBytes);
                vertexView.StrideInBytes = boundVertexBuffer.m_stride;
            }
        }
        commandList->IASetVertexBuffers(0, numVertexViews, vertexViews);
    }

    commandList->DrawInstanced(UINT(vertexCount), 1, UINT(startVertex), 0);
}

void D3D12Renderer::dispatchCompute(int x, int y, int z)
{
    ID3D12GraphicsCommandList* commandList = m_commandList;
    RenderState* renderState = calcRenderState();

    // Submit binding for compute
    {
        ComputeSubmitter submitter(commandList);
        _bindRenderState(renderState, commandList, &submitter);
    }

    commandList->Dispatch(x, y, z);
}

BindingState* D3D12Renderer::createBindingState(const BindingState::Desc& bindingStateDesc)
{
    RefPtr<BindingStateImpl> bindingState(new BindingStateImpl(bindingStateDesc));

    SLANG_RETURN_NULL_ON_FAIL(bindingState->init(m_device));

    const auto& srcBindings = bindingStateDesc.m_bindings;
    const int numBindings = int(srcBindings.Count());

    auto& dstDetails = bindingState->m_bindingDetails;
    dstDetails.SetSize(numBindings);

    for (int i = 0; i < numBindings; ++i)
    {
        const auto& srcEntry = srcBindings[i];
        auto& dstDetail = dstDetails[i];

        const int bindingIndex = srcEntry.registerRange.getSingleIndex();

        switch (srcEntry.bindingType)
        {
            case BindingType::Buffer:
            {
                assert(srcEntry.resource && srcEntry.resource->isBuffer());
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(srcEntry.resource.Ptr());
                const BufferResource::Desc& bufferDesc = bufferResource->getDesc();

                const size_t bufferSize = bufferDesc.sizeInBytes;
                const int elemSize = bufferDesc.elementSize <= 0 ? sizeof(uint32_t) : bufferDesc.elementSize;

                const  bool createSrv = false;

                // NOTE! In this arrangement the buffer can either be a ConstantBuffer or a 'StorageBuffer'.
                // If it's a storage buffer then it has a 'uav'.
                // In neither circumstance is there an associated srv
                // This departs a little from dx11 code - in that it will create srv and uav for a storage buffer.
                if (bufferDesc.bindFlags & Resource::BindFlag::UnorderedAccess)
                {
                    dstDetail.m_uavIndex = bindingState->m_viewHeap.allocate();
                    if (dstDetail.m_uavIndex < 0)
                    {
                        return nullptr;
                    }

                    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                    uavDesc.Format = D3DUtil::getMapFormat(bufferDesc.format);

                    uavDesc.Buffer.StructureByteStride = elemSize;

                    uavDesc.Buffer.FirstElement = 0;
                    uavDesc.Buffer.NumElements = (UINT)(bufferSize / elemSize);
                    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

                    if (bufferDesc.elementSize == 0 && bufferDesc.format == Format::Unknown)
                    {
                        uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
                        uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;

                        uavDesc.Buffer.StructureByteStride = 0;
                    }
                    else if( bufferDesc.format != Format::Unknown )
                    {
                        uavDesc.Buffer.StructureByteStride = 0;
                    }

                    m_device->CreateUnorderedAccessView(bufferResource->m_resource, nullptr, &uavDesc, bindingState->m_viewHeap.getCpuHandle(dstDetail.m_uavIndex));
                }
                if (createSrv && (bufferDesc.bindFlags & (Resource::BindFlag::NonPixelShaderResource | Resource::BindFlag::PixelShaderResource)))
                {
                    dstDetail.m_srvIndex = bindingState->m_viewHeap.allocate();
                    if (dstDetail.m_srvIndex < 0)
                    {
                        return nullptr;
                    }

                    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;

                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
                    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                    srvDesc.Buffer.FirstElement = 0;
                    srvDesc.Buffer.NumElements = (UINT)(bufferSize / elemSize);
                    srvDesc.Buffer.StructureByteStride = elemSize;
                    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

                    if (bufferDesc.elementSize == 0)
                    {
                        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
                    }

                    m_device->CreateShaderResourceView(bufferResource->m_resource, &srvDesc, bindingState->m_viewHeap.getCpuHandle(dstDetail.m_srvIndex));
                }

                break;
            }
            case BindingType::Texture:
            {
                assert(srcEntry.resource && srcEntry.resource->isTexture());

                TextureResourceImpl* textureResource = static_cast<TextureResourceImpl*>(srcEntry.resource.Ptr());

                dstDetail.m_srvIndex = bindingState->m_viewHeap.allocate();
                if (dstDetail.m_srvIndex < 0)
                {
                    return nullptr;
                }

                {
                    const D3D12_RESOURCE_DESC resourceDesc = textureResource->m_resource.getResource()->GetDesc();
                    const DXGI_FORMAT pixelFormat = resourceDesc.Format;

                    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
                    _initSrvDesc(textureResource->getType(), textureResource->getDesc(), resourceDesc, pixelFormat, srvDesc);

                    // Create descriptor
                    m_device->CreateShaderResourceView(textureResource->m_resource, &srvDesc, bindingState->m_viewHeap.getCpuHandle(dstDetail.m_srvIndex));
                }

                break;
            }
            case BindingType::Sampler:
            {
                const BindingState::SamplerDesc& samplerDesc = bindingStateDesc.m_samplerDescs[srcEntry.descIndex];

                const int samplerIndex = bindingIndex;
                dstDetail.m_samplerIndex = samplerIndex;
                bindingState->m_samplerHeap.placeAt(samplerIndex);

                D3D12_SAMPLER_DESC desc = {};
                desc.AddressU = desc.AddressV = desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

                if (samplerDesc.isCompareSampler)
                {
                    desc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
                    desc.Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                }
                else
                {
                    desc.Filter = D3D12_FILTER_ANISOTROPIC;
                    desc.MaxAnisotropy = 8;
                    desc.MinLOD = 0.0f;
                    desc.MaxLOD = 100.0f;
                }

                m_device->CreateSampler(&desc, bindingState->m_samplerHeap.getCpuHandle(samplerIndex));

                break;
            }
            case BindingType::CombinedTextureSampler:
            {
                assert(!"Not implemented");
                return nullptr;
            }
        }
    }

    return bindingState.detach();
}

void D3D12Renderer::setBindingState(BindingState* state)
{
    m_boundBindingState = static_cast<BindingStateImpl*>(state);
}

ShaderProgram* D3D12Renderer::createProgram(const ShaderProgram::Desc& desc)
{
    RefPtr<ShaderProgramImpl> program(new ShaderProgramImpl());
    program->m_pipelineType = desc.pipelineType;

    if (desc.pipelineType == PipelineType::Compute)
    {
        auto computeKernel = desc.findKernel(StageType::Compute);
        program->m_computeShader.InsertRange(0, (const uint8_t*) computeKernel->codeBegin, computeKernel->getCodeSize());
    }
    else
    {
        auto vertexKernel = desc.findKernel(StageType::Vertex);
        auto fragmentKernel = desc.findKernel(StageType::Fragment);

        program->m_vertexShader.InsertRange(0, (const uint8_t*) vertexKernel->codeBegin, vertexKernel->getCodeSize());
        program->m_pixelShader.InsertRange(0, (const uint8_t*) fragmentKernel->codeBegin, fragmentKernel->getCodeSize());
    }

    return program.detach();
}


} // renderer_test
