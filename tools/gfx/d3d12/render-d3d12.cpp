// render-d3d12.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "render-d3d12.h"

//WORKING:#include "options.h"
#include "../renderer-shared.h"
#include "../transient-resource-heap-base.h"
#include "../simple-render-pass-layout.h"
#include "../d3d/d3d-swapchain.h"
#include "core/slang-blob.h"
#include "core/slang-basic.h"
#include "core/slang-chunked-list.h"

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
//#include <d3dcompiler.h>

#ifndef __ID3D12GraphicsCommandList1_FWD_DEFINED__
// If can't find a definition of CommandList1, just use an empty definition
struct ID3D12GraphicsCommandList1 {};
#endif

#ifdef GFX_NVAPI
#   include "../nvapi/nvapi-include.h"
#endif

#include "slang-com-ptr.h"
#include "../flag-combiner.h"

#include "resource-d3d12.h"
#include "descriptor-heap-d3d12.h"

#include "../d3d/d3d-util.h"

#include "../nvapi/nvapi-util.h"

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

class D3D12Device : public RendererBase
{
public:
    // Renderer    implementation
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc,
        ITransientResourceHeap** outHeap) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc,
        WindowHandle window,
        ISwapchain** outSwapchain) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture,
        IResourceView::Desc const& desc,
        IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFrameBuffer) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebufferLayout(IFramebufferLayout::Desc const& desc, IFramebufferLayout** outLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc,
        IRenderPassLayout** outRenderPassLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override;

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout,
        ShaderObjectLayoutBase** outLayout) override;
    virtual Result createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
        override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc, IQueryPool** outState) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* resource,
        ResourceState state,
        ISlangBlob** outBlob,
        size_t* outRowPitch,
        size_t* outPixelSize) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* resource,
        size_t offset,
        size_t size,
        ISlangBlob** outBlob) override;

    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
    }

    ~D3D12Device();

#if SLANG_GFX_HAS_DXR_SUPPORT
    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc,
        IAccelerationStructure** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRayTracingPipelineState(
        const RayTracingPipelineStateDesc& desc, IPipelineState** outState) override;
#endif

public:
    
    static const Int kMaxNumRenderFrames = 4;
    static const Int kMaxNumRenderTargets = 3;

    static const Int kMaxRTVCount = 8;
    static const Int kMaxDescriptorSetCount = 16;

    struct DeviceInfo
    {
        void clear()
        {
            m_dxgiFactory.setNull();
            m_device.setNull();
            m_adapter.setNull();
            m_desc = {}; 
            m_desc1 = {};
            m_isWarp = false;
        }

        bool m_isWarp;
        ComPtr<IDXGIFactory> m_dxgiFactory;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12Device5> m_device5;
        ComPtr<IDXGIAdapter> m_adapter;
        DXGI_ADAPTER_DESC m_desc;
        DXGI_ADAPTER_DESC1 m_desc1;
    };

    struct Submitter
    {
        virtual void setRootConstantBufferView(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) = 0;
        virtual void setRootDescriptorTable(int index, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
        virtual void setRootSignature(ID3D12RootSignature* rootSignature) = 0;
        virtual void setRootConstants(Index rootParamIndex, Index dstOffsetIn32BitValues, Index countOf32BitValues, void const* srcData) = 0;
        virtual void setPipelineState(PipelineStateBase* pipelineState) = 0;
    };

    class BufferResourceImpl: public gfx::BufferResource
    {
    public:
        typedef BufferResource Parent;

        BufferResourceImpl(const Desc& desc)
            : Parent(desc)
            , m_defaultState(D3DUtil::translateResourceState(desc.defaultState))
        {
        }

        D3D12Resource m_resource;           ///< The resource typically in gpu memory
        D3D12Resource m_uploadResource;     ///< If the resource can be written to, and is in gpu memory (ie not Memory backed), will have upload resource

        D3D12_RESOURCE_STATES m_defaultState;

        virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
        {
            return (DeviceAddress)m_resource.getResource()->GetGPUVirtualAddress();
        }
    };

    class TextureResourceImpl: public TextureResource
    {
    public:
        typedef TextureResource Parent;

        TextureResourceImpl(const Desc& desc)
            : Parent(desc)
            , m_defaultState(D3DUtil::translateResourceState(desc.defaultState))
        {
        }

        D3D12Resource m_resource;
        D3D12_RESOURCE_STATES m_defaultState;
    };

    class SamplerStateImpl : public ISamplerState, public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL

        ISamplerState* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISamplerState)
                return static_cast<ISamplerState*>(this);
            return nullptr;
        }
    public:
        D3D12Descriptor m_descriptor;
        Slang::RefPtr<D3D12GeneralDescriptorHeap> m_allocator;
        ~SamplerStateImpl()
        {
            m_allocator->free(m_descriptor);
        }
    };

    class ResourceViewInternalImpl
    {
    public:
        D3D12Descriptor m_descriptor;
        RefPtr<D3D12GeneralDescriptorHeap> m_allocator;
        ~ResourceViewInternalImpl() { m_allocator->free(m_descriptor); }
    };

    class ResourceViewImpl
        : public ResourceViewBase
        , public ResourceViewInternalImpl
    {
    public:
        RefPtr<Resource> m_resource;
    };

    class FramebufferLayoutImpl : public FramebufferLayoutBase
    {
    public:
        ShortList<IFramebufferLayout::AttachmentLayout> m_renderTargets;
        bool m_hasDepthStencil = false;
        IFramebufferLayout::AttachmentLayout m_depthStencil;
    };

    class FramebufferImpl
        : public IFramebuffer
        , public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        IFramebuffer* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IFramebuffer)
                return static_cast<IFramebuffer*>(this);
            return nullptr;
        }

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
        void init(const IRenderPassLayout::Desc& desc)
        {
            SimpleRenderPassLayout::init(desc);
            m_framebufferLayout = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
        }
    };

    class InputLayoutImpl : public InputLayoutBase
	{
    public:
        List<D3D12_INPUT_ELEMENT_DESC> m_elements;
        List<char> m_text;                              ///< Holds all strings to keep in scope
    };

    class PipelineStateImpl : public PipelineStateBase
    {
    public:
        ComPtr<ID3D12PipelineState> m_pipelineState;
        void init(const GraphicsPipelineStateDesc& inDesc)
        {
            PipelineStateDesc pipelineDesc;
            pipelineDesc.type = PipelineType::Graphics;
            pipelineDesc.graphics = inDesc;
            initializeBase(pipelineDesc);
        }
        void init(const ComputePipelineStateDesc& inDesc)
        {
            PipelineStateDesc pipelineDesc;
            pipelineDesc.type = PipelineType::Compute;
            pipelineDesc.compute = inDesc;
            initializeBase(pipelineDesc);
        }
    };

#if SLANG_GFX_HAS_DXR_SUPPORT
    class RayTracingPipelineStateImpl : public PipelineStateBase
    {
    public:
        ComPtr<ID3D12StateObject> m_stateObject;
        D3D12_DISPATCH_RAYS_DESC m_dispatchDesc = {};
        Dictionary<String, int32_t> m_mapRayGenShaderNameToShaderTableIndex;
        // Shader Tables for each ray-tracing stage stored in GPU memory.
        RefPtr<BufferResourceImpl> m_rayGenShaderTable;
        RefPtr<BufferResourceImpl> m_hitgroupShaderTable;
        RefPtr<BufferResourceImpl> m_missShaderTable;
        void init(const RayTracingPipelineStateDesc& inDesc)
        {
            PipelineStateDesc pipelineDesc;
            pipelineDesc.type = PipelineType::RayTracing;
            pipelineDesc.rayTracing = inDesc;
            initializeBase(pipelineDesc);
        }
        Result createShaderTables(
            D3D12Device* device,
            slang::IComponentType* slangProgram,
            const RayTracingPipelineStateDesc& desc);
    };
