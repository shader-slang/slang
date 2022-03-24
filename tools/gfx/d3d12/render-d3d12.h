// render-d3d12.h
#pragma once

#include "../command-encoder-com-forward.h"
#include "../d3d/d3d-swapchain.h"
#include "../mutable-shader-object.h"
#include "../renderer-shared.h"
#include "../simple-render-pass-layout.h"
#include "../transient-resource-heap-base.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-chunked-list.h"
#include "descriptor-heap-d3d12.h"
#include "resource-d3d12.h"

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <d3d12.h>
#include <dxgi1_4.h>

#ifndef __ID3D12GraphicsCommandList1_FWD_DEFINED__
// If can't find a definition of CommandList1, just use an empty definition
struct ID3D12GraphicsCommandList1
{};
#endif

namespace gfx
{
namespace d3d12
{

using namespace Slang;

// Define function pointer types for PIX library.
typedef HRESULT(WINAPI* PFN_BeginEventOnCommandList)(
    ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);
typedef HRESULT(WINAPI* PFN_EndEventOnCommandList)(ID3D12GraphicsCommandList* commandList);

static const Int kMaxNumRenderFrames = 4;
static const Int kMaxRTVCount = 8;

struct D3D12DeviceInfo
{
    void clear()
    {
        m_dxgiFactory.setNull();
        m_device.setNull();
        m_adapter.setNull();
        m_desc = {};
        m_desc1 = {};
        m_isWarp = false;
        m_isSoftware = false;
    }

    bool m_isWarp;
    bool m_isSoftware;
    ComPtr<IDXGIFactory> m_dxgiFactory;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Device5> m_device5;
    ComPtr<IDXGIAdapter> m_adapter;
    DXGI_ADAPTER_DESC m_desc;
    DXGI_ADAPTER_DESC1 m_desc1;
};

class CommandQueueImpl;
class TransientResourceHeapImpl;
class TextureResourceImpl;
class CommandBufferImpl;

class DeviceImpl : public RendererBase
{
public:
    Desc m_desc;
    D3D12DeviceExtendedDesc m_extendedDesc;

    DeviceInfo m_info;
    String m_adapterName;

    bool m_isInitialized = false;

    ComPtr<ID3D12Debug> m_dxDebug;

    D3D12DeviceInfo m_deviceInfo;
    ID3D12Device* m_device = nullptr;
    ID3D12Device5* m_device5 = nullptr;

    VirtualObjectPool m_queueIndexAllocator;

    RefPtr<CommandQueueImpl> m_resourceCommandQueue;
    RefPtr<TransientResourceHeapImpl> m_resourceCommandTransientHeap;

    RefPtr<D3D12GeneralExpandingDescriptorHeap> m_rtvAllocator;
    RefPtr<D3D12GeneralExpandingDescriptorHeap> m_dsvAllocator;

    // Space in the GPU-visible heaps is precious, so we will also keep
    // around CPU-visible heaps for storing shader-objects' descriptors in a format
    // that is ready for copying into the GPU-visible heaps as needed.
    //
    RefPtr<D3D12GeneralExpandingDescriptorHeap> m_cpuViewHeap; ///< Cbv, Srv, Uav
    RefPtr<D3D12GeneralExpandingDescriptorHeap> m_cpuSamplerHeap; ///< Heap for samplers

    // Dll entry points
    PFN_D3D12_GET_DEBUG_INTERFACE m_D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE m_D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE m_D3D12SerializeRootSignature = nullptr;

    PFN_BeginEventOnCommandList m_BeginEventOnCommandList = nullptr;
    PFN_EndEventOnCommandList m_EndEventOnCommandList = nullptr;

    bool m_nvapi = false;

    // Command signatures required for indirect draws. These indicate the format of the indirect
    // as well as the command type to be used (DrawInstanced and DrawIndexedInstanced, in this
    // case).
    ComPtr<ID3D12CommandSignature> drawIndirectCmdSignature;
    ComPtr<ID3D12CommandSignature> drawIndexedIndirectCmdSignature;
    ComPtr<ID3D12CommandSignature> dispatchIndirectCmdSignature;

public:
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc, ITransientResourceHeap** outHeap) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureAllocationInfo(
        const ITextureResource::Desc& desc, size_t* outSize, size_t* outAlignment) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureRowAlignment(size_t* outAlignment) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureFromNativeHandle(
        InteropHandle handle,
        const ITextureResource::Desc& srcDesc,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromNativeHandle(
        InteropHandle handle,
        const IBufferResource::Desc& srcDesc,
        IBufferResource** outResource) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture,
        IResourceView::Desc const& desc,
        IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer,
        IBufferResource* counterBuffer,
        IResourceView::Desc const& desc,
        IResourceView** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFrameBuffer) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        IFramebufferLayout::Desc const& desc, IFramebufferLayout** outLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc, IRenderPassLayout** outRenderPassLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createInputLayout(IInputLayout::Desc const& desc, IInputLayout** outLayout) override;

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout, ShaderObjectLayoutBase** outLayout) override;
    virtual Result createShaderObject(
        ShaderObjectLayoutBase* layout, IShaderObject** outObject) override;
    virtual Result createMutableShaderObject(
        ShaderObjectLayoutBase* layout, IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createMutableRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outShaderTable) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createProgram(
        const IShaderProgram::Desc& desc,
        IShaderProgram** outProgram,
        ISlangBlob** outDiagnostics) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outState) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFence(const IFence::Desc& desc, IFence** outFence) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFences(
        uint32_t fenceCount,
        IFence** fences,
        uint64_t* fenceValues,
        bool waitForAll,
        uint64_t timeout) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* resource,
        ResourceState state,
        ISlangBlob** outBlob,
        size_t* outRowPitch,
        size_t* outPixelSize) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* resource, size_t offset, size_t size, ISlangBlob** outBlob) override;

    virtual SLANG_NO_THROW const gfx::DeviceInfo& SLANG_MCALL getDeviceInfo() const override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeDeviceHandles(InteropHandles* outHandles) override;

    ~DeviceImpl();

    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc, IAccelerationStructure** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRayTracingPipelineState(
        const RayTracingPipelineStateDesc& desc, IPipelineState** outState) override;

