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

namespace gfx {
using namespace Slang;

class D3D12Renderer : public Renderer
{
public:
    // Renderer    implementation
    virtual SlangResult initialize(const Desc& desc, void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    TextureResource::Desc getSwapChainTextureDesc() override;

    Result createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData, TextureResource** outResource) override;
    Result createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& desc, const void* initData, BufferResource** outResource) override;
    Result createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler) override;

    Result createTextureView(TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView) override;
    Result createBufferView(BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView) override;

    Result createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount, InputLayout** outLayout) override;

    Result createDescriptorSetLayout(const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout) override;
    Result createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout) override;
    Result createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet) override;

    Result createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram) override;
    Result createGraphicsPipelineState(const GraphicsPipelineStateDesc& desc, PipelineState** outState) override;
    Result createComputePipelineState(const ComputePipelineStateDesc& desc, PipelineState** outState) override;

    virtual SlangResult captureScreenSurface(Surface& surfaceOut) override;

    virtual void* map(BufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(BufferResource* buffer) override;
//    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual void setDescriptorSet(PipelineType pipelineType, PipelineLayout* layout, UInt index, DescriptorSet* descriptorSet) override;

    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual void setDepthStencilTarget(ResourceView* depthStencilView) override;
    void setViewports(UInt count, Viewport const* viewports) override;
    void setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual void setPipelineState(PipelineType pipelineType, PipelineState* state) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override;
    virtual void waitForGpu() override;
    virtual RendererType getRendererType() const override { return RendererType::DirectX12; }

    ~D3D12Renderer();

protected:
    static const Int kMaxNumRenderFrames = 4;
    static const Int kMaxNumRenderTargets = 3;

    static const Int kMaxRTVCount = 8;
    static const Int kMaxDescriptorSetCount = 16;

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
            // Note: the D3D12 back-end has support for "versioning" of constant buffers,
            // where the same logical `BufferResource` can actually point to different
            // backing storage over its lifetime, to emulate the ability to modify the
            // buffer contents as in D3D11, etc.
            //
            // The VK back-end doesn't have the same behavior, and it is difficult
            // to both support this degree of flexibility *and* efficeintly exploit
            // descriptor tables (since any table referencing the buffer would need
            // to be updated when a new buffer "version" gets allocated).
            //
            // I'm choosing to disable this for now, and make all buffers be memory-backed,
            // although this creates synchronization issues that we'll have to address
            // next.

            return BackingStyle::ResourceBacked;
#if 0
            switch (usage)
            {
                case Usage::ConstantBuffer:     return BackingStyle::MemoryBacked;
                default:                        return BackingStyle::ResourceBacked;
            }
#endif
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

    class SamplerStateImpl : public SamplerState
    {
    public:
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
    };

    class ResourceViewImpl : public ResourceView
    {
    public:
        RefPtr<Resource>            m_resource;
        D3D12HostVisibleDescriptor  m_descriptor;
    };

    class InputLayoutImpl: public InputLayout
    {
        public:
        List<D3D12_INPUT_ELEMENT_DESC> m_elements;
        List<char> m_text;                              ///< Holds all strings to keep in scope
    };

#if 0
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
    };
#endif

    class DescriptorSetLayoutImpl : public DescriptorSetLayout
    {
    public:
        struct RangeInfo
        {
            DescriptorSlotType  type;
            Int                 count;
            Int                 arrayIndex;
        };

        List<RangeInfo> m_ranges;

        List<D3D12_DESCRIPTOR_RANGE>    m_dxRanges;
        List<D3D12_ROOT_PARAMETER>      m_dxRootParameters;

        Int m_resourceCount;
        Int m_samplerCount;
    };

    class PipelineLayoutImpl : public PipelineLayout
    {
    public:
        ComPtr<ID3D12RootSignature> m_rootSignature;
        UInt                        m_descriptorSetCount;
    };

    class DescriptorSetImpl : public DescriptorSet
    {
    public:
        virtual void setConstantBuffer(UInt range, UInt index, BufferResource* buffer) override;
        virtual void setResource(UInt range, UInt index, ResourceView* view) override;
        virtual void setSampler(UInt range, UInt index, SamplerState* sampler) override;
        virtual void setCombinedTextureSampler(
            UInt range,
            UInt index,
            ResourceView*   textureView,
            SamplerState*   sampler) override;

        D3D12Renderer*           m_renderer = nullptr;          ///< Weak pointer - must be because if set on Renderer, will have a circular reference
        RefPtr<DescriptorSetLayoutImpl> m_layout;

        D3D12DescriptorHeap*        m_resourceHeap = nullptr;
        D3D12DescriptorHeap*        m_samplerHeap = nullptr;

        Int                         m_resourceTable = 0;
        Int                         m_samplerTable = 0;

        // The following arrays are used to retain the relevant
        // objects so that they will not be released while this
        // descriptor-set is still alive.
        //
        // For the `m_resourceObjects` array, the values are either
        // the relevant `ResourceViewImpl` for SRV/UAV slots, or
        // a `BufferResourceImpl` for a CBV slot.
        //
        List<RefPtr<RefObject>>         m_resourceObjects;
        List<RefPtr<SamplerStateImpl>>  m_samplerObjects;
    };


    // During command submission, we need all the descriptor tables that get
    // used to come from a single heap (for each descriptor heap type).
    //
    // We will thus keep a single heap of each type that we hope will hold
    // all the descriptors that actually get needed in a frame.
    //
    // TODO: we need an allocation policy to reallocate and resize these
    // if/when we run out of space during a frame.
    //
    D3D12DescriptorHeap m_viewHeap;             ///< Cbv, Srv, Uav
    D3D12DescriptorHeap m_samplerHeap;          ///< Heap for samplers

    D3D12HostVisibleDescriptorAllocator m_rtvAllocator;
    D3D12HostVisibleDescriptorAllocator m_dsvAllocator;

    D3D12HostVisibleDescriptorAllocator m_viewAllocator;
    D3D12HostVisibleDescriptorAllocator m_samplerAllocator;

    // Space in the GPU-visible heaps is precious, so we will also keep
    // around CPU-visible heaps for storing descriptors in a format
    // that is ready for copying into the GPU-visible heaps as needed.
    //
    D3D12DescriptorHeap m_cpuViewHeap;          ///< Cbv, Srv, Uav
    D3D12DescriptorHeap m_cpuSamplerHeap;       ///< Heap for samplers

    class PipelineStateImpl : public PipelineState
    {
    public:
        PipelineType                m_pipelineType;
        RefPtr<PipelineLayoutImpl>  m_pipelineLayout;
        ComPtr<ID3D12PipelineState> m_pipelineState;
    };

    struct BoundVertexBuffer
    {
        RefPtr<BufferResourceImpl> m_buffer;
        int m_stride;
        int m_offset;
    };