#endif

    class QueryPoolImpl : public IQueryPool, public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        IQueryPool* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IQueryPool)
                return static_cast<IQueryPool*>(this);
            return nullptr;
        }
    public:
        Result init(const IQueryPool::Desc& desc, D3D12Device* device);

        virtual SLANG_NO_THROW Result SLANG_MCALL getResult(SlangInt queryIndex, SlangInt count, uint64_t* data) override
        {
            m_commandList->Reset(m_commandAllocator, nullptr);
            m_commandList->ResolveQueryData(m_queryHeap, m_queryType, (UINT)queryIndex, (UINT)count, m_readBackBuffer, 0);
            m_commandList->Close();
            ID3D12CommandList* cmdList = m_commandList;
            m_commandQueue->ExecuteCommandLists(1, &cmdList);
            m_eventValue++;
            m_fence->SetEventOnCompletion(m_eventValue, m_waitEvent);
            m_commandQueue->Signal(m_fence, m_eventValue);
            WaitForSingleObject(m_waitEvent, INFINITE);

            int8_t* mappedData = nullptr;
            D3D12_RANGE readRange = { sizeof(uint64_t) * queryIndex,sizeof(uint64_t) * (queryIndex + count) };
            m_readBackBuffer.getResource()->Map(0, &readRange, (void**)&mappedData);
            memcpy(data, mappedData + sizeof(uint64_t) * queryIndex, sizeof(uint64_t) * count);
            m_readBackBuffer.getResource()->Unmap(0, nullptr);
            return SLANG_OK;
        }

        void writeTimestamp(ID3D12GraphicsCommandList* cmdList, SlangInt index)
        {
            cmdList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, (UINT)index);
        }

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
    class PlainBufferProxyQueryPoolImpl
        : public IQueryPool
        , public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        IQueryPool* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IQueryPool)
                return static_cast<IQueryPool*>(this);
            return nullptr;
        }

    public:
        Result init(const IQueryPool::Desc& desc, D3D12Device* device, uint32_t stride);

        virtual SLANG_NO_THROW Result SLANG_MCALL
            getResult(SlangInt queryIndex, SlangInt count, uint64_t* data) override
        {
            ComPtr<ISlangBlob> blob;
            SLANG_RETURN_ON_FAIL(m_device->readBufferResource(
                m_bufferResource, m_stride * queryIndex, m_stride * count,
                blob.writeRef()));
            for (Int i = 0; i < count; i++)
            {
                memcpy(
                    data + i,
                    (uint8_t*)blob->getBufferPointer() + m_stride * i,
                    sizeof(uint64_t));
            }
            return SLANG_OK;
        }
    public:
        QueryType m_queryType;
        RefPtr<BufferResourceImpl> m_bufferResource;
        RefPtr<D3D12Device> m_device;
        uint32_t m_stride = 0;
    };

    struct BoundVertexBuffer
    {
        RefPtr<BufferResourceImpl> m_buffer;
        int m_stride;
        int m_offset;
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
        void setRootConstants(
            Index rootParamIndex,
            Index dstOffsetIn32BitValues,
            Index countOf32BitValues,
            void const* srcData) override
        {
            m_commandList->SetGraphicsRoot32BitConstants(UINT(rootParamIndex), UINT(countOf32BitValues), srcData, UINT(dstOffsetIn32BitValues));
        }
        virtual void setPipelineState(PipelineStateBase* pipeline) override
        {
            auto pipelineImpl = static_cast<PipelineStateImpl*>(pipeline);
            m_commandList->SetPipelineState(pipelineImpl->m_pipelineState.get());
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
        void setRootConstants(
            Index rootParamIndex,
            Index dstOffsetIn32BitValues,
            Index countOf32BitValues,
            void const* srcData) override
        {
            m_commandList->SetComputeRoot32BitConstants(UINT(rootParamIndex), UINT(countOf32BitValues), srcData, UINT(dstOffsetIn32BitValues));
        }
        virtual void setPipelineState(PipelineStateBase* pipeline) override
        {
            auto pipelineImpl = static_cast<PipelineStateImpl*>(pipeline);
            m_commandList->SetPipelineState(pipelineImpl->m_pipelineState.get());
        }
        ComputeSubmitter(ID3D12GraphicsCommandList* commandList) :
            m_commandList(commandList)
        {
        }

        ID3D12GraphicsCommandList* m_commandList;
    };

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

    static Result _uploadBufferData(
        ID3D12GraphicsCommandList* cmdList,
        BufferResourceImpl* buffer,
        size_t offset,
        size_t size,
        void* data)
    {
        D3D12_RANGE readRange = {};
        readRange.Begin = offset;
        readRange.End = offset + size;

        void* uploadData;
        SLANG_RETURN_ON_FAIL(buffer->m_uploadResource.getResource()->Map(
            0, &readRange, reinterpret_cast<void**>(&uploadData)));
        memcpy((uint8_t*)uploadData + offset, data, size);
        buffer->m_uploadResource.getResource()->Unmap(0, &readRange);
        {
            D3D12BarrierSubmitter submitter(cmdList);
            submitter.transition(
                buffer->m_resource, buffer->m_defaultState, D3D12_RESOURCE_STATE_COPY_DEST);
        }
        cmdList->CopyBufferRegion(
            buffer->m_resource.getResource(),
            offset,
            buffer->m_uploadResource.getResource(),
            offset,
            size);
        {
            D3D12BarrierSubmitter submitter(cmdList);
            submitter.transition(
                buffer->m_resource, D3D12_RESOURCE_STATE_COPY_DEST, buffer->m_defaultState);
        }
        return SLANG_OK;
    }
    
    class TransientResourceHeapImpl
        : public TransientResourceHeapBase<D3D12Device, BufferResourceImpl>
    {
    private:
        typedef TransientResourceHeapBase<D3D12Device, BufferResourceImpl> Super;
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
            ID3D12Fence* fence = nullptr;
        };
        ShortList<QueueWaitInfo, 4> m_waitInfos;

        QueueWaitInfo& getQueueWaitInfo(uint32_t queueIndex)
        {
            if (queueIndex < (uint32_t)m_waitInfos.getCount())
            {
                return m_waitInfos[queueIndex];
            }
            auto oldCount = m_waitInfos.getCount();
            m_waitInfos.setCount(queueIndex + 1);
            for (auto i = oldCount; i < m_waitInfos.getCount(); i++)
            {
                m_waitInfos[i].waitValue = 0;
                m_waitInfos[i].fenceEvent = CreateEventEx(
                    nullptr,
                    false,
                    0,
                    EVENT_ALL_ACCESS);
            }
            return m_waitInfos[queueIndex];
        }
        // During command submission, we need all the descriptor tables that get
        // used to come from a single heap (for each descriptor heap type).
        //
        // We will thus keep a single heap of each type that we hope will hold
        // all the descriptors that actually get needed in a frame.
        D3D12DescriptorHeap m_viewHeap; // Cbv, Srv, Uav
        D3D12DescriptorHeap m_samplerHeap; // Heap for samplers

        ~TransientResourceHeapImpl()
        {
            synchronizeAndReset();
            for (auto& waitInfo : m_waitInfos)
                CloseHandle(waitInfo.fenceEvent);
        }

        Result init(
            const ITransientResourceHeap::Desc& desc,
            D3D12Device* device,
            uint32_t viewHeapSize,
            uint32_t samplerHeapSize)
        {
            Super::init(desc, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, device);
            auto d3dDevice = device->m_device;
            SLANG_RETURN_ON_FAIL(d3dDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.writeRef())));
            
            SLANG_RETURN_ON_FAIL(m_viewHeap.init(
                d3dDevice,
                viewHeapSize,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
            SLANG_RETURN_ON_FAIL(m_samplerHeap.init(
                d3dDevice,
                samplerHeapSize,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));

            if (desc.constantBufferSize != 0)
            {
                ComPtr<IBufferResource> bufferResourcePtr;
                IBufferResource::Desc bufferDesc;
                bufferDesc.type = IResource::Type::Buffer;
                bufferDesc.defaultState = ResourceState::ConstantBuffer;
                bufferDesc.allowedStates =
                    ResourceStateSet(ResourceState::ConstantBuffer, ResourceState::CopyDestination);
                bufferDesc.sizeInBytes = desc.constantBufferSize;
                bufferDesc.cpuAccessFlags |= AccessFlag::Write;
                SLANG_RETURN_ON_FAIL(device->createBufferResource(
                    bufferDesc,
                    nullptr,
                    bufferResourcePtr.writeRef()));
                m_constantBuffers.add(static_cast<BufferResourceImpl*>(bufferResourcePtr.get()));
            }
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            createCommandBuffer(ICommandBuffer** outCommandBuffer) override;

        virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override;
    };

    class CommandBufferImpl;

    class PipelineCommandEncoder
    {
    public:
        bool m_isOpen = false;
        bool m_bindingDirty = true;
        CommandBufferImpl* m_commandBuffer;
        TransientResourceHeapImpl* m_transientHeap;
        D3D12Device* m_renderer;
        ID3D12Device* m_device;
        ID3D12GraphicsCommandList* m_d3dCmdList;
        ID3D12GraphicsCommandList* m_preCmdList = nullptr;

        RefPtr<PipelineStateBase> m_currentPipeline;

        static int getBindPointIndex(PipelineType type)
        {
            switch (type)
            {
            case PipelineType::Graphics:
                return 0;
            case PipelineType::Compute:
                return 1;
            case PipelineType::RayTracing:
                return 2;
            default:
                assert(!"unknown pipeline type.");
                return -1;
            }
        }

        void init(CommandBufferImpl* commandBuffer)
        {
            m_commandBuffer = commandBuffer;
            m_d3dCmdList = m_commandBuffer->m_cmdList;
            m_renderer = commandBuffer->m_renderer;
            m_transientHeap = commandBuffer->m_transientHeap;
            m_device = commandBuffer->m_renderer->m_device;
        }

        void endEncodingImpl() { m_isOpen = false; }

        Result bindPipelineImpl(IPipelineState* pipelineState, IShaderObject** outRootObject)
        {
            m_currentPipeline = static_cast<PipelineStateBase*>(pipelineState);
            auto rootObject = &m_commandBuffer->m_rootShaderObject;
            SLANG_RETURN_ON_FAIL(rootObject->reset(
                m_renderer,
                m_currentPipeline->getProgram<ShaderProgramImpl>()->m_rootObjectLayout,
                m_commandBuffer->m_transientHeap));
            *outRootObject = rootObject;
            m_bindingDirty = true;
            return SLANG_OK;
        }

        /// Specializes the pipeline according to current root-object argument values,
        /// applys the root object bindings and binds the pipeline state.
        /// The newly specialized pipeline is held alive by the pipeline cache so users of
        /// `newPipeline` do not need to maintain its lifespan.
        Result _bindRenderState(Submitter* submitter, RefPtr<PipelineStateBase>& newPipeline);
    };

    struct DescriptorTable
    {
        DescriptorHeapReference m_heap;
        uint32_t                m_offset    = 0;
        uint32_t                m_count     = 0;

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
            if(m_count)
            {
                m_heap.freeIfSupported(m_offset, m_count);
                m_offset = 0;
                m_count = 0;
            }
        }

        void allocate(uint32_t count)
        {
            m_offset = m_heap.allocate(count);
            m_count = count;
        }

        void allocate(DescriptorHeapReference heap, uint32_t count)
        {
            m_heap = heap;
            m_offset = heap.allocate(count);
            m_count = count;
        }
    };

        /// Contextual data and operations required when binding shader objects to the pipeline state
    struct BindingContext
    {
        PipelineCommandEncoder*     encoder;
        Submitter*                  submitter;
        TransientResourceHeapImpl*  transientHeap;
        D3D12Device*                device;
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

        uint32_t    rootParam = 0;
        uint32_t    resource = 0;
        uint32_t    sampler = 0;

        void operator+=(BindingOffset const& offset)
        {
            rootParam += offset.rootParam;
            resource += offset.resource;
            sampler += offset.sampler;
        }
    };

        /// A reprsentation of an allocated descriptor set, consisting of an option resource table and an optional sampler table
    struct DescriptorSet
    {
        DescriptorTable resourceTable;
        DescriptorTable samplerTable;

        void freeIfSupported()
        {
            resourceTable.freeIfSupported();
            samplerTable .freeIfSupported();
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

                /// The number of distinct bindings in this range.
            uint32_t count;

                /// A "flat" index for this range in whatever array provides backing storage for it
            uint32_t baseIndex;

                /// An index into the sub-object array if this binding range is treated
                /// as a sub-object.
            uint32_t subObjectIndex;
        };

            /// Offset information for a sub-object range
        struct SubObjectRangeOffset : BindingOffset
        {
            SubObjectRangeOffset()
            {}

            SubObjectRangeOffset(slang::VariableLayoutReflection* varLayout)
            {
                if(auto pendingLayout = varLayout->getPendingDataLayout())
                {
                    pendingOrdinaryData = (uint32_t) pendingLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);
                }
            }

                /// The offset for "pending" ordinary data related to this range
            uint32_t pendingOrdinaryData = 0;
        };

            /// Stride information for a sub-object range
        struct SubObjectRangeStride : BindingOffset
        {
            SubObjectRangeStride()
            {}

            SubObjectRangeStride(slang::TypeLayoutReflection* typeLayout)
            {
                if(auto pendingLayout = typeLayout->getPendingDataTypeLayout())
                {
                    pendingOrdinaryData = (uint32_t) pendingLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
                }
            }

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
            D3D12_ROOT_PARAMETER rootParameter;
        };

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

                /// The number of sub-objects (not just sub-object *ranges*) stored in instances of this layout
            uint32_t m_subObjectCount = 0;

                /// Counters for the number of root parameters, resources, and samplers in this object itself
            BindingOffset m_ownCounts;

                /// Counters for the number of root parameters, resources, and sampler in this object and transitive sub-objects
            BindingOffset m_totalCounts;

                /// The number of root parameter consumed by (transitive) sub-objects
            uint32_t m_childRootParameterCount = 0;

                /// The total size in bytes of the ordinary data for this object and transitive sub-object.
            uint32_t m_totalOrdinaryDataSize = 0;

                /// The container type of this shader object. When `m_containerType` is
                /// `StructuredBuffer` or `UnsizedArray`, this shader object represents a collection
                /// instead of a single object.
            ShaderObjectContainerType m_containerType = ShaderObjectContainerType::None;

            Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout)
            {
                typeLayout = _unwrapParameterGroups(typeLayout, m_containerType);
                m_elementTypeLayout = typeLayout;

                // If the type contains any ordinary data, then we must reserve a buffer
                // descriptor to hold it when binding as a parameter block.
                //
                m_totalOrdinaryDataSize = (uint32_t) typeLayout->getSize();
                if (m_totalOrdinaryDataSize != 0)
                {
                    m_ownCounts.resource++;
                }

                // We will scan over the reflected Slang binding ranges and add them
                // to our array. There are two main things we compute along the way:
                //
                // * For each binding range we compute a `flatIndex` that can be
                //   used to identify where the values for the given range begin
                //   in the flattened arrays (e.g., `m_objects`) and descriptor
                //   tables that hold the state of a shader object.
                //
                // * We also update the various counters taht keep track of the number
                //   of sub-objects, resources, samplers, etc. that are being
                //   consumed. These counters will contribute to figuring out
                //   the descriptor table(s) that might be needed to represent
                //   the object.
                //
                SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
                for (SlangInt r = 0; r < bindingRangeCount; ++r)
                {
                    slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                    uint32_t count = (uint32_t)typeLayout->getBindingRangeBindingCount(r);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(r);
                    BindingRangeInfo bindingRangeInfo = {};
                    bindingRangeInfo.bindingType = slangBindingType;
                    bindingRangeInfo.count = count;

                    switch (slangBindingType)
                    {
                    case slang::BindingType::ConstantBuffer:
                    case slang::BindingType::ParameterBlock:
                    case slang::BindingType::ExistentialValue:
                        bindingRangeInfo.baseIndex = m_subObjectCount;
                        bindingRangeInfo.subObjectIndex = m_subObjectCount;
                        m_subObjectCount += count;
                        break;
                    case slang::BindingType::RawBuffer:
                    case slang::BindingType::MutableRawBuffer:
                        if (slangLeafTypeLayout->getType()->getElementType() != nullptr)
                        {
                            // A structured buffer occupies both a resource slot and
                            // a sub-object slot.
                            bindingRangeInfo.subObjectIndex = m_subObjectCount;
                            m_subObjectCount += count;
                        }
                        bindingRangeInfo.baseIndex = m_ownCounts.resource;
                        m_ownCounts.resource += count;
                        break;
                    case slang::BindingType::Sampler:
                        bindingRangeInfo.baseIndex = m_ownCounts.sampler;
                        m_ownCounts.sampler += count;
                        break;

                    case slang::BindingType::CombinedTextureSampler:
                        // TODO: support this case...
                        break;

                    case slang::BindingType::VaryingInput:
                    case slang::BindingType::VaryingOutput:
                        break;

                    default:
                        bindingRangeInfo.baseIndex = m_ownCounts.resource;
                        m_ownCounts.resource += count;
                        break;
                    }
                    m_bindingRanges.add(bindingRangeInfo);
                }

                // At this point we've computed the number of resources/samplers that
                // the type needs to represent its *own* state, and stored those counts
                // in `m_ownCounts`. Next we need to consider any resources/samplers
                // and root parameters needed to represent the state of the transitive
                // sub-objects of this objet, so that we can compute the total size
                // of the object when bound to the pipeline.

                m_totalCounts = m_ownCounts;

                SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
                for (SlangInt r = 0; r < subObjectRangeCount; ++r)
                {
                    SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
                    auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
                    auto count = (uint32_t) typeLayout->getBindingRangeBindingCount(bindingRangeIndex);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

                    // A sub-object range can either represent a sub-object of a known
                    // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
                    // (in which case we can pre-compute a layout to use, based on
                    // the type `Foo`) *or* it can represent a sub-object of some
                    // existential type (e.g., `IBar`) in which case we cannot
                    // know the appropraite type/layout of sub-object to allocate.
                    //
                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    if (slangBindingType == slang::BindingType::ExistentialValue)
                    {
                        if(auto pendingTypeLayout = slangLeafTypeLayout->getPendingDataTypeLayout())
                        {
                            createForElementType(
                                m_renderer,
                                pendingTypeLayout,
                                subObjectLayout.writeRef());
                        }
                    }
                    else
                    {
                        createForElementType(
                            m_renderer,
                            slangLeafTypeLayout->getElementTypeLayout(),
                            subObjectLayout.writeRef());
                    }

                    SubObjectRangeInfo subObjectRange;
                    subObjectRange.bindingRangeIndex = bindingRangeIndex;
                    subObjectRange.layout = subObjectLayout;

                    // The Slang reflection API stors offset information for sub-object ranges,
                    // and we care about *some* of that information: in particular, we need
                    // the offset of sub-objects in terms of uniform/ordinary data for the
                    // cases where we need to fill in "pending" data in our ordinary buffer.
                    //
                    subObjectRange.offset = SubObjectRangeOffset(typeLayout->getSubObjectRangeOffset(r));
                    subObjectRange.stride = SubObjectRangeStride(slangLeafTypeLayout);

                    // The remaining offset information is computed based on the counters
                    // we are generating here, which depend only on the in-memory layout
                    // decisions being made in our implementation. Remember that the
                    // `register` and `space` values coming from DXBC/DXIL do *not*
                    // dictate the in-memory layout we use.
                    //
                    // Note: One subtle point here is that the `.rootParam` offset we are computing
                    // here does *not* include any root parameters that would be allocated
                    // for the parent object type itself (e.g., for descriptor tables
                    // used if it were bound as a parameter block). The later logic when
                    // we actually go to bind things will need to apply those offsets.
                    //
                    // Note: An even *more* subtle point is that the `.resource` offset
                    // being computed here *does* include the resource descriptor allocated
                    // for holding the ordinary data buffer, if any. The implications of
                    // this for later offset math is subtle.
                    //
                    subObjectRange.offset.rootParam = m_childRootParameterCount;
                    subObjectRange.offset.resource = m_totalCounts.resource;
                    subObjectRange.offset.sampler = m_totalCounts.sampler;

                    // Along with the offset information, we also need to compute the
                    // "stride" between consecutive sub-objects in the range. The actual
                    // size/stride of a single object depends on the type of range we
                    // are dealing with.
                    //
                    BindingOffset objectCounts;
                    switch(slangBindingType)
                    {
                    default:
                        break;

                    case slang::BindingType::ConstantBuffer:
                        {
                            SLANG_ASSERT(subObjectLayout);

                            // The resource and sampler descriptors of a nested
                            // constant buffer will "leak" into those of the
                            // parent type, and we need to account for them
                            // whenever we allocate storage.
                            //
                            objectCounts.resource   = subObjectLayout->getTotalResourceDescriptorCount();
                            objectCounts.sampler    = subObjectLayout->getTotalSamplerDescriptorCount();
                            objectCounts.rootParam  = subObjectRange.layout->getChildRootParameterCount();
                        }
                        break;

                    case slang::BindingType::ParameterBlock:
                        {
                            SLANG_ASSERT(subObjectLayout);

                            // In contrast to a constant buffer, a parameter block can hide
                            // the resource and sampler descriptor allocation it uses (since they
                            // are allocated into the tables that make up the parameter block.
                            //
                            // The only resource usage that leaks into the surrounding context
                            // is the number of root parameters consumed.
                            //
                            objectCounts.rootParam  = subObjectRange.layout->getTotalRootParameterCount();
                        }
                        break;

                    case slang::BindingType::ExistentialValue:
                        // An unspecialized existential/interface value cannot consume any resources
                        // as part of the parent object (it needs to fit inside the fixed-size
                        // represnetation of existential types).
                        //
                        // However, if we are statically specializing to a type that doesn't "fit"
                        // we may need to account for additional information that needs to be
                        // allocaated.
                        //
                        if(subObjectLayout)
                        {
                            // The ordinary data for an existential-type value is allocated into
                            // the same buffer as the parent object, so we only want to consider
                            // the resource descriptors *other than* the ordinary data buffer.
                            //
                            // Otherwise the logic here is identical to the constant buffer case.
                            //
                            objectCounts.resource   = subObjectLayout->getTotalResourceDescriptorCountWithoutOrdinaryDataBuffer();
                            objectCounts.sampler    = subObjectLayout->getTotalSamplerDescriptorCount();
                            objectCounts.rootParam  = subObjectRange.layout->getChildRootParameterCount();

                            // Note: In the implementation for some other graphics API (e.g., Vulkan) there
                            // needs to be more work done to handle the fact that "pending" data from
                            // interface-type sub-objects get allocated to a distinct offset after all the
                            // "primary" data. We are consciously ignoring that issue here, and the physical
                            // layout of a shader object into the D3D12 binding state may end up interleaving
                            // resources/samplers for "primary" and "pending" data.
                            //
                            // If this choice ever causes issues, we can revisit the approach here.

                            // An interface-type range that includes ordinary data can
                            // increase the size of the ordinary data buffer we need to
                            // allocate for the parent object.
                            //
                            uint32_t ordinaryDataEnd = subObjectRange.offset.pendingOrdinaryData
                                + (uint32_t) count * subObjectRange.stride.pendingOrdinaryData;

                            if(ordinaryDataEnd > m_totalOrdinaryDataSize)
                            {
                                m_totalOrdinaryDataSize = ordinaryDataEnd;
                            }
                        }
                        break;
                    }

                    // Once we've computed the usage for each object in the range, we can
                    // easily compute the rusage for the entire range.
                    //
                    auto rangeResourceCount     = count * objectCounts.resource;
                    auto rangeSamplerCount      = count * objectCounts.sampler;
                    auto rangeRootParamCount    = count * objectCounts.rootParam;

                    m_totalCounts.resource      += rangeResourceCount;
                    m_totalCounts.sampler       += rangeSamplerCount;
                    m_childRootParameterCount   += rangeRootParamCount;

                    m_subObjectRanges.add(subObjectRange);
                }

                // Once we have added up the resource usage from all the sub-objects
                // we can look at the total number of resources and samplers that
                // need to be bound as part of this objects descriptor tables and
                // that will allow us to decide whether we need to allocate a root
                // parameter for a resource table or not, ans similarly for a
                // sampler table.
                //
                if(m_totalCounts.resource)  m_ownCounts.rootParam++;
                if(m_totalCounts.sampler)   m_ownCounts.rootParam++;

                m_totalCounts.rootParam = m_ownCounts.rootParam + m_childRootParameterCount;

                return SLANG_OK;
            }

            SlangResult build(ShaderObjectLayoutImpl** outLayout)
            {
                auto layout = RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
                SLANG_RETURN_ON_FAIL(layout->_init(this));

                returnRefPtrMove(outLayout, layout);
                return SLANG_OK;
            }
        };

        static Result createForElementType(
            RendererBase* renderer,
            slang::TypeLayoutReflection* elementType,
            ShaderObjectLayoutImpl** outLayout)
        {
            Builder builder(renderer);
            builder.setElementTypeLayout(elementType);
            return builder.build(outLayout);
        }

        List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

        Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

        BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

        uint32_t getResourceSlotCount() { return m_ownCounts.resource; }
        uint32_t getSamplerSlotCount() { return m_ownCounts.sampler; }
        Index getSubObjectSlotCount() { return m_subObjectCount; }

        uint32_t getTotalResourceDescriptorCount() { return m_totalCounts.resource; }
        uint32_t getTotalSamplerDescriptorCount() { return m_totalCounts.sampler; }

        uint32_t getOrdinaryDataBufferCount() { return m_totalOrdinaryDataSize ? 1 : 0; }
        bool hasOrdinaryDataBuffer() { return m_totalOrdinaryDataSize != 0; }

        uint32_t getTotalResourceDescriptorCountWithoutOrdinaryDataBuffer() { return m_totalCounts.resource - getOrdinaryDataBufferCount(); }

        uint32_t getTotalRootParameterCount() { return m_totalCounts.rootParam; }
        uint32_t getChildRootParameterCount() { return m_childRootParameterCount; }

        uint32_t getTotalOrdinaryDataSize() const { return m_totalOrdinaryDataSize; }

        SubObjectRangeInfo const& getSubObjectRange(Index index)
        {
            return m_subObjectRanges[index];
        }
        List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

        RendererBase* getRenderer() { return m_renderer; }

        slang::TypeReflection* getType() { return m_elementTypeLayout->getType(); }

    protected:
        Result _init(Builder* builder)
        {
            auto renderer = builder->m_renderer;

            initBase(renderer, builder->m_elementTypeLayout);

            m_containerType = builder->m_containerType;

            m_bindingRanges = _Move(builder->m_bindingRanges);
            m_subObjectRanges = builder->m_subObjectRanges;

            m_ownCounts = builder->m_ownCounts;
            m_totalCounts = builder->m_totalCounts;
            m_subObjectCount = builder->m_subObjectCount;
            m_childRootParameterCount = builder->m_childRootParameterCount;
            m_totalOrdinaryDataSize = builder->m_totalOrdinaryDataSize;

            return SLANG_OK;
        }

        List<BindingRangeInfo> m_bindingRanges;
        List<SubObjectRangeInfo> m_subObjectRanges;

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

            Result build(RootShaderObjectLayoutImpl** outLayout)
            {
                RefPtr<RootShaderObjectLayoutImpl> layout = new RootShaderObjectLayoutImpl();
                SLANG_RETURN_ON_FAIL(layout->_init(this));

                returnRefPtrMove(outLayout, layout);
                return SLANG_OK;
            }

            void addGlobalParams(slang::VariableLayoutReflection* globalsLayout)
            {
                setElementTypeLayout(globalsLayout->getTypeLayout());
            }

            void addEntryPoint(SlangStage stage, ShaderObjectLayoutImpl* entryPointLayout)
            {
                EntryPointInfo info;
                info.layout = entryPointLayout;

                info.offset.resource = m_totalCounts.resource;
                info.offset.sampler = m_totalCounts.sampler;
                info.offset.rootParam = m_childRootParameterCount;

                m_totalCounts.resource += entryPointLayout->getTotalResourceDescriptorCount();
                m_totalCounts.sampler += entryPointLayout->getTotalSamplerDescriptorCount();

                // TODO(tfoley): Check this to make sure it is reasonable...
                m_childRootParameterCount += entryPointLayout->getChildRootParameterCount();

                m_entryPoints.add(info);
            }

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
            // We will use one descriptor set for the global scope and one additional
            // descriptor set for each `ParameterBlock` binding range in the shader object
            // hierarchy, regardless of the shader's `space` indices.
            List<DescriptorSetLayout> m_descriptorSets;
            List<D3D12_ROOT_PARAMETER> m_rootParameters;
            D3D12_ROOT_SIGNATURE_DESC m_rootSignatureDesc = {};

            static Result translateDescriptorRangeType(
                slang::BindingType c,
                D3D12_DESCRIPTOR_RANGE_TYPE* outType)
            {
                switch (c)
                {
                case slang::BindingType::ConstantBuffer:
                    *outType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    return SLANG_OK;
                case slang::BindingType::RawBuffer:
                case slang::BindingType::Texture:
                case slang::BindingType::TypedBuffer:
                case slang::BindingType::RayTracingAccelerationStructure:
                    *outType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    return SLANG_OK;
                case slang::BindingType::MutableRawBuffer:
                case slang::BindingType::MutableTexture:
                case slang::BindingType::MutableTypedBuffer:
                    *outType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    return SLANG_OK;
                case slang::BindingType::Sampler:
                    *outType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                    return SLANG_OK;
                default:
                    return SLANG_FAIL;
                }
            }

                /// Stores offset information to apply to the reflected register/space for a descriptor range.
                ///
            struct BindingRegisterOffset
            {
                uint32_t spaceOffset = 0; // The `space` index as specified in shader.

                enum { kRangeTypeCount = 4 };

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

                BindingRegisterOffset()
                {}

                BindingRegisterOffset(slang::VariableLayoutReflection* varLayout)
                {
                    if(varLayout)
                    {
                        spaceOffset                                             = (UINT) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_REGISTER_SPACE);
                        offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_CBV]     = (UINT) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
                        offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_SRV]     = (UINT) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE);
                        offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_UAV]     = (UINT) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS);
                        offsetForRangeType[D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER] = (UINT) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SAMPLER_STATE);
                    }
                }

                void operator+=(BindingRegisterOffset const& other)
                {
                    spaceOffset += other.spaceOffset;
                    for(int i = 0; i < kRangeTypeCount; ++i)
                    {
                        offsetForRangeType[i] += other.offsetForRangeType[i];
                    }
                }

            };

            struct BindingRegisterOffsetPair
            {
                BindingRegisterOffset primary;
                BindingRegisterOffset pending;

                BindingRegisterOffsetPair()
                {}

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

            Index reserveRootParameters(Index count)
            {
                Index result = m_rootParameters.getCount();
                m_rootParameters.setCount(result + count);
                return result;
            }

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
            uint32_t addDescriptorSet()
            {
                auto result = (uint32_t) m_descriptorSets.getCount();
                m_descriptorSets.add(DescriptorSetLayout{});
                return result;
            }

            Result addDescriptorRange(
                Index physicalDescriptorSetIndex,
                D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
                UINT registerIndex,
                UINT spaceIndex,
                UINT count)
            {
                auto& descriptorSet = m_descriptorSets[physicalDescriptorSetIndex];

                D3D12_DESCRIPTOR_RANGE range = {};
                range.RangeType = rangeType;
                range.NumDescriptors = count;
                range.BaseShaderRegister = registerIndex;
                range.RegisterSpace = spaceIndex;
                range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                {
                    descriptorSet.m_samplerRanges.add(range);
                    descriptorSet.m_samplerCount += range.NumDescriptors;
                }
                else
                {
                    descriptorSet.m_resourceRanges.add(range);
                    descriptorSet.m_resourceCount += range.NumDescriptors;
                }

                return SLANG_OK;
            }
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
                slang::TypeLayoutReflection*    typeLayout,
                Index                           physicalDescriptorSetIndex,
                BindingRegisterOffset const&    offset,
                Index                           logicalDescriptorSetIndex,
                Index                           descriptorRangeIndex)
            {
                auto bindingType = typeLayout->getDescriptorSetDescriptorRangeType(logicalDescriptorSetIndex, descriptorRangeIndex);
                auto count = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(logicalDescriptorSetIndex, descriptorRangeIndex);
                auto index = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(logicalDescriptorSetIndex, descriptorRangeIndex);
                auto space = typeLayout->getDescriptorSetSpaceOffset(logicalDescriptorSetIndex);

                D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
                SLANG_RETURN_ON_FAIL(translateDescriptorRangeType(bindingType, &rangeType));

                return addDescriptorRange(
                    physicalDescriptorSetIndex,
                    rangeType,
                    (UINT)index + offset[rangeType],
                    (UINT)space,
                    (UINT)count);
            }

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
                slang::TypeLayoutReflection*    typeLayout,
                Index                           physicalDescriptorSetIndex,
                BindingRegisterOffset const&    offset,
                Index                           bindingRangeIndex)
            {
                auto logicalDescriptorSetIndex  = typeLayout->getBindingRangeDescriptorSetIndex(bindingRangeIndex);
                auto firstDescriptorRangeIndex  = typeLayout->getBindingRangeFirstDescriptorRangeIndex(bindingRangeIndex);
                Index descriptorRangeCount      = typeLayout->getBindingRangeDescriptorRangeCount(bindingRangeIndex);
                for( Index i = 0; i < descriptorRangeCount; ++i )
                {
                    auto descriptorRangeIndex = firstDescriptorRangeIndex + i;

                    // Note: we ignore the `Result` returned by `addDescriptorRange()` because we
                    // want to silently skip any ranges that represent kinds of bindings that
                    // don't actually exist in D3D12.
                    //
                    addDescriptorRange(typeLayout, physicalDescriptorSetIndex, offset, logicalDescriptorSetIndex, descriptorRangeIndex);
                }
            }

            void addAsValue(
                slang::VariableLayoutReflection*    varLayout,
                Index                               physicalDescriptorSetIndex)
            {
                BindingRegisterOffsetPair offset(varLayout);
                addAsValue(varLayout->getTypeLayout(), physicalDescriptorSetIndex, offset);
            }


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
                slang::TypeLayoutReflection*        typeLayout,
                Index                               physicalDescriptorSetIndex,
                BindingRegisterOffsetPair const&    containerOffset,
                BindingRegisterOffsetPair const&    elementOffset)
            {
                if(typeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
                {
                    auto descriptorRangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    auto& offsetForRangeType = containerOffset.primary.offsetForRangeType[descriptorRangeType];
                    addDescriptorRange(
                        physicalDescriptorSetIndex,
                        descriptorRangeType,
                        offsetForRangeType,
                        containerOffset.primary.spaceOffset,
                        1);
                }

                addAsValue(typeLayout, physicalDescriptorSetIndex, elementOffset);
            }

            void addAsValue(
                slang::TypeLayoutReflection*        typeLayout,
                Index                               physicalDescriptorSetIndex,
                BindingRegisterOffsetPair const&    offset)
            {
                // Our first task is to add the binding ranges for stuff that is
                // directly contained in `typeLayout` rather than via sub-objects.
                //
                // Our goal is to have the descriptors for directly-contained views/samplers
                // always be contiguous in CPU and GPU memory, so that we can write
                // to them easily with a single operaiton.
                //
                Index bindingRangeCount = typeLayout->getBindingRangeCount();
                for (Index bindingRangeIndex = 0; bindingRangeIndex < bindingRangeCount; bindingRangeIndex++)
                {
                    // We will look at the type of each binding range and intentionally
                    // skip those that represent sub-objects.
                    //
                    auto bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
                    switch(bindingType)
                    {
                    case slang::BindingType::ConstantBuffer:
                    case slang::BindingType::ParameterBlock:
                    case slang::BindingType::ExistentialValue:
                        continue;

                    default:
                        break;
                    }

                    // For binding ranges that don't represent sub-objects, we will add
                    // all of the descriptor ranges they encompass to the root signature.
                    //
                    addBindingRange(typeLayout, physicalDescriptorSetIndex, offset.primary, bindingRangeIndex);
                }

                // Next we need to recursively include everything bound via sub-objects
                Index subObjectRangeCount = typeLayout->getSubObjectRangeCount();
                for (Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount; subObjectRangeIndex++)
                {
                    auto bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(subObjectRangeIndex);
                    auto bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);

                    auto subObjectTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

                    BindingRegisterOffsetPair subObjectRangeOffset = offset;
                    subObjectRangeOffset += BindingRegisterOffsetPair(typeLayout->getSubObjectRangeOffset(subObjectRangeIndex));

                    switch(bindingType)
                    {
                    case slang::BindingType::ConstantBuffer:
                        {
                            auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                            SLANG_ASSERT(containerVarLayout);

                            auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                            SLANG_ASSERT(elementVarLayout);

                            auto elementTypeLayout = elementVarLayout->getTypeLayout();
                            SLANG_ASSERT(elementTypeLayout);

                            BindingRegisterOffsetPair containerOffset = subObjectRangeOffset;
                            containerOffset += BindingRegisterOffsetPair(containerVarLayout);

                            BindingRegisterOffsetPair elementOffset = subObjectRangeOffset;
                            elementOffset += BindingRegisterOffsetPair(elementVarLayout);

                            addAsConstantBuffer(elementTypeLayout, physicalDescriptorSetIndex, containerOffset, elementOffset);
                        }
                        break;

                    case slang::BindingType::ParameterBlock:
                        {
                            auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                            SLANG_ASSERT(containerVarLayout);

                            auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                            SLANG_ASSERT(elementVarLayout);

                            auto elementTypeLayout = elementVarLayout->getTypeLayout();
                            SLANG_ASSERT(elementTypeLayout);

                            BindingRegisterOffsetPair subDescriptorSetOffset;
                            subDescriptorSetOffset.primary.spaceOffset = subObjectRangeOffset.primary.spaceOffset;
                            subDescriptorSetOffset.pending.spaceOffset = subObjectRangeOffset.pending.spaceOffset;

                            auto subPhysicalDescriptorSetIndex = addDescriptorSet();

                            BindingRegisterOffsetPair containerOffset = subDescriptorSetOffset;
                            containerOffset += BindingRegisterOffsetPair(containerVarLayout);

                            BindingRegisterOffsetPair elementOffset = subDescriptorSetOffset;
                            elementOffset += BindingRegisterOffsetPair(elementVarLayout);

                            addAsConstantBuffer(elementTypeLayout, subPhysicalDescriptorSetIndex, containerOffset, elementOffset);
                        }
                        break;

                    case slang::BindingType::ExistentialValue:
                        {
                            // Any nested binding ranges in the sub-object will "leak" into the
                            // binding ranges for the surrounding context.
                            //
                            auto specializedTypeLayout = subObjectTypeLayout->getPendingDataTypeLayout();
                            if(specializedTypeLayout)
                            {
                                BindingRegisterOffsetPair pendingOffset;
                                pendingOffset.primary = subObjectRangeOffset.pending;

                                addAsValue(specializedTypeLayout, physicalDescriptorSetIndex, pendingOffset);
                            }
                        }
                        break;
                    }
                }

//                BindingRegisterOffsetPair pendingOffset;
//                pendingOffset.primary = offset.pending;
//                addPendingResourceBindingRanges(typeLayout, physicalDescriptorSetIndex, pendingOffset);
            }

            D3D12_ROOT_SIGNATURE_DESC& build()
            {
                for (Index i = 0; i < m_descriptorSets.getCount(); i++)
                {
                    auto& descriptorSet = m_descriptorSets[i];
//                    D3D12Device::DescriptorSetInfo setInfo;
//                    setInfo.resourceDescriptorCount = descriptorSet.m_resourceCount;
//                    setInfo.samplerDescriptorCount = descriptorSet.m_samplerCount;
//                    outRootDescriptorSetInfos.add(setInfo);
                    if (descriptorSet.m_resourceRanges.getCount())
                    {
                        D3D12_ROOT_PARAMETER rootParam = {};
                        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                        rootParam.DescriptorTable.NumDescriptorRanges =
                            (UINT)descriptorSet.m_resourceRanges.getCount();
                        rootParam.DescriptorTable.pDescriptorRanges =
                            descriptorSet.m_resourceRanges.getBuffer();
                        m_rootParameters.add(rootParam);
                    }
                    if (descriptorSet.m_samplerRanges.getCount())
                    {
                        D3D12_ROOT_PARAMETER rootParam = {};
                        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                        rootParam.DescriptorTable.NumDescriptorRanges =
                            (UINT)descriptorSet.m_samplerRanges.getCount();
                        rootParam.DescriptorTable.pDescriptorRanges =
                            descriptorSet.m_samplerRanges.getBuffer();
                        m_rootParameters.add(rootParam);
                    }
                }

                m_rootSignatureDesc.NumParameters = UINT(m_rootParameters.getCount());
                m_rootSignatureDesc.pParameters = m_rootParameters.getBuffer();

                // TODO: static samplers should be reasonably easy to support...
                m_rootSignatureDesc.NumStaticSamplers = 0;
                m_rootSignatureDesc.pStaticSamplers = nullptr;

                // TODO: only set this flag if needed (requires creating root
                // signature at same time as pipeline state...).
                //
                m_rootSignatureDesc.Flags =
                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

                return m_rootSignatureDesc;
            }
        };

        static Result createRootSignatureFromSlang(
            D3D12Device* device,
            RootShaderObjectLayoutImpl* rootLayout,
            slang::IComponentType* program,
            ID3D12RootSignature** outRootSignature)
        {
            // We are going to build up the root signature by adding
            // binding/descritpor ranges and nested parameter blocks
            // based on the computed layout information for `program`.
            //
            RootSignatureDescBuilder builder;
            auto layout = program->getLayout();

            // The layout information computed by Slang breaks up shader
            // parameters into what we can think of as "logical" descriptor
            // sets based on whether or not parameters have the same `space`.
            //
            // We want to basically ignore that decomposition and generate a
            // single descriptor set to hold all top-level parameters, and only
            // generate distinct descriptor sets when the shader has opted in
            // via explicit parameter blocks.
            //
            // To achieve this goal, we will manually allocate a default descriptor
            // set for root parameters in our signature, and then recursively
            // add all the binding/descriptor ranges implied by the global-scope
            // parameters.
            //
            auto rootDescriptorSetIndex = builder.addDescriptorSet();
            builder.addAsValue(layout->getGlobalParamsVarLayout(), rootDescriptorSetIndex);

            for (SlangUInt i = 0; i < layout->getEntryPointCount(); i++)
            {
                // Entry-point parameters should also be added to the default root
                // descriptor set.
                //
                // We add the parameters using the "variable layout" for the entry point
                // and not just its type layout, to ensure that any offset information is
                // applied correctly to the `register` and `space` information for entry-point
                // parameters.
                //
                // Note: When we start to support DXR we will need to handle entry-point parameters
                // differently because they will need to map to local root signatures rather than
                // being included in the global root signature as is being done here.
                //
                auto entryPoint = layout->getEntryPointByIndex(i);
                builder.addAsValue(entryPoint->getVarLayout(), rootDescriptorSetIndex);
            }

            auto& rootSignatureDesc = builder.build();

            ComPtr<ID3DBlob> signature;
            ComPtr<ID3DBlob> error;
            if (SLANG_FAILED(device->m_D3D12SerializeRootSignature(
                    &rootSignatureDesc,
                    D3D_ROOT_SIGNATURE_VERSION_1,
                    signature.writeRef(),
                    error.writeRef())))
            {
                fprintf(stderr, "error: D3D12SerializeRootSignature failed");
                if (error)
                {
                    fprintf(stderr, ": %s\n", (const char*)error->GetBufferPointer());
                }
                return SLANG_FAIL;
            }

            SLANG_RETURN_ON_FAIL(device->m_device->CreateRootSignature(
                0,
                signature->GetBufferPointer(),
                signature->GetBufferSize(),
                IID_PPV_ARGS(outRootSignature)));
            return SLANG_OK;
        }

        static Result create(
            D3D12Device* device,
            slang::IComponentType* program,
            slang::ProgramLayout* programLayout,
            RootShaderObjectLayoutImpl** outLayout)
        {
            RootShaderObjectLayoutImpl::Builder builder(device, program, programLayout);
            builder.addGlobalParams(programLayout->getGlobalParamsVarLayout());

            SlangInt entryPointCount = programLayout->getEntryPointCount();
            for (SlangInt e = 0; e < entryPointCount; ++e)
            {
                auto slangEntryPoint = programLayout->getEntryPointByIndex(e);
                RefPtr<ShaderObjectLayoutImpl> entryPointLayout;
                SLANG_RETURN_ON_FAIL(ShaderObjectLayoutImpl::createForElementType(
                    device, slangEntryPoint->getTypeLayout(), entryPointLayout.writeRef()));
                builder.addEntryPoint(slangEntryPoint->getStage(), entryPointLayout);
            }

            RefPtr<RootShaderObjectLayoutImpl> layout;
            SLANG_RETURN_ON_FAIL(builder.build(layout.writeRef()));

            if (program->getSpecializationParamCount() == 0)
            {
                // For root object, we would like know the union of all binding slots
                // including all sub-objects in the shader-object hierarchy, so at
                // parameter binding time we can easily know how many GPU descriptor tables
                // to create without walking through the shader-object hierarchy again.
                // We build out this array along with root signature construction and store
                // it in `m_gpuDescriptorSetInfos`.
                SLANG_RETURN_ON_FAIL(createRootSignatureFromSlang(
                    device,
                    layout,
                    program,
                    layout->m_rootSignature.writeRef()));
            }

            *outLayout = layout.detach();

            return SLANG_OK;
        }

        slang::IComponentType* getSlangProgram() const { return m_program; }
        slang::ProgramLayout* getSlangProgramLayout() const { return m_programLayout; }

    protected:
        Result _init(Builder* builder)
        {
            auto renderer = builder->m_renderer;

            SLANG_RETURN_ON_FAIL(Super::_init(builder));

            m_program = builder->m_program;
            m_programLayout = builder->m_programLayout;
            m_entryPoints = builder->m_entryPoints;
            return SLANG_OK;
        }

        ComPtr<slang::IComponentType> m_program;
        slang::ProgramLayout* m_programLayout = nullptr;

        List<EntryPointInfo> m_entryPoints;

    public:
        ComPtr<ID3D12RootSignature> m_rootSignature;