public:
    static PROC loadProc(HMODULE module, char const* name);

    Result createCommandQueueImpl(CommandQueueImpl** outQueue);

    Result createTransientResourceHeapImpl(
        ITransientResourceHeap::Flags::Enum flags,
        size_t constantBufferSize,
        uint32_t viewDescriptors,
        uint32_t samplerDescriptors,
        TransientResourceHeapImpl** outHeap);

    Result createBuffer(
        const D3D12_RESOURCE_DESC& resourceDesc,
        const void* srcData,
        size_t srcDataSize,
        D3D12_RESOURCE_STATES finalState,
        D3D12Resource& resourceOut,
        bool isShared,
        MemoryType access = MemoryType::DeviceLocal);

    Result captureTextureToSurface(
        TextureResourceImpl* resource,
        ResourceState state,
        ISlangBlob** blob,
        size_t* outRowPitch,
        size_t* outPixelSize);

    Result _createDevice(
        DeviceCheckFlags deviceCheckFlags,
        const UnownedStringSlice& nameMatch,
        D3D_FEATURE_LEVEL featureLevel,
        D3D12DeviceInfo& outDeviceInfo);

    struct ResourceCommandRecordInfo
    {
        ComPtr<ICommandBuffer> commandBuffer;
        ID3D12GraphicsCommandList* d3dCommandList;
    };
    ResourceCommandRecordInfo encodeResourceCommands();
    void submitResourceCommandsAndWait(const ResourceCommandRecordInfo& info);
};

class BufferResourceImpl : public gfx::BufferResource
{
public:
    typedef BufferResource Parent;

    BufferResourceImpl(const Desc& desc);

    ~BufferResourceImpl();

    D3D12Resource m_resource; ///< The resource in gpu memory, allocated on the correct heap
                              ///< relative to the cpu access flag

    D3D12_RESOURCE_STATES m_defaultState;

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeResourceHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        map(MemoryRange* rangeToRead, void** outPointer) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL unmap(MemoryRange* writtenRange) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
};

class TextureResourceImpl : public TextureResource
{
public:
    typedef TextureResource Parent;

    TextureResourceImpl(const Desc& desc);

    ~TextureResourceImpl();

    D3D12Resource m_resource;
    D3D12_RESOURCE_STATES m_defaultState;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeResourceHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) override;
};

class SamplerStateImpl : public SamplerStateBase
{
public:
    D3D12Descriptor m_descriptor;
    RefPtr<D3D12GeneralExpandingDescriptorHeap> m_allocator;
    ~SamplerStateImpl();
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class ResourceViewInternalImpl
{
public:
    D3D12Descriptor m_descriptor;
    RefPtr<D3D12GeneralExpandingDescriptorHeap> m_allocator;
    ~ResourceViewInternalImpl();
};

class ResourceViewImpl
    : public ResourceViewBase
    , public ResourceViewInternalImpl
{
public:
    Slang::RefPtr<Resource> m_resource;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

class FramebufferLayoutImpl : public FramebufferLayoutBase
{
public:
    ShortList<IFramebufferLayout::AttachmentLayout> m_renderTargets;
    bool m_hasDepthStencil = false;
    IFramebufferLayout::AttachmentLayout m_depthStencil;
};

class FramebufferImpl : public FramebufferBase
{
public:
    ShortList<RefPtr<ResourceViewImpl>> renderTargetViews;
    RefPtr<ResourceViewImpl> depthStencilView;
    ShortList<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
    struct Color4f
    {
        float values[4];
    };
    ShortList<Color4f> renderTargetClearValues;
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor;
    DepthStencilClearValue depthStencilClearValue;
};

class RenderPassLayoutImpl : public SimpleRenderPassLayout
{
public:
    RefPtr<FramebufferLayoutImpl> m_framebufferLayout;
    void init(const IRenderPassLayout::Desc& desc);
};

class InputLayoutImpl : public InputLayoutBase
{
public:
    List<D3D12_INPUT_ELEMENT_DESC> m_elements;
    List<UINT> m_vertexStreamStrides;
    List<char> m_text; ///< Holds all strings to keep in scope
};

class PipelineStateImpl : public PipelineStateBase
{
public:
    PipelineStateImpl(DeviceImpl* device)
        : m_device(device)
    {}
    DeviceImpl* m_device;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    void init(const GraphicsPipelineStateDesc& inDesc);
    void init(const ComputePipelineStateDesc& inDesc);
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
    virtual Result ensureAPIPipelineStateCreated() override;
};

#if SLANG_GFX_HAS_DXR_SUPPORT
class RayTracingPipelineStateImpl : public PipelineStateBase
{
public:
    ComPtr<ID3D12StateObject> m_stateObject;
    DeviceImpl* m_device;
    RayTracingPipelineStateImpl(DeviceImpl* device);
    void init(const RayTracingPipelineStateDesc& inDesc);
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
    virtual Result ensureAPIPipelineStateCreated() override;
};
#endif

class QueryPoolImpl : public QueryPoolBase
{
public:
    Result init(const IQueryPool::Desc& desc, DeviceImpl* device);

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getResult(SlangInt queryIndex, SlangInt count, uint64_t* data) override;

    void writeTimestamp(ID3D12GraphicsCommandList* cmdList, SlangInt index);

public:
    D3D12_QUERY_TYPE m_queryType;
    ComPtr<ID3D12QueryHeap> m_queryHeap;
    D3D12Resource m_readBackBuffer;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12Fence> m_fence;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    HANDLE m_waitEvent;
    UINT64 m_eventValue = 0;
};

/// Implements the IQueryPool interface with a plain buffer.
/// Used for query types that does not correspond to a D3D query,
/// such as ray-tracing acceleration structure post-build info.
class PlainBufferProxyQueryPoolImpl : public QueryPoolBase
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    IQueryPool* getInterface(const Guid& guid);

public:
    Result init(const IQueryPool::Desc& desc, DeviceImpl* device, uint32_t stride);

    virtual SLANG_NO_THROW Result SLANG_MCALL reset() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getResult(SlangInt queryIndex, SlangInt count, uint64_t* data) override;

public:
    QueryType m_queryType;
    RefPtr<BufferResourceImpl> m_bufferResource;
    RefPtr<DeviceImpl> m_device;
    List<uint8_t> m_result;
    bool m_resultDirty = true;
    uint32_t m_stride = 0;
    uint32_t m_count = 0;
};

struct BoundVertexBuffer
{
    RefPtr<BufferResourceImpl> m_buffer;
    int m_offset;
};

class TransientResourceHeapImpl
    : public TransientResourceHeapBaseImpl<DeviceImpl, BufferResourceImpl>
    , public ID3D12TransientResourceHeap
{
private:
    typedef TransientResourceHeapBaseImpl<DeviceImpl, BufferResourceImpl> Super;

public:
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    List<ComPtr<ID3D12GraphicsCommandList>> m_d3dCommandListPool;
    List<RefPtr<RefObject>> m_commandBufferPool;
    uint32_t m_commandListAllocId = 0;
    // Wait values for each command queue.
    struct QueueWaitInfo
    {
        uint64_t waitValue;
        HANDLE fenceEvent;
        ComPtr<ID3D12CommandQueue> queue;
        ComPtr<ID3D12Fence> fence = nullptr;
    };
    ShortList<QueueWaitInfo, 4> m_waitInfos;
    ShortList<HANDLE, 4> m_waitHandles;

    QueueWaitInfo& getQueueWaitInfo(uint32_t queueIndex);
    // During command submission, we need all the descriptor tables that get
    // used to come from a single heap (for each descriptor heap type).
    //
    // We will thus keep a single heap of each type that we hope will hold
    // all the descriptors that actually get needed in a frame.
    ShortList<D3D12DescriptorHeap, 4> m_viewHeaps; // Cbv, Srv, Uav
    ShortList<D3D12DescriptorHeap, 4> m_samplerHeaps; // Heap for samplers
    int32_t m_currentViewHeapIndex = -1;
    int32_t m_currentSamplerHeapIndex = -1;
    bool m_canResize = false;

    uint32_t m_viewHeapSize;
    uint32_t m_samplerHeapSize;

    D3D12DescriptorHeap& getCurrentViewHeap();
    D3D12DescriptorHeap& getCurrentSamplerHeap();

    D3D12LinearExpandingDescriptorHeap m_stagingCpuViewHeap;
    D3D12LinearExpandingDescriptorHeap m_stagingCpuSamplerHeap;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        queryInterface(SlangUUID const& uuid, void** outObject) override;

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return Super::addRef(); }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return Super::release(); }

    virtual SLANG_NO_THROW Result SLANG_MCALL allocateTransientDescriptorTable(
        DescriptorType type,
        uint32_t count,
        uint64_t& outDescriptorOffset,
        void** outD3DDescriptorHeapHandle) override;

    ~TransientResourceHeapImpl();

    bool canResize() { return m_canResize; }

    Result init(
        const ITransientResourceHeap::Desc& desc,
        DeviceImpl* device,
        uint32_t viewHeapSize,
        uint32_t samplerHeapSize);

    Result allocateNewViewDescriptorHeap(DeviceImpl* device);

    Result allocateNewSamplerDescriptorHeap(DeviceImpl* device);

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL finish() override;
};