#if 0
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
#endif

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

//    RenderState* calcRenderState();

        /// From current bindings calculate the root signature and pipeline state
//    Result calcGraphicsPipelineState(ComPtr<ID3D12RootSignature>& sigOut, ComPtr<ID3D12PipelineState>& pipelineStateOut);
//    Result calcComputePipelineState(ComPtr<ID3D12RootSignature>& signatureOut, ComPtr<ID3D12PipelineState>& pipelineStateOut);

    Result _bindRenderState(PipelineStateImpl* pipelineStateImpl, ID3D12GraphicsCommandList* commandList, Submitter* submitter);

//    Result _calcBindParameters(BindParameters& params);
//    RenderState* findRenderState(PipelineType pipelineType);

    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE m_D3D12SerializeRootSignature = nullptr;

    D3D12CircularResourceHeap m_circularResourceHeap;

    int m_commandListOpenCount = 0;            ///< If >0 the command list should be open

    List<BoundVertexBuffer> m_boundVertexBuffers;

    RefPtr<BufferResourceImpl> m_boundIndexBuffer;
    DXGI_FORMAT m_boundIndexFormat;
    UINT m_boundIndexOffset;

    RefPtr<PipelineStateImpl> m_currentPipelineState;

//    RefPtr<ShaderProgramImpl> m_boundShaderProgram;
//    RefPtr<InputLayoutImpl> m_boundInputLayout;

//    RefPtr<BindingStateImpl> m_boundBindingState;
    RefPtr<DescriptorSetImpl> m_boundDescriptorSets[int(PipelineType::CountOf)][kMaxDescriptorSetCount];

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
//    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    D3D12_RECT m_scissorRect = {};

//    List<RefPtr<RenderState> > m_renderStates;                ///< Holds list of all render state combinations
//    RenderState* m_currentRenderState = nullptr;            ///< The current combination

    UINT m_rtvDescriptorSize = 0;

//    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
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

    RefPtr<ResourceViewImpl> m_rtvs[kMaxRTVCount];
    RefPtr<ResourceViewImpl> m_dsv;

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

    // TIM: when should this get set?
//    commandList->OMSetRenderTargets(
//        1,
//        &m_rtvs[0]->m_descriptor.cpuHandle,
//        FALSE,
//        m_dsv ? &m_dsv->m_descriptor.cpuHandle : nullptr);

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

#if 0
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
#endif

#if 0
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
#endif