//        List<DescriptorSetInfo> m_gpuDescriptorSetInfos;
    };

    struct ShaderBinary
    {
        SlangStage stage;
        List<uint8_t> code;
    };

    class ShaderProgramImpl : public ShaderProgramBase
    {
    public:
        PipelineType m_pipelineType;
        List<ShaderBinary> m_shaders;
        RefPtr<RootShaderObjectLayoutImpl> m_rootObjectLayout;
    };

    class ShaderObjectImpl
        : public ShaderObjectBaseImpl<
              ShaderObjectImpl,
              ShaderObjectLayoutImpl,
              SimpleShaderObjectData>
    {
    public:
        static Result create(
            D3D12Device* device,
            ShaderObjectLayoutImpl* layout,
            ShaderObjectImpl** outShaderObject)
        {
            auto object = RefPtr<ShaderObjectImpl>(new ShaderObjectImpl());
            SLANG_RETURN_ON_FAIL(
                object->init(device, layout, device->m_cpuViewHeap.Ptr(), device->m_cpuSamplerHeap.Ptr()));
            returnRefPtrMove(outShaderObject, object);
            return SLANG_OK;
        }

        ~ShaderObjectImpl()
        {
            m_descriptorSet.freeIfSupported();
        }

        RendererBase* getDevice() { return m_device.get(); }

        SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return 0; }

        SLANG_NO_THROW Result SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint)
            SLANG_OVERRIDE
        {
            *outEntryPoint = nullptr;
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL
            setData(ShaderOffset const& inOffset, void const* data, size_t inSize) SLANG_OVERRIDE
        {
            Index offset = inOffset.uniformOffset;
            Index size = inSize;

            char* dest = m_data.getBuffer();
            Index availableSize = m_data.getCount();

            // TODO: We really should bounds-check access rather than silently ignoring sets
            // that are too large, but we have several test cases that set more data than
            // an object actually stores on several targets...
            //
            if (offset < 0)
            {
                size += offset;
                offset = 0;
            }
            if ((offset + size) >= availableSize)
            {
                size = availableSize - offset;
            }

            memcpy(dest + offset, data, size);

            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL
            setResource(ShaderOffset const& offset, IResourceView* resourceView) SLANG_OVERRIDE;

        SLANG_NO_THROW Result SLANG_MCALL
            setSampler(ShaderOffset const& offset, ISamplerState* sampler) SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
            auto samplerImpl = static_cast<SamplerStateImpl*>(sampler);
            ID3D12Device* d3dDevice = static_cast<D3D12Device*>(getDevice())->m_device;
            d3dDevice->CopyDescriptorsSimple(
                1,
                m_descriptorSet.samplerTable.getCpuHandle(
                    bindingRange.baseIndex +
                    (int32_t)offset.bindingArrayIndex),
                samplerImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
            ShaderOffset const& offset,
            IResourceView* textureView,
            ISamplerState* sampler) SLANG_OVERRIDE
        {
#if 0
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
            auto resourceViewImpl = static_cast<ResourceViewImpl*>(textureView);
            ID3D12Device* d3dDevice = static_cast<D3D12Device*>(getDevice())->m_device;
            d3dDevice->CopyDescriptorsSimple(
                1,
                m_resourceHeap.getCpuHandle(
                    m_descriptorSet.m_resourceTable +
                    bindingRange.binding.offsetInDescriptorTable.resource +
                    (int32_t)offset.bindingArrayIndex),
                resourceViewImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            auto samplerImpl = static_cast<SamplerStateImpl*>(sampler);
            d3dDevice->CopyDescriptorsSimple(
                1,
                m_samplerHeap.getCpuHandle(
                    m_descriptorSet.m_samplerTable +
                    bindingRange.binding.offsetInDescriptorTable.sampler +
                    (int32_t)offset.bindingArrayIndex),
                samplerImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
#endif
            return SLANG_OK;
        }

    public:

    protected:
        Result init(
            D3D12Device* device,
            ShaderObjectLayoutImpl* layout,
            DescriptorHeapReference viewHeap,
            DescriptorHeapReference samplerHeap)
        {
            m_device = device;

            m_layout = layout;

            m_upToDateConstantBufferHeapVersion = 0;

            // If the layout tells us that there is any uniform data,
            // then we will allocate a CPU memory buffer to hold that data
            // while it is being set from the host.
            //
            // Once the user is done setting the parameters/fields of this
            // shader object, we will produce a GPU-memory version of the
            // uniform data (which includes values from this object and
            // any existential-type sub-objects).
            //
            size_t uniformSize = layout->getElementTypeLayout()->getSize();
            if (uniformSize)
            {
                m_data.setCount(uniformSize);
                memset(m_data.getBuffer(), 0, uniformSize);
            }

            // Each shader object will own CPU descriptor heap memory
            // for any resource or sampler descriptors it might store
            // as part of its value.
            //
            // This allocate includes a reservation for any constant
            // buffer descriptor pertaining to the ordinary data,
            // but does *not* include any descriptors that are managed
            // as part of sub-objects.
            //
            if(auto resourceCount = layout->getResourceSlotCount())
            {
                m_descriptorSet.resourceTable.allocate(viewHeap, resourceCount);

                // We must also ensure that the memory for any resources
                // referenced by descriptors in this object does not get
                // freed while the object is still live.
                //
                m_boundResources.setCount(resourceCount);
            }
            if(auto samplerCount = layout->getSamplerSlotCount())
            {
                m_descriptorSet.samplerTable.allocate(samplerHeap, samplerCount);
            }


            // If the layout specifies that we have any sub-objects, then
            // we need to size the array to account for them.
            //
            Index subObjectCount = layout->getSubObjectSlotCount();
            m_objects.setCount(subObjectCount);

            for (auto subObjectRangeInfo : layout->getSubObjectRanges())
            {
                auto subObjectLayout = subObjectRangeInfo.layout;

                // In the case where the sub-object range represents an
                // existential-type leaf field (e.g., an `IBar`), we
                // cannot pre-allocate the object(s) to go into that
                // range, since we can't possibly know what to allocate
                // at this point.
                //
                if (!subObjectLayout)
                    continue;
                //
                // Otherwise, we will allocate a sub-object to fill
                // in each entry in this range, based on the layout
                // information we already have.

                auto& bindingRangeInfo =
                    layout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);
                for (uint32_t i = 0; i < bindingRangeInfo.count; ++i)
                {
                    RefPtr<ShaderObjectImpl> subObject;
                    SLANG_RETURN_ON_FAIL(
                        ShaderObjectImpl::create(device, subObjectLayout, subObject.writeRef()));
                    m_objects[bindingRangeInfo.subObjectIndex + i] = subObject;
                }
            }

            return SLANG_OK;
        }

        /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given
        /// `offset`
        Result _writeOrdinaryData(
            PipelineCommandEncoder* encoder,
            BufferResourceImpl* buffer,
            size_t offset,
            size_t destSize,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            auto src = m_data.getBuffer();
            auto srcSize = size_t(m_data.getCount());

            SLANG_ASSERT(srcSize <= destSize);

            _uploadBufferData(encoder->m_d3dCmdList, buffer, offset, srcSize, src);

            // In the case where this object has any sub-objects of
            // existential/interface type, we need to recurse on those objects
            // that need to write their state into an appropriate "pending" allocation.
            //
            // Note: Any values that could fit into the "payload" included
            // in the existential-type field itself will have already been
            // written as part of `setObject()`. This loop only needs to handle
            // those sub-objects that do not "fit."
            //
            // An implementers looking at this code might wonder if things could be changed
            // so that *all* writes related to sub-objects for interface-type fields could
            // be handled in this one location, rather than having some in `setObject()` and
            // others handled here.
            //
            Index subObjectRangeCounter = 0;
            for (auto const& subObjectRangeInfo : specializedLayout->getSubObjectRanges())
            {
                Index subObjectRangeIndex = subObjectRangeCounter++;
                auto const& bindingRangeInfo =
                    specializedLayout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);

                // We only need to handle sub-object ranges for interface/existential-type fields,
                // because fields of constant-buffer or parameter-block type are responsible for
                // the ordinary/uniform data of their own existential/interface-type sub-objects.
                //
                if (bindingRangeInfo.bindingType != slang::BindingType::ExistentialValue)
                    continue;

                // Each sub-object range represents a single "leaf" field, but might be nested
                // under zero or more outer arrays, such that the number of existential values
                // in the same range can be one or more.
                //
                auto count = bindingRangeInfo.count;

                // We are not concerned with the case where the existential value(s) in the range
                // git into the payload part of the leaf field.
                //
                // In the case where the value didn't fit, the Slang layout strategy would have
                // considered the requirements of the value as a "pending" allocation, and would
                // allocate storage for the ordinary/uniform part of that pending allocation inside
                // of the parent object's type layout.
                //
                // Here we assume that the Slang reflection API can provide us with a single byte
                // offset and stride for the location of the pending data allocation in the
                // specialized type layout, which will store the values for this sub-object range.
                //
                // TODO: The reflection API functions we are assuming here haven't been implemented
                // yet, so the functions being called here are stubs.
                //
                // TODO: It might not be that a single sub-object range can reliably map to a single
                // contiguous array with a single stride; we need to carefully consider what the
                // layout logic does for complex cases with multiple layers of nested arrays and
                // structures.
                //
                size_t subObjectRangePendingDataOffset = subObjectRangeInfo.offset.pendingOrdinaryData;
                size_t subObjectRangePendingDataStride = subObjectRangeInfo.stride.pendingOrdinaryData;

                // If the range doesn't actually need/use the "pending" allocation at all, then
                // we need to detect that case and skip such ranges.
                //
                // TODO: This should probably be handled on a per-object basis by caching a "does it
                // fit?" bit as part of the information for bound sub-objects, given that we already
                // compute the "does it fit?" status as part of `setObject()`.
                //
                if (subObjectRangePendingDataOffset == 0)
                    continue;

                for (uint32_t i = 0; i < count; ++i)
                {
                    auto subObject = m_objects[bindingRangeInfo.subObjectIndex + i];

                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    SLANG_RETURN_ON_FAIL(
                        subObject->getSpecializedLayout(subObjectLayout.writeRef()));

                    auto subObjectOffset =
                        subObjectRangePendingDataOffset + i * subObjectRangePendingDataStride;

                    subObject->_writeOrdinaryData(
                        encoder,
                        buffer,
                        offset + subObjectOffset,
                        destSize - subObjectOffset,
                        subObjectLayout);
                }
            }

            return SLANG_OK;
        }

        /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
        Result _ensureOrdinaryDataBufferCreatedIfNeeded(
            PipelineCommandEncoder* encoder,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // If we have already created a buffer to hold ordinary data, then we should
            // simply re-use that buffer rather than re-create it.
            //
            // TODO: Simply re-using the buffer without any kind of validation checks
            // means that we are assuming that users cannot or will not perform any `set`
            // operations on a shader object once an operation has requested this buffer
            // be created. We need to enforce that rule if we want to rely on it.
            //
            if (m_upToDateConstantBufferHeapVersion ==
                encoder->m_commandBuffer->m_transientHeap->getVersion())
            {
                return SLANG_OK;
            }

            // Computing the size of the ordinary data buffer is *not* just as simple
            // as using the size of the `m_ordinayData` array that we store. The reason
            // for the added complexity is that interface-type fields may lead to the
            // storage being specialized such that it needs extra appended data to
            // store the concrete values that logically belong in those interface-type
            // fields but wouldn't fit in the fixed-size allocation we gave them.
            //
            m_constantBufferSize = specializedLayout->getTotalOrdinaryDataSize();
            if (m_constantBufferSize == 0)
            {
                m_upToDateConstantBufferHeapVersion =
                    encoder->m_commandBuffer->m_transientHeap->getVersion();
                return SLANG_OK;
            }

            // Once we have computed how large the buffer should be, we can allocate
            // it from the transient resource heap.
            //
            auto alignedConstantBufferSize = D3DUtil::calcAligned(m_constantBufferSize, 256);
            SLANG_RETURN_ON_FAIL(encoder->m_commandBuffer->m_transientHeap->allocateConstantBuffer(
                alignedConstantBufferSize, m_constantBufferWeakPtr, m_constantBufferOffset));

            // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
            //
            // Note that `_writeOrdinaryData` is potentially recursive in the case
            // where this object contains interface/existential-type fields, so we
            // don't need or want to inline it into this call site.
            //
            SLANG_RETURN_ON_FAIL(_writeOrdinaryData(
                encoder,
                static_cast<BufferResourceImpl*>(m_constantBufferWeakPtr),
                m_constantBufferOffset,
                m_constantBufferSize,
                specializedLayout));

            // Update version tracker so that we don't redundantly alloc and fill in
            // constant buffers for the same transient heap.
            m_upToDateConstantBufferHeapVersion =
                encoder->m_commandBuffer->m_transientHeap->getVersion();

            {
                // We also create and store a descriptor for our root constant buffer
                // into the descriptor table allocation that was reserved for them.
                //
                // We always know that the ordinary data buffer will be the first descriptor
                // in the table of resource views.
                //
                auto descriptorTable = m_descriptorSet.resourceTable;
                D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
                viewDesc.BufferLocation = static_cast<BufferResourceImpl*>(m_constantBufferWeakPtr)
                                              ->m_resource.getResource()
                                              ->GetGPUVirtualAddress() +
                                          m_constantBufferOffset;
                viewDesc.SizeInBytes = (UINT)alignedConstantBufferSize;
                encoder->m_device->CreateConstantBufferView(
                    &viewDesc, descriptorTable.getCpuHandle());
            }

            return SLANG_OK;
        }

    public:

            /// Prepare to bind this object as a parameter block.
            ///
            /// This involves allocating and binding any descriptor tables necessary
            /// to to store the state of the object. The function returns a descriptor
            /// set formed from any table(s) allocated. In addition, the `ioOffset`
            /// parameter will be adjusted to be correct for binding values into
            /// the resulting descriptor set.
            ///
        DescriptorSet prepareToBindAsParameterBlock(
            BindingContext*         context,
            BindingOffset&          ioOffset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            auto transientHeap = context->transientHeap;
            auto submitter = context->submitter;

            // When writing into the new descriptor set, resource and sampler
            // descriptors will need to start at index zero in the respective
            // tables.
            //
            ioOffset.resource = 0;
            ioOffset.sampler = 0;

            // The index of the next root parameter to bind will be maintained,
            // but needs to be incremented by the number of descriptor tables
            // we allocate (zero or one resource table and zero or one sampler
            // table).
            //
            auto& rootParamIndex = ioOffset.rootParam;
            DescriptorSet descriptorSet;

            if(auto descriptorCount = specializedLayout->getTotalResourceDescriptorCount())
            {
                // There is a non-zero number of resource descriptors needed,
                // so we will allocate a table out of the appropriate heap,
                // and store it into the appropriate part of `descriptorSet`.
                //
                auto descriptorHeap = &transientHeap->m_viewHeap;
                auto& table = descriptorSet.resourceTable;

                // Allocate the table.
                //
                table.allocate(descriptorHeap, descriptorCount);

                // Bind the table to the pipeline, consuming the next available
                // root parameter.
                //
                auto tableRootParamIndex = rootParamIndex++;
                submitter->setRootDescriptorTable(tableRootParamIndex, table.getGpuHandle());
            }
            if(auto descriptorCount = specializedLayout->getTotalSamplerDescriptorCount())
            {
                // There is a non-zero number of sampler descriptors needed,
                // so we will allocate a table out of the appropriate heap,
                // and store it into the appropriate part of `descriptorSet`.
                //
                auto descriptorHeap = &transientHeap->m_samplerHeap; 
                auto& table = descriptorSet.samplerTable;

                // Allocate the table.
                //
                table.allocate(descriptorHeap, descriptorCount);

                // Bind the table to the pipeline, consuming the next available
                // root parameter.
                //
                auto tableRootParamIndex = rootParamIndex++;
                submitter->setRootDescriptorTable(tableRootParamIndex, table.getGpuHandle());
            }

            return descriptorSet;
        }

            /// Bind this object as a `ParameterBlock<X>`
        Result bindAsParameterBlock(
            BindingContext*         context,
            BindingOffset const&    offset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // The first step to binding an object as a parameter block is to allocate a descriptor
            // set (consisting of zero or one resource descriptor table and zero or one sampler
            // descriptor table) to represent its values.
            //
            BindingOffset subOffset = offset;
            auto descriptorSet = prepareToBindAsParameterBlock(context, /* inout */ subOffset, specializedLayout);

            // Next we bind the object into that descriptor set as if it were being used
            // as a `ConstantBuffer<X>`.
            //
            SLANG_RETURN_ON_FAIL(bindAsConstantBuffer(context, descriptorSet, subOffset, specializedLayout));
            return SLANG_OK;
        }

            /// Bind this object as a `ConstantBuffer<X>`
        Result bindAsConstantBuffer(
            BindingContext*         context,
            DescriptorSet const&    descriptorSet,
            BindingOffset const&    offset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // If we are to bind as a constant buffer we first need to ensure that
            // the ordinary data buffer is created, if this object needs one.
            //
            SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(context->encoder, specializedLayout));

            // Next, we need to bind all of the resource descriptors for this object
            // (including any ordinary data buffer) into the provided `descriptorSet`.
            //
            auto resourceCount = specializedLayout->getResourceSlotCount();
            if(resourceCount)
            {
                auto& dstTable = descriptorSet.resourceTable;
                auto& srcTable = m_descriptorSet.resourceTable;

                context->device->m_device->CopyDescriptorsSimple(
                    UINT(resourceCount),
                    dstTable.getCpuHandle(offset.resource),
                    srcTable.getCpuHandle(),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            // Finally, we delegate to `_bindImpl` to bind samplers and sub-objects,
            // since the logic is shared with the `bindAsValue()` case below.
            //
            SLANG_RETURN_ON_FAIL(_bindImpl(context, descriptorSet, offset, specializedLayout));
            return SLANG_OK;
        }

            /// Bind this object as a value (for an interface-type parameter)
        Result bindAsValue(
            BindingContext*         context,
            DescriptorSet const&    descriptorSet,
            BindingOffset const&    offset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // When binding a value for an interface-type field we do *not* want
            // to bind a buffer for the ordinary data (if there is any) because
            // ordinary data for interface-type fields gets allocated into the
            // parent object's ordinary data buffer.
            //
            // This CPU-memory descriptor table that holds resource descriptors
            // will have already been allocated to have space for an ordinary data
            // buffer (if needed), so we need to take care to skip over that
            // descriptor when copying descriptors from the CPU-memory set
            // to the GPU-memory `descriptorSet`.
            //
            auto skipResourceCount = specializedLayout->getOrdinaryDataBufferCount();
            auto resourceCount = specializedLayout->getResourceSlotCount() - skipResourceCount;
            if(resourceCount)
            {
                auto& dstTable = descriptorSet.resourceTable;
                auto& srcTable = m_descriptorSet.resourceTable;

                context->device->m_device->CopyDescriptorsSimple(
                    UINT(resourceCount),
                    dstTable.getCpuHandle(offset.resource),
                    srcTable.getCpuHandle(skipResourceCount),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }

            // Finally, we delegate to `_bindImpl` to bind samplers and sub-objects,
            // since the logic is shared with the `bindAsConstantBuffer()` case above.
            //
            // Note: Just like we had to do some subtle handling of the ordinary data buffer
            // above, here we need to contend with the fact that the `offset.resource` fields
            // computed for sub-object ranges were baked to take the ordinary data buffer
            // into account, so that if `skipResourceCount` is non-zero then they are all
            // too high by `skipResourceCount`.
            //
            // We will address the problem here by computing a modified offset that adjusts
            // for the ordinary data buffer that we have not bound after all.
            //
            BindingOffset subOffset = offset;
            subOffset.resource -= skipResourceCount;
            SLANG_RETURN_ON_FAIL(_bindImpl(context, descriptorSet, subOffset, specializedLayout));
            return SLANG_OK;
        }

            /// Shared logic for `bindAsConstantBuffer()` and `bindAsValue()`
        Result _bindImpl(
            BindingContext*         context,
            DescriptorSet const&    descriptorSet,
            BindingOffset const&    offset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // We start by binding all the sampler decriptors, if needed.
            //
            // Note: resource descriptors were handled in either `bindAsConstantBuffer()`
            // or `bindAsValue()` before calling into `_bindImpl()`.
            //
            if (auto samplerCount = specializedLayout->getSamplerSlotCount())
            {
                auto& dstTable = descriptorSet.samplerTable;
                auto& srcTable = m_descriptorSet.samplerTable;

                context->device->m_device->CopyDescriptorsSimple(
                    UINT(samplerCount),
                    dstTable.getCpuHandle(offset.sampler),
                    srcTable.getCpuHandle(),
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            }

            // Next we iterate over the sub-object ranges and bind anything they require.
            //
            auto& subObjectRanges = specializedLayout->getSubObjectRanges();
            auto subObjectRangeCount = subObjectRanges.getCount();
            for (Index i = 0; i < subObjectRangeCount; i++)
            {
                auto& subObjectRange = specializedLayout->getSubObjectRange(i);
                auto& bindingRange = specializedLayout->getBindingRange(subObjectRange.bindingRangeIndex);
                auto subObjectIndex = bindingRange.subObjectIndex;
                auto subObjectLayout = subObjectRange.layout.Ptr();

                BindingOffset rangeOffset = offset;
                rangeOffset += subObjectRange.offset;

                BindingOffset rangeStride = subObjectRange.stride;

                switch(bindingRange.bindingType)
                {
                case slang::BindingType::ConstantBuffer:
                    {
                        auto objOffset = rangeOffset;
                        for (uint32_t j = 0; j < bindingRange.count; j++)
                        {
                            auto& object = m_objects[subObjectIndex + j];
                            object->bindAsConstantBuffer(context, descriptorSet, objOffset, subObjectLayout);
                            objOffset += rangeStride;
                        }
                    }
                    break;

                case slang::BindingType::ParameterBlock:
                    {
                        auto objOffset = rangeOffset;
                        for (uint32_t j = 0; j < bindingRange.count; j++)
                        {
                            auto& object = m_objects[subObjectIndex + j];
                            object->bindAsParameterBlock(context, objOffset, subObjectLayout);
                            objOffset += rangeStride;
                        }
                    }
                    break;

                case slang::BindingType::ExistentialValue:
                    if(subObjectLayout)
                    {
                        auto objOffset = rangeOffset;
                        for (uint32_t j = 0; j < bindingRange.count; j++)
                        {
                            auto& object = m_objects[subObjectIndex + j];
                            object->bindAsValue(context, descriptorSet, objOffset, subObjectLayout);
                            objOffset += rangeStride;
                        }
                    }
                    break;
                }
            }

            return SLANG_OK;
        }


            /// A CPU-memory descriptor set holding any descriptors used to represent the resources/samplers in this object's state
        DescriptorSet m_descriptorSet;

        ShortList<RefPtr<Resource>, 8> m_boundResources;

        /// A constant buffer used to stored ordinary data for this object
        /// and existential-type sub-objects.
        ///
        /// Allocated from transient heap on demand with `_createOrdinaryDataBufferIfNeeded()`
        IBufferResource* m_constantBufferWeakPtr = nullptr;
        size_t m_constantBufferOffset = 0;
        size_t m_constantBufferSize = 0;

        /// The version number of the transient resource heap that contains up-to-date
        /// constant buffer content for this shader object. If this is equal to the version number
        /// of currently active transient heap, then the current set-up of constant buffer contents
        /// as defined by the above `m_constantBuffer*` fields is valid and up-to-date so we can
        /// use them directly.
        uint64_t m_upToDateConstantBufferHeapVersion;

        /// Get the layout of this shader object with specialization arguments considered
        ///
        /// This operation should only be called after the shader object has been
        /// fully filled in and finalized.
        ///
        Result getSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
        {
            if (!m_specializedLayout)
            {
                SLANG_RETURN_ON_FAIL(_createSpecializedLayout(m_specializedLayout.writeRef()));
            }
            returnRefPtr(outLayout, m_specializedLayout);
            return SLANG_OK;
        }

        /// Create the layout for this shader object with specialization arguments considered
        ///
        /// This operation is virtual so that it can be customized by `RootShaderObject`.
        ///
        virtual Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
        {
            ExtendedShaderObjectType extendedType;
            SLANG_RETURN_ON_FAIL(getSpecializedShaderObjectType(&extendedType));

            auto renderer = getRenderer();
            RefPtr<ShaderObjectLayoutImpl> layout;
            SLANG_RETURN_ON_FAIL(renderer->getShaderObjectLayout(
                extendedType.slangType,
                m_layout->getContainerType(),
                (ShaderObjectLayoutBase**)layout.writeRef()));

            returnRefPtrMove(outLayout, layout);
            return SLANG_OK;
        }

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
        RootShaderObjectLayoutImpl* getLayout()
        {
            return static_cast<RootShaderObjectLayoutImpl*>(m_layout.Ptr());
        }

        UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE
        {
            return (UInt)m_entryPoints.getCount();
        }
        SlangResult SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint)
            SLANG_OVERRIDE
        {
            returnComPtr(outEntryPoint, m_entryPoints[index]);
            return SLANG_OK;
        }

        virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
        {
            SLANG_RETURN_ON_FAIL(ShaderObjectImpl::collectSpecializationArgs(args));
            for (auto& entryPoint : m_entryPoints)
            {
                SLANG_RETURN_ON_FAIL(entryPoint->collectSpecializationArgs(args));
            }
            return SLANG_OK;
        }

    public:
        Result bindAsRoot(
            BindingContext*             context,
            RootShaderObjectLayoutImpl* specializedLayout)
        {
            // A root shader object always binds as if it were a parameter block,
            // insofar as it needs to allocate a descriptor set to hold the bindings
            // for its own state and any sub-objects.
            //
            // Note: We do not direclty use `bindAsParameterBlock` here because we also
            // need to bind the entry points into the same descriptor set that is
            // being used for the root object.
            //
            BindingOffset rootOffset;
            auto descriptorSet = prepareToBindAsParameterBlock(context, /* inout */ rootOffset, specializedLayout);

            SLANG_RETURN_ON_FAIL(Super::bindAsConstantBuffer(context, descriptorSet, rootOffset, specializedLayout));

            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPoint = m_entryPoints[i];
                auto& entryPointInfo = specializedLayout->getEntryPoint(i);

                auto entryPointOffset = rootOffset;
                entryPointOffset += entryPointInfo.offset;

                SLANG_RETURN_ON_FAIL(entryPoint->bindAsConstantBuffer(context, descriptorSet, entryPointOffset, entryPointInfo.layout));
            }

            return SLANG_OK;
        }

    public:

        Result init(D3D12Device* device)
        {
            SLANG_RETURN_ON_FAIL(m_cpuViewHeap.init(
                device->m_device,
                64,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
            SLANG_RETURN_ON_FAIL(m_cpuSamplerHeap.init(
                device->m_device,
                8,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
            return SLANG_OK;
        }

        Result reset(
            D3D12Device* device,
            RootShaderObjectLayoutImpl* layout,
            TransientResourceHeapImpl* heap)
        {
            m_cpuViewHeap.deallocateAll();
            m_cpuSamplerHeap.deallocateAll();
            SLANG_RETURN_ON_FAIL(Super::init(device, layout, &m_cpuViewHeap, &m_cpuSamplerHeap));
            m_specializedLayout = nullptr;
            m_entryPoints.clear();
            for (auto entryPointInfo : layout->getEntryPoints())
            {
                RefPtr<ShaderObjectImpl> entryPoint;
                SLANG_RETURN_ON_FAIL(
                    ShaderObjectImpl::create(device, entryPointInfo.layout, entryPoint.writeRef()));
                m_entryPoints.add(entryPoint);
            }
            return SLANG_OK;
        }

    protected:
        Result _createSpecializedLayout(ShaderObjectLayoutImpl** outLayout) SLANG_OVERRIDE
        {
            ExtendedShaderObjectTypeList specializationArgs;
            SLANG_RETURN_ON_FAIL(collectSpecializationArgs(specializationArgs));

            // Note: There is an important policy decision being made here that we need
            // to approach carefully.
            //
            // We are doing two different things that affect the layout of a program:
            //
            // 1. We are *composing* one or more pieces of code (notably the shared global/module
            //    stuff and the per-entry-point stuff).
            //
            // 2. We are *specializing* code that includes generic/existential parameters
            //    to concrete types/values.
            //
            // We need to decide the relative *order* of these two steps, because of how it impacts
            // layout. The layout for `specialize(compose(A,B), X, Y)` is potentially different
            // form that of `compose(specialize(A,X), speciealize(B,Y))`, even when both are
            // semantically equivalent programs.
            //
            // Right now we are using the first option: we are first generating a full composition
            // of all the code we plan to use (global scope plus all entry points), and then
            // specializing it to the concatenated specialization argumenst for all of that.
            //
            // In some cases, though, this model isn't appropriate. For example, when dealing with
            // ray-tracing shaders and local root signatures, we really want the parameters of each
            // entry point (actually, each entry-point *group*) to be allocated distinct storage,
            // which really means we want to compute something like:
            //
            //      SpecializedGlobals = specialize(compose(ModuleA, ModuleB, ...), X, Y, ...)
            //
            //      SpecializedEP1 = compose(SpecializedGlobals, specialize(EntryPoint1, T, U, ...))
            //      SpecializedEP2 = compose(SpecializedGlobals, specialize(EntryPoint2, A, B, ...))
            //
            // Note how in this case all entry points agree on the layout for the shared/common
            // parmaeters, but their layouts are also independent of one another.
            //
            // Furthermore, in this example, loading another entry point into the system would not
            // rquire re-computing the layouts (or generated kernel code) for any of the entry
            // points that had already been loaded (in contrast to a compose-then-specialize
            // approach).
            //
            ComPtr<slang::IComponentType> specializedComponentType;
            ComPtr<slang::IBlob> diagnosticBlob;
            auto result = getLayout()->getSlangProgram()->specialize(
                specializationArgs.components.getArrayView().getBuffer(),
                specializationArgs.getCount(),
                specializedComponentType.writeRef(),
                diagnosticBlob.writeRef());

            // TODO: print diagnostic message via debug output interface.

            if (result != SLANG_OK)
                return result;

            auto slangSpecializedLayout = specializedComponentType->getLayout();
            RefPtr<RootShaderObjectLayoutImpl> specializedLayout;
            RootShaderObjectLayoutImpl::create(
                static_cast<D3D12Device*>(getRenderer()),
                specializedComponentType,
                slangSpecializedLayout,
                specializedLayout.writeRef());

            // Note: Computing the layout for the specialized program will have also computed
            // the layouts for the entry points, and we really need to attach that information
            // to them so that they don't go and try to compute their own specializations.
            //
            // TODO: Well, if we move to the specialization model described above then maybe
            // we *will* want entry points to do their own specialization work...
            //
            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPointInfo = specializedLayout->getEntryPoint(i);
                auto entryPointVars = m_entryPoints[i];

                entryPointVars->m_specializedLayout = entryPointInfo.layout;
            }

            returnRefPtrMove(outLayout, specializedLayout);
            return SLANG_OK;
        }

        List<RefPtr<ShaderObjectImpl>> m_entryPoints;

    public:
        // Descriptor heaps for the root object. Resets with the life cycle of each root shader
        // object use.
        D3D12DescriptorHeap m_cpuViewHeap;
        D3D12DescriptorHeap m_cpuSamplerHeap;
    };

    class CommandBufferImpl
        : public ICommandBuffer
        , public ComObject
    {
    public:
        // There are a pair of cyclic references between a `TransientResourceHeap` and
        // a `CommandBuffer` created from the heap. We need to break the cycle upon
        // the public reference count of a command buffer dropping to 0.
        SLANG_COM_OBJECT_IUNKNOWN_ALL

        ICommandBuffer* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
                return static_cast<ICommandBuffer*>(this);
            return nullptr;
        }
        virtual void comFree() override { m_transientHeap.breakStrongReference(); }
    public:
        ComPtr<ID3D12GraphicsCommandList> m_cmdList;
        ComPtr<ID3D12GraphicsCommandList4> m_cmdList4;

        BreakableReference<TransientResourceHeapImpl> m_transientHeap;
        // Weak reference is fine here since `m_transientHeap` already holds strong reference to
        // device.
        D3D12Device* m_renderer;
        RootShaderObjectImpl m_rootShaderObject;

        void init(
            D3D12Device* renderer,
            ID3D12GraphicsCommandList* d3dCommandList,
            TransientResourceHeapImpl* transientHeap)
        {
            m_transientHeap = transientHeap;
            m_renderer = renderer;
            m_cmdList = d3dCommandList;

            ID3D12DescriptorHeap* heaps[] = {
                m_transientHeap->m_viewHeap.getHeap(),
                m_transientHeap->m_samplerHeap.getHeap(),
            };
            m_cmdList->SetDescriptorHeaps(SLANG_COUNT_OF(heaps), heaps);
            m_rootShaderObject.init(renderer);

#if SLANG_GFX_HAS_DXR_SUPPORT
            m_cmdList->QueryInterface<ID3D12GraphicsCommandList4>(m_cmdList4.writeRef());
#endif
        }

        class RenderCommandEncoderImpl
            : public IRenderCommandEncoder
            , public PipelineCommandEncoder
        {
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
                D3D12Device* renderer,
                TransientResourceHeapImpl* transientHeap,
                CommandBufferImpl* cmdBuffer,
                RenderPassLayoutImpl* renderPass,
                FramebufferImpl* framebuffer)
            {
                PipelineCommandEncoder::init(cmdBuffer);
                m_preCmdList = nullptr;
                m_renderPass = renderPass;
                m_framebuffer = framebuffer;
                m_transientHeap = transientHeap;
                m_boundVertexBuffers.clear();
                m_boundIndexBuffer = nullptr;
                m_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                m_boundIndexFormat = DXGI_FORMAT_UNKNOWN;
                m_boundIndexOffset = 0;
                m_currentPipeline = nullptr;

                // Set render target states.
                m_d3dCmdList->OMSetRenderTargets(
                    (UINT)framebuffer->renderTargetViews.getCount(),
                    framebuffer->renderTargetDescriptors.getArrayView().getBuffer(),
                    FALSE,
                    framebuffer->depthStencilView ? &framebuffer->depthStencilDescriptor : nullptr);

                // Issue clear commands based on render pass set up.
                for (Index i = 0; i < renderPass->m_renderTargetAccesses.getCount(); i++)
                {
                    auto& access = renderPass->m_renderTargetAccesses[i];

                    // Transit resource states.
                    {
                        D3D12BarrierSubmitter submitter(m_d3dCmdList);
                        auto resourceViewImpl = framebuffer->renderTargetViews[i].Ptr();
                        auto textureResource =
                            static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
                        D3D12_RESOURCE_STATES initialState;
                        if (access.initialState == ResourceState::Undefined)
                        {
                            initialState = textureResource->m_defaultState;
                        }
                        else
                        {
                            initialState = D3DUtil::translateResourceState(access.initialState);
                        }
                        textureResource->m_resource.transition(
                            initialState,
                            D3D12_RESOURCE_STATE_RENDER_TARGET,
                            submitter);
                    }
                    // Clear.
                    if (access.loadOp == IRenderPassLayout::AttachmentLoadOp::Clear)
                    {
                        m_d3dCmdList->ClearRenderTargetView(
                            framebuffer->renderTargetDescriptors[i],
                            framebuffer->renderTargetClearValues[i].values,
                            0,
                            nullptr);
                    }
                }

                if (renderPass->m_hasDepthStencil)
                {
                    // Transit resource states.
                    {
                        D3D12BarrierSubmitter submitter(m_d3dCmdList);
                        auto resourceViewImpl = framebuffer->depthStencilView.Ptr();
                        auto textureResource =
                            static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
                        D3D12_RESOURCE_STATES initialState;
                        if (renderPass->m_depthStencilAccess.initialState ==
                            ResourceState::Undefined)
                        {
                            initialState = textureResource->m_defaultState;
                        }
                        else
                        {
                            initialState = D3DUtil::translateResourceState(
                                renderPass->m_depthStencilAccess.initialState);
                        }
                        textureResource->m_resource.transition(
                            initialState,
                            D3D12_RESOURCE_STATE_DEPTH_WRITE,
                            submitter);
                    }
                    // Clear.
                    uint32_t clearFlags = 0;
                    if (renderPass->m_depthStencilAccess.loadOp ==
                        IRenderPassLayout::AttachmentLoadOp::Clear)
                    {
                        clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                    }
                    if (renderPass->m_depthStencilAccess.stencilLoadOp ==
                        IRenderPassLayout::AttachmentLoadOp::Clear)
                    {
                        clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                    }
                    if (clearFlags)
                    {
                        m_d3dCmdList->ClearDepthStencilView(
                            framebuffer->depthStencilDescriptor,
                            (D3D12_CLEAR_FLAGS)clearFlags,
                            framebuffer->depthStencilClearValue.depth,
                            framebuffer->depthStencilClearValue.stencil,
                            0,
                            nullptr);
                    }
                }
            }

            virtual SLANG_NO_THROW Result SLANG_MCALL
                bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override
            {
                return bindPipelineImpl(state, outRootObject);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setViewports(uint32_t count, const Viewport* viewports) override
            {
                static const int kMaxViewports =
                    D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
                assert(count <= kMaxViewports && count <= kMaxRTVCount);
                for (UInt ii = 0; ii < count; ++ii)
                {
                    auto& inViewport = viewports[ii];
                    auto& dxViewport = m_viewports[ii];

                    dxViewport.TopLeftX = inViewport.originX;
                    dxViewport.TopLeftY = inViewport.originY;
                    dxViewport.Width = inViewport.extentX;
                    dxViewport.Height = inViewport.extentY;
                    dxViewport.MinDepth = inViewport.minZ;
                    dxViewport.MaxDepth = inViewport.maxZ;
                }
                m_d3dCmdList->RSSetViewports(UINT(count), m_viewports);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setScissorRects(uint32_t count, const ScissorRect* rects) override
            {
                static const int kMaxScissorRects =
                    D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
                assert(count <= kMaxScissorRects && count <= kMaxRTVCount);

                for (UInt ii = 0; ii < count; ++ii)
                {
                    auto& inRect = rects[ii];
                    auto& dxRect = m_scissorRects[ii];

                    dxRect.left = LONG(inRect.minX);
                    dxRect.top = LONG(inRect.minY);
                    dxRect.right = LONG(inRect.maxX);
                    dxRect.bottom = LONG(inRect.maxY);
                }

                m_d3dCmdList->RSSetScissorRects(UINT(count), m_scissorRects);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setPrimitiveTopology(PrimitiveTopology topology) override
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

            virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
                UInt startSlot,
                UInt slotCount,
                IBufferResource* const* buffers,
                const UInt* strides,
                const UInt* offsets) override
            {
                {
                    const Index num = startSlot + slotCount;
                    if (num > m_boundVertexBuffers.getCount())
                    {
                        m_boundVertexBuffers.setCount(num);
                    }
                }

                for (UInt i = 0; i < slotCount; i++)
                {
                    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[i]);

                    BoundVertexBuffer& boundBuffer = m_boundVertexBuffers[startSlot + i];
                    boundBuffer.m_buffer = buffer;
                    boundBuffer.m_stride = int(strides[i]);
                    boundBuffer.m_offset = int(offsets[i]);
                }
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setIndexBuffer(
                IBufferResource* buffer,
                Format indexFormat,
                UInt offset = 0) override
            {
                m_boundIndexBuffer = (BufferResourceImpl*)buffer;
                m_boundIndexFormat = D3DUtil::getMapFormat(indexFormat);
                m_boundIndexOffset = UINT(offset);
            }

            void prepareDraw()
            {
                auto pipelineState = m_currentPipeline.Ptr();
                if (!pipelineState || (pipelineState->desc.type != PipelineType::Graphics))
                {
                    assert(!"No graphics pipeline state set");
                    return;
                }

                // Submit - setting for graphics
                {
                    GraphicsSubmitter submitter(m_d3dCmdList);
                    RefPtr<PipelineStateBase> newPipeline;
                    if(SLANG_FAILED(_bindRenderState(&submitter, newPipeline)))
                    {
                        assert(!"Failed to bind render state");
                    }
                }

                m_d3dCmdList->IASetPrimitiveTopology(m_primitiveTopology);

                // Set up vertex buffer views
                {
                    int numVertexViews = 0;
                    D3D12_VERTEX_BUFFER_VIEW vertexViews[16];
                    for (Index i = 0; i < m_boundVertexBuffers.getCount(); i++)
                    {
                        const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[i];
                        BufferResourceImpl* buffer = boundVertexBuffer.m_buffer;
                        if (buffer)
                        {
                            D3D12_VERTEX_BUFFER_VIEW& vertexView = vertexViews[numVertexViews++];
                            vertexView.BufferLocation =
                                buffer->m_resource.getResource()->GetGPUVirtualAddress() +
                                boundVertexBuffer.m_offset;
                            vertexView.SizeInBytes =
                                UINT(buffer->getDesc()->sizeInBytes - boundVertexBuffer.m_offset);
                            vertexView.StrideInBytes = UINT(boundVertexBuffer.m_stride);
                        }
                    }
                    m_d3dCmdList->IASetVertexBuffers(0, numVertexViews, vertexViews);
                }
                // Set up index buffer
                if (m_boundIndexBuffer)
                {
                    D3D12_INDEX_BUFFER_VIEW indexBufferView;
                    indexBufferView.BufferLocation =
                        m_boundIndexBuffer->m_resource.getResource()->GetGPUVirtualAddress() +
                        m_boundIndexOffset;
                    indexBufferView.SizeInBytes =
                        UINT(m_boundIndexBuffer->getDesc()->sizeInBytes - m_boundIndexOffset);
                    indexBufferView.Format = m_boundIndexFormat;

                    m_d3dCmdList->IASetIndexBuffer(&indexBufferView);
                }
            }
            virtual SLANG_NO_THROW void SLANG_MCALL
                draw(UInt vertexCount, UInt startVertex = 0) override
            {
                prepareDraw();
                m_d3dCmdList->DrawInstanced(UINT(vertexCount), 1, UINT(startVertex), 0);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL
                drawIndexed(UInt indexCount, UInt startIndex = 0, UInt baseVertex = 0) override
            {
                prepareDraw();
                m_d3dCmdList->DrawIndexedInstanced(
                    (UINT)indexCount, 1, (UINT)startIndex, (UINT)baseVertex, 0);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                PipelineCommandEncoder::endEncodingImpl();
                // Issue clear commands based on render pass set up.
                for (Index i = 0; i < m_renderPass->m_renderTargetAccesses.getCount(); i++)
                {
                    auto& access = m_renderPass->m_renderTargetAccesses[i];

                    // Transit resource states.
                    {
                        D3D12BarrierSubmitter submitter(m_d3dCmdList);
                        auto resourceViewImpl = m_framebuffer->renderTargetViews[i].Ptr();
                        auto textureResource =
                            static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
                        textureResource->m_resource.transition(
                            D3D12_RESOURCE_STATE_RENDER_TARGET,
                            D3DUtil::translateResourceState(access.finalState),
                            submitter);
                    }
                }

                if (m_renderPass->m_hasDepthStencil)
                {
                    // Transit resource states.
                    D3D12BarrierSubmitter submitter(m_d3dCmdList);
                    auto resourceViewImpl = m_framebuffer->depthStencilView.Ptr();
                    auto textureResource =
                        static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
                    textureResource->m_resource.transition(
                        D3D12_RESOURCE_STATE_DEPTH_WRITE,
                        D3DUtil::translateResourceState(
                            m_renderPass->m_depthStencilAccess.finalState),
                        submitter);
                }
                m_framebuffer = nullptr;
            }

            virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* pool, SlangInt index) override
            {
                static_cast<QueryPoolImpl*>(pool)->writeTimestamp(m_d3dCmdList, index);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setStencilReference(uint32_t referenceValue) override
            {
                m_d3dCmdList->OMSetStencilRef((UINT)referenceValue);
            }
        };

        RenderCommandEncoderImpl m_renderCommandEncoder;
        virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
            IRenderPassLayout* renderPass,
            IFramebuffer* framebuffer,
            IRenderCommandEncoder** outEncoder) override
        {
            m_renderCommandEncoder.init(
                m_renderer,
                m_transientHeap,
                this,
                static_cast<RenderPassLayoutImpl*>(renderPass),
                static_cast<FramebufferImpl*>(framebuffer));
            *outEncoder = &m_renderCommandEncoder;
        }

        class ComputeCommandEncoderImpl
            : public IComputeCommandEncoder
            , public PipelineCommandEncoder
        {
        public:
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                PipelineCommandEncoder::endEncodingImpl();
            }
            virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* pool, SlangInt index) override
            {
                static_cast<QueryPoolImpl*>(pool)->writeTimestamp(m_d3dCmdList, index);
            }
            void init(
                D3D12Device* renderer,
                TransientResourceHeapImpl* transientHeap,
                CommandBufferImpl* cmdBuffer)
            {
                PipelineCommandEncoder::init(cmdBuffer);
                m_preCmdList = nullptr;
                m_transientHeap = transientHeap;
                m_currentPipeline = nullptr;
            }

            virtual SLANG_NO_THROW Result SLANG_MCALL
                bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override
            {
                return bindPipelineImpl(state, outRootObject);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
            {
                // Submit binding for compute
                {
                    ComputeSubmitter submitter(m_d3dCmdList);
                    RefPtr<PipelineStateBase> newPipeline;
                    if (SLANG_FAILED(_bindRenderState(&submitter, newPipeline)))
                    {
                        assert(!"Failed to bind render state");
                    }
                }
                m_d3dCmdList->Dispatch(x, y, z);
            }
        };

        ComputeCommandEncoderImpl m_computeCommandEncoder;
        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeComputeCommands(IComputeCommandEncoder** outEncoder) override
        {
            m_computeCommandEncoder.init(m_renderer, m_transientHeap, this);
            *outEncoder = &m_computeCommandEncoder;
        }

        class ResourceCommandEncoderImpl : public IResourceCommandEncoder
        {
        public:
            CommandBufferImpl* m_commandBuffer;
            void init(D3D12Device* renderer, CommandBufferImpl* commandBuffer)
            {
                m_commandBuffer = commandBuffer;
            }
            virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
                IBufferResource* dst,
                size_t dstOffset,
                IBufferResource* src,
                size_t srcOffset,
                size_t size) override
            {
                auto dstBuffer = static_cast<BufferResourceImpl*>(dst);
                auto srcBuffer = static_cast<BufferResourceImpl*>(src);

                m_commandBuffer->m_cmdList->CopyBufferRegion(
                    dstBuffer->m_resource.getResource(),
                    dstOffset,
                    srcBuffer->m_resource.getResource(),
                    srcOffset,
                    size);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL uploadBufferData(
                IBufferResource* dst,
                size_t offset,
                size_t size,
                void* data) override
            {
                _uploadBufferData(
                    m_commandBuffer->m_cmdList,
                    static_cast<BufferResourceImpl*>(dst),
                    offset,
                    size,
                    data);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() {}
            virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* pool, SlangInt index) override
            {
                static_cast<QueryPoolImpl*>(pool)->writeTimestamp(m_commandBuffer->m_cmdList, index);
            }
        };

        ResourceCommandEncoderImpl m_resourceCommandEncoder;

        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeResourceCommands(IResourceCommandEncoder** outEncoder) override
        {
            m_resourceCommandEncoder.init(m_renderer, this);
            *outEncoder = &m_resourceCommandEncoder;
        }

#if SLANG_GFX_HAS_DXR_SUPPORT
        class RayTracingCommandEncoderImpl
            : public IRayTracingCommandEncoder
            , public PipelineCommandEncoder
        {
        public:
            CommandBufferImpl* m_commandBuffer;
            void init(D3D12Device* renderer, CommandBufferImpl* commandBuffer)
            {
                PipelineCommandEncoder::init(commandBuffer);
                m_commandBuffer = commandBuffer;
            }
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
            virtual SLANG_NO_THROW void SLANG_MCALL serializeAccelerationStructure(
                DeviceAddress dest,
                IAccelerationStructure* source) override;
            virtual SLANG_NO_THROW void SLANG_MCALL deserializeAccelerationStructure(
                IAccelerationStructure* dest,
                DeviceAddress source) override;
            virtual SLANG_NO_THROW void SLANG_MCALL memoryBarrier(
                int count,
                IAccelerationStructure* const* structures,
                AccessFlag::Enum sourceAccess,
                AccessFlag::Enum destAccess) override;
            virtual SLANG_NO_THROW void SLANG_MCALL
                bindPipeline(IPipelineState* state, IShaderObject** outRootObject) override;
            virtual SLANG_NO_THROW void SLANG_MCALL dispatchRays(
                const char* rayGenShaderName,
                int32_t width,
                int32_t height,
                int32_t depth) override;
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() {}
            virtual SLANG_NO_THROW void SLANG_MCALL
                writeTimestamp(IQueryPool* pool, SlangInt index) override
            {
                static_cast<QueryPoolImpl*>(pool)->writeTimestamp(
                    m_commandBuffer->m_cmdList, index);
            }
        };
        RayTracingCommandEncoderImpl m_rayTracingCommandEncoder;
        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) override
        {
            m_rayTracingCommandEncoder.init(m_renderer, this);
            *outEncoder = &m_rayTracingCommandEncoder;
        }
#else
        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) override
        {
            *outEncoder = nullptr;
        }
#endif

        virtual SLANG_NO_THROW void SLANG_MCALL close() override { m_cmdList->Close(); }
    };

    class CommandQueueImpl
        : public ICommandQueue
        , public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        ICommandQueue* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandQueue)
                return static_cast<ICommandQueue*>(this);
            return nullptr;
        }
        void breakStrongReferenceToDevice() { m_renderer.breakStrongReference(); }

    public:
        BreakableReference<D3D12Device> m_renderer;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12CommandQueue> m_d3dQueue;
        ComPtr<ID3D12Fence> m_fence;
        uint64_t m_fenceValue = 0;
        HANDLE globalWaitHandle;
        Desc m_desc;
        uint32_t m_queueIndex = 0;
        
        Result init(D3D12Device* device, uint32_t queueIndex)
        {
            m_queueIndex = queueIndex;
            m_renderer = device;
            m_device = device->m_device;
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            SLANG_RETURN_ON_FAIL(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_d3dQueue.writeRef())));
            SLANG_RETURN_ON_FAIL(
                m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.writeRef())));
            globalWaitHandle = CreateEventEx(
                nullptr,
                nullptr,
                CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET,
                EVENT_ALL_ACCESS);
            return SLANG_OK;
        }
        ~CommandQueueImpl()
        {
            wait();
            CloseHandle(globalWaitHandle);
            m_renderer->m_queueIndexAllocator.free((int)m_queueIndex, 1);
        }
        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override
        {
            return m_desc;
        }
        
        virtual SLANG_NO_THROW void SLANG_MCALL
            executeCommandBuffers(uint32_t count, ICommandBuffer* const* commandBuffers) override
        {
            ShortList<ID3D12CommandList*> commandLists;
            for (uint32_t i = 0; i < count; i++)
            {
                auto cmdImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
                commandLists.add(cmdImpl->m_cmdList);
            }
            m_d3dQueue->ExecuteCommandLists((UINT)count, commandLists.getArrayView().getBuffer());

            m_fenceValue++;

            for (uint32_t i = 0; i < count; i++)
            {
                if (i > 0 && commandBuffers[i] == commandBuffers[i - 1])
                    continue;
                auto cmdImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
                auto transientHeap = cmdImpl->m_transientHeap;
                auto& waitInfo = transientHeap->getQueueWaitInfo(m_queueIndex);
                waitInfo.waitValue = m_fenceValue;
                waitInfo.fence = m_fence;
            }
            m_d3dQueue->Signal(m_fence, m_fenceValue);
            ResetEvent(globalWaitHandle);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL wait() override
        {
            m_fenceValue++;
            m_d3dQueue->Signal(m_fence, m_fenceValue);
            ResetEvent(globalWaitHandle);
            m_fence->SetEventOnCompletion(m_fenceValue, globalWaitHandle);
            WaitForSingleObject(globalWaitHandle, INFINITE);
        }
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
        Result init(
            D3D12Device* renderer,
            const ISwapchain::Desc& swapchainDesc,
            WindowHandle window)
        {
            m_queue = static_cast<CommandQueueImpl*>(swapchainDesc.queue)->m_d3dQueue;
            m_dxgiFactory = renderer->m_deviceInfo.m_dxgiFactory;
            SLANG_RETURN_ON_FAIL(
                D3DSwapchainBase::init(swapchainDesc, window, DXGI_SWAP_EFFECT_FLIP_DISCARD));
            renderer->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.writeRef()));

            SLANG_RETURN_ON_FAIL(m_swapChain->QueryInterface(m_swapChain3.writeRef()));
            for (uint32_t i = 0; i < swapchainDesc.imageCount; i++)
            {
                m_frameEvents.add(CreateEventEx(
                    nullptr,
                    false,
                    CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET,
                    EVENT_ALL_ACCESS));
            }
            return SLANG_OK;
        }

        virtual void createSwapchainBufferImages() override
        {
            m_images.clear();
            
            for (uint32_t i = 0; i < m_desc.imageCount; i++)
            {
                ComPtr<ID3D12Resource> d3dResource;
                m_swapChain->GetBuffer(i, IID_PPV_ARGS(d3dResource.writeRef()));
                ITextureResource::Desc imageDesc = {};
                imageDesc.allowedStates = ResourceStateSet(
                    ResourceState::Present,
                    ResourceState::RenderTarget,
                    ResourceState::CopyDestination);
                imageDesc.type = IResource::Type::Texture2D;
                imageDesc.arraySize = 0;
                imageDesc.format = m_desc.format;
                imageDesc.size.width = m_desc.width;
                imageDesc.size.height = m_desc.height;
                imageDesc.size.depth = 1;
                imageDesc.numMipLevels = 1;
                imageDesc.defaultState = ResourceState::Present;
                RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc);
                image->m_resource.setResource(d3dResource.get());
                image->m_defaultState = D3D12_RESOURCE_STATE_PRESENT;
                m_images.add(image);
            }
            for (auto evt : m_frameEvents)
                SetEvent(evt);
        }
        virtual IDXGIFactory* getDXGIFactory() override { return m_dxgiFactory; }
        virtual IUnknown* getOwningDevice() override { return m_queue; }
        virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override
        {
            auto result = (int)m_swapChain3->GetCurrentBackBufferIndex();
            WaitForSingleObject(m_frameEvents[result], INFINITE);
            ResetEvent(m_frameEvents[result]);
            return result;
        }
        virtual SLANG_NO_THROW Result SLANG_MCALL present() override
        {
            SLANG_RETURN_ON_FAIL(D3DSwapchainBase::present());
            fenceValue++;
            m_fence->SetEventOnCompletion(fenceValue, m_frameEvents[m_swapChain3->GetCurrentBackBufferIndex()]);
            m_queue->Signal(m_fence, fenceValue);
            return SLANG_OK;
        }
    };

    static PROC loadProc(HMODULE module, char const* name);

    Result createCommandQueueImpl(CommandQueueImpl** outQueue);

    Result createTransientResourceHeapImpl(
        size_t constantBufferSize,
        uint32_t viewDescriptors,
        uint32_t samplerDescriptors,
        TransientResourceHeapImpl** outHeap);

    Result createBuffer(
        const D3D12_RESOURCE_DESC& resourceDesc,
        const void* srcData,
        size_t srcDataSize,
        D3D12Resource& uploadResource,
        D3D12_RESOURCE_STATES finalState,
        D3D12Resource& resourceOut);

    Result captureTextureToSurface(
        D3D12Resource& resource,
        ResourceState state,
        ISlangBlob** blob,
        size_t* outRowPitch,
        size_t* outPixelSize);

    Result _createDevice(
        DeviceCheckFlags deviceCheckFlags,
        const UnownedStringSlice& nameMatch,
        D3D_FEATURE_LEVEL featureLevel,
        DeviceInfo& outDeviceInfo);

    struct ResourceCommandRecordInfo
    {
        ComPtr<ICommandBuffer> commandBuffer;
        ID3D12GraphicsCommandList* d3dCommandList;
    };
    ResourceCommandRecordInfo encodeResourceCommands()
    {
        ResourceCommandRecordInfo info;
        m_resourceCommandTransientHeap->createCommandBuffer(info.commandBuffer.writeRef());
        info.d3dCommandList = static_cast<CommandBufferImpl*>(info.commandBuffer.get())->m_cmdList;
        return info;
    }
    void submitResourceCommandsAndWait(const ResourceCommandRecordInfo& info)
    {
        info.commandBuffer->close();
        m_resourceCommandQueue->executeCommandBuffer(info.commandBuffer);
        m_resourceCommandTransientHeap->synchronizeAndReset();
    }

    // D3D12Device members.

    Desc m_desc;

    gfx::DeviceInfo m_info;
    String m_adapterName;

    bool m_isInitialized = false;

    ComPtr<ID3D12Debug> m_dxDebug;

    DeviceInfo m_deviceInfo;
    ID3D12Device* m_device = nullptr;
    ID3D12Device5* m_device5 = nullptr;

    VirtualObjectPool m_queueIndexAllocator;

    RefPtr<CommandQueueImpl> m_resourceCommandQueue;
    RefPtr<TransientResourceHeapImpl> m_resourceCommandTransientHeap;

    RefPtr<D3D12GeneralDescriptorHeap> m_rtvAllocator;
    RefPtr<D3D12GeneralDescriptorHeap> m_dsvAllocator;
    // Space in the GPU-visible heaps is precious, so we will also keep
    // around CPU-visible heaps for storing descriptors in a format
    // that is ready for copying into the GPU-visible heaps as needed.
    //
    RefPtr<D3D12GeneralDescriptorHeap> m_cpuViewHeap; ///< Cbv, Srv, Uav
    RefPtr<D3D12GeneralDescriptorHeap> m_cpuSamplerHeap; ///< Heap for samplers

    // Dll entry points
    PFN_D3D12_GET_DEBUG_INTERFACE m_D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE m_D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE m_D3D12SerializeRootSignature = nullptr;

    bool m_nvapi = false;
};