struct Submitter
{
    virtual void setRootConstantBufferView(
        int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) = 0;
    virtual void setRootUAV(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) = 0;
    virtual void setRootSRV(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) = 0;
    virtual void setRootDescriptorTable(int index, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
    virtual void setRootSignature(ID3D12RootSignature* rootSignature) = 0;
    virtual void setRootConstants(
        Index rootParamIndex,
        Index dstOffsetIn32BitValues,
        Index countOf32BitValues,
        void const* srcData) = 0;
    virtual void setPipelineState(PipelineStateBase* pipelineState) = 0;
};

class PipelineCommandEncoder
{
public:
    bool m_isOpen = false;
    bool m_bindingDirty = true;
    CommandBufferImpl* m_commandBuffer;
    TransientResourceHeapImpl* m_transientHeap;
    DeviceImpl* m_renderer;
    ID3D12Device* m_device;
    ID3D12GraphicsCommandList* m_d3dCmdList;
    ID3D12GraphicsCommandList* m_preCmdList = nullptr;

    RefPtr<PipelineStateBase> m_currentPipeline;

    static int getBindPointIndex(PipelineType type);

    void init(CommandBufferImpl* commandBuffer);

    void endEncodingImpl() { m_isOpen = false; }

    Result bindPipelineImpl(IPipelineState* pipelineState, IShaderObject** outRootObject);

    Result bindPipelineWithRootObjectImpl(IPipelineState* pipelineState, IShaderObject* rootObject);

    /// Specializes the pipeline according to current root-object argument values,
    /// applys the root object bindings and binds the pipeline state.
    /// The newly specialized pipeline is held alive by the pipeline cache so users of
    /// `newPipeline` do not need to maintain its lifespan.
    Result _bindRenderState(Submitter* submitter, RefPtr<PipelineStateBase>& newPipeline);
};

struct DescriptorTable
{
    DescriptorHeapReference m_heap;
    uint32_t m_offset = 0;
    uint32_t m_count = 0;

    SLANG_FORCE_INLINE uint32_t getDescriptorCount() const { return m_count; }

    /// Get the GPU handle at the specified index
    SLANG_FORCE_INLINE D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle(uint32_t index = 0) const
    {
        SLANG_ASSERT(index < getDescriptorCount());
        return m_heap.getGpuHandle(m_offset + index);
    }

    /// Get the CPU handle at the specified index
    SLANG_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(uint32_t index = 0) const
    {
        SLANG_ASSERT(index < getDescriptorCount());
        return m_heap.getCpuHandle(m_offset + index);
    }

    void freeIfSupported()
    {
        if (m_count)
        {
            m_heap.freeIfSupported(m_offset, m_count);
            m_offset = 0;
            m_count = 0;
        }
    }

    bool allocate(uint32_t count)
    {
        auto allocatedOffset = m_heap.allocate(count);
        if (allocatedOffset == -1)
            return false;
        m_offset = allocatedOffset;
        m_count = count;
        return true;
    }

    bool allocate(DescriptorHeapReference heap, uint32_t count)
    {
        auto allocatedOffset = heap.allocate(count);
        if (allocatedOffset == -1)
            return false;
        m_heap = heap;
        m_offset = allocatedOffset;
        m_count = count;
        return true;
    }
};

/// Contextual data and operations required when binding shader objects to the pipeline state
struct BindingContext
{
    PipelineCommandEncoder* encoder;
    Submitter* submitter;
    TransientResourceHeapImpl* transientHeap;
    DeviceImpl* device;
    D3D12_DESCRIPTOR_HEAP_TYPE
    outOfMemoryHeap; // The type of descriptor heap that is OOM during binding.
};

/// A representation of the offset at which to bind a shader parameter or sub-object
struct BindingOffset
{
    // Note: When we actually bind a shader object to the pipeline we do not care about
    // HLSL-specific notions like `t` registers and `space`s. Those concepts are all
    // mediated by the root signature.
    //
    // Instead, we need to consider the offsets at which the object will be bound
    // into the actual D3D12 API state, which consists of the index of the current
    // root parameter to bind from, as well as indices into the current descriptor
    // tables (for resource views and samplers).

    uint32_t rootParam = 0;
    uint32_t resource = 0;
    uint32_t sampler = 0;

    void operator+=(BindingOffset const& offset)
    {
        rootParam += offset.rootParam;
        resource += offset.resource;
        sampler += offset.sampler;
    }
};

/// A reprsentation of an allocated descriptor set, consisting of an option resource table and
/// an optional sampler table
struct DescriptorSet
{
    DescriptorTable resourceTable;
    DescriptorTable samplerTable;