Result D3D12Renderer::_bindRenderState(PipelineStateImpl* pipelineStateImpl, ID3D12GraphicsCommandList* commandList, Submitter* submitter)
{
    // TODO: we should only set some of this state as needed...

    auto pipelineTypeIndex = (int) pipelineStateImpl->m_pipelineType;
    auto pipelineLayout = pipelineStateImpl->m_pipelineLayout;

    submitter->setRootSignature(pipelineLayout->m_rootSignature);
    commandList->SetPipelineState(pipelineStateImpl->m_pipelineState);

    ID3D12DescriptorHeap* heaps[] =
    {
        m_viewHeap.getHeap(),
        m_samplerHeap.getHeap(),
    };
    commandList->SetDescriptorHeaps(SLANG_COUNT_OF(heaps), heaps);

    // We need to copy descriptors over from the descriptor sets
    // (where they are stored in CPU-visible heaps) to the GPU-visible
    // heaps so that they can be accessed by shader code.

    Int descriptorSetCount = pipelineLayout->m_descriptorSetCount;
    Int rootParameterIndex = 0;
    for(Int dd = 0; dd < descriptorSetCount; ++dd)
    {
        auto descriptorSet = m_boundDescriptorSets[pipelineTypeIndex][dd];
        auto descriptorSetLayout = descriptorSet->m_layout;

        // TODO: require that `descriptorSetLayout` is compatible with
        // `pipelineLayout->descriptorSetlayouts[dd]`.

        {
            if(auto descriptorCount = descriptorSetLayout->m_resourceCount)
            {
                auto& gpuHeap = m_viewHeap;
                auto gpuDescriptorTable = gpuHeap.allocate(int(descriptorCount));

                auto& cpuHeap = *descriptorSet->m_resourceHeap;
                auto cpuDescriptorTable = descriptorSet->m_resourceTable;

                m_device->CopyDescriptorsSimple(
                    UINT(descriptorCount),
                    gpuHeap.getCpuHandle(gpuDescriptorTable),
                    cpuHeap.getCpuHandle(int(cpuDescriptorTable)),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                submitter->setRootDescriptorTable(int(rootParameterIndex++), gpuHeap.getGpuHandle(gpuDescriptorTable));
            }
        }
        {
            if(auto descriptorCount = descriptorSetLayout->m_samplerCount)
            {
                auto& gpuHeap = m_samplerHeap;
                auto gpuDescriptorTable = gpuHeap.allocate(int(descriptorCount));

                auto& cpuHeap = *descriptorSet->m_samplerHeap;
                auto cpuDescriptorTable = descriptorSet->m_samplerTable;

                m_device->CopyDescriptorsSimple(
                    UINT(descriptorCount),
                    gpuHeap.getCpuHandle(gpuDescriptorTable),
                    cpuHeap.getCpuHandle(int(cpuDescriptorTable)),
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

                submitter->setRootDescriptorTable(int(rootParameterIndex++), gpuHeap.getGpuHandle(gpuDescriptorTable));
            }
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
            else
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice_(candidateAdapter, featureLevel, IID_PPV_ARGS(m_device.writeRef()))))
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

    // set up debug layer
#ifndef NDEBUG
    {

        LOAD_D3D_PROC(PFN_D3D12_GET_DEBUG_INTERFACE, D3D12GetDebugInterface);
        if (!D3D12GetDebugInterface_)
        {
            return SLANG_FAIL;
        }

        ComPtr<ID3D12Debug> debug;

        if (!SUCCEEDED(D3D12GetDebugInterface_(IID_PPV_ARGS(debug.writeRef()))))
        {
            return SLANG_FAIL;
        }

        debug->EnableDebugLayer();
    }
#endif

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

    SLANG_RETURN_ON_FAIL(m_viewHeap.init   (m_device, 256, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
    SLANG_RETURN_ON_FAIL(m_samplerHeap.init(m_device, 16,  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,     D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));

    SLANG_RETURN_ON_FAIL(m_cpuViewHeap.init   (m_device, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
    SLANG_RETURN_ON_FAIL(m_cpuSamplerHeap.init(m_device, 64,   D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,     D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

    SLANG_RETURN_ON_FAIL(m_rtvAllocator.init    (m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
    SLANG_RETURN_ON_FAIL(m_dsvAllocator.init    (m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
    SLANG_RETURN_ON_FAIL(m_viewAllocator.init   (m_device, 64, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    SLANG_RETURN_ON_FAIL(m_samplerAllocator.init(m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

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
//        D3D12_CPU_DESCRIPTOR_HANDLE rtvStart(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

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

            D3D12HostVisibleDescriptor rtvDescriptor;
            SLANG_RETURN_ON_FAIL(m_rtvAllocator.allocate(&rtvDescriptor));

            m_device->CreateRenderTargetView(*m_renderTargets[i], nullptr, rtvDescriptor.cpuHandle);
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

#if 0
        SLANG_RETURN_ON_FAIL(m_depthStencil.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue));

        // Set the depth stencil
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = depthStencilFormat;
        depthStencilDesc.ViewDimension = m_isMultiSampled ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        // Set up as the depth stencil view
        m_device->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
        m_depthStencilView = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
#endif
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
    if(auto rtv = m_rtvs[0])
    {
        m_commandList->ClearRenderTargetView(rtv->m_descriptor.cpuHandle, m_clearColor, 0, nullptr);
    }
    if (m_dsv)
    {
        m_commandList->ClearDepthStencilView(m_dsv->m_descriptor.cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
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

TextureResource::Desc D3D12Renderer::getSwapChainTextureDesc()
{
    TextureResource::Desc desc;
    desc.init2D(Resource::Type::Texture2D, Format::Unknown, m_desc.width, m_desc.height, 1);

    return desc;
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

Result D3D12Renderer::createTextureResource(Resource::Usage initialUsage, const TextureResource::Desc& descIn, const TextureResource::Data* initData, TextureResource** outResource)
{
    // Description of uploading on Dx12
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215%28v=vs.85%29.aspx

    TextureResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    const DXGI_FORMAT pixelFormat = D3DUtil::getMapFormat(srcDesc.format);
    if (pixelFormat == DXGI_FORMAT_UNKNOWN)
    {
        return SLANG_FAIL;
    }

    const int arraySize = srcDesc.calcEffectiveArraySize();

    const D3D12_RESOURCE_DIMENSION dimension = _calcResourceDimension(srcDesc.type);
    if (dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN)
    {
        return SLANG_FAIL;
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

        SLANG_RETURN_ON_FAIL(texture->m_resource.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr));

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

            SLANG_RETURN_ON_FAIL(uploadTexture.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, uploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr));

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

        // Block - waiting for copy to complete (so can drop upload texture)
        submitGpuWorkAndWait();
    }

    {
        const D3D12_RESOURCE_STATES finalState = _calcResourceState(initialUsage);
        D3D12BarrierSubmitter submitter(m_commandList);
        texture->m_resource.transition(finalState, submitter);

        submitGpuWorkAndWait();
    }

    *outResource = texture.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData, BufferResource** outResource)
{
    typedef BufferResourceImpl::BackingStyle Style;

    BufferResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    // Always align up to 256 bytes, since that is required for constant buffers.
    //
    // TODO: only do this for buffers that could potentially be bound as constant buffers...
    //
    const size_t alignedSizeInBytes = D3DUtil::calcAligned(srcDesc.sizeInBytes, 256);

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(initialUsage, srcDesc));

    // Save the style
    buffer->m_backingStyle = BufferResourceImpl::_calcResourceBackingStyle(initialUsage);

    D3D12_RESOURCE_DESC bufferDesc;
    _initBufferResourceDesc(alignedSizeInBytes, bufferDesc);

    bufferDesc.Flags = _calcResourceBindFlags(initialUsage, srcDesc.bindFlags);

    switch (buffer->m_backingStyle)
    {
        case Style::MemoryBacked:
        {
            // Assume the constant buffer will change every frame. We'll just keep a copy of the contents
            // in regular memory until it needed
            buffer->m_memory.SetSize(UInt(alignedSizeInBytes));
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
            SLANG_RETURN_ON_FAIL(createBuffer(bufferDesc, initData, buffer->m_uploadResource, initialState, buffer->m_resource));
            break;
        }
        default:
            return SLANG_FAIL;
    }

    *outResource = buffer.detach();
    return SLANG_OK;
}

D3D12_FILTER_TYPE translateFilterMode(TextureFilteringMode mode)
{
    switch (mode)
    {
    default:
        return D3D12_FILTER_TYPE(0);

#define CASE(SRC, DST) \
    case TextureFilteringMode::SRC: return D3D12_FILTER_TYPE_##DST

        CASE(Point, POINT);
        CASE(Linear, LINEAR);

#undef CASE
    }
}

D3D12_FILTER_REDUCTION_TYPE translateFilterReduction(TextureReductionOp op)
{
    switch (op)
    {
    default:
        return D3D12_FILTER_REDUCTION_TYPE(0);

#define CASE(SRC, DST) \
    case TextureReductionOp::SRC: return D3D12_FILTER_REDUCTION_TYPE_##DST

        CASE(Average, STANDARD);
        CASE(Comparison, COMPARISON);
        CASE(Minimum, MINIMUM);
        CASE(Maximum, MAXIMUM);

#undef CASE
    }
}

D3D12_TEXTURE_ADDRESS_MODE translateAddressingMode(TextureAddressingMode mode)
{
    switch (mode)
    {
    default:
        return D3D12_TEXTURE_ADDRESS_MODE(0);

#define CASE(SRC, DST) \
    case TextureAddressingMode::SRC: return D3D12_TEXTURE_ADDRESS_MODE_##DST

    CASE(Wrap,          WRAP);
    CASE(ClampToEdge,   CLAMP);
    CASE(ClampToBorder, BORDER);
    CASE(MirrorRepeat,  MIRROR);
    CASE(MirrorOnce,    MIRROR_ONCE);

#undef CASE
    }
}

static D3D12_COMPARISON_FUNC translateComparisonFunc(ComparisonFunc func)
{
    switch (func)
    {
    default:
        // TODO: need to report failures
        return D3D12_COMPARISON_FUNC_ALWAYS;

#define CASE(FROM, TO) \
    case ComparisonFunc::FROM: return D3D12_COMPARISON_FUNC_##TO

        CASE(Never, NEVER);
        CASE(Less, LESS);
        CASE(Equal, EQUAL);
        CASE(LessEqual, LESS_EQUAL);
        CASE(Greater, GREATER);
        CASE(NotEqual, NOT_EQUAL);
        CASE(GreaterEqual, GREATER_EQUAL);
        CASE(Always, ALWAYS);
#undef CASE
    }
}

Result D3D12Renderer::createSamplerState(SamplerState::Desc const& desc, SamplerState** outSampler)
{
    D3D12_FILTER_REDUCTION_TYPE dxReduction = translateFilterReduction(desc.reductionOp);
    D3D12_FILTER dxFilter;
    if (desc.maxAnisotropy > 1)
    {
        dxFilter = D3D12_ENCODE_ANISOTROPIC_FILTER(dxReduction);
    }
    else
    {
        D3D12_FILTER_TYPE dxMin = translateFilterMode(desc.minFilter);
        D3D12_FILTER_TYPE dxMag = translateFilterMode(desc.magFilter);
        D3D12_FILTER_TYPE dxMip = translateFilterMode(desc.mipFilter);

        dxFilter = D3D12_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, dxReduction);
    }

    D3D12_SAMPLER_DESC dxDesc = {};
    dxDesc.Filter = dxFilter;
    dxDesc.AddressU = translateAddressingMode(desc.addressU);
    dxDesc.AddressV = translateAddressingMode(desc.addressV);
    dxDesc.AddressW = translateAddressingMode(desc.addressW);
    dxDesc.MipLODBias = desc.mipLODBias;
    dxDesc.MaxAnisotropy = desc.maxAnisotropy;
    dxDesc.ComparisonFunc = translateComparisonFunc(desc.comparisonFunc);
    for (int ii = 0; ii < 4; ++ii)
        dxDesc.BorderColor[ii] = desc.borderColor[ii];
    dxDesc.MinLOD = desc.minLOD;
    dxDesc.MaxLOD = desc.maxLOD;

    auto samplerHeap = &m_cpuSamplerHeap;

    int indexInSamplerHeap = samplerHeap->allocate();
    if(indexInSamplerHeap < 0)
    {
        // We ran out of room in our CPU sampler heap.
        //
        // TODO: this should not be a catastrophic failure, because
        // we should just allocate another CPU sampler heap that
        // can service subsequent allocation.
        //
        return SLANG_FAIL;
    }
    auto cpuDescriptorHandle = samplerHeap->getCpuHandle(indexInSamplerHeap);

    m_device->CreateSampler(&dxDesc, cpuDescriptorHandle);

    // TODO: We really ought to have a free-list of sampler-heap
    // entries that we check before we go to the heap, and then
    // when we are done with a sampler we simply add it to the free list.
    //
    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_cpuHandle = cpuDescriptorHandle;
    *outSampler = samplerImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createTextureView(TextureResource* texture, ResourceView::Desc const& desc, ResourceView** outView)
{
    auto resourceImpl = (TextureResourceImpl*) texture;

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case ResourceView::Type::RenderTarget:
        {
            SLANG_RETURN_ON_FAIL(m_rtvAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateRenderTargetView(resourceImpl->m_resource, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case ResourceView::Type::DepthStencil:
        {
            SLANG_RETURN_ON_FAIL(m_dsvAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateDepthStencilView(resourceImpl->m_resource, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case ResourceView::Type::UnorderedAccess:
        {
            // TODO: need to support the separate "counter resource" for the case
            // of append/consume buffers with attached counters.

            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case ResourceView::Type::ShaderResource:
        {
            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateShaderResourceView(resourceImpl->m_resource, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;
    }

    *outView = viewImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createBufferView(BufferResource* buffer, ResourceView::Desc const& desc, ResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;
    auto resourceDesc = resourceImpl->getDesc();

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case ResourceView::Type::UnorderedAccess:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Format = D3DUtil::getMapFormat(desc.format);
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes);

            if(resourceDesc.elementSize)
            {
                uavDesc.Buffer.StructureByteStride = resourceDesc.elementSize;
                uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / resourceDesc.elementSize);
            }
            else if(desc.format == Format::Unknown)
            {
                uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            }


            // TODO: need to support the separate "counter resource" for the case
            // of append/consume buffers with attached counters.

            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, &uavDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case ResourceView::Type::ShaderResource:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Format = D3DUtil::getMapFormat(desc.format);
            srvDesc.Buffer.StructureByteStride = 0;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes);

            if(resourceDesc.elementSize)
            {
                srvDesc.Buffer.StructureByteStride = resourceDesc.elementSize;
                srvDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / resourceDesc.elementSize);
            }

            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateShaderResourceView(resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;
    }

    *outView = viewImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount, InputLayout** outLayout)
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

    *outLayout = layout.detach();
    return SLANG_OK;
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

#if 0
void D3D12Renderer::setInputLayout(InputLayout* inputLayout)
{
    m_boundInputLayout = static_cast<InputLayoutImpl*>(inputLayout);
}
#endif

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

void D3D12Renderer::setIndexBuffer(BufferResource* buffer, Format indexFormat, UInt offset)
{
    m_boundIndexBuffer = (BufferResourceImpl*) buffer;
    m_boundIndexFormat = D3DUtil::getMapFormat(indexFormat);
    m_boundIndexOffset = UINT(offset);
}

void D3D12Renderer::setDepthStencilTarget(ResourceView* depthStencilView)
{
}

void D3D12Renderer::setViewports(UInt count, Viewport const* viewports)
{
    static const int kMaxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    assert(count <= kMaxViewports);

    D3D12_VIEWPORT dxViewports[kMaxViewports];
    for(UInt ii = 0; ii < count; ++ii)
    {
        auto& inViewport = viewports[ii];
        auto& dxViewport = dxViewports[ii];

        dxViewport.TopLeftX = inViewport.originX;
        dxViewport.TopLeftY = inViewport.originY;
        dxViewport.Width    = inViewport.extentX;
        dxViewport.Height   = inViewport.extentY;
        dxViewport.MinDepth = inViewport.minZ;
        dxViewport.MaxDepth = inViewport.maxZ;
    }

    m_commandList->RSSetViewports(UINT(count), dxViewports);
}

void D3D12Renderer::setScissorRects(UInt count, ScissorRect const* rects)
{
    static const int kMaxScissorRects = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    assert(count <= kMaxScissorRects);

    D3D12_RECT dxRects[kMaxScissorRects];
    for(UInt ii = 0; ii < count; ++ii)
    {
        auto& inRect = rects[ii];
        auto& dxRect = dxRects[ii];

        dxRect.left     = LONG(inRect.minX);
        dxRect.top      = LONG(inRect.minY);
        dxRect.right    = LONG(inRect.maxX);
        dxRect.bottom   = LONG(inRect.maxY);
    }

    m_commandList->RSSetScissorRects(UINT(count), dxRects);
}

void D3D12Renderer::setPipelineState(PipelineType pipelineType, PipelineState* state)
{
    m_currentPipelineState = (PipelineStateImpl*)state;
}

void D3D12Renderer::draw(UInt vertexCount, UInt startVertex)
{
    ID3D12GraphicsCommandList* commandList = m_commandList;

    auto pipelineState = m_currentPipelineState.Ptr();
    if (!pipelineState || (pipelineState->m_pipelineType != PipelineType::Graphics))
    {
        assert(!"No graphics pipeline state set");
        return;
    }

    // Submit - setting for graphics
    {
        GraphicsSubmitter submitter(commandList);
        _bindRenderState(pipelineState, commandList, &submitter);
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
                vertexView.BufferLocation = buffer->m_resource.getResource()->GetGPUVirtualAddress()
                    + boundVertexBuffer.m_offset;
                vertexView.SizeInBytes = UINT(buffer->getDesc().sizeInBytes - boundVertexBuffer.m_offset);
                vertexView.StrideInBytes = UINT(boundVertexBuffer.m_stride);
            }
        }
        commandList->IASetVertexBuffers(0, numVertexViews, vertexViews);
    }

    // Set up index buffer
    if(m_boundIndexBuffer)
    {
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
        indexBufferView.BufferLocation = m_boundIndexBuffer->m_resource.getResource()->GetGPUVirtualAddress()
            + m_boundIndexOffset;
        indexBufferView.SizeInBytes = UINT(m_boundIndexBuffer->getDesc().sizeInBytes - m_boundIndexOffset);
        indexBufferView.Format = m_boundIndexFormat;

        commandList->IASetIndexBuffer(&indexBufferView);
    }

    commandList->DrawInstanced(UINT(vertexCount), 1, UINT(startVertex), 0);
}

void D3D12Renderer::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
}

void D3D12Renderer::dispatchCompute(int x, int y, int z)
{
    ID3D12GraphicsCommandList* commandList = m_commandList;
    auto pipelineStateImpl = m_currentPipelineState;

    // Submit binding for compute
    {
        ComputeSubmitter submitter(commandList);
        _bindRenderState(pipelineStateImpl, commandList, &submitter);
    }

    commandList->Dispatch(x, y, z);
}

#if 0
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
                const BufferResource::Desc& desc = bufferResource->getDesc();

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
#endif

void D3D12Renderer::DescriptorSetImpl::setConstantBuffer(UInt range, UInt index, BufferResource* buffer)
{
    auto dxDevice = m_renderer->m_device.get();

    auto resourceImpl = (BufferResourceImpl*) buffer;
    auto resourceDesc = resourceImpl->getDesc();

    // Constant buffer view size must be a multiple of 256 bytes, so we round it up here.
    const size_t alignedSizeInBytes = D3DUtil::calcAligned(resourceDesc.sizeInBytes, 256);

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = resourceImpl->m_resource.getResource()->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = UINT(alignedSizeInBytes);

    auto& rangeInfo = m_layout->m_ranges[range];

#ifdef _DEBUG
    switch(rangeInfo.type)
    {
    default:
        assert(!"incorrect slot type");
        break;

    case DescriptorSlotType::UniformBuffer:
    case DescriptorSlotType::DynamicUniformBuffer:
        break;
    }
#endif

    auto arrayIndex = rangeInfo.arrayIndex + index;
    auto descriptorIndex = m_resourceTable + arrayIndex;

    m_resourceObjects[arrayIndex] = resourceImpl;
    dxDevice->CreateConstantBufferView(
        &cbvDesc,
        m_resourceHeap->getCpuHandle(int(descriptorIndex)));
}

void D3D12Renderer::DescriptorSetImpl::setResource(UInt range, UInt index, ResourceView* view)
{
    auto dxDevice = m_renderer->m_device;

    auto viewImpl = (ResourceViewImpl*) view;

    auto& rangeInfo = m_layout->m_ranges[range];

    // TODO: validation that slot type matches view

    auto arrayIndex = rangeInfo.arrayIndex + index;
    auto descriptorIndex = m_resourceTable + arrayIndex;

    m_resourceObjects[arrayIndex] = viewImpl;
    dxDevice->CopyDescriptorsSimple(
        1,
        m_resourceHeap->getCpuHandle(int(descriptorIndex)),
        viewImpl->m_descriptor.cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3D12Renderer::DescriptorSetImpl::setSampler(UInt range, UInt index, SamplerState* sampler)
{
    auto dxDevice = m_renderer->m_device;

    auto samplerImpl = (SamplerStateImpl*) sampler;

    auto& rangeInfo = m_layout->m_ranges[range];

#ifdef _DEBUG
    switch(rangeInfo.type)
    {
    default:
        assert(!"incorrect slot type");
        break;

    case DescriptorSlotType::Sampler:
        break;
    }
#endif

    auto arrayIndex = rangeInfo.arrayIndex + index;
    auto descriptorIndex = m_resourceTable + arrayIndex;

    m_samplerObjects[arrayIndex] = samplerImpl;
    dxDevice->CopyDescriptorsSimple(
        1,
        m_samplerHeap->getCpuHandle(int(descriptorIndex)),
        samplerImpl->m_cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void D3D12Renderer::DescriptorSetImpl::setCombinedTextureSampler(
    UInt range,
    UInt index,
    ResourceView*   textureView,
    SamplerState*   sampler)
{
    auto dxDevice = m_renderer->m_device;

    auto viewImpl = (ResourceViewImpl*) textureView;
    auto samplerImpl = (SamplerStateImpl*) sampler;

    auto& rangeInfo = m_layout->m_ranges[range];

#ifdef _DEBUG
    switch(rangeInfo.type)
    {
    default:
        assert(!"incorrect slot type");
        break;

    case DescriptorSlotType::CombinedImageSampler:
        break;
    }
#endif

    auto arrayIndex = rangeInfo.arrayIndex + index;
    auto resourceDescriptorIndex = m_resourceTable + arrayIndex;
    auto samplerDescriptorIndex = m_samplerTable + arrayIndex;

    m_resourceObjects[arrayIndex] = viewImpl;
    dxDevice->CopyDescriptorsSimple(
        1,
        m_resourceHeap->getCpuHandle(int(resourceDescriptorIndex)),
        viewImpl->m_descriptor.cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    m_samplerObjects[arrayIndex] = samplerImpl;
    dxDevice->CopyDescriptorsSimple(
        1,
        m_samplerHeap->getCpuHandle(int(samplerDescriptorIndex)),
        samplerImpl->m_cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void D3D12Renderer::setDescriptorSet(PipelineType pipelineType, PipelineLayout* layout, UInt index, DescriptorSet* descriptorSet)
{
    // In D3D12, unlike Vulkan, binding a root signature invalidates *all* descriptor table
    // bindings (rather than preserving those that are part of the longest common prefix
    // between the old and new layout).
    //
    // In order to accomodate having descriptor-set bindings that persist across changes
    // in pipeline state (which may also change pipeline layout), we will shadow the
    // descriptor-set bindings and only flush them on-demand at draw tiume once the final
    // pipline layout is known.
    //

    auto descriptorSetImpl = (DescriptorSetImpl*) descriptorSet;
    m_boundDescriptorSets[int(pipelineType)][index] = descriptorSetImpl;
}

Result D3D12Renderer::createProgram(const ShaderProgram::Desc& desc, ShaderProgram** outProgram)
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

    *outProgram = program.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createDescriptorSetLayout(const DescriptorSetLayout::Desc& desc, DescriptorSetLayout** outLayout)
{
    Int rangeCount = desc.slotRangeCount;

    // For our purposes, there are three main cases of descriptor ranges to consider:
    //
    // 1. Resources: CBV, SRV, UAV
    //
    // 2. Samplers
    //
    // 3. Combined texture/sampler pairs
    //
    // The combined case presents challenges, because we will implement
    // them as both a resource slot and a sampler slot, and for conveience
    // in the indexing logic, it would be nice it they "lined up."
    //
    // We will start by counting how many ranges, and how many
    // descriptors, of each type we have.
    //

    Int dedicatedResourceCount = 0;
    Int dedicatedSamplerCount = 0;
    Int combinedCount = 0;

    Int dedicatedResourceRangeCount = 0;
    Int dedicatedSamplerRangeCount = 0;
    Int combinedRangeCount = 0;

    for(Int rr = 0; rr < rangeCount; ++rr)
    {
        auto rangeDesc = desc.slotRanges[rr];
        switch(rangeDesc.type)
        {
        case DescriptorSlotType::Sampler:
            dedicatedSamplerCount += rangeDesc.count;
            dedicatedSamplerRangeCount++;
            break;

        case DescriptorSlotType::CombinedImageSampler:
            combinedCount += rangeDesc.count;
            combinedRangeCount++;
            break;

        default:
            dedicatedResourceCount += rangeDesc.count;
            dedicatedResourceRangeCount++;
            break;
        }
    }

    // Now we know how many ranges we have to allocate space for,
    // and also how they need to be arranged.
    //
    // Each "combined" range will map to two ranges in the D3D
    // descriptor tables.

    RefPtr<DescriptorSetLayoutImpl> descriptorSetLayoutImpl = new DescriptorSetLayoutImpl();

    // We know the total number of resource and sampler "slots" that an instance
    // of this descriptor-set layout would need:
    //
    descriptorSetLayoutImpl->m_resourceCount = combinedCount + dedicatedResourceCount;
    descriptorSetLayoutImpl->m_samplerCount = combinedCount + dedicatedSamplerCount;

    // We can start by allocating the D3D root parameter info needed for the
    // descriptor set, based on the total number or ranges we need, which
    // we can compute from the combined and dedicated counts:
    //
    Int totalResourceRangeCount = combinedRangeCount + dedicatedResourceRangeCount;
    Int totalSamplerRangeCount  = combinedRangeCount + dedicatedSamplerRangeCount;

    if( totalResourceRangeCount )
    {
        D3D12_ROOT_PARAMETER dxRootParameter = {};
        dxRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        dxRootParameter.DescriptorTable.NumDescriptorRanges = UINT(totalResourceRangeCount);
        descriptorSetLayoutImpl->m_dxRootParameters.Add(dxRootParameter);
    }
    if( totalSamplerRangeCount )
    {
        D3D12_ROOT_PARAMETER dxRootParameter = {};
        dxRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        dxRootParameter.DescriptorTable.NumDescriptorRanges = UINT(totalSamplerRangeCount);
        descriptorSetLayoutImpl->m_dxRootParameters.Add(dxRootParameter);
    }

    // Next we can allocate space for all the D3D register ranges we need,
    // again based on totals that we can compute easily:
    //
    Int totalRangeCount = totalResourceRangeCount + totalSamplerRangeCount;
    descriptorSetLayoutImpl->m_dxRanges.SetSize(totalRangeCount);

    // Now we will walk through the ranges in  the order they were
    // specified, so that we can fill in the "range info" required for
    // binding parameters into descriptor sets allocated with this layout.
    //
    // This effectively determines the space required in two arrays
    // in each descriptor set: one for resources, and one for samplers.
    // A "combined" descriptor requires space in both arrays. The entries
    // for "dedicated" samplers/resources always come after those for
    // "combined" descriptors in the same array, so that a single index
    // can be used for both arrays in the combined case.
    //

    {
        Int samplerCounter = 0;
        Int resourceCounter = 0;
        Int combinedCounter = 0;
        for(Int rr = 0; rr < rangeCount; ++rr)
        {
            auto rangeDesc = desc.slotRanges[rr];

            DescriptorSetLayoutImpl::RangeInfo rangeInfo;

            rangeInfo.type = rangeDesc.type;
            rangeInfo.count = rangeDesc.count;

            switch(rangeDesc.type)
            {
            default:
                // Default case is a dedicated resource, and its index in the
                // resource array will come after all the combined entries.
                rangeInfo.arrayIndex = combinedCount + resourceCounter;
                resourceCounter += rangeInfo.count;
                break;

            case DescriptorSlotType::Sampler:
                // A dedicated sampler comes after all the entries for
                // combined texture/samplers in the sampler array.
                rangeInfo.arrayIndex = combinedCount + samplerCounter;
                samplerCounter += rangeInfo.count;
                break;

            case DescriptorSlotType::CombinedImageSampler:
                // Combined descriptors take entries at the front of
                // the resource and sampler arrays.
                rangeInfo.arrayIndex = combinedCounter;
                combinedCounter += rangeInfo.count;
                break;
            }

            descriptorSetLayoutImpl->m_ranges.Add(rangeInfo);
        }
    }

    // Finally, we will go through and fill in ready-to-go D3D
    // register range information.
    {
        UInt cbvCounter = 0;
        UInt srvCounter = 0;
        UInt uavCounter = 0;
        UInt samplerCounter = 0;

        Int resourceRangeCounter = 0;
        Int samplerRangeCounter = 0;
        Int combinedRangeCounter = 0;

        for(Int rr = 0; rr < rangeCount; ++rr)
        {
            auto rangeDesc = desc.slotRanges[rr];
            Int bindingCount = rangeDesc.count;

            // All of these descriptor ranges will be initialized
            // with a "space" of zero, with the assumption that
            // the actual space number will come from when they are
            // used as part of a pipeline layout.
            //
            Int bindingSpace = 0;

            Int dxRangeIndex = -1;
            Int dxPairedSamplerRangeIndex = -1;

            switch(rangeDesc.type)
            {
            default:
                // Default case is a dedicated resource, and its index in the
                // resource array will come after all the combined entries.
                dxRangeIndex = combinedRangeCount + resourceRangeCounter;
                resourceRangeCounter++;
                break;

            case DescriptorSlotType::Sampler:
                // A dedicated sampler comes after all the entries for
                // combined texture/samplers in the sampler array.
                dxRangeIndex = totalResourceRangeCount + combinedRangeCount + samplerRangeCounter;
                samplerRangeCounter++;
                break;

            case DescriptorSlotType::CombinedImageSampler:
                // Combined descriptors take entries at the front of
                // the resource and sampler arrays.
                dxRangeIndex = combinedRangeCounter;
                dxPairedSamplerRangeIndex = totalResourceRangeCount + combinedRangeCounter;
                combinedRangeCounter++;
                break;
            }

            D3D12_DESCRIPTOR_RANGE& dxRange = descriptorSetLayoutImpl->m_dxRanges[dxRangeIndex];
            memset(&dxRange, 0, sizeof(dxRange));

            switch(rangeDesc.type)
            {
            default:
                // ERROR: unsupported slot type.
                break;

            case DescriptorSlotType::Sampler:
                {
                    UInt bindingIndex = samplerCounter; samplerCounter += bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(bindingIndex);
                    dxRange.RegisterSpace = UINT(bindingSpace);
                    dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                }
                break;

            case DescriptorSlotType::SampledImage:
            case DescriptorSlotType::UniformTexelBuffer:
                {
                    UInt bindingIndex = srvCounter; srvCounter += bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(bindingIndex);
                    dxRange.RegisterSpace = UINT(bindingSpace);
                    dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                }
                break;

            case DescriptorSlotType::CombinedImageSampler:
                {
                    // The combined texture/sampler case basically just
                    // does the work of both the SRV and sampler cases above.

                    {
                        // Here's the SRV logic:

                        UInt bindingIndex = srvCounter; srvCounter += bindingCount;

                        dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                        dxRange.NumDescriptors = UINT(bindingCount);
                        dxRange.BaseShaderRegister = UINT(bindingIndex);
                        dxRange.RegisterSpace = UINT(bindingSpace);
                        dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                    }

                    {
                        // And here we do the sampler logic at the "paired" index.
                        D3D12_DESCRIPTOR_RANGE& dxPairedSamplerRange = descriptorSetLayoutImpl->m_dxRanges[dxPairedSamplerRangeIndex];
                        memset(&dxPairedSamplerRange, 0, sizeof(dxPairedSamplerRange));

                        UInt pairedSamplerBindingIndex = srvCounter; srvCounter += bindingCount;

                        dxPairedSamplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                        dxPairedSamplerRange.NumDescriptors = UINT(bindingCount);
                        dxPairedSamplerRange.BaseShaderRegister = UINT(pairedSamplerBindingIndex);
                        dxPairedSamplerRange.RegisterSpace = UINT(bindingSpace);
                        dxPairedSamplerRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                    }

                }
                break;


            case DescriptorSlotType::InputAttachment:
            case DescriptorSlotType::StorageImage:
            case DescriptorSlotType::StorageTexelBuffer:
            case DescriptorSlotType::StorageBuffer:
            case DescriptorSlotType::DynamicStorageBuffer:
                {
                    UInt bindingIndex = uavCounter; uavCounter += bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(bindingIndex);
                    dxRange.RegisterSpace = UINT(bindingSpace);
                    dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                }
                break;

            case DescriptorSlotType::UniformBuffer:
            case DescriptorSlotType::DynamicUniformBuffer:
                {
                    UInt bindingIndex = cbvCounter; cbvCounter += bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(bindingIndex);
                    dxRange.RegisterSpace = UINT(bindingSpace);
                    dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                }
                break;
            }
        }
    }

    *outLayout = descriptorSetLayoutImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createPipelineLayout(const PipelineLayout::Desc& desc, PipelineLayout** outLayout)
{
    static const UInt kMaxRanges = 16;
    static const UInt kMaxRootParameters = 32;

    D3D12_DESCRIPTOR_RANGE ranges[kMaxRanges];
    D3D12_ROOT_PARAMETER rootParameters[kMaxRootParameters];

    UInt rangeCount = 0;
    UInt rootParameterCount = 0;

    auto descriptorSetCount = desc.descriptorSetCount;

    // We are going to make two passes over the descriptor set layouts
    // that are being used to build the pipeline layout. In the first
    // pass we will collect all the descriptor ranges that have been
    // specified, applying an offset to their register spaces as needed.
    //
    for(UInt dd = 0; dd < descriptorSetCount; ++dd)
    {
        auto& descriptorSetInfo = desc.descriptorSets[dd];
        auto descriptorSetLayout = (DescriptorSetLayoutImpl*) descriptorSetInfo.layout;

        // For now we assume that the register space used for
        // logical descriptor set #N will be space N.
        //
        // TODO: This might need to be revisited in the future because
        // a single logical descriptor set might need to encompass stuff
        // that comes from multiple spaces (e.g., if it contains an unbounded
        // array).
        //
        UInt bindingSpace   = dd;

        // Copy descriptor range information from the set layout into our
        // temporary copy (this is required because the same set layout
        // might be applied to different ranges).
        //
        // API design note: this copy step could be avoided if the D3D
        // API allowed for a "space offset" to be applied as part of
        // a descriptor-table root parameter.
        //
        for(auto setDescriptorRange : descriptorSetLayout->m_dxRanges)
        {
            auto& range = ranges[rangeCount++];
            range = setDescriptorRange;
            range.RegisterSpace = UINT(bindingSpace);

            // HACK: in order to deal with SM5.0 shaders, `u` registers
            // in `space0` need to start with a number *after* the number
            // of `SV_Target` outputs that will be used.
            //
            // TODO: This is clearly a mess, and doing this behavior here
            // means it *won't* work for SM5.1 where the restriction is
            // lifted. The only real alternative is to rely on explicit
            // register numbers (e.g., from shader reflection) but that
            // goes against the simplicity that this API layer strives for
            // (everything so far has been set up to work correctly with
            // automatic assignment of bindings).
            //
            if( range.RegisterSpace == 0
                && range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV )
            {
                range.BaseShaderRegister += UINT(desc.renderTargetCount);
            }
        }
    }

    // In our second pass, we will copy over root parameters, which
    // may end up pointing into the list of ranges from the first step.
    //
    auto rangePtr = &ranges[0];
    for(UInt dd = 0; dd < descriptorSetCount; ++dd)
    {
        auto& descriptorSetInfo = desc.descriptorSets[dd];
        auto descriptorSetLayout = (DescriptorSetLayoutImpl*) descriptorSetInfo.layout;

        // Copy root parameter information from the set layout to our
        // overall pipeline layout.
        for( auto setRootParameter : descriptorSetLayout->m_dxRootParameters )
        {
            auto& rootParameter = rootParameters[rootParameterCount++];
            rootParameter = setRootParameter;

            // In the case where this parameter is a descriptor table, it
            // needs to point into our array of ranges (with offsets applied),
            // so we will fix up those pointers here.
            //
            if(rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                rootParameter.DescriptorTable.pDescriptorRanges = rangePtr;
                rangePtr += rootParameter.DescriptorTable.NumDescriptorRanges;
            }
        }
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = UINT(rootParameterCount);
    rootSignatureDesc.pParameters = rootParameters;

    // TODO: static samplers should be reasonably easy to support...
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;

    // TODO: only set this flag if needed (requires creating root
    // signature at same time as pipeline state...).
    //
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    if( SLANG_FAILED(m_D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.writeRef(), error.writeRef())) )
    {
        fprintf(stderr, "error: D3D12SerializeRootSignature failed");
        if( error )
        {
            fprintf(stderr, ": %s\n", (const char*) error->GetBufferPointer());
        }
        return SLANG_FAIL;
    }

    ComPtr<ID3D12RootSignature> rootSignature;
    SLANG_RETURN_ON_FAIL(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature.writeRef())));


    RefPtr<PipelineLayoutImpl> pipelineLayoutImpl = new PipelineLayoutImpl();
    pipelineLayoutImpl->m_rootSignature = rootSignature;
    pipelineLayoutImpl->m_descriptorSetCount = descriptorSetCount;
    *outLayout = pipelineLayoutImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createDescriptorSet(DescriptorSetLayout* layout, DescriptorSet** outDescriptorSet)
{
    auto layoutImpl = (DescriptorSetLayoutImpl*) layout;

    RefPtr<DescriptorSetImpl> descriptorSetImpl = new DescriptorSetImpl();
    descriptorSetImpl->m_renderer = this;
    descriptorSetImpl->m_layout = layoutImpl;

    // We allocate CPU-visible descriptor tables to providing the
    // backing storage for each descriptor set. GPU-visible storage
    // will only be allocated as needed during per-frame logic in
    // order to ensure that a descriptor set it available for use
    // in rendering.
    //
    Int resourceCount = layoutImpl->m_resourceCount;
    if( resourceCount )
    {
        auto resourceHeap = &m_cpuViewHeap;
        descriptorSetImpl->m_resourceHeap = resourceHeap;
        descriptorSetImpl->m_resourceTable = resourceHeap->allocate(int(resourceCount));
        descriptorSetImpl->m_resourceObjects.SetSize(resourceCount);
    }

    Int samplerCount = layoutImpl->m_samplerCount;
    if( samplerCount )
    {
        auto samplerHeap = &m_cpuSamplerHeap;
        descriptorSetImpl->m_samplerHeap = samplerHeap;
        descriptorSetImpl->m_samplerTable = samplerHeap->allocate(int(samplerCount));
        descriptorSetImpl->m_samplerObjects.SetSize(samplerCount);
    }

    *outDescriptorSet = descriptorSetImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createGraphicsPipelineState(const GraphicsPipelineStateDesc& desc, PipelineState** outState)
{
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto programImpl = (ShaderProgramImpl*) desc.program;
    auto inputLayoutImpl = (InputLayoutImpl*) desc.inputLayout;

    // Describe and create the graphics pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.pRootSignature = pipelineLayoutImpl->m_rootSignature;

    psoDesc.VS = { programImpl->m_vertexShader.Buffer(), programImpl->m_vertexShader.Count() };
    psoDesc.PS = { programImpl->m_pixelShader .Buffer(), programImpl->m_pixelShader .Count() };

    psoDesc.InputLayout = { inputLayoutImpl->m_elements.Buffer(), UINT(inputLayoutImpl->m_elements.Count()) };
    psoDesc.PrimitiveTopologyType = m_primitiveTopologyType;

    {
        const int numRenderTargets = int(desc.renderTargetCount);

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

    psoDesc.PrimitiveTopologyType = m_primitiveTopologyType;

    ComPtr<ID3D12PipelineState> pipelineState;
    SLANG_RETURN_ON_FAIL(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.writeRef())));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_pipelineType = PipelineType::Graphics;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_pipelineState = pipelineState;
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createComputePipelineState(const ComputePipelineStateDesc& desc, PipelineState** outState)
{
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto programImpl = (ShaderProgramImpl*) desc.program;

    // Describe and create the compute pipeline state object
    D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
    computeDesc.pRootSignature = pipelineLayoutImpl->m_rootSignature;
    computeDesc.CS = { programImpl->m_computeShader.Buffer(), programImpl->m_computeShader.Count() };

    ComPtr<ID3D12PipelineState> pipelineState;
    SLANG_RETURN_ON_FAIL(m_device->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(pipelineState.writeRef())));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_pipelineType = PipelineType::Compute;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_pipelineState = pipelineState;
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

} // renderer_test