SLANG_NO_THROW Result SLANG_MCALL D3D12Device::TransientResourceHeapImpl::synchronizeAndReset()
{
    Array<HANDLE, 16> waitHandles;
    for (auto& waitInfo : m_waitInfos)
    {
        if (waitInfo.waitValue == 0)
            continue;
        if (waitInfo.fence)
        {
            waitInfo.fence->SetEventOnCompletion(waitInfo.waitValue, waitInfo.fenceEvent);
            waitHandles.add(waitInfo.fenceEvent);
        }
    }
    WaitForMultipleObjects((DWORD)waitHandles.getCount(), waitHandles.getBuffer(), TRUE, INFINITE);
    m_viewHeap.deallocateAll();
    m_samplerHeap.deallocateAll();
    m_commandListAllocId = 0;
    SLANG_RETURN_ON_FAIL(m_commandAllocator->Reset());
    Super::reset();
    return SLANG_OK;
}

Result D3D12Device::TransientResourceHeapImpl::createCommandBuffer(ICommandBuffer** outCmdBuffer)
{
    if ((Index)m_commandListAllocId < m_commandBufferPool.getCount())
    {
        auto result = static_cast<D3D12Device::CommandBufferImpl*>(
            m_commandBufferPool[m_commandListAllocId].Ptr());
        m_d3dCommandListPool[m_commandListAllocId]->Reset(m_commandAllocator, nullptr);
        result->init(m_device, m_d3dCommandListPool[m_commandListAllocId], this);
        ++m_commandListAllocId;
        returnComPtr(outCmdBuffer, result);
        return SLANG_OK;
    }
    ComPtr<ID3D12GraphicsCommandList> cmdList;
    m_device->m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator,
        nullptr,
        IID_PPV_ARGS(cmdList.writeRef()));

    m_d3dCommandListPool.add(cmdList);
    RefPtr<CommandBufferImpl> cmdBuffer = new CommandBufferImpl();
    cmdBuffer->init(m_device, cmdList, this);
    m_commandBufferPool.add(cmdBuffer);
    ++m_commandListAllocId;
    returnComPtr(outCmdBuffer, cmdBuffer);
    return SLANG_OK;
}