    void freeIfSupported()
    {
        resourceTable.freeIfSupported();
        samplerTable.freeIfSupported();
    }
};

// Provides information on how binding ranges are stored in descriptor tables for
// a shader object.
// We allocate one CPU descriptor table for each descriptor heap type for the shader
// object. In `ShaderObjectLayoutImpl`, we store the offset into the descriptor tables
// for each binding, so we know where to write the descriptor when the user sets
// a resource or sampler binding.
class ShaderObjectLayoutImpl : public ShaderObjectLayoutBase
{
public:
    /// Information about a single logical binding range
    struct BindingRangeInfo
    {
        // Some of the information we store on binding ranges is redundant with
        // the information that Slang's reflection information stores, but having
        // it here can make the code more compact and obvious.

        /// The type of binding in this range.
        slang::BindingType bindingType;

        /// The shape of the resource
        SlangResourceShape resourceShape;

        /// The number of distinct bindings in this range.
        uint32_t count;

        /// A "flat" index for this range in whatever array provides backing storage for it
        uint32_t baseIndex;

        /// An index into the sub-object array if this binding range is treated
        /// as a sub-object.
        uint32_t subObjectIndex;

        bool isRootParameter;
    };

    /// Offset information for a sub-object range
    struct SubObjectRangeOffset : BindingOffset
    {
        SubObjectRangeOffset() {}

        SubObjectRangeOffset(slang::VariableLayoutReflection* varLayout);

        /// The offset for "pending" ordinary data related to this range
        uint32_t pendingOrdinaryData = 0;
    };

    /// Stride information for a sub-object range
    struct SubObjectRangeStride : BindingOffset
    {
        SubObjectRangeStride() {}

        SubObjectRangeStride(slang::TypeLayoutReflection* typeLayout);

        /// The strid for "pending" ordinary data related to this range
        uint32_t pendingOrdinaryData = 0;
    };

    /// Information about a sub-objecrt range
    struct SubObjectRangeInfo
    {
        /// The index of the binding range corresponding to this sub-object range
        Index bindingRangeIndex = 0;

        /// Layout information for the type of sub-object expected to be bound, if known
        RefPtr<ShaderObjectLayoutImpl> layout;

        /// The offset to use when binding the first object in this range
        SubObjectRangeOffset offset;

        /// Stride between consecutive objects in this range
        SubObjectRangeStride stride;
    };

    struct RootParameterInfo
    {
        IResourceView::Type type;
    };

    static bool isBindingRangeRootParameter(
        SlangSession* globalSession,
        const char* rootParameterAttributeName,
        slang::TypeLayoutReflection* typeLayout,
        Index bindingRangeIndex);

    struct Builder
    {
    public:
        Builder(RendererBase* renderer)
            : m_renderer(renderer)
        {}

        RendererBase* m_renderer;
        slang::TypeLayoutReflection* m_elementTypeLayout;
        List<BindingRangeInfo> m_bindingRanges;
        List<SubObjectRangeInfo> m_subObjectRanges;
        List<RootParameterInfo> m_rootParamsInfo;

        /// The number of sub-objects (not just sub-object *ranges*) stored in instances of this
        /// layout
        uint32_t m_subObjectCount = 0;

        /// Counters for the number of root parameters, resources, and samplers in this object
        /// itself
        BindingOffset m_ownCounts;

        /// Counters for the number of root parameters, resources, and sampler in this object
        /// and transitive sub-objects
        BindingOffset m_totalCounts;

        /// The number of root parameter consumed by (transitive) sub-objects
        uint32_t m_childRootParameterCount = 0;

        /// The total size in bytes of the ordinary data for this object and transitive
        /// sub-object.
        uint32_t m_totalOrdinaryDataSize = 0;

        /// The container type of this shader object. When `m_containerType` is
        /// `StructuredBuffer` or `UnsizedArray`, this shader object represents a collection
        /// instead of a single object.
        ShaderObjectContainerType m_containerType = ShaderObjectContainerType::None;

        Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout);

        Result build(ShaderObjectLayoutImpl** outLayout);
    };

    static Result createForElementType(
        RendererBase* renderer,
        slang::TypeLayoutReflection* elementType,
        ShaderObjectLayoutImpl** outLayout);

    List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

    Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

    BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

    uint32_t getResourceSlotCount() { return m_ownCounts.resource; }
    uint32_t getSamplerSlotCount() { return m_ownCounts.sampler; }
    Index getSubObjectSlotCount() { return m_subObjectCount; }
    Index getSubObjectCount() { return m_subObjectCount; }

    uint32_t getTotalResourceDescriptorCount() { return m_totalCounts.resource; }
    uint32_t getTotalSamplerDescriptorCount() { return m_totalCounts.sampler; }

    uint32_t getOrdinaryDataBufferCount() { return m_totalOrdinaryDataSize ? 1 : 0; }
    bool hasOrdinaryDataBuffer() { return m_totalOrdinaryDataSize != 0; }

    uint32_t getTotalResourceDescriptorCountWithoutOrdinaryDataBuffer()
    {
        return m_totalCounts.resource - getOrdinaryDataBufferCount();
    }

    uint32_t getOwnUserRootParameterCount() { return (uint32_t)m_rootParamsInfo.getCount(); }
    uint32_t getTotalRootTableParameterCount() { return m_totalCounts.rootParam; }
    uint32_t getChildRootParameterCount() { return m_childRootParameterCount; }

    uint32_t getTotalOrdinaryDataSize() const { return m_totalOrdinaryDataSize; }

    SubObjectRangeInfo const& getSubObjectRange(Index index) { return m_subObjectRanges[index]; }
    List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

    RendererBase* getRenderer() { return m_renderer; }

    slang::TypeReflection* getType() { return m_elementTypeLayout->getType(); }

    const RootParameterInfo& getRootParameterInfo(Index index) { return m_rootParamsInfo[index]; }

protected:
    Result init(Builder* builder);

    List<BindingRangeInfo> m_bindingRanges;
    List<SubObjectRangeInfo> m_subObjectRanges;
    List<RootParameterInfo> m_rootParamsInfo;

    BindingOffset m_ownCounts;
    BindingOffset m_totalCounts;

    uint32_t m_subObjectCount = 0;
    uint32_t m_childRootParameterCount = 0;

    uint32_t m_totalOrdinaryDataSize = 0;
};

class RootShaderObjectLayoutImpl : public ShaderObjectLayoutImpl
{
    typedef ShaderObjectLayoutImpl Super;

public:
    struct EntryPointInfo
    {
        RefPtr<ShaderObjectLayoutImpl> layout;
        BindingOffset offset;
    };

    struct Builder : Super::Builder
    {
        Builder(
            RendererBase* renderer,
            slang::IComponentType* program,
            slang::ProgramLayout* programLayout)
            : Super::Builder(renderer)
            , m_program(program)
            , m_programLayout(programLayout)
        {}

        Result build(RootShaderObjectLayoutImpl** outLayout);

        void addGlobalParams(slang::VariableLayoutReflection* globalsLayout);

        void addEntryPoint(SlangStage stage, ShaderObjectLayoutImpl* entryPointLayout);