Result D3D12Device::PipelineCommandEncoder::_bindRenderState(Submitter* submitter, RefPtr<PipelineStateBase>& newPipeline)
{
    RootShaderObjectImpl* rootObjectImpl = &m_commandBuffer->m_rootShaderObject;
    m_renderer->maybeSpecializePipeline(m_currentPipeline, rootObjectImpl, newPipeline);
    PipelineStateBase* newPipelineImpl = static_cast<PipelineStateBase*>(newPipeline.Ptr());
    auto commandList = m_d3dCmdList;
    auto pipelineTypeIndex = (int)newPipelineImpl->desc.type;
    auto programImpl = static_cast<ShaderProgramImpl*>(newPipelineImpl->m_program.Ptr());
    submitter->setPipelineState(newPipelineImpl);
    submitter->setRootSignature(programImpl->m_rootObjectLayout->m_rootSignature);
    RefPtr<ShaderObjectLayoutImpl> specializedRootLayout;
    SLANG_RETURN_ON_FAIL(rootObjectImpl->getSpecializedLayout(specializedRootLayout.writeRef()));
    RootShaderObjectLayoutImpl* rootLayoutImpl =
        static_cast<RootShaderObjectLayoutImpl*>(specializedRootLayout.Ptr());

    // We need to set up a context for binding shader objects to the pipeline state.
    // This type mostly exists to bundle together a bunch of parameters that would
    // otherwise need to be tunneled down through all the shader object binding
    // logic.
    //
    BindingContext context = {};
    context.encoder = this;
    context.submitter = submitter;
    context.device = m_renderer;
    context.transientHeap = m_transientHeap;

    // We kick off binding of shader objects at the root object, and the objects
    // themselves will be responsible for allocating, binding, and filling in
    // any descriptor tables or other root parameters needed.
    //
    SLANG_RETURN_ON_FAIL(rootObjectImpl->bindAsRoot(&context, rootLayoutImpl));
    
    return SLANG_OK;
}

Result D3D12Device::createTransientResourceHeapImpl(
    size_t constantBufferSize,
    uint32_t viewDescriptors,
    uint32_t samplerDescriptors,
    TransientResourceHeapImpl** outHeap)
{
    RefPtr<TransientResourceHeapImpl> result = new TransientResourceHeapImpl();
    ITransientResourceHeap::Desc desc = {};
    desc.constantBufferSize = constantBufferSize;
    SLANG_RETURN_ON_FAIL(result->init(desc, this, viewDescriptors, samplerDescriptors));
    returnRefPtrMove(outHeap, result);
    return SLANG_OK;
}

Result D3D12Device::createCommandQueueImpl(D3D12Device::CommandQueueImpl** outQueue)
{
    int queueIndex = m_queueIndexAllocator.alloc(1);
    // If we run out of queue index space, then the user is requesting too many queues.
    if (queueIndex == -1)
        return SLANG_FAIL;

    RefPtr<D3D12Device::CommandQueueImpl> queue = new D3D12Device::CommandQueueImpl();
    SLANG_RETURN_ON_FAIL(queue->init(this, (uint32_t)queueIndex));
    returnRefPtrMove(outQueue, queue);
    return SLANG_OK;
}

SlangResult SLANG_MCALL createD3D12Device(const IDevice::Desc* desc, IDevice** outDevice)
{
    RefPtr<D3D12Device> result = new D3D12Device();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outDevice, result);
    return SLANG_OK;
}

/* static */PROC D3D12Device::loadProc(HMODULE module, char const* name)
{
    PROC proc = ::GetProcAddress(module, name);
    if (!proc)
    {
        fprintf(stderr, "error: failed load symbol '%s'\n", name);
        return nullptr;
    }
    return proc;
}

D3D12Device::~D3D12Device() { m_shaderObjectLayoutCache = decltype(m_shaderObjectLayoutCache)(); }

static void _initSrvDesc(IResource::Type resourceType, const ITextureResource::Desc& textureDesc, const D3D12_RESOURCE_DESC& desc, DXGI_FORMAT pixelFormat, D3D12_SHADER_RESOURCE_VIEW_DESC& descOut)
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
    else if (resourceType == IResource::Type::TextureCube)
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

Result D3D12Device::createBuffer(const D3D12_RESOURCE_DESC& resourceDesc, const void* srcData, size_t srcDataSize, D3D12Resource& uploadResource, D3D12_RESOURCE_STATES finalState, D3D12Resource& resourceOut)
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
        ::memcpy(dstData, srcData, srcDataSize);
        dxUploadResource->Unmap(0, nullptr);

        auto encodeInfo = encodeResourceCommands();
        encodeInfo.d3dCommandList->CopyBufferRegion(resourceOut, 0, uploadResource, 0, bufferSize);
        submitResourceCommandsAndWait(encodeInfo);
    }

    return SLANG_OK;
}

Result D3D12Device::captureTextureToSurface(
    D3D12Resource& resource,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    const D3D12_RESOURCE_STATES initialState = D3DUtil::translateResourceState(state);

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
    if (outRowPitch)
        *outRowPitch = rowPitch;
    if (outPixelSize)
        *outPixelSize = bytesPerPixel;
 
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

    auto encodeInfo = encodeResourceCommands();
    auto currentState = D3DUtil::translateResourceState(state);

    {
        D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
        resource.transition(currentState, D3D12_RESOURCE_STATE_COPY_SOURCE, submitter);
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

        encodeInfo.d3dCommandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
    }

    {
        D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
        resource.transition(D3D12_RESOURCE_STATE_COPY_SOURCE, currentState, submitter);
    }

    // Submit the copy, and wait for copy to complete
    submitResourceCommandsAndWait(encodeInfo);

    {
        ID3D12Resource* dxResource = stagingResource;

        UINT8* data;
        D3D12_RANGE readRange = {0, bufferSize};

        SLANG_RETURN_ON_FAIL(dxResource->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        RefPtr<Slang::ListBlob> resultBlob = new Slang::ListBlob();
        resultBlob->m_data.setCount(bufferSize);
        memcpy(resultBlob->m_data.getBuffer(), data, bufferSize);
        dxResource->Unmap(0, nullptr);
        returnComPtr(outBlob, resultBlob);
        return SLANG_OK;
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

Result D3D12Device::_createDevice(DeviceCheckFlags deviceCheckFlags, const UnownedStringSlice& nameMatch, D3D_FEATURE_LEVEL featureLevel, DeviceInfo& outDeviceInfo)
{
    outDeviceInfo.clear();

    ComPtr<IDXGIFactory> dxgiFactory;
    SLANG_RETURN_ON_FAIL(D3DUtil::createFactory(deviceCheckFlags, dxgiFactory));

    List<ComPtr<IDXGIAdapter>> dxgiAdapters;
    SLANG_RETURN_ON_FAIL(D3DUtil::findAdapters(deviceCheckFlags, nameMatch, dxgiFactory, dxgiAdapters));

    ComPtr<ID3D12Device> device;
    ComPtr<IDXGIAdapter> adapter;

    for (Index i = 0; i < dxgiAdapters.getCount(); ++i)
    {
        IDXGIAdapter* dxgiAdapter = dxgiAdapters[i];
        if (SLANG_SUCCEEDED(m_D3D12CreateDevice(dxgiAdapter, featureLevel, IID_PPV_ARGS(device.writeRef()))))
        {
            adapter = dxgiAdapter;
            break;
        }
    }

    if (!device)
    {
        return SLANG_FAIL;
    }

    if (m_dxDebug && (deviceCheckFlags & DeviceCheckFlag::UseDebug))
    {
        m_dxDebug->EnableDebugLayer();

        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SLANG_SUCCEEDED(device->QueryInterface(infoQueue.writeRef())))
        {
            // Make break 
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            // infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

            // Apparently there is a problem with sm 6.3 with spurious errors, with debug layer enabled
            D3D12_FEATURE_DATA_SHADER_MODEL featureShaderModel;
            featureShaderModel.HighestShaderModel = D3D_SHADER_MODEL(0x63);
            SLANG_SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &featureShaderModel, sizeof(featureShaderModel)));

            if (featureShaderModel.HighestShaderModel >= D3D_SHADER_MODEL(0x63))
            {
                // Filter out any messages that cause issues
                // TODO: Remove this when the debug layers work properly
                D3D12_MESSAGE_ID messageIds[] =
                {
                    // When the debug layer is enabled this error is triggered sometimes after a CopyDescriptorsSimple
                    // call The failed check validates that the source and destination ranges of the copy do not
                    // overlap. The check assumes descriptor handles are pointers to memory, but this is not always the
                    // case and the check fails (even though everything is okay).
                    D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES,
                };

                // We filter INFO messages because they are way too many
                D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

                D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
                infoQueueFilter.DenyList.NumSeverities = SLANG_COUNT_OF(severities);
                infoQueueFilter.DenyList.pSeverityList = severities;
                infoQueueFilter.DenyList.NumIDs = SLANG_COUNT_OF(messageIds);
                infoQueueFilter.DenyList.pIDList = messageIds;

                infoQueue->PushStorageFilter(&infoQueueFilter);
            }
        }
    }

    // Get the descs
    {
        adapter->GetDesc(&outDeviceInfo.m_desc);

        // Look up GetDesc1 info
        ComPtr<IDXGIAdapter1> adapter1;
        if (SLANG_SUCCEEDED(adapter->QueryInterface(adapter1.writeRef())))
        {
            adapter1->GetDesc1(&outDeviceInfo.m_desc1);
        }
    }

    // Save other info
    outDeviceInfo.m_device = device;
    outDeviceInfo.m_dxgiFactory = dxgiFactory;
    outDeviceInfo.m_adapter = adapter;
    outDeviceInfo.m_isWarp = D3DUtil::isWarp(dxgiFactory, adapter);

    return SLANG_OK;
}

static bool _isSupportedNVAPIOp(ID3D12Device* dev, uint32_t op)
{
#ifdef GFX_NVAPI
    {
        bool isSupported;
        NvAPI_Status status = NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(dev, NvU32(op), &isSupported);
        return status == NVAPI_OK && isSupported;
    }
#else
    return false;
#endif
}

Result D3D12Device::initialize(const Desc& desc)
{

    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

    // Initialize queue index allocator.
    // Support max 32 queues.
    m_queueIndexAllocator.initPool(32);

    // Initialize DeviceInfo
    {
        m_info.deviceType = DeviceType::DirectX12;
        m_info.bindingStyle = BindingStyle::DirectX;
        m_info.projectionStyle = ProjectionStyle::DirectX;
        m_info.apiName = "Direct3D 12";
        static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    // Rather than statically link against D3D, we load it dynamically.

    HMODULE d3dModule = LoadLibraryA("d3d12.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d12.dll'\n");
        return SLANG_FAIL;
    }

    // Get all the dll entry points
    m_D3D12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)loadProc(d3dModule, "D3D12SerializeRootSignature");
    if (!m_D3D12SerializeRootSignature)
    {
        return SLANG_FAIL;
    }

#if ENABLE_DEBUG_LAYER
    m_D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)loadProc(d3dModule, "D3D12GetDebugInterface");
    if (m_D3D12GetDebugInterface)
    {
        if (SLANG_SUCCEEDED(m_D3D12GetDebugInterface(IID_PPV_ARGS(m_dxDebug.writeRef()))))
        {
#if 0
            // Can enable for extra validation. NOTE! That d3d12 warns if you do.... 
            // D3D12 MESSAGE : Device Debug Layer Startup Options : GPU - Based Validation is enabled(disabled by default).
            // This results in new validation not possible during API calls on the CPU, by creating patched shaders that have validation
            // added directly to the shader. However, it can slow things down a lot, especially for applications with numerous
            // PSOs.Time to see the first render frame may take several minutes.
            // [INITIALIZATION MESSAGE #1016: CREATEDEVICE_DEBUG_LAYER_STARTUP_OPTIONS]

            ComPtr<ID3D12Debug1> debug1;
            if (SLANG_SUCCEEDED(m_dxDebug->QueryInterface(debug1.writeRef())))
            {
                debug1->SetEnableGPUBasedValidation(true);
            }
#endif
            m_dxDebug->EnableDebugLayer();
        }
    }
#endif

    m_D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)loadProc(d3dModule, "D3D12CreateDevice");
    if (!m_D3D12CreateDevice)
    {
        return SLANG_FAIL;
    }

    FlagCombiner combiner;
    // TODO: we should probably provide a command-line option
    // to override UseDebug of default rather than leave it
    // up to each back-end to specify.
#if ENABLE_DEBUG_LAYER
    combiner.add(DeviceCheckFlag::UseDebug, ChangeType::OnOff);                 ///< First try debug then non debug
#else
    combiner.add(DeviceCheckFlag::UseDebug, ChangeType::Off);                   ///< Don't bother with debug
#endif
    combiner.add(DeviceCheckFlag::UseHardwareDevice, ChangeType::OnOff);        ///< First try hardware, then reference
    
    const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    const int numCombinations = combiner.getNumCombinations();
    for (int i = 0; i < numCombinations; ++i)
    {
        if (SLANG_SUCCEEDED(_createDevice(combiner.getCombination(i), UnownedStringSlice(desc.adapter), featureLevel, m_deviceInfo)))
        {
            break;
        }
    }

    if (!m_deviceInfo.m_adapter)
    {
        // Couldn't find an adapter
        return SLANG_FAIL;
    }

    // Set the device
    m_device = m_deviceInfo.m_device;

    // NVAPI
    if (desc.nvapiExtnSlot >= 0)
    {
        if (SLANG_FAILED(NVAPIUtil::initialize()))
        {
            return SLANG_E_NOT_AVAILABLE;
        }

#ifdef GFX_NVAPI
        // From DOCS: Applications are expected to bind null UAV to this slot.
        // NOTE! We don't currently do this, but doesn't seem to be a problem.

        const NvAPI_Status status = NvAPI_D3D12_SetNvShaderExtnSlotSpace(m_device, NvU32(desc.nvapiExtnSlot), NvU32(0));
        
        if (status != NVAPI_OK)
        {
            return SLANG_E_NOT_AVAILABLE;
        }

        if (_isSupportedNVAPIOp(m_device, NV_EXTN_OP_UINT64_ATOMIC))
        {
            m_features.add("atomic-int64");
        }
        if (_isSupportedNVAPIOp(m_device, NV_EXTN_OP_FP32_ATOMIC))
        {
            m_features.add("atomic-float");
        }

        m_nvapi = true;
#endif

    }

    // Find what features are supported
    {
        // Check this is how this is laid out...
        SLANG_COMPILE_TIME_ASSERT(D3D_SHADER_MODEL_6_0 == 0x60);

        {
            D3D12_FEATURE_DATA_SHADER_MODEL featureShaderModel;
            featureShaderModel.HighestShaderModel = D3D_SHADER_MODEL(0x62);

            // TODO: Currently warp causes a crash when using half, so disable for now
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &featureShaderModel, sizeof(featureShaderModel))) &&
                m_deviceInfo.m_isWarp == false &&
                featureShaderModel.HighestShaderModel >= 0x62)
            {
                // With sm_6_2 we have half
                m_features.add("half");
            }
        }
        // Check what min precision support we have
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
            {
                auto minPrecisionSupport = options.MinPrecisionSupport;
            }
        }
        // Check ray tracing support
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                    D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options))))
            {
                if (options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
                {
                    m_features.add("ray-tracing");
                }
                if (options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1)
                {
                    m_features.add("ray-query");
                }
            }
        }
    }

    m_desc = desc;

    // Create a command queue for internal resource transfer operations.
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(m_resourceCommandQueue.writeRef()));
    // `CommandQueueImpl` holds a back reference to `D3D12Device`, make it a weak reference here
    // since this object is already owned by `D3D12Device`.
    m_resourceCommandQueue->breakStrongReferenceToDevice();
    // Retrieve timestamp frequency.
    m_resourceCommandQueue->m_d3dQueue->GetTimestampFrequency(&m_info.timestampFrequency);

    SLANG_RETURN_ON_FAIL(createTransientResourceHeapImpl(0, 8, 4, m_resourceCommandTransientHeap.writeRef()));
    // `TransientResourceHeap` holds a back reference to `D3D12Device`, make it a weak reference here
    // since this object is already owned by `D3D12Device`.
    m_resourceCommandTransientHeap->breakStrongReferenceToDevice();

    m_cpuViewHeap = new D3D12GeneralDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_cpuViewHeap->init(
        m_device, 8192, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
    m_cpuSamplerHeap = new D3D12GeneralDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_cpuSamplerHeap->init(
        m_device, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

    m_rtvAllocator = new D3D12GeneralDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_rtvAllocator->init(
        m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
    m_dsvAllocator = new D3D12GeneralDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_dsvAllocator->init(
        m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

    ComPtr<IDXGIDevice> dxgiDevice;
    if (m_deviceInfo.m_adapter)
    {
        DXGI_ADAPTER_DESC adapterDesc;
        m_deviceInfo.m_adapter->GetDesc(&adapterDesc);
        m_adapterName = String::fromWString(adapterDesc.Description);
        m_info.adapterName = m_adapterName.begin();
    }

    // Initialize DXR interface.
#if SLANG_GFX_HAS_DXR_SUPPORT
    m_device->QueryInterface<ID3D12Device5>(m_deviceInfo.m_device5.writeRef());
    m_device5 = m_deviceInfo.m_device5.get();
#endif
    SLANG_RETURN_ON_FAIL(slangContext.initialize(
        desc.slang,
        m_device5 ? SLANG_DXIL : SLANG_DXBC,
        m_device5 ? "sm_6_5" : "sm_5_1",
        makeArray(slang::PreprocessorMacroDesc{"__D3D12__", "1"}).getView()));

    m_isInitialized = true;
    return SLANG_OK;
}

Result D3D12Device::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc,
    ITransientResourceHeap** outHeap)
{
    RefPtr<TransientResourceHeapImpl> heap;
    SLANG_RETURN_ON_FAIL(
        createTransientResourceHeapImpl(desc.constantBufferSize, 8192, 1024, heap.writeRef()));
    returnComPtr(outHeap, heap);
    return SLANG_OK;
}

Result D3D12Device::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    RefPtr<CommandQueueImpl> queue;
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(queue.writeRef()));
    returnComPtr(outQueue, queue);
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL D3D12Device::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> swapchain = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(swapchain->init(this, desc, window));
    returnComPtr(outSwapchain, swapchain);
    return SLANG_OK;
}

SlangResult D3D12Device::readTextureResource(
    ITextureResource* resource,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    return captureTextureToSurface(
        static_cast<TextureResourceImpl*>(resource)->m_resource,
        state,
        outBlob,
        outRowPitch,
        outPixelSize);
}

static D3D12_RESOURCE_FLAGS _calcResourceFlag(ResourceState state)
{
    switch (state)
    {
    case ResourceState::RenderTarget:
        return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    case ResourceState::DepthRead:
    case ResourceState::DepthWrite:
        return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    case ResourceState::UnorderedAccess:
    case ResourceState::AccelerationStructure:
        return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    default:
        return D3D12_RESOURCE_FLAG_NONE;
    }
}

static D3D12_RESOURCE_FLAGS _calcResourceFlags(ResourceStateSet states)
{
    int dstFlags = 0;
    for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; i++)
    {
        auto state = (ResourceState)i;
        if (states.contains(state))
            dstFlags |= _calcResourceFlag(state);
    }
    return (D3D12_RESOURCE_FLAGS)dstFlags;
}