        slang::IComponentType* m_program;
        slang::ProgramLayout* m_programLayout;
        List<EntryPointInfo> m_entryPoints;
    };

    EntryPointInfo& getEntryPoint(Index index) { return m_entryPoints[index]; }

    List<EntryPointInfo>& getEntryPoints() { return m_entryPoints; }

    struct DescriptorSetLayout
    {
        List<D3D12_DESCRIPTOR_RANGE> m_resourceRanges;
        List<D3D12_DESCRIPTOR_RANGE> m_samplerRanges;
        uint32_t m_resourceCount = 0;
        uint32_t m_samplerCount = 0;
    };

    struct RootSignatureDescBuilder
    {
        DeviceImpl* m_device;

        RootSignatureDescBuilder(DeviceImpl* device)
            : m_device(device)
        {}

        // We will use one descriptor set for the global scope and one additional
        // descriptor set for each `ParameterBlock` binding range in the shader object
        // hierarchy, regardless of the shader's `space` indices.
        List<DescriptorSetLayout> m_descriptorSets;
        List<D3D12_ROOT_PARAMETER> m_rootParameters;
        D3D12_ROOT_SIGNATURE_DESC m_rootSignatureDesc = {};

        static Result translateDescriptorRangeType(
            slang::BindingType c, D3D12_DESCRIPTOR_RANGE_TYPE* outType);

        /// Stores offset information to apply to the reflected register/space for a descriptor
        /// range.
        ///
        struct BindingRegisterOffset
        {
            uint32_t spaceOffset = 0; // The `space` index as specified in shader.

            enum
            {
                kRangeTypeCount = 4
            };

            /// An offset to apply for each D3D12 register class, as given
            /// by a `D3D12_DESCRIPTOR_RANGE_TYPE`.
            ///
            /// Note that the `D3D12_DESCRIPTOR_RANGE_TYPE` enumeration has
            /// values between 0 and 3, inclusive.
            ///
            uint32_t offsetForRangeType[kRangeTypeCount] = {0, 0, 0, 0};

            uint32_t& operator[](D3D12_DESCRIPTOR_RANGE_TYPE type)
            {
                return offsetForRangeType[int(type)];
            }

            uint32_t operator[](D3D12_DESCRIPTOR_RANGE_TYPE type) const
            {
                return offsetForRangeType[int(type)];
            }

            BindingRegisterOffset() {}

            BindingRegisterOffset(slang::VariableLayoutReflection* varLayout)
            {
                if (varLayout)
                {
                    spaceOffset =
                        (UINT)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_REGISTER_SPACE);
                    offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_CBV] =
                        (UINT)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
                    offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_SRV] =
                        (UINT)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE);
                    offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_UAV] =
                        (UINT)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS);
                    offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER] =
                        (UINT)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SAMPLER_STATE);
                }
            }

            void operator+=(BindingRegisterOffset const& other)
            {
                spaceOffset += other.spaceOffset;
                for (int i = 0; i < kRangeTypeCount; ++i)
                {
                    offsetForRangeType[i] += other.offsetForRangeType[i];
                }
            }
        };

        struct BindingRegisterOffsetPair
        {
            BindingRegisterOffset primary;
            BindingRegisterOffset pending;

            BindingRegisterOffsetPair() {}

            BindingRegisterOffsetPair(slang::VariableLayoutReflection* varLayout)
                : primary(varLayout)
                , pending(varLayout->getPendingDataLayout())
            {}

            void operator+=(BindingRegisterOffsetPair const& other)
            {
                primary += other.primary;
                pending += other.pending;
            }
        };
        /// Add a new descriptor set to the layout being computed.
        ///
        /// Note that a "descriptor set" in the layout may amount to
        /// zero, one, or two different descriptor *tables* in the
        /// final D3D12 root signature. Each descriptor set may
        /// contain zero or more view ranges (CBV/SRV/UAV) and zero
        /// or more sampler ranges. It maps to a view descriptor table
        /// if the number of view ranges is non-zero and to a sampler
        /// descriptor table if the number of sampler ranges is non-zero.
        ///
        uint32_t addDescriptorSet();

        Result addDescriptorRange(
            Index physicalDescriptorSetIndex,
            D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
            UINT registerIndex,
            UINT spaceIndex,
            UINT count,
            bool isRootParameter);
        /// Add one descriptor range as specified in Slang reflection information to the layout.
        ///
        /// The layout information is taken from `typeLayout` for the descriptor
        /// range with the given `descriptorRangeIndex` within the logical
        /// descriptor set (reflected by Slang) with the given `logicalDescriptorSetIndex`.
        ///
        /// The `physicalDescriptorSetIndex` is the index in the `m_descriptorSets` array of
        /// the descriptor set that the range should be added to.
        ///
        /// The `offset` encodes information about space and/or register offsets that
        /// should be applied to descrptor ranges.
        ///
        /// This operation can fail if the given descriptor range encodes a range that
        /// doesn't map to anything directly supported by D3D12. Higher-level routines
        /// will often want to ignore such failures.
        ///
        Result addDescriptorRange(
            slang::TypeLayoutReflection* typeLayout,
            Index physicalDescriptorSetIndex,
            BindingRegisterOffset const& containerOffset,
            BindingRegisterOffset const& elementOffset,
            Index logicalDescriptorSetIndex,
            Index descriptorRangeIndex,
            bool isRootParameter);

        /// Add one binding range to the computed layout.
        ///
        /// The layout information is taken from `typeLayout` for the binding
        /// range with the given `bindingRangeIndex`.
        ///
        /// The `physicalDescriptorSetIndex` is the index in the `m_descriptorSets` array of
        /// the descriptor set that the range should be added to.
        ///
        /// The `offset` encodes information about space and/or register offsets that
        /// should be applied to descrptor ranges.
        ///
        /// Note that a single binding range may encompass zero or more descriptor ranges.
        ///
        void addBindingRange(
            slang::TypeLayoutReflection* typeLayout,
            Index physicalDescriptorSetIndex,
            BindingRegisterOffset const& containerOffset,
            BindingRegisterOffset const& elementOffset,
            Index bindingRangeIndex);

        void addAsValue(
            slang::VariableLayoutReflection* varLayout, Index physicalDescriptorSetIndex);

        /// Add binding ranges and parameter blocks to the root signature.
        ///
        /// The layout information is taken from `typeLayout` which should
        /// be a layout for either a program or an entry point.
        ///
        /// The `physicalDescriptorSetIndex` is the index in the `m_descriptorSets` array of
        /// the descriptor set that binding ranges not belonging to nested
        /// parameter blocks should be added to.
        ///
        /// The `offset` encodes information about space and/or register offsets that
        /// should be applied to descrptor ranges.
        ///
        void addAsConstantBuffer(
            slang::TypeLayoutReflection* typeLayout,
            Index physicalDescriptorSetIndex,
            BindingRegisterOffsetPair const& containerOffset,
            BindingRegisterOffsetPair const& elementOffset);

        void addAsValue(
            slang::TypeLayoutReflection* typeLayout,
            Index physicalDescriptorSetIndex,
            BindingRegisterOffsetPair const& containerOffset,
            BindingRegisterOffsetPair const& elementOffset);

        D3D12_ROOT_SIGNATURE_DESC& build();
    };

    static Result createRootSignatureFromSlang(
        DeviceImpl* device,
        RootShaderObjectLayoutImpl* rootLayout,
        slang::IComponentType* program,
        ID3D12RootSignature** outRootSignature,
        ID3DBlob** outError);

    static Result create(
        DeviceImpl* device,
        slang::IComponentType* program,
        slang::ProgramLayout* programLayout,
        RootShaderObjectLayoutImpl** outLayout,
        ID3DBlob** outError);

    slang::IComponentType* getSlangProgram() const { return m_program; }
    slang::ProgramLayout* getSlangProgramLayout() const { return m_programLayout; }