static D3D12_RESOURCE_DIMENSION _calcResourceDimension(IResource::Type type)
{
    switch (type)
    {
        case IResource::Type::Buffer:        return D3D12_RESOURCE_DIMENSION_BUFFER;
        case IResource::Type::Texture1D:     return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case IResource::Type::TextureCube:
        case IResource::Type::Texture2D:
        {
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        }
        case IResource::Type::Texture3D:     return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:                            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}

Result D3D12Device::createTextureResource(const ITextureResource::Desc& descIn, const ITextureResource::SubresourceData* initData, ITextureResource** outResource)
{
    // Description of uploading on Dx12
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215%28v=vs.85%29.aspx

    TextureResource::Desc srcDesc = fixupTextureDesc(descIn);

    const DXGI_FORMAT pixelFormat = D3DUtil::getMapFormat(srcDesc.format);
    if (pixelFormat == DXGI_FORMAT_UNKNOWN)
    {
        return SLANG_FAIL;
    }

    const int arraySize = calcEffectiveArraySize(srcDesc);

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

    resourceDesc.Flags |= _calcResourceFlags(srcDesc.allowedStates);

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

        D3D12_CLEAR_VALUE clearValue;
        D3D12_CLEAR_VALUE* clearValuePtr = &clearValue;
        if ((resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                                   D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
        {
            clearValuePtr = nullptr;
        }
        clearValue.Format = pixelFormat;
        memcpy(clearValue.Color, &descIn.optimalClearValue.color, sizeof(clearValue.Color));
        clearValue.DepthStencil.Depth = descIn.optimalClearValue.depthStencil.depth;
        clearValue.DepthStencil.Stencil = descIn.optimalClearValue.depthStencil.stencil;
        SLANG_RETURN_ON_FAIL(texture->m_resource.initCommitted(
            m_device,
            heapProps,
            D3D12_HEAP_FLAG_NONE,
            resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            clearValuePtr));

        texture->m_resource.setDebugName(L"Texture");
    }

    // Calculate the layout
    List<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
    layouts.setCount(numMipMaps);
    List<UInt64> mipRowSizeInBytes;
    mipRowSizeInBytes.setCount(numMipMaps);
    List<UInt32> mipNumRows;
    mipNumRows.setCount(numMipMaps);

    // NOTE! This is just the size for one array upload -> not for the whole texture
    UInt64 requiredSize = 0;
    m_device->GetCopyableFootprints(&resourceDesc, 0, numMipMaps, 0, layouts.begin(), mipNumRows.begin(), mipRowSizeInBytes.begin(), &requiredSize);

    // Sub resource indexing
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx#subresource_indexing
    if (initData)
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
        // Get the pointer to the upload resource
        ID3D12Resource* uploadResource = uploadTexture;

        int subResourceIndex = 0;
        for (int arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
        {
            uint8_t* p;
            uploadResource->Map(0, nullptr, reinterpret_cast<void**>(&p));

            for (int j = 0; j < numMipMaps; ++j)
            {
                auto srcSubresource = initData[j];

                const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[j];
                const D3D12_SUBRESOURCE_FOOTPRINT& footprint = layout.Footprint;

                const TextureResource::Size mipSize = calcMipSize(srcDesc.size, j);

                assert(footprint.Width == mipSize.width && footprint.Height == mipSize.height && footprint.Depth == mipSize.depth);

                auto mipRowSize = mipRowSizeInBytes[j];

                const ptrdiff_t dstMipRowPitch = ptrdiff_t(footprint.RowPitch);
                const ptrdiff_t srcMipRowPitch = ptrdiff_t(srcSubresource.strideY);

                const ptrdiff_t dstMipLayerPitch = ptrdiff_t(footprint.RowPitch*footprint.Height);
                const ptrdiff_t srcMipLayerPitch = ptrdiff_t(srcSubresource.strideZ);

                // Our outer loop will copy the depth layers one at a time.
                //
                const uint8_t* srcLayer = (const uint8_t*) srcSubresource.data;
                uint8_t* dstLayer = p + layouts[j].Offset;
                for (int l = 0; l < mipSize.depth; l++)
                {
                    // Our inner loop will copy the rows one at a time.
                    //
                    const uint8_t* srcRow = srcLayer;
                    uint8_t* dstRow = dstLayer;
                    for (int k = 0; k < mipSize.height; ++k)
                    {
                        ::memcpy(dstRow, srcRow, (size_t)mipRowSize);

                        srcRow += srcMipRowPitch;
                        dstRow += dstMipRowPitch;
                    }

                    srcLayer += srcMipLayerPitch;
                    dstLayer += dstMipLayerPitch;
                }

                //assert(srcRow == (const uint8_t*)(srcMip.getBuffer() + srcMip.getCount()));
            }
            uploadResource->Unmap(0, nullptr);

            auto encodeInfo = encodeResourceCommands();
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
                encodeInfo.d3dCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

                subResourceIndex++;
            }

            // Block - waiting for copy to complete (so can drop upload texture)
            submitResourceCommandsAndWait(encodeInfo);
        }
    }
    {
        auto encodeInfo = encodeResourceCommands();
        {
            D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
            texture->m_resource.transition(
                D3D12_RESOURCE_STATE_COPY_DEST, texture->m_defaultState, submitter);
        }
        submitResourceCommandsAndWait(encodeInfo);
    }

    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result D3D12Device::createBufferResource(const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc srcDesc = fixupBufferDesc(descIn);

    // Always align up to 256 bytes, since that is required for constant buffers.
    //
    // TODO: only do this for buffers that could potentially be bound as constant buffers...
    //
    const size_t alignedSizeInBytes = D3DUtil::calcAligned(srcDesc.sizeInBytes, 256);

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc));

    D3D12_RESOURCE_DESC bufferDesc;
    _initBufferResourceDesc(alignedSizeInBytes, bufferDesc);

    bufferDesc.Flags |= _calcResourceFlags(srcDesc.allowedStates);

    const D3D12_RESOURCE_STATES initialState = buffer->m_defaultState;
    SLANG_RETURN_ON_FAIL(createBuffer(bufferDesc, initData, srcDesc.sizeInBytes, buffer->m_uploadResource, initialState, buffer->m_resource));

    returnComPtr(outResource, buffer);
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

Result D3D12Device::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
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

    auto& samplerHeap = m_cpuSamplerHeap;

    D3D12Descriptor cpuDescriptor;
    samplerHeap->allocate(&cpuDescriptor);
    m_device->CreateSampler(&dxDesc, cpuDescriptor.cpuHandle);

    // TODO: We really ought to have a free-list of sampler-heap
    // entries that we check before we go to the heap, and then
    // when we are done with a sampler we simply add it to the free list.
    //
    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_allocator = samplerHeap;
    samplerImpl->m_descriptor = cpuDescriptor;
    returnComPtr(outSampler, samplerImpl);
    return SLANG_OK;
}

Result D3D12Device::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (TextureResourceImpl*) texture;

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_desc = desc;
    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::RenderTarget:
        {
            SLANG_RETURN_ON_FAIL(m_rtvAllocator->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_rtvAllocator;
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = D3DUtil::getMapFormat(desc.format);
            switch (desc.renderTarget.shape)
            {
            case IResource::Type::Texture1D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                rtvDesc.Texture1D.MipSlice = desc.renderTarget.mipSlice;
                break;
            case IResource::Type::Texture2D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = desc.renderTarget.mipSlice;
                rtvDesc.Texture2D.PlaneSlice = desc.renderTarget.planeIndex;
                break;
            case IResource::Type::Texture3D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                rtvDesc.Texture3D.MipSlice = desc.renderTarget.mipSlice;
                rtvDesc.Texture3D.FirstWSlice = desc.renderTarget.arrayIndex;
                rtvDesc.Texture3D.WSize = desc.renderTarget.arraySize;
                break;
            default:
                return SLANG_FAIL;
            }
            m_device->CreateRenderTargetView(
                resourceImpl->m_resource, &rtvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::DepthStencil:
        {
            SLANG_RETURN_ON_FAIL(m_dsvAllocator->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_dsvAllocator;
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = D3DUtil::getMapFormat(desc.format);
            switch (desc.renderTarget.shape)
            {
            case IResource::Type::Texture1D:
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                dsvDesc.Texture1D.MipSlice = desc.renderTarget.mipSlice;
                break;
            case IResource::Type::Texture2D:
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice = desc.renderTarget.mipSlice;
                break;
            default:
                return SLANG_FAIL;
            }
            m_device->CreateDepthStencilView(
                resourceImpl->m_resource, &dsvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::UnorderedAccess:
        {
            // TODO: need to support the separate "counter resource" for the case
            // of append/consume buffers with attached counters.

            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;
            m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;

            // Need to construct the D3D12_SHADER_RESOURCE_VIEW_DESC because otherwise TextureCube is not accessed
            // appropriately (rather than just passing nullptr to CreateShaderResourceView)
            const D3D12_RESOURCE_DESC resourceDesc = resourceImpl->m_resource.getResource()->GetDesc();
            const DXGI_FORMAT pixelFormat = resourceDesc.Format;

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
            _initSrvDesc(resourceImpl->getType(), *resourceImpl->getDesc(), resourceDesc, pixelFormat, srvDesc);

            m_device->CreateShaderResourceView(resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;
    }

    returnComPtr(outView, viewImpl);
    return SLANG_OK;
}

Result D3D12Device::createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;
    auto resourceDesc = *resourceImpl->getDesc();

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_desc = desc;

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::UnorderedAccess:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Format = D3DUtil::getMapFormat(desc.format);
            uavDesc.Buffer.FirstElement = 0;

            if(resourceDesc.elementSize)
            {
                uavDesc.Buffer.StructureByteStride = resourceDesc.elementSize;
                uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / resourceDesc.elementSize);
            }
            else if(desc.format == Format::Unknown)
            {
                uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / 4);
            }
            else
            {
                uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / gfxGetFormatSize(desc.format));
            }


            // TODO: need to support the separate "counter resource" for the case
            // of append/consume buffers with attached counters.

            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;
            m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, &uavDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Format = D3DUtil::getMapFormat(desc.format);
            srvDesc.Buffer.StructureByteStride = 0;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            if(resourceDesc.elementSize)
            {
                srvDesc.Buffer.StructureByteStride = resourceDesc.elementSize;
                srvDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / resourceDesc.elementSize);
            }
            else if(desc.format == Format::Unknown)
            {
                srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / 4);
            }
            else
            {
                srvDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / gfxGetFormatSize(desc.format));
            }

            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;
            m_device->CreateShaderResourceView(resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;
    }

    returnComPtr(outView, viewImpl);
    return SLANG_OK;
}

Result D3D12Device::createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFb)
{
    RefPtr<FramebufferImpl> framebuffer = new FramebufferImpl();
    framebuffer->renderTargetViews.setCount(desc.renderTargetCount);
    framebuffer->renderTargetDescriptors.setCount(desc.renderTargetCount);
    framebuffer->renderTargetClearValues.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        framebuffer->renderTargetViews[i] = static_cast<ResourceViewImpl*>(desc.renderTargetViews[i]);
        framebuffer->renderTargetDescriptors[i] =
            framebuffer->renderTargetViews[i]->m_descriptor.cpuHandle;
        auto clearValue =
            static_cast<TextureResourceImpl*>(
                static_cast<ResourceViewImpl*>(desc.renderTargetViews[i])->m_resource.Ptr())
                ->getDesc()
                ->optimalClearValue.color;
        memcpy(&framebuffer->renderTargetClearValues[i], &clearValue, sizeof(ColorClearValue));
    }
    framebuffer->depthStencilView = static_cast<ResourceViewImpl*>(desc.depthStencilView);
    if (desc.depthStencilView)
    {
        framebuffer->depthStencilClearValue =
            static_cast<TextureResourceImpl*>(
                static_cast<ResourceViewImpl*>(desc.depthStencilView)->m_resource.Ptr())
                ->getDesc()
                ->optimalClearValue.depthStencil;
        framebuffer->depthStencilDescriptor =
            static_cast<ResourceViewImpl*>(desc.depthStencilView)->m_descriptor.cpuHandle;
    }
    else
    {
        framebuffer->depthStencilDescriptor.ptr = 0;
    }
    returnComPtr(outFb, framebuffer);
    return SLANG_OK;
}

Result D3D12Device::createFramebufferLayout(
    IFramebufferLayout::Desc const& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    layout->m_renderTargets.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        layout->m_renderTargets[i] = desc.renderTargets[i];
    }
    
    if (desc.depthStencil)
    {
        layout->m_hasDepthStencil = true;
        layout->m_depthStencil = *desc.depthStencil;
    }
    else
    {
        layout->m_hasDepthStencil = false;
    }
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result D3D12Device::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc,
    IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl();
    result->init(desc);
    returnComPtr(outRenderPassLayout, result);
    return SLANG_OK;
}

Result D3D12Device::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount, IInputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    // Work out a buffer size to hold all text
    size_t textSize = 0;
    for (int i = 0; i < Int(inputElementCount); ++i)
    {
        const char* text = inputElements[i].semanticName;
        textSize += text ? (::strlen(text) + 1) : 0;
    }
    layout->m_text.setCount(textSize);
    char* textPos = layout->m_text.getBuffer();

    //
    List<D3D12_INPUT_ELEMENT_DESC>& elements = layout->m_elements;
    elements.setCount(inputElementCount);


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

    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result D3D12Device::readBufferResource(
    IBufferResource* bufferIn,
    size_t offset,
    size_t size,
    ISlangBlob** outBlob)
{
    auto encodeInfo = encodeResourceCommands();

    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);

    const size_t bufferSize = buffer->getDesc()->sizeInBytes;

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
    _initBufferResourceDesc(size, stagingDesc);

    D3D12Resource stageBuf;
    SLANG_RETURN_ON_FAIL(stageBuf.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, stagingDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr));

    // Do the copy
    encodeInfo.d3dCommandList->CopyBufferRegion(stageBuf, 0, resource, offset, size);

    // Wait until complete
    submitResourceCommandsAndWait(encodeInfo);

    // Map and copy
    RefPtr<ListBlob> blob = new ListBlob();
    {
        UINT8* data;
        D3D12_RANGE readRange = { 0, size };

        SLANG_RETURN_ON_FAIL(stageBuf.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        // Copy to memory buffer
        blob->m_data.setCount(size);
        ::memcpy(blob->m_data.getBuffer(), data, size);

        stageBuf.getResource()->Unmap(0, nullptr);
    }
    returnComPtr(outBlob, blob);
    return SLANG_OK;
}

Result D3D12Device::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
    shaderProgram->m_pipelineType = desc.pipelineType;
    shaderProgram->slangProgram = desc.slangProgram;
    RootShaderObjectLayoutImpl::create(
        this,
        desc.slangProgram,
        desc.slangProgram->getLayout(),
        shaderProgram->m_rootObjectLayout.writeRef());
    if (desc.slangProgram->getSpecializationParamCount() != 0)
    {
        // For a specializable program, we don't invoke any actual slang compilation yet.
        returnComPtr(outProgram, shaderProgram);
        return SLANG_OK;
    }
    // For a fully specialized program, read and store its kernel code in `shaderProgram`.
    auto programReflection = desc.slangProgram->getLayout();
    for (SlangUInt i = 0; i < programReflection->getEntryPointCount(); i++)
    {
        auto entryPointInfo = programReflection->getEntryPointByIndex(i);
        auto stage = entryPointInfo->getStage();
        ComPtr<ISlangBlob> kernelCode;
        ComPtr<ISlangBlob> diagnostics;
        auto compileResult = desc.slangProgram->getEntryPointCode(
            (SlangInt)i, 0, kernelCode.writeRef(), diagnostics.writeRef());
        if (diagnostics)
        {
            getDebugCallback()->handleMessage(
                compileResult == SLANG_OK ? DebugMessageType::Warning : DebugMessageType::Error,
                DebugMessageSource::Slang,
                (char*)diagnostics->getBufferPointer());
        }
        SLANG_RETURN_ON_FAIL(compileResult);
        ShaderBinary shaderBin;
        shaderBin.stage = stage;
        shaderBin.code.addRange(
            reinterpret_cast<const uint8_t*>(kernelCode->getBufferPointer()),
            (Index)kernelCode->getBufferSize());
        shaderProgram->m_shaders.add(_Move(shaderBin));
    }
    returnComPtr(outProgram, shaderProgram);
    return SLANG_OK;
}

Result D3D12Device::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout,
    ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(
        ShaderObjectLayoutImpl::createForElementType(
        this, typeLayout, layout.writeRef()));
    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result D3D12Device::createShaderObject(
    ShaderObjectLayoutBase* layout,
    IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(
        this, reinterpret_cast<ShaderObjectLayoutImpl*>(layout),
        shaderObject.writeRef()));
    returnComPtr(outObject, shaderObject);
    return SLANG_OK;
}

Result D3D12Device::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    auto programImpl = (ShaderProgramImpl*) desc.program;

    if (!programImpl->m_rootObjectLayout->m_rootSignature)
    {
        RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
        pipelineStateImpl->init(desc);
        returnComPtr(outState, pipelineStateImpl);
        return SLANG_OK;
    }

    // Only actually create a D3D12 pipeline state if the pipeline is fully specialized.
    auto inputLayoutImpl = (InputLayoutImpl*) desc.inputLayout;

    // Describe and create the graphics pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.pRootSignature = programImpl->m_rootObjectLayout->m_rootSignature;
    for (auto& shaderBin : programImpl->m_shaders)
    {
        switch (shaderBin.stage)
        {
        case SLANG_STAGE_VERTEX:
            psoDesc.VS = { shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount()) };
            break;
        case SLANG_STAGE_FRAGMENT:
            psoDesc.PS = { shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount()) };
            break;
        case SLANG_STAGE_DOMAIN:
            psoDesc.DS = { shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount()) };
            break;
        case SLANG_STAGE_HULL:
            psoDesc.HS = { shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount()) };
            break;
        case SLANG_STAGE_GEOMETRY:
            psoDesc.GS = { shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount()) };
            break;
        default:
            getDebugCallback()->handleMessage(
                DebugMessageType::Error, DebugMessageSource::Layer, "Unsupported shader stage.");
            return SLANG_E_NOT_AVAILABLE;
        }
    }

    psoDesc.InputLayout = { inputLayoutImpl->m_elements.getBuffer(), UINT(inputLayoutImpl->m_elements.getCount()) };
    psoDesc.PrimitiveTopologyType = D3DUtil::getPrimitiveType(desc.primitiveType);

    {
        auto framebufferLayout = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
        const int numRenderTargets = int(framebufferLayout->m_renderTargets.getCount());

        if (framebufferLayout->m_hasDepthStencil)
        {
            psoDesc.DSVFormat = D3DUtil::getMapFormat(framebufferLayout->m_depthStencil.format);
        }
        else
        {
            psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        }
        psoDesc.NumRenderTargets = numRenderTargets;
        for (Int i = 0; i < numRenderTargets; i++)
        {
            psoDesc.RTVFormats[i] =
                D3DUtil::getMapFormat(framebufferLayout->m_renderTargets[i].format);
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

        ds.DepthEnable = inDesc.depthStencil.depthTestEnable;
        ds.DepthWriteMask = inDesc.depthStencil.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL
                                                                 : D3D12_DEPTH_WRITE_MASK_ZERO;
        ds.DepthFunc = D3DUtil::getComparisonFunc(inDesc.depthStencil.depthFunc);
        ds.StencilEnable = inDesc.depthStencil.stencilEnable;
        ds.StencilReadMask = (UINT8)inDesc.depthStencil.stencilReadMask;
        ds.StencilWriteMask = (UINT8)inDesc.depthStencil.stencilWriteMask;
        ds.FrontFace = D3DUtil::translateStencilOpDesc(inDesc.depthStencil.frontFace);
        ds.BackFace = D3DUtil::translateStencilOpDesc(inDesc.depthStencil.backFace);
    }

    psoDesc.PrimitiveTopologyType = D3DUtil::getPrimitiveType(desc.primitiveType);

    ComPtr<ID3D12PipelineState> pipelineState;
    SLANG_RETURN_ON_FAIL(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.writeRef())));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_pipelineState = pipelineState;
    pipelineStateImpl->init(desc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result D3D12Device::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;

    auto programImpl = (ShaderProgramImpl*) desc.program;
    if (!programImpl->m_rootObjectLayout->m_rootSignature)
    {
        RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
        pipelineStateImpl->init(desc);
        returnComPtr(outState, pipelineStateImpl);
        return SLANG_OK;
    }

    // Only actually create a D3D12 pipeline state if the pipeline is fully specialized.
    ComPtr<ID3D12PipelineState> pipelineState;
    if (!programImpl->slangProgram || programImpl->slangProgram->getSpecializationParamCount() == 0)
    {
        // Describe and create the compute pipeline state object
        D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
        computeDesc.pRootSignature = programImpl->m_rootObjectLayout->m_rootSignature;
        computeDesc.CS = {
            programImpl->m_shaders[0].code.getBuffer(),
            SIZE_T(programImpl->m_shaders[0].code.getCount())};

#ifdef GFX_NVAPI
        if (m_nvapi)
        {
            // Also fill the extension structure.
            // Use the same UAV slot index and register space that are declared in the shader.

            // For simplicities sake we just use u0
            NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC extensionDesc;
            extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
            extensionDesc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
            extensionDesc.uavSlot = 0;
            extensionDesc.registerSpace = 0;

            // Put the pointer to the extension into an array - there can be multiple extensions
            // enabled at once.
            const NVAPI_D3D12_PSO_EXTENSION_DESC* extensions[] = {&extensionDesc};

            // Now create the PSO.
            const NvAPI_Status nvapiStatus = NvAPI_D3D12_CreateComputePipelineState(
                m_device,
                &computeDesc,
                SLANG_COUNT_OF(extensions),
                extensions,
                pipelineState.writeRef());

            if (nvapiStatus != NVAPI_OK)
            {
                return SLANG_FAIL;
            }
        }
        else
#endif
        {
            SLANG_RETURN_ON_FAIL(m_device->CreateComputePipelineState(
                &computeDesc, IID_PPV_ARGS(pipelineState.writeRef())));
        }
    }
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_pipelineState = pipelineState;
    pipelineStateImpl->init(desc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result D3D12Device::QueryPoolImpl::init(const IQueryPool::Desc& desc, D3D12Device* device)
{
    // Translate query type.
    D3D12_QUERY_HEAP_DESC heapDesc = {};
    heapDesc.Count = (UINT)desc.count;
    heapDesc.NodeMask = 1;
    switch (desc.type)
    {
    case QueryType::Timestamp:
        heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        m_queryType = D3D12_QUERY_TYPE_TIMESTAMP;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }

    // Create query heap.
    auto d3dDevice = device->m_device;
    SLANG_RETURN_ON_FAIL(d3dDevice->CreateQueryHeap(
        &heapDesc, IID_PPV_ARGS(m_queryHeap.writeRef())));

    // Create readback buffer.
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;
    D3D12_RESOURCE_DESC resourceDesc = {};
    _initBufferResourceDesc(sizeof(uint64_t) * desc.count, resourceDesc);
    SLANG_RETURN_ON_FAIL(m_readBackBuffer.initCommitted(
        d3dDevice,
        heapProps,
        D3D12_HEAP_FLAG_NONE,
        resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr));

    // Create command allocator.
    SLANG_RETURN_ON_FAIL(d3dDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.writeRef())));

    // Create command list.
    SLANG_RETURN_ON_FAIL(d3dDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator,
        nullptr,
        IID_PPV_ARGS(m_commandList.writeRef())));
    m_commandList->Close();

    // Create fence.
    SLANG_RETURN_ON_FAIL(d3dDevice->CreateFence(
        0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.writeRef())));

    // Get command queue from device.
    m_commandQueue = device->m_resourceCommandQueue->m_d3dQueue;

    // Create wait event.
    m_waitEvent = CreateEventEx(
        nullptr,
        false,
        0,
        EVENT_ALL_ACCESS);

    return SLANG_OK;
}

Result D3D12Device::PlainBufferProxyQueryPoolImpl::init(
    const IQueryPool::Desc& desc,
    D3D12Device* device,
    uint32_t stride)
{
    ComPtr<IBufferResource> bufferResource;
    IBufferResource::Desc bufferDesc = {};
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.elementSize = 0;
    bufferDesc.type = IResource::Type::Buffer;
    bufferDesc.sizeInBytes = desc.count * sizeof(uint64_t);
    bufferDesc.format = Format::Unknown;
    SLANG_RETURN_ON_FAIL(
        device->createBufferResource(bufferDesc, nullptr, bufferResource.writeRef()));
    m_bufferResource = static_cast<D3D12Device::BufferResourceImpl*>(bufferResource.get());
    m_queryType = desc.type;
    m_device = device;
    m_stride = stride;
    return SLANG_OK;
}

Result D3D12Device::createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outState)
{
    switch (desc.type)
    {
    case QueryType::AccelerationStructureCompactedSize:
    case QueryType::AccelerationStructureSerializedSize:
        {
            RefPtr<PlainBufferProxyQueryPoolImpl> queryPoolImpl =
                new PlainBufferProxyQueryPoolImpl();
            uint32_t stride = 8;
            if (desc.type == QueryType::AccelerationStructureSerializedSize)
                stride = 16;
            SLANG_RETURN_ON_FAIL(queryPoolImpl->init(desc, this, stride));
            returnComPtr(outState, queryPoolImpl);
            return SLANG_OK;
        }
    default:
        {
            RefPtr<QueryPoolImpl> queryPoolImpl = new QueryPoolImpl();
            SLANG_RETURN_ON_FAIL(queryPoolImpl->init(desc, this));
            returnComPtr(outState, queryPoolImpl);
            return SLANG_OK;
        }
    }   
}

#if SLANG_GFX_HAS_DXR_SUPPORT

class D3D12AccelerationStructureImpl
    : public AccelerationStructureBase
    , public D3D12Device::ResourceViewInternalImpl
{
public:
    RefPtr<D3D12Device::BufferResourceImpl> m_buffer;
    uint64_t m_offset;
    uint64_t m_size;
    ComPtr<ID3D12Device5> m_device5;

public:
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
    {
        return m_buffer->getDeviceAddress() + m_offset;
    }
};

Result D3D12Device::getAccelerationStructurePrebuildInfo(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IAccelerationStructure::PrebuildInfo* outPrebuildInfo)
{
    if (!m_device5)
        return SLANG_E_NOT_AVAILABLE;
    
    D3DAccelerationStructureInputsBuilder inputsBuilder;
    SLANG_RETURN_ON_FAIL(inputsBuilder.build(buildInputs, getDebugCallback()));

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
    m_device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputsBuilder.desc, &prebuildInfo);

    outPrebuildInfo->resultDataMaxSize = prebuildInfo.ResultDataMaxSizeInBytes;
    outPrebuildInfo->scratchDataSize = prebuildInfo.ScratchDataSizeInBytes;
    outPrebuildInfo->updateScratchDataSize = prebuildInfo.UpdateScratchDataSizeInBytes;
    return SLANG_OK;
}

Result D3D12Device::createAccelerationStructure(
    const IAccelerationStructure::CreateDesc& desc,
    IAccelerationStructure** outAS)
{
    RefPtr<D3D12AccelerationStructureImpl> result = new D3D12AccelerationStructureImpl();
    result->m_device5 = m_device5;
    result->m_buffer = static_cast<BufferResourceImpl*>(desc.buffer);
    result->m_size = desc.size;
    result->m_offset = desc.offset;
    result->m_allocator = m_cpuViewHeap;
    result->m_desc.type = IResourceView::Type::AccelerationStructure;
    SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&result->m_descriptor));
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location =
        result->m_buffer->getDeviceAddress()+ desc.offset;
    m_device->CreateShaderResourceView(nullptr, &srvDesc, result->m_descriptor.cpuHandle);
    returnComPtr(outAS, result);
    return SLANG_OK;
}

void translatePostBuildInfoDescs(
    int propertyQueryCount,
    AccelerationStructureQueryDesc* queryDescs,
    List<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC>& postBuildInfoDescs)
{
    postBuildInfoDescs.setCount(propertyQueryCount);
    for (int i = 0; i < propertyQueryCount; i++)
    {
        switch (queryDescs[i].queryType)
        {
        case QueryType::AccelerationStructureCompactedSize:
            postBuildInfoDescs[i].InfoType =
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE;
            postBuildInfoDescs[i].DestBuffer =
                static_cast<D3D12Device::PlainBufferProxyQueryPoolImpl*>(queryDescs[i].queryPool)
                    ->m_bufferResource->getDeviceAddress() +
                sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC) *
                    queryDescs[i].firstQueryIndex;
            break;
        case QueryType::AccelerationStructureSerializedSize:
            postBuildInfoDescs[i].InfoType =
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_SERIALIZATION;
            postBuildInfoDescs[i].DestBuffer =
                static_cast<D3D12Device::PlainBufferProxyQueryPoolImpl*>(queryDescs[i].queryPool)
                    ->m_bufferResource->getDeviceAddress() +
                sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_SERIALIZATION_DESC) *
                    queryDescs[i].firstQueryIndex;
            break;
        }
    }
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::buildAccelerationStructure(
    const IAccelerationStructure::BuildDesc& desc,
    int propertyQueryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    if (!m_commandBuffer->m_cmdList4)
    {
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "Ray-tracing is not supported on current system.");
        return;
    }
    D3D12AccelerationStructureImpl* destASImpl = nullptr;
    if (desc.dest)
        destASImpl = static_cast<D3D12AccelerationStructureImpl*>(desc.dest);
    D3D12AccelerationStructureImpl* srcASImpl = nullptr;
    if (desc.source)
        srcASImpl = static_cast<D3D12AccelerationStructureImpl*>(desc.source);
    
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.DestAccelerationStructureData = destASImpl->getDeviceAddress();
    buildDesc.SourceAccelerationStructureData = srcASImpl?srcASImpl->getDeviceAddress() : 0;
    buildDesc.ScratchAccelerationStructureData = desc.scratchData;
    D3DAccelerationStructureInputsBuilder builder;
    builder.build(desc.inputs, getDebugCallback());
    buildDesc.Inputs = builder.desc;

    List<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC> postBuildInfoDescs;
    translatePostBuildInfoDescs(propertyQueryCount, queryDescs, postBuildInfoDescs);
    m_commandBuffer->m_cmdList4->BuildRaytracingAccelerationStructure(
        &buildDesc, (UINT)propertyQueryCount, postBuildInfoDescs.getBuffer());
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::copyAccelerationStructure(
    IAccelerationStructure* dest,
    IAccelerationStructure* src,
    AccelerationStructureCopyMode mode)
{
    auto destASImpl = static_cast<D3D12AccelerationStructureImpl*>(dest);
    auto srcASImpl = static_cast<D3D12AccelerationStructureImpl*>(src);
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE copyMode;
    switch (mode)
    {
    case AccelerationStructureCopyMode::Clone:
        copyMode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE;
        break;
    case AccelerationStructureCopyMode::Compact:
        copyMode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT;
        break;
    default:
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "Unsupported AccelerationStructureCopyMode.");
        return;
    }
    m_commandBuffer->m_cmdList4->CopyRaytracingAccelerationStructure(
        destASImpl->getDeviceAddress(), srcASImpl->getDeviceAddress(), copyMode);
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::
    queryAccelerationStructureProperties(
        int accelerationStructureCount,
        IAccelerationStructure* const* accelerationStructures,
        int queryCount,
        AccelerationStructureQueryDesc* queryDescs)
{
    List<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC> postBuildInfoDescs;
    List<DeviceAddress> asAddresses;
    asAddresses.setCount(accelerationStructureCount);
    for (int i = 0; i < accelerationStructureCount; i++)
        asAddresses[i] = accelerationStructures[i]->getDeviceAddress();
    translatePostBuildInfoDescs(queryCount, queryDescs, postBuildInfoDescs);
    m_commandBuffer->m_cmdList4->EmitRaytracingAccelerationStructurePostbuildInfo(
        postBuildInfoDescs.getBuffer(),
        (UINT)accelerationStructureCount,
        asAddresses.getBuffer());
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::serializeAccelerationStructure(
    DeviceAddress dest,
    IAccelerationStructure* src)
{
    auto srcASImpl = static_cast<D3D12AccelerationStructureImpl*>(src);
    m_commandBuffer->m_cmdList4->CopyRaytracingAccelerationStructure(
        dest,
        srcASImpl->getDeviceAddress(),
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_SERIALIZE);
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::deserializeAccelerationStructure(
    IAccelerationStructure* dest,
    DeviceAddress source)
{
    auto destASImpl = static_cast<D3D12AccelerationStructureImpl*>(dest);
    m_commandBuffer->m_cmdList4->CopyRaytracingAccelerationStructure(
        dest->getDeviceAddress(),
        source,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_DESERIALIZE);
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::memoryBarrier(
    int count,
    IAccelerationStructure* const* structures,
    AccessFlag::Enum sourceAccess,
    AccessFlag::Enum destAccess)
{
    ShortList<D3D12_RESOURCE_BARRIER> barriers;
    barriers.setCount(count);
    for (int i = 0; i < count; i++)
    {
        barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barriers[i].UAV.pResource = static_cast<D3D12AccelerationStructureImpl*>(structures[i])
                                        ->m_buffer->m_resource.getResource();
    }
    m_commandBuffer->m_cmdList4->ResourceBarrier((UINT)count, barriers.getArrayView().getBuffer());
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::bindPipeline(
    IPipelineState* state, IShaderObject** outRootObject)
{
    bindPipelineImpl(state, outRootObject);
}

void D3D12Device::CommandBufferImpl::RayTracingCommandEncoderImpl::dispatchRays(
    const char* rayGenShaderName,
    int32_t width,
    int32_t height,
    int32_t depth)
{
    RefPtr<PipelineStateBase> newPipeline;
    PipelineStateBase* pipeline = m_currentPipeline.Ptr();
    {
        struct RayTracingSubmitter : public ComputeSubmitter
        {
            ID3D12GraphicsCommandList4* m_cmdList4;
            RayTracingSubmitter(ID3D12GraphicsCommandList4* cmdList4)
                : ComputeSubmitter(cmdList4), m_cmdList4(cmdList4)
            {
            }
            virtual void setPipelineState(PipelineStateBase* pipeline) override
            {
                auto pipelineImpl = static_cast<RayTracingPipelineStateImpl*>(pipeline);
                m_cmdList4->SetPipelineState1(pipelineImpl->m_stateObject.get());
            }
        };
        RayTracingSubmitter submitter(m_commandBuffer->m_cmdList4);
        if (SLANG_FAILED(_bindRenderState(&submitter, newPipeline)))
        {
            assert(!"Failed to bind render state");
        }
        if (newPipeline)
            pipeline = newPipeline.Ptr();
    }
    auto pipelineImpl = static_cast<RayTracingPipelineStateImpl*>(pipeline);
    auto dispatchDesc = pipelineImpl->m_dispatchDesc;
    int32_t rayGenShaderOffset = 0;
    if (rayGenShaderName)
    {
        rayGenShaderOffset =
            pipelineImpl->m_mapRayGenShaderNameToShaderTableIndex[rayGenShaderName].GetValue() *
            D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }
    dispatchDesc.RayGenerationShaderRecord.StartAddress += rayGenShaderOffset;
    dispatchDesc.Width = (UINT)width;
    dispatchDesc.Height = (UINT)height;
    dispatchDesc.Depth = (UINT)depth;
    m_commandBuffer->m_cmdList4->DispatchRays(&dispatchDesc);
}

Result D3D12Device::createRayTracingPipelineState(const RayTracingPipelineStateDesc& inDesc, IPipelineState** outState)
{
    if (!m_device5)
    {
        return SLANG_E_NOT_AVAILABLE;
    }

    RefPtr<RayTracingPipelineStateImpl> pipelineStateImpl = new RayTracingPipelineStateImpl();
    pipelineStateImpl->init(inDesc);

    auto program = static_cast<ShaderProgramImpl*>(inDesc.program);
    auto slangProgram = program->slangProgram;
    auto programLayout = slangProgram->getLayout();

    if (!program->m_rootObjectLayout->m_rootSignature)
    {
        returnComPtr(outState, pipelineStateImpl);
        return SLANG_OK;
    }
    List<D3D12_STATE_SUBOBJECT> subObjects;
    ChunkedList<D3D12_DXIL_LIBRARY_DESC> dxilLibraries;
    ChunkedList<D3D12_HIT_GROUP_DESC> hitGroups;
    ChunkedList<ComPtr<ISlangBlob>> codeBlobs;
    ComPtr<ISlangBlob> diagnostics;
    ChunkedList<OSString> stringPool;
    int32_t rayGenIndex = 0;
    for (SlangUInt i = 0; i < programLayout->getEntryPointCount(); i++)
    {
        ComPtr<ISlangBlob> codeBlob;
        auto compileResult =
            slangProgram->getEntryPointCode(i, 0, codeBlob.writeRef(), diagnostics.writeRef());
        if (diagnostics.get())
        {
            getDebugCallback()->handleMessage(
                compileResult == SLANG_OK ? DebugMessageType::Warning : DebugMessageType::Error,
                DebugMessageSource::Slang,
                (char*)diagnostics->getBufferPointer());
        }
        SLANG_RETURN_ON_FAIL(compileResult);
        codeBlobs.add(codeBlob);
        D3D12_DXIL_LIBRARY_DESC library = {};
        library.DXILLibrary.BytecodeLength = codeBlob->getBufferSize();;
        library.DXILLibrary.pShaderBytecode = codeBlob->getBufferPointer();

        D3D12_STATE_SUBOBJECT dxilSubObject = {};
        dxilSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
        dxilSubObject.pDesc = dxilLibraries.add(library);
        subObjects.add(dxilSubObject);

        auto entryPointLayout = programLayout->getEntryPointByIndex(i);
        switch (entryPointLayout->getStage())
        {
        case SLANG_STAGE_RAY_GENERATION:
            pipelineStateImpl
                ->m_mapRayGenShaderNameToShaderTableIndex[entryPointLayout->getName()] =
                rayGenIndex;
            rayGenIndex++;
            break;
        default:
            break;
        }
    }
    auto getWStr = [&](const char* name)
    {
        String str = String(name);
        auto wstr = str.toWString();
        return stringPool.add(wstr)->begin();
    };
    for (int i = 0; i < inDesc.hitGroupCount; i++)
    {
        auto hitGroup = inDesc.hitGroups[i];
        D3D12_HIT_GROUP_DESC hitGroupDesc = {};
        hitGroupDesc.Type = hitGroup.intersectionEntryPoint == nullptr
                                ? D3D12_HIT_GROUP_TYPE_TRIANGLES
                                : D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;

        if (hitGroup.anyHitEntryPoint)
        {
            hitGroupDesc.AnyHitShaderImport = getWStr(hitGroup.anyHitEntryPoint);
        }
        if (hitGroup.closestHitEntryPoint)
        {
            hitGroupDesc.ClosestHitShaderImport = getWStr(hitGroup.closestHitEntryPoint);
        }
        if (hitGroup.intersectionEntryPoint)
        {
            hitGroupDesc.IntersectionShaderImport = getWStr(hitGroup.intersectionEntryPoint);
        }
        StringBuilder hitGroupName;
        hitGroupName << "hitgroup_" << i;
        hitGroupDesc.HitGroupExport = getWStr(hitGroupName.ToString().getBuffer());

        D3D12_STATE_SUBOBJECT hitGroupSubObject = {};
        hitGroupSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        hitGroupSubObject.pDesc = hitGroups.add(hitGroupDesc);
        subObjects.add(hitGroupSubObject);
    }

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    // According to DXR spec, fixed function triangle intersections must use float2 as ray attributes
    // that defines the barycentric coordinates at intersection.
    shaderConfig.MaxAttributeSizeInBytes = sizeof(float) * 2;
    shaderConfig.MaxPayloadSizeInBytes = inDesc.maxRayPayloadSize;
    D3D12_STATE_SUBOBJECT shaderConfigSubObject = {};
    shaderConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    shaderConfigSubObject.pDesc = &shaderConfig;
    subObjects.add(shaderConfigSubObject);

    D3D12_GLOBAL_ROOT_SIGNATURE globalSignatureDesc = {};
    globalSignatureDesc.pGlobalRootSignature = program->m_rootObjectLayout->m_rootSignature.get();
    D3D12_STATE_SUBOBJECT globalSignatureSubobject = {};
    globalSignatureSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    globalSignatureSubobject.pDesc = &globalSignatureDesc;
    subObjects.add(globalSignatureSubobject);

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = inDesc.maxRecursion;
    D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
    pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    pipelineConfigSubobject.pDesc = &pipelineConfig;
    subObjects.add(pipelineConfigSubobject);

    D3D12_STATE_OBJECT_DESC rtpsoDesc = {};
    rtpsoDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    rtpsoDesc.NumSubobjects = (UINT)subObjects.getCount();
    rtpsoDesc.pSubobjects = subObjects.getBuffer();
    SLANG_RETURN_ON_FAIL(m_device5->CreateStateObject(&rtpsoDesc, IID_PPV_ARGS(pipelineStateImpl->m_stateObject.writeRef())));

    SLANG_RETURN_ON_FAIL(pipelineStateImpl->createShaderTables(this, slangProgram, inDesc));

    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result D3D12Device::RayTracingPipelineStateImpl::createShaderTables(
    D3D12Device* device,
    slang::IComponentType* slangProgram,
    const RayTracingPipelineStateDesc& desc)
{
    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    m_stateObject->QueryInterface(stateObjectProperties.writeRef());
    auto programLayout = slangProgram->getLayout();
    struct ShaderIdentifier { uint32_t data[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES / sizeof(uint32_t)]; };
    List<ShaderIdentifier> rayGenIdentifiers, missIdentifiers, hitgroupIdentifiers;
    for (SlangUInt i = 0; i < programLayout->getEntryPointCount(); i++)
    {
        auto entryPointLayout = programLayout->getEntryPointByIndex(i);
        ShaderIdentifier identifier;
        switch (entryPointLayout->getStage())
        {
        case SLANG_STAGE_RAY_GENERATION:
            memcpy(
                &identifier,
                stateObjectProperties->GetShaderIdentifier(
                    String(entryPointLayout->getName()).toWString().begin()),
                sizeof(ShaderIdentifier));
            rayGenIdentifiers.add(identifier);
            break;
        case SLANG_STAGE_MISS:
            memcpy(
                &identifier,
                stateObjectProperties->GetShaderIdentifier(
                    String(entryPointLayout->getName()).toWString().begin()),
                sizeof(ShaderIdentifier));
            missIdentifiers.add(identifier);
            break;
        default:
            break;
        }
    }
    for (int i = 0; i < desc.shaderTableHitGroupCount; i++)
    {
        StringBuilder hitgroupName;
        hitgroupName << "hitgroup_" << desc.shaderTableHitGroupIndices[i];
        ShaderIdentifier hitgroupIdentifier;
        memcpy(
            &hitgroupIdentifier,
            stateObjectProperties->GetShaderIdentifier(hitgroupName.toWString().begin()),
            sizeof(ShaderIdentifier));
        hitgroupIdentifiers.add(hitgroupIdentifier);
    }

    auto createShaderTableResource = [&](ArrayView<ShaderIdentifier> content,
                                         RefPtr<BufferResourceImpl>& outResource) -> Result
    {
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.type = IResource::Type::Buffer;
        bufferDesc.defaultState = ResourceState::ShaderResource;
        bufferDesc.allowedStates = ResourceStateSet(
            ResourceState::CopySource,
            ResourceState::UnorderedAccess,
            ResourceState::ShaderResource);
        bufferDesc.elementSize = 0;
        bufferDesc.sizeInBytes = content.getCount() * sizeof(ShaderIdentifier);
        bufferDesc.format = Format::Unknown;
        ComPtr<IBufferResource> shaderTableResource;
        SLANG_RETURN_ON_FAIL(device->createBufferResource(
            bufferDesc, content.getBuffer(), shaderTableResource.writeRef()));
        outResource = static_cast<BufferResourceImpl*>(shaderTableResource.get());
        return SLANG_OK;
    };

    if (desc.shaderTableHitGroupCount)
    {
        SLANG_RETURN_ON_FAIL(
            createShaderTableResource(hitgroupIdentifiers.getArrayView(), m_hitgroupShaderTable));
        m_dispatchDesc.HitGroupTable.SizeInBytes =
            (uint64_t)(sizeof(ShaderIdentifier)) * desc.shaderTableHitGroupCount;
        m_dispatchDesc.HitGroupTable.StrideInBytes = sizeof(ShaderIdentifier);
        m_dispatchDesc.HitGroupTable.StartAddress = m_hitgroupShaderTable->getDeviceAddress();
    }
    if (rayGenIdentifiers.getCount())
    {
        SLANG_RETURN_ON_FAIL(
            createShaderTableResource(rayGenIdentifiers.getArrayView(), m_rayGenShaderTable));
        m_dispatchDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderIdentifier);
        m_dispatchDesc.RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->getDeviceAddress();
    }
    if (missIdentifiers.getCount())
    {
        SLANG_RETURN_ON_FAIL(
            createShaderTableResource(missIdentifiers.getArrayView(), m_missShaderTable));
        m_dispatchDesc.MissShaderTable.SizeInBytes =
            (uint64_t)(sizeof(ShaderIdentifier)) * missIdentifiers.getCount();
        m_dispatchDesc.MissShaderTable.StrideInBytes = sizeof(ShaderIdentifier);
        m_dispatchDesc.MissShaderTable.StartAddress = m_missShaderTable->getDeviceAddress();
    }
    return SLANG_OK;
}

#endif // SLANG_GFX_HAS_DXR_SUPPORT

Result D3D12Device::ShaderObjectImpl::setResource(ShaderOffset const& offset, IResourceView* resourceView)
{
    if (offset.bindingRangeIndex < 0)
        return SLANG_E_INVALID_ARG;
    auto layout = getLayout();
    if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
        return SLANG_E_INVALID_ARG;

    auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

    ResourceViewInternalImpl* internalResourceView = nullptr;
    switch (resourceView->getViewDesc()->type)
    {
#if SLANG_GFX_HAS_DXR_SUPPORT
    case IResourceView::Type::AccelerationStructure:
        {
            auto asImpl = static_cast<D3D12AccelerationStructureImpl*>(resourceView);
            // Hold a reference to the resource to prevent its destruction.
            m_boundResources[bindingRange.baseIndex + offset.bindingArrayIndex] = asImpl->m_buffer;
            internalResourceView = asImpl;
        }
        break;
#endif
    default:
        {
            auto resourceViewImpl = static_cast<ResourceViewImpl*>(resourceView);
            // Hold a reference to the resource to prevent its destruction.
            m_boundResources[bindingRange.baseIndex + offset.bindingArrayIndex] =
                resourceViewImpl->m_resource;
            internalResourceView = resourceViewImpl;
        }
        break;
    }

    auto descriptorSlotIndex = bindingRange.baseIndex + (int32_t)offset.bindingArrayIndex;
    ID3D12Device* d3dDevice = static_cast<D3D12Device*>(getDevice())->m_device;
    d3dDevice->CopyDescriptorsSimple(
        1,
        m_descriptorSet.resourceTable.getCpuHandle(
            bindingRange.baseIndex + (int32_t)offset.bindingArrayIndex),
        internalResourceView->m_descriptor.cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    return SLANG_OK;
}

} // renderer_test