protected:
    Result init(Builder* builder);

    ComPtr<slang::IComponentType> m_program;
    slang::ProgramLayout* m_programLayout = nullptr;

    List<EntryPointInfo> m_entryPoints;

public:
    ComPtr<ID3D12RootSignature> m_rootSignature;
};

struct ShaderBinary
{
    SlangStage stage;
    slang::EntryPointReflection* entryPointInfo;
    String actualEntryPointNameInAPI;
    List<uint8_t> code;
};

class ShaderProgramImpl : public ShaderProgramBase
{
public:
    RefPtr<RootShaderObjectLayoutImpl> m_rootObjectLayout;
    List<ShaderBinary> m_shaders;

    virtual Result createShaderModule(
        slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode) override;
};

class ShaderObjectImpl
    : public ShaderObjectBaseImpl<ShaderObjectImpl, ShaderObjectLayoutImpl, SimpleShaderObjectData>
{
    typedef ShaderObjectBaseImpl<ShaderObjectImpl, ShaderObjectLayoutImpl, SimpleShaderObjectData>
        Super;

public:
    static Result create(
        DeviceImpl* device, ShaderObjectLayoutImpl* layout, ShaderObjectImpl** outShaderObject);

    ~ShaderObjectImpl();

    RendererBase* getDevice() { return m_device.get(); }

    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** outEntryPoint) override;

    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() override;

    virtual SLANG_NO_THROW size_t SLANG_MCALL getSize() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& inOffset, void const* data, size_t inSize) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setObject(ShaderOffset const& offset, IShaderObject* object) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) override;

protected:
    Result init(
        DeviceImpl* device,
        ShaderObjectLayoutImpl* layout,
        DescriptorHeapReference viewHeap,
        DescriptorHeapReference samplerHeap);

    /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given
    /// `offset`
    Result _writeOrdinaryData(
        PipelineCommandEncoder* encoder,
        BufferResourceImpl* buffer,
        size_t offset,
        size_t destSize,
        ShaderObjectLayoutImpl* specializedLayout);

    bool shouldAllocateConstantBuffer(TransientResourceHeapImpl* transientHeap);

    /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
    Result _ensureOrdinaryDataBufferCreatedIfNeeded(
        PipelineCommandEncoder* encoder, ShaderObjectLayoutImpl* specializedLayout);

public:
    void updateSubObjectsRecursive();
    /// Prepare to bind this object as a parameter block.
    ///
    /// This involves allocating and binding any descriptor tables necessary
    /// to to store the state of the object. The function returns a descriptor
    /// set formed from any table(s) allocated. In addition, the `ioOffset`
    /// parameter will be adjusted to be correct for binding values into
    /// the resulting descriptor set.
    ///
    /// Returns:
    ///   SLANG_OK when successful,
    ///   SLANG_E_OUT_OF_MEMORY when descriptor heap is full.
    ///
    Result prepareToBindAsParameterBlock(
        BindingContext* context,
        BindingOffset& ioOffset,
        ShaderObjectLayoutImpl* specializedLayout,
        DescriptorSet& outDescriptorSet);

    bool checkIfCachedDescriptorSetIsValidRecursive(BindingContext* context);

    /// Bind this object as a `ParameterBlock<X>`
    Result bindAsParameterBlock(
        BindingContext* context,
        BindingOffset const& offset,
        ShaderObjectLayoutImpl* specializedLayout);

    /// Bind this object as a `ConstantBuffer<X>`
    Result bindAsConstantBuffer(
        BindingContext* context,
        DescriptorSet const& descriptorSet,
        BindingOffset const& offset,
        ShaderObjectLayoutImpl* specializedLayout);

    /// Bind this object as a value (for an interface-type parameter)
    Result bindAsValue(
        BindingContext* context,
        DescriptorSet const& descriptorSet,
        BindingOffset const& offset,
        ShaderObjectLayoutImpl* specializedLayout);

    /// Shared logic for `bindAsConstantBuffer()` and `bindAsValue()`
    Result _bindImpl(
        BindingContext* context,
        DescriptorSet const& descriptorSet,
        BindingOffset const& offset,
        ShaderObjectLayoutImpl* specializedLayout);

    Result bindRootArguments(BindingContext* context, uint32_t& index);
    /// A CPU-memory descriptor set holding any descriptors used to represent the
    /// resources/samplers in this object's state
    DescriptorSet m_descriptorSet;
    /// A cached descriptor set on GPU heap.
    DescriptorSet m_cachedGPUDescriptorSet;

    ShortList<RefPtr<Resource>, 8> m_boundResources;
    List<D3D12_GPU_VIRTUAL_ADDRESS> m_rootArguments;
    /// A constant buffer used to stored ordinary data for this object
    /// and existential-type sub-objects.
    ///
    /// Allocated from transient heap on demand with `_createOrdinaryDataBufferIfNeeded()`
    IBufferResource* m_constantBufferWeakPtr = nullptr;
    size_t m_constantBufferOffset = 0;
    size_t m_constantBufferSize = 0;

    /// Dirty bit tracking whether the constant buffer needs to be updated.
    bool m_isConstantBufferDirty = true;
    /// The transient heap from which the constant buffer and descriptor set is allocated.
    TransientResourceHeapImpl* m_cachedTransientHeap;
    /// The version of the transient heap when the constant buffer and descriptor set is
    /// allocated.
    uint64_t m_cachedTransientHeapVersion;

    /// Whether this shader object is allowed to be mutable.
    bool m_isMutable = false;
    /// The version of a mutable shader object.
    uint32_t m_version = 0;
    /// The version of this mutable shader object when the gpu descriptor table is cached.
    uint32_t m_cachedGPUDescriptorSetVersion = -1;
    /// The versions of bound subobjects.
    List<uint32_t> m_subObjectVersions;

    /// Get the layout of this shader object with specialization arguments considered
    ///
    /// This operation should only be called after the shader object has been
    /// fully filled in and finalized.
    ///
    Result getSpecializedLayout(ShaderObjectLayoutImpl** outLayout);

    /// Create the layout for this shader object with specialization arguments considered
    ///
    /// This operation is virtual so that it can be customized by `RootShaderObject`.
    ///
    virtual Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout);

    RefPtr<ShaderObjectLayoutImpl> m_specializedLayout;
};

class RootShaderObjectImpl : public ShaderObjectImpl
{
    typedef ShaderObjectImpl Super;

public:
    // Override default reference counting behavior to disable lifetime management via ComPtr.
    // Root objects are managed by command buffer and does not need to be freed by the user.
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }

public:
    RootShaderObjectLayoutImpl* getLayout();

    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** outEntryPoint) override;
    virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        copyFrom(IShaderObject* object, ITransientResourceHeap* transientHeap) override;

public:
    Result bindAsRoot(BindingContext* context, RootShaderObjectLayoutImpl* specializedLayout);

public:
    Result init(DeviceImpl* device) { return SLANG_OK; }

    Result resetImpl(
        DeviceImpl* device,
        RootShaderObjectLayoutImpl* layout,
        DescriptorHeapReference viewHeap,
        DescriptorHeapReference samplerHeap,
        bool isMutable);

    Result reset(
        DeviceImpl* device, RootShaderObjectLayoutImpl* layout, TransientResourceHeapImpl* heap);

protected:
    virtual Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout) override;

    List<RefPtr<ShaderObjectImpl>> m_entryPoints;
};

class MutableRootShaderObjectImpl : public RootShaderObjectImpl
{
public:
    // Override default reference counting behavior to disable lifetime management via ComPtr.
    // Root objects are managed by command buffer and does not need to be freed by the user.
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return ShaderObjectBase::addRef(); }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return ShaderObjectBase::release(); }
};

class ShaderTableImpl : public ShaderTableBase
{
public:
    uint32_t m_rayGenTableOffset;
    uint32_t m_missTableOffset;
    uint32_t m_hitGroupTableOffset;

    DeviceImpl* m_device;

    virtual RefPtr<BufferResource> createDeviceBuffer(
        PipelineStateBase* pipeline,
        TransientResourceHeapBase* transientHeap,
        IResourceCommandEncoder* encoder) override;
};

class ResourceCommandEncoderImpl
    : public IResourceCommandEncoder
    , public PipelineCommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data) override;
    virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(
        size_t count,
        ITextureResource* const* textures,
        ResourceState src,
        ResourceState dst) override;
    virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(
        size_t count,
        IBufferResource* const* buffers,
        ResourceState src,
        ResourceState dst) override;
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() {}
    virtual SLANG_NO_THROW void SLANG_MCALL
        writeTimestamp(IQueryPool* pool, SlangInt index) override;
    virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(
        ITextureResource* dst,
        ResourceState dstState,
        SubresourceRange dstSubresource,
        ITextureResource::Offset3D dstOffset,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Size extent) override;

    virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(
        ITextureResource* dst,
        SubresourceRange subResourceRange,
        ITextureResource::Offset3D offset,
        ITextureResource::Size extent,
        ITextureResource::SubresourceData* subResourceData,
        size_t subResourceDataCount) override;

    virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(
        IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags) override;

    virtual SLANG_NO_THROW void SLANG_MCALL resolveResource(
        ITextureResource* source,
        ResourceState sourceState,
        SubresourceRange sourceRange,
        ITextureResource* dest,
        ResourceState destState,
        SubresourceRange destRange) override;

    virtual SLANG_NO_THROW void SLANG_MCALL resolveQuery(
        IQueryPool* queryPool,
        uint32_t index,
        uint32_t count,
        IBufferResource* buffer,
        uint64_t offset) override;

    virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        size_t dstSize,
        size_t dstRowStride,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Size extent) override;

    virtual SLANG_NO_THROW void SLANG_MCALL textureSubresourceBarrier(
        ITextureResource* texture,
        SubresourceRange subresourceRange,
        ResourceState src,
        ResourceState dst) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        beginDebugEvent(const char* name, float rgbColor[3]) override;
    virtual SLANG_NO_THROW void SLANG_MCALL endDebugEvent() override;
};

class ComputeCommandEncoderImpl
    : public IComputeCommandEncoder
    , public ResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoderImpl)
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;
    void init(
        DeviceImpl* renderer,
        TransientResourceHeapImpl* transientHeap,
        CommandBufferImpl* cmdBuffer);

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override;

    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        dispatchComputeIndirect(IBufferResource* argBuffer, uint64_t offset) override;
};
class RenderCommandEncoderImpl
    : public IRenderCommandEncoder
    , public ResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoderImpl)
public:
    RefPtr<RenderPassLayoutImpl> m_renderPass;
    RefPtr<FramebufferImpl> m_framebuffer;

    List<BoundVertexBuffer> m_boundVertexBuffers;

    RefPtr<BufferResourceImpl> m_boundIndexBuffer;

    D3D12_VIEWPORT m_viewports[kMaxRTVCount];
    D3D12_RECT m_scissorRects[kMaxRTVCount];

    DXGI_FORMAT m_boundIndexFormat;
    UINT m_boundIndexOffset;

    D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveTopologyType;
    D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology;

    void init(
        DeviceImpl* renderer,
        TransientResourceHeapImpl* transientHeap,
        CommandBufferImpl* cmdBuffer,
        RenderPassLayoutImpl* renderPass,
        FramebufferImpl* framebuffer);

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(uint32_t count, const Viewport* viewports) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(uint32_t count, const ScissorRect* rects) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        uint32_t startSlot,
        uint32_t slotCount,
        IBufferResource* const* buffers,
        const uint32_t* offsets) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, uint32_t offset = 0) override;

    void prepareDraw();
    virtual SLANG_NO_THROW void SLANG_MCALL
        draw(uint32_t vertexCount, uint32_t startVertex = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(uint32_t indexCount, uint32_t startIndex = 0, uint32_t baseVertex = 0) override;
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override;

    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) override;

    virtual SLANG_NO_THROW void SLANG_MCALL drawIndirect(
        uint32_t maxDrawCount,
        IBufferResource* argBuffer,
        uint64_t argOffset,
        IBufferResource* countBuffer,
        uint64_t countOffset) override;

    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedIndirect(
        uint32_t maxDrawCount,
        IBufferResource* argBuffer,
        uint64_t argOffset,
        IBufferResource* countBuffer,
        uint64_t countOffset) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setSamplePositions(
        uint32_t samplesPerPixel,
        uint32_t pixelCount,
        const SamplePosition* samplePositions) override;

    virtual SLANG_NO_THROW void SLANG_MCALL drawInstanced(
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t startVertex,
        uint32_t startInstanceLocation) override;

    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedInstanced(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t startIndexLocation,
        int32_t baseVertexLocation,
        uint32_t startInstanceLocation) override;
};

#if SLANG_GFX_HAS_DXR_SUPPORT
class RayTracingCommandEncoderImpl
    : public IRayTracingCommandEncoder
    , public ResourceCommandEncoderImpl
{
public:
    SLANG_GFX_FORWARD_RESOURCE_COMMAND_ENCODER_IMPL(ResourceCommandEncoderImpl)
public:
    virtual SLANG_NO_THROW void SLANG_MCALL buildAccelerationStructure(
        const IAccelerationStructure::BuildDesc& desc,
        int propertyQueryCount,
        AccelerationStructureQueryDesc* queryDescs) override;
    virtual SLANG_NO_THROW void SLANG_MCALL copyAccelerationStructure(
        IAccelerationStructure* dest,
        IAccelerationStructure* src,
        AccelerationStructureCopyMode mode) override;
    virtual SLANG_NO_THROW void SLANG_MCALL queryAccelerationStructureProperties(
        int accelerationStructureCount,
        IAccelerationStructure* const* accelerationStructures,
        int queryCount,
        AccelerationStructureQueryDesc* queryDescs) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        serializeAccelerationStructure(DeviceAddress dest, IAccelerationStructure* source) override;
    virtual SLANG_NO_THROW void SLANG_MCALL deserializeAccelerationStructure(
        IAccelerationStructure* dest, DeviceAddress source) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) override
    {
        return bindPipelineWithRootObjectImpl(state, rootObject);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchRays(
        uint32_t rayGenShaderIndex,
        IShaderTable* shaderTable,
        int32_t width,
        int32_t height,
        int32_t depth) override;
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() {}
};
#endif

class CommandBufferImpl
    : public ICommandBuffer
    , public ComObject
{
public:
    // There are a pair of cyclic references between a `TransientResourceHeap` and
    // a `CommandBuffer` created from the heap. We need to break the cycle upon
    // the public reference count of a command buffer dropping to 0.
    SLANG_COM_OBJECT_IUNKNOWN_ALL

    ICommandBuffer* getInterface(const Guid& guid);
    virtual void comFree() override { m_transientHeap.breakStrongReference(); }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* handle) override;

public:
    ComPtr<ID3D12GraphicsCommandList> m_cmdList;
    ComPtr<ID3D12GraphicsCommandList1> m_cmdList1;
    ComPtr<ID3D12GraphicsCommandList4> m_cmdList4;

    BreakableReference<TransientResourceHeapImpl> m_transientHeap;
    // Weak reference is fine here since `m_transientHeap` already holds strong reference to
    // device.
    DeviceImpl* m_renderer;
    RootShaderObjectImpl m_rootShaderObject;
    RefPtr<MutableRootShaderObjectImpl> m_mutableRootShaderObject;
    bool m_descriptorHeapsBound = false;

    void bindDescriptorHeaps();

    void invalidateDescriptorHeapBinding() { m_descriptorHeapsBound = false; }

    void reinit();

    void init(
        DeviceImpl* renderer,
        ID3D12GraphicsCommandList* d3dCommandList,
        TransientResourceHeapImpl* transientHeap);

    ResourceCommandEncoderImpl m_resourceCommandEncoder;

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeResourceCommands(IResourceCommandEncoder** outEncoder) override;

    RenderCommandEncoderImpl m_renderCommandEncoder;
    virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
        IRenderPassLayout* renderPass,
        IFramebuffer* framebuffer,
        IRenderCommandEncoder** outEncoder) override;

    ComputeCommandEncoderImpl m_computeCommandEncoder;
    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeComputeCommands(IComputeCommandEncoder** outEncoder) override;

#if SLANG_GFX_HAS_DXR_SUPPORT
    RayTracingCommandEncoderImpl m_rayTracingCommandEncoder;
#endif
    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) override;
    virtual SLANG_NO_THROW void SLANG_MCALL close() override;
};

class FenceImpl : public FenceBase
{
public:
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_waitEvent = 0;

    ~FenceImpl();

    HANDLE getWaitEvent();

    Result init(DeviceImpl* device, const IFence::Desc& desc);

    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentValue(uint64_t* outValue) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL setCurrentValue(uint64_t value) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        getNativeHandle(InteropHandle* outNativeHandle) override;
};

class CommandQueueImpl
    : public ICommandQueue
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ICommandQueue* getInterface(const Guid& guid);
    void breakStrongReferenceToDevice() { m_renderer.breakStrongReference(); }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* handle) override;

public:
    BreakableReference<DeviceImpl> m_renderer;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_d3dQueue;
    ComPtr<ID3D12Fence> m_fence;
    uint64_t m_fenceValue = 0;
    HANDLE globalWaitHandle;
    Desc m_desc;
    uint32_t m_queueIndex = 0;

    Result init(DeviceImpl* device, uint32_t queueIndex);
    ~CommandQueueImpl();
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override;

    virtual SLANG_NO_THROW void SLANG_MCALL executeCommandBuffers(
        uint32_t count,
        ICommandBuffer* const* commandBuffers,
        IFence* fence,
        uint64_t valueToSignal) override;

    virtual SLANG_NO_THROW void SLANG_MCALL waitOnHost() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFenceValuesOnDevice(
        uint32_t fenceCount, IFence** fences, uint64_t* waitValues) override;
};

class SwapchainImpl : public D3DSwapchainBase
{
public:
    ComPtr<ID3D12CommandQueue> m_queue;
    ComPtr<IDXGIFactory> m_dxgiFactory;
    ComPtr<IDXGISwapChain3> m_swapChain3;
    ComPtr<ID3D12Fence> m_fence;
    ShortList<HANDLE, kMaxNumRenderFrames> m_frameEvents;
    uint64_t fenceValue = 0;
    Result init(DeviceImpl* renderer, const ISwapchain::Desc& swapchainDesc, WindowHandle window);

    virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) override;

    virtual void createSwapchainBufferImages() override;
    virtual IDXGIFactory* getDXGIFactory() override { return m_dxgiFactory; }
    virtual IUnknown* getOwningDevice() override { return m_queue; }
    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL present() override;
    virtual SLANG_NO_THROW bool SLANG_MCALL isOccluded() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL setFullScreenMode(bool mode) override;
};

#if SLANG_GFX_HAS_DXR_SUPPORT

class AccelerationStructureImpl
    : public AccelerationStructureBase
    , public ResourceViewInternalImpl
{
public:
    RefPtr<BufferResourceImpl> m_buffer;
    uint64_t m_offset;
    uint64_t m_size;
    ComPtr<ID3D12Device5> m_device5;

public:
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) override;
};

#endif

} // namespace d3d12
} // namespace gfx
