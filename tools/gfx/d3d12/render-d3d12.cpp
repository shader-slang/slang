// render-d3d12.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "render-d3d12.h"

//WORKING:#include "options.h"
#include "../renderer-shared.h"
#include "../render-graphics-common.h"
#include "../simple-render-pass-layout.h"
#include "../d3d/d3d-swapchain.h"
#include "core/slang-blob.h"
#include "core/slang-basic.h"

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
#include "circular-resource-heap-d3d12.h"

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

static D3D12_RESOURCE_STATES _calcResourceState(IResource::Usage usage);

class D3D12Device : public GraphicsAPIRenderer
{
public:
    // Renderer    implementation
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc,
        WindowHandle window,
        ISwapchain** outSwapchain) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        IResource::Usage initialUsage,
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

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createPipelineLayout(
        const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSet(
        IDescriptorSetLayout* layout,
        IDescriptorSet::Flag::Enum flag,
        IDescriptorSet** outDescriptorSet) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

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

protected:
    
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

        // During command submission, we need all the descriptor tables that get
        // used to come from a single heap (for each descriptor heap type).
        //
        // We will thus keep a single heap of each type that we hope will hold
        // all the descriptors that actually get needed in a frame.
        //
        // TODO: we need an allocation policy to reallocate and resize these
        // if/when we run out of space during a frame.
        //
        D3D12DescriptorHeap m_viewHeap; ///< Cbv, Srv, Uav
        D3D12DescriptorHeap m_samplerHeap; ///< Heap for samplers
    };

    class ShaderProgramImpl : public GraphicsCommonShaderProgram
    {
    public:
        PipelineType m_pipelineType;
        List<uint8_t> m_vertexShader;
        List<uint8_t> m_pixelShader;
        List<uint8_t> m_computeShader;
    };

    class BufferResourceImpl: public gfx::BufferResource
    {
    public:
        typedef BufferResource Parent;

        void bindConstantBufferView(D3D12CircularResourceHeap& circularHeap, int index, Submitter* submitter) const
        {
            // Set the constant buffer
            submitter->setRootConstantBufferView(index, m_resource.getResource()->GetGPUVirtualAddress());
        }

        BufferResourceImpl(IResource::Usage initialUsage, const Desc& desc):
            Parent(desc), m_initialUsage(initialUsage)
            , m_defaultState(_calcResourceState(initialUsage))
        {
        }

        D3D12Resource m_resource;           ///< The resource typically in gpu memory
        D3D12Resource m_uploadResource;     ///< If the resource can be written to, and is in gpu memory (ie not Memory backed), will have upload resource

        Usage m_initialUsage;
        D3D12_RESOURCE_STATES m_defaultState;
    };

    class TextureResourceImpl: public TextureResource
    {
    public:
        typedef TextureResource Parent;

        TextureResourceImpl(const Desc& desc):
            Parent(desc)
        {
            m_defaultState = _calcResourceState(desc.initialUsage);
        }

        D3D12Resource m_resource;
        D3D12_RESOURCE_STATES m_defaultState;
    };

    class SamplerStateImpl : public ISamplerState, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISamplerState* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISamplerState)
                return static_cast<ISamplerState*>(this);
            return nullptr;
        }
    public:
        D3D12HostVisibleDescriptor m_descriptor;
        D3D12Device* m_renderer;
        ~SamplerStateImpl()
        {
            m_renderer->m_samplerAllocator.free(m_descriptor);
        }
    };

    class ResourceViewImpl : public IResourceView, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IResourceView* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IResourceView)
                return static_cast<IResourceView*>(this);
            return nullptr;
        }
    public:
        RefPtr<Resource>            m_resource;
        D3D12HostVisibleDescriptor  m_descriptor;
        D3D12HostVisibleDescriptorAllocator* m_allocator;
        ~ResourceViewImpl()
        {
            m_allocator->free(m_descriptor);
        }
    };

    class FramebufferLayoutImpl
        : public IFramebufferLayout
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IFramebufferLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IFramebufferLayout)
                return static_cast<IFramebufferLayout*>(this);
            return nullptr;
        }

    public:
        ShortList<IFramebufferLayout::AttachmentLayout> m_renderTargets;
        bool m_hasDepthStencil = false;
        IFramebufferLayout::AttachmentLayout m_depthStencil;
    };

    class FramebufferImpl
        : public IFramebuffer
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IFramebuffer* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IFramebuffer)
                return static_cast<IFramebuffer*>(this);
            return nullptr;
        }

    public:
        ShortList<ComPtr<IResourceView>> renderTargetViews;
        ComPtr<IResourceView> depthStencilView;
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

    class InputLayoutImpl: public IInputLayout, public RefObject
	{
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IInputLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IInputLayout)
                return static_cast<IInputLayout*>(this);
            return nullptr;
        }
    public:
        List<D3D12_INPUT_ELEMENT_DESC> m_elements;
        List<char> m_text;                              ///< Holds all strings to keep in scope
    };

    class DescriptorSetLayoutImpl : public IDescriptorSetLayout, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IDescriptorSetLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IDescriptorSetLayout)
                return static_cast<IDescriptorSetLayout*>(this);
            return nullptr;
        }
    public:
        // A "descriptor set" at the level of the `Renderer` API
        // is similar to a D3D12 "descriptor table," but the match
        // isn't perfect for a few reasons:
        //
        // * Our descriptor sets can contain both resources and
        //   samplers, while D3D12 descriptor tables are always
        //   resource-only or sampler-only.
        //
        // * Our descriptor sets can include root constant ranges,
        //   while under D3D12 a root constant range is thought
        //   of as belonging to the root signature directly.
        //
        // We navigate this mismatch in our implementation with
        // the idea that a single `Renderer`-level descriptor set
        // maps to zero or more D3D12 root parameters, which can
        // include:
        //
        // * Zero or one root parameter that is used to bind a
        //   descriptor table of resources.
        //
        // * Zero or one root parameter that is used to bind a
        //   descriptor table of samplers.
        //
        // * Zero or more root parameters that represent ranges
        //   of root constants.
        //
        // Binding a descriptor set will band all of its associated
        // root parameters.
        //
        // (Note: this representation could in theory be extended
        // to also support root resources that are not table-bound)
        //
        // Each descriptor slot range in the original `Desc` maps
        // to a single `RangeInfo` stored here, which captures
        // derived information used when binding values into
        // a descriptor table.
        //
        struct RangeInfo
        {
                /// The type of descriptor slot in the original `Desc`
            DescriptorSlotType  type;

                /// The number of slots in this range
            Int                 count;

                /// The start index of this range in the appropriate type-specific array.
                ///
                /// E.g., for a sampler slot range, this would be the start index
                /// for the range in the descriptor table used to store all the samplers.
            Int                 arrayIndex;
        };
        List<RangeInfo> m_ranges;

        // We need to track additional information about
        // root cosntant ranges that isn't captured in
        // `RangeInfo`, so we store an additional array
        // that just captures the root constant ranges.
        //
        struct RootConstantRangeInfo
        {
                /// The D3D12 "root parameter index" for this range
            Int rootParamIndex;

                /// The size in bytes of this range
            Int size;

                /// The byte offset of this range's data in the backing storage for a descriptor set
            Int offset;
        };
        List<RootConstantRangeInfo> m_rootConstantRanges;

            /// The total size (in bytes) of root constant data across all contained ranged.
        Int                         m_rootConstantDataSize = 0;

            /// The D3D12-format descriptions of the descriptor ranges in this set
        List<D3D12_DESCRIPTOR_RANGE>    m_dxRanges;

            /// The D3D12-format description of the root parameters introduced by this set
        List<D3D12_ROOT_PARAMETER>      m_dxRootParameters;

            /// How many resource slots (total) were introduced by ranges?
        Int m_resourceCount;

            /// How many sampler slots (total) were introduce by ranges?
        Int m_samplerCount;
    };

    class PipelineLayoutImpl : public IPipelineLayout, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IPipelineLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IPipelineLayout)
            {
                return static_cast<IPipelineLayout*>(this);
            }
            return nullptr;
        }
    public:
        ComPtr<ID3D12RootSignature> m_rootSignature;
        UInt                        m_descriptorSetCount;
    };

    class DescriptorSetImpl : public IDescriptorSet, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IDescriptorSet* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IDescriptorSet)
                return static_cast<IDescriptorSet*>(this);
            return nullptr;
        }
    public:
        virtual SLANG_NO_THROW void SLANG_MCALL
            setConstantBuffer(UInt range, UInt index, IBufferResource* buffer) override;
        virtual SLANG_NO_THROW void SLANG_MCALL
            setResource(UInt range, UInt index, IResourceView* view) override;
        virtual SLANG_NO_THROW void SLANG_MCALL
            setSampler(UInt range, UInt index, ISamplerState* sampler) override;
        virtual SLANG_NO_THROW void SLANG_MCALL setCombinedTextureSampler(
            UInt range,
            UInt index,
            IResourceView*   textureView,
            ISamplerState*   sampler) override;
        virtual SLANG_NO_THROW void SLANG_MCALL
            setRootConstants(
            UInt range,
            UInt offset,
            UInt size,
            void const* data) override;

        D3D12Device*           m_renderer = nullptr;          ///< Weak pointer - must be because if set on Renderer, will have a circular reference
        RefPtr<DescriptorSetLayoutImpl> m_layout;

        D3D12HostVisibleDescriptorAllocator*        m_resourceHeap = nullptr;
        D3D12HostVisibleDescriptorAllocator*        m_samplerHeap = nullptr;

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

            /// Backing storage for root constant ranges in this descriptor set.
        List<char>                      m_rootConstantData;

        ~DescriptorSetImpl()
        {
            if (m_layout->m_resourceCount)
                m_resourceHeap->free((int)m_resourceTable, (int)m_layout->m_resourceCount);
            if (m_layout->m_samplerCount)
                m_samplerHeap->free((int)m_samplerTable, (int)m_layout->m_samplerCount);
        }
    };

    D3D12HostVisibleDescriptorAllocator m_rtvAllocator;
    D3D12HostVisibleDescriptorAllocator m_dsvAllocator;

    D3D12HostVisibleDescriptorAllocator m_viewAllocator;
    D3D12HostVisibleDescriptorAllocator m_samplerAllocator;

    // Space in the GPU-visible heaps is precious, so we will also keep
    // around CPU-visible heaps for storing descriptors in a format
    // that is ready for copying into the GPU-visible heaps as needed.
    //
    D3D12HostVisibleDescriptorAllocator m_cpuViewHeap; ///< Cbv, Srv, Uav
    D3D12HostVisibleDescriptorAllocator m_cpuSamplerHeap; ///< Heap for samplers

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

        ComputeSubmitter(ID3D12GraphicsCommandList* commandList) :
            m_commandList(commandList)
        {
        }

        ID3D12GraphicsCommandList* m_commandList;
    };

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
        memcpy(uploadData, data, size);
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
    
    // Use a circular buffer of execution frames to manage in-flight GPU command buffers.
    // Each call to `executeCommandLists` advances the frame by 1.
    // If we run out of avaialble frames, wait for the earliest submitted frame to finish.
    struct ExecutionFrameResources
    {
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        List<ComPtr<ID3D12GraphicsCommandList>> m_commandListPool;
        uint32_t m_commandListAllocId = 0;
        HANDLE fenceEvent;

        // During command submission, we need all the descriptor tables that get
        // used to come from a single heap (for each descriptor heap type).
        //
        // We will thus keep a single heap of each type that we hope will hold
        // all the descriptors that actually get needed in a frame.
        //
        // TODO: we need an allocation policy to reallocate and resize these
        // if/when we run out of space during a frame.
        D3D12DescriptorHeap m_viewHeap; // Cbv, Srv, Uav
        D3D12DescriptorHeap m_samplerHeap; // Heap for samplers

        ~ExecutionFrameResources() { CloseHandle(fenceEvent); }
        Result init(ID3D12Device* device, uint32_t viewHeapSize, uint32_t samplerHeapSize)
        {
            SLANG_RETURN_ON_FAIL(device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.writeRef())));
            fenceEvent = CreateEventEx(
                nullptr,
                false,
                CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET,
                EVENT_ALL_ACCESS);
            SLANG_RETURN_ON_FAIL(m_viewHeap.init(
                device,
                viewHeapSize,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
            SLANG_RETURN_ON_FAIL(m_samplerHeap.init(
                device,
                samplerHeapSize,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
                D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
            return SLANG_OK;
        }
        void reset()
        {
            WaitForSingleObject(fenceEvent, INFINITE);
            m_viewHeap.deallocateAll();
            m_samplerHeap.deallocateAll();
            m_commandListAllocId = 0;
            m_commandAllocator->Reset();
            for (auto cmdBuffer : m_commandListPool)
                cmdBuffer->Reset(m_commandAllocator, nullptr);
        }
        ComPtr<ID3D12GraphicsCommandList> createCommandList(ID3D12Device* device)
        {
            if (m_commandListAllocId == m_commandListPool.getCount())
            {
                ComPtr<ID3D12GraphicsCommandList> cmdList;
                device->CreateCommandList(
                    0,
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    m_commandAllocator,
                    nullptr,
                    IID_PPV_ARGS(cmdList.writeRef()));
                m_commandListPool.add(cmdList);
            }
            assert((Index)m_commandListAllocId < m_commandListPool.getCount());
            auto& result = m_commandListPool[m_commandListAllocId];
            ++m_commandListAllocId;
            return result;
        }
    };

    class CommandBufferImpl
        : public ICommandBuffer
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ICommandBuffer* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
                return static_cast<ICommandBuffer*>(this);
            return nullptr;
        }
    public:
        ComPtr<ID3D12GraphicsCommandList> m_cmdList;
        ExecutionFrameResources* m_frame;
        D3D12Device* m_renderer;
        void init(D3D12Device* renderer, ExecutionFrameResources* frame)
        {
            m_frame = frame;
            m_renderer = renderer;
            m_cmdList = m_frame->createCommandList(renderer->m_device);
        }
        class PipelineCommandEncoder : public GraphicsComputeCommandEncoderBase
        {
        public:
            bool m_isOpen = false;
            CommandBufferImpl* m_commandBuffer;
            ExecutionFrameResources* m_frame;
            ID3D12Device* m_device;
            ID3D12GraphicsCommandList* m_d3dCmdList;
            ID3D12GraphicsCommandList* m_preCmdList = nullptr;

            ID3D12PipelineState* m_boundPipelines[3] = {};
            RefPtr<DescriptorSetImpl> m_boundDescriptorSets[int(PipelineType::CountOf)]
                                                           [kMaxDescriptorSetCount];
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

            RefPtr<PipelineLayoutImpl> m_currentPipelineLayout;

            void init(CommandBufferImpl* commandBuffer)
            {
                m_commandBuffer = commandBuffer;
                m_rendererBase = static_cast<RendererBase*>(commandBuffer->m_renderer);
                m_d3dCmdList = m_commandBuffer->m_cmdList;
            }

            void endEncodingImpl()
            {
                m_isOpen = false;
                for (int i = 0; i < int(PipelineType::CountOf); i++)
                {
                    for (auto& descSet : m_boundDescriptorSets[i])
                    {
                        descSet = nullptr;
                    }
                }
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSetImpl(
                PipelineType pipelineType,
                IPipelineLayout* layout,
                UInt index,
                IDescriptorSet* descriptorSet) override
            {
                // In D3D12, unlike Vulkan, binding a root signature invalidates *all* descriptor
                // table
                // bindings (rather than preserving those that are part of the longest common prefix
                // between the old and new layout).
                //
                // In order to accomodate having descriptor-set bindings that persist across changes
                // in pipeline state (which may also change pipeline layout), we will shadow the
                // descriptor-set bindings and only flush them on-demand at draw tiume once the
                // final pipline layout is known.
                //

                auto descriptorSetImpl = (DescriptorSetImpl*)descriptorSet;
                m_boundDescriptorSets[int(pipelineType)][index] = descriptorSetImpl;
            }

            virtual SLANG_NO_THROW void SLANG_MCALL uploadBufferDataImpl(
                IBufferResource* buffer,
                size_t offset,
                size_t size,
                void* data) override
            {
                _uploadBufferData(
                    m_commandBuffer->m_cmdList,
                    static_cast<BufferResourceImpl*>(buffer),
                    offset,
                    size,
                    data);
            }

            void setPipelineStateImpl(IPipelineState* state)
            {
                m_currentPipeline = static_cast<PipelineStateImpl*>(state);
            }

            Result _bindRenderState(
                PipelineStateImpl* pipelineStateImpl,
                Submitter* submitter);
        };

        class RenderCommandEncoderImpl
            : public IRenderCommandEncoder
            , public PipelineCommandEncoder
        {
        public:
            virtual SLANG_NO_THROW SlangResult SLANG_MCALL
                queryInterface(SlangUUID const& uuid, void** outObject) override
            {
                if (uuid == GfxGUID::IID_ISlangUnknown ||
                    uuid == GfxGUID::IID_IRenderCommandEncoder)
                {
                    *outObject = static_cast<IRenderCommandEncoder*>(this);
                    return SLANG_OK;
                }
                *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() { return 1; }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() { return 1; }
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
                ExecutionFrameResources* frame,
                CommandBufferImpl* cmdBuffer,
                RenderPassLayoutImpl* renderPass,
                FramebufferImpl* framebuffer)
            {
                m_commandBuffer = cmdBuffer;
                m_d3dCmdList = cmdBuffer->m_cmdList;
                m_preCmdList = nullptr;
                m_device = renderer->m_device;
                m_rendererBase = renderer;
                m_renderPass = renderPass;
                m_framebuffer = framebuffer;
                m_frame = frame;
                m_boundVertexBuffers.clear();
                m_boundIndexBuffer = nullptr;
                m_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                m_boundIndexFormat = DXGI_FORMAT_UNKNOWN;
                m_boundIndexOffset = 0;
                for (auto& boundPipeline : m_boundPipelines)
                    boundPipeline = nullptr;

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
                        auto resourceViewImpl =
                            static_cast<ResourceViewImpl*>(framebuffer->renderTargetViews[i].get());
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
                        auto resourceViewImpl =
                            static_cast<ResourceViewImpl*>(framebuffer->depthStencilView.get());
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
            
            virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) override
            {
                setPipelineStateImpl(state);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL
                bindRootShaderObject(IShaderObject* object) override
            {
                bindRootShaderObjectImpl(PipelineType::Graphics, object);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
                IPipelineLayout* layout,
                UInt index,
                IDescriptorSet* descriptorSet) override
            {
                setDescriptorSetImpl(PipelineType::Graphics, layout, index, descriptorSet);
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
                    if (buffer)
                    {
                        assert(buffer->m_initialUsage == IResource::Usage::VertexBuffer);
                    }

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
                    _bindRenderState(static_cast<PipelineStateImpl*>(pipelineState), &submitter);
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
                        auto resourceViewImpl = static_cast<ResourceViewImpl*>(
                            m_framebuffer->renderTargetViews[i].get());
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
                    auto resourceViewImpl =
                        static_cast<ResourceViewImpl*>(m_framebuffer->depthStencilView.get());
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
                m_frame,
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
            virtual SLANG_NO_THROW SlangResult SLANG_MCALL
                queryInterface(SlangUUID const& uuid, void** outObject) override
            {
                if (uuid == GfxGUID::IID_ISlangUnknown ||
                    uuid == GfxGUID::IID_IComputeCommandEncoder)
                {
                    *outObject = static_cast<IComputeCommandEncoder*>(this);
                    return SLANG_OK;
                }
                *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() { return 1; }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() { return 1; }

        public:
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                PipelineCommandEncoder::endEncodingImpl();
            }
            void init(
                D3D12Device* renderer,
                ExecutionFrameResources* frame,
                CommandBufferImpl* cmdBuffer)
            {
                m_rendererBase = renderer;
                m_commandBuffer = cmdBuffer;
                m_d3dCmdList = cmdBuffer->m_cmdList;
                m_preCmdList = nullptr;
                m_device = renderer->m_device;
                m_frame = frame;
                for (auto& boundPipeline : m_boundPipelines)
                    boundPipeline = nullptr;
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) override
            {
                setPipelineStateImpl(state);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL
                bindRootShaderObject(IShaderObject* object) override
            {
                bindRootShaderObjectImpl(PipelineType::Compute, object);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
                IPipelineLayout* layout,
                UInt index,
                IDescriptorSet* descriptorSet) override
            {
                setDescriptorSetImpl(PipelineType::Compute, layout, index, descriptorSet);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
            {
                auto pipelineStateImpl = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());

                // Submit binding for compute
                {
                    ComputeSubmitter submitter(m_d3dCmdList);
                    _bindRenderState(pipelineStateImpl, &submitter);
                }

                m_d3dCmdList->Dispatch(x, y, z);
            }
        };

        ComputeCommandEncoderImpl m_computeCommandEncoder;
        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeComputeCommands(IComputeCommandEncoder** outEncoder) override
        {
            m_computeCommandEncoder.init(m_renderer, m_frame, this);
            *outEncoder = &m_computeCommandEncoder;
        }

        class ResourceCommandEncoderImpl : public IResourceCommandEncoder
        {
        public:
            virtual SLANG_NO_THROW SlangResult SLANG_MCALL
                queryInterface(SlangUUID const& uuid, void** outObject) override
            {
                if (uuid == GfxGUID::IID_ISlangUnknown ||
                    uuid == GfxGUID::IID_IResourceCommandEncoder)
                {
                    *outObject = static_cast<IResourceCommandEncoder*>(this);
                    return SLANG_OK;
                }
                *outObject = nullptr;
                return SLANG_E_NO_INTERFACE;
            }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() { return 1; }
            virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() { return 1; }

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
                SLANG_UNUSED(dst);
                SLANG_UNUSED(srcOffset);
                SLANG_UNUSED(src);
                SLANG_UNUSED(dstOffset);
                SLANG_UNUSED(size);
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
        };

        ResourceCommandEncoderImpl m_resourceCommandEncoder;

        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeResourceCommands(IResourceCommandEncoder** outEncoder) override
        {
            m_resourceCommandEncoder.init(m_renderer, this);
            *outEncoder = &m_resourceCommandEncoder;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL close() override { m_cmdList->Close(); }
    };

    class CommandQueueImpl
        : public ICommandQueue
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ICommandQueue* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandQueue)
                return static_cast<ICommandQueue*>(this);
            return nullptr;
        }

    public:
        struct CommandBufferPool
        {
            List<RefPtr<CommandBufferImpl>> pool;
            uint32_t allocIndex = 0;
            RefPtr<CommandBufferImpl> allocCommandBuffer(D3D12Device* renderer, ExecutionFrameResources* frame)
            {
                if ((Index)allocIndex < pool.getCount())
                {
                    RefPtr<CommandBufferImpl> result = pool[allocIndex];
                    result->init(renderer, frame);
                    allocIndex++;
                    return result;
                }
                RefPtr<CommandBufferImpl> cmdBuffer = new CommandBufferImpl();
                cmdBuffer->init(renderer, frame);
                pool.add(cmdBuffer);
                return cmdBuffer;
            }
            void reset()
            {
                allocIndex = 0;
            }
        };
        List<CommandBufferPool> m_commandBufferPools;
        List<ExecutionFrameResources> m_frames;
        uint32_t m_frameIndex = 0;
        D3D12Device* m_renderer;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12CommandQueue> m_d3dQueue;
        ComPtr<ID3D12Fence> m_fence;
        uint64_t m_fenceValue = 0;
        HANDLE globalWaitHandle;
        Desc m_desc;
        Result init(
            D3D12Device* renderer,
            uint32_t frameCount,
            uint32_t viewHeapSize,
            uint32_t samplerHeapSize)
        {
            m_renderer = renderer;
            m_device = renderer->m_device;
            m_frames.setCount(frameCount);
            m_commandBufferPools.setCount(frameCount);
            for (uint32_t i = 0; i < frameCount; i++)
            {
                SLANG_RETURN_ON_FAIL(m_frames[i].init(m_device, viewHeapSize, samplerHeapSize));
            }
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
        }
        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override
        {
            return m_desc;
        }
        virtual SLANG_NO_THROW Result SLANG_MCALL
            createCommandBuffer(ICommandBuffer** outCommandBuffer) override
        {
            RefPtr<CommandBufferImpl> result =
                m_commandBufferPools[m_frameIndex].allocCommandBuffer(
                    m_renderer, &m_frames[m_frameIndex]);
            *outCommandBuffer = result.detach();
            return SLANG_OK;
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

            auto& frame = m_frames[m_frameIndex];
            m_fenceValue++;
            m_d3dQueue->Signal(m_fence, m_fenceValue);
            ResetEvent(frame.fenceEvent);
            ResetEvent(globalWaitHandle);
            m_fence->SetEventOnCompletion(m_fenceValue, frame.fenceEvent);
            swapExecutionFrame();
        }

        void swapExecutionFrame()
        {
            m_frameIndex = (m_frameIndex + 1) % m_frames.getCount();
            auto& frame = m_frames[m_frameIndex];
            frame.reset();
            m_commandBufferPools[m_frameIndex].reset();
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

        Result init(
            D3D12Device* renderer,
            const ISwapchain::Desc& swapchainDesc,
            WindowHandle window)
        {
            m_queue = static_cast<CommandQueueImpl*>(swapchainDesc.queue)->m_d3dQueue;
            m_dxgiFactory = renderer->m_deviceInfo.m_dxgiFactory;
            SLANG_RETURN_ON_FAIL(
                D3DSwapchainBase::init(swapchainDesc, window, DXGI_SWAP_EFFECT_FLIP_DISCARD));
            SLANG_RETURN_ON_FAIL(m_swapChain->QueryInterface(m_swapChain3.writeRef()));
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
                imageDesc.setDefaults(IResource::Usage::RenderTarget);
                imageDesc.init2D(
                    IResource::Type::Texture2D, m_desc.format, m_desc.width, m_desc.height, 0);
                RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc);
                image->m_resource.setResource(d3dResource.get());
                image->m_defaultState = D3D12_RESOURCE_STATE_PRESENT;
                ComPtr<ITextureResource> imageResourcePtr;
                imageResourcePtr = image.Ptr();
                m_images.add(imageResourcePtr);
            }
        }
        virtual IDXGIFactory* getDXGIFactory() override { return m_dxgiFactory; }
        virtual IUnknown* getOwningDevice() override { return m_queue; }
        virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override
        {
            return (int)m_swapChain3->GetCurrentBackBufferIndex();
        }
    };

    static PROC loadProc(HMODULE module, char const* name);

    Result createCommandQueueImpl(
        uint32_t frameCount,
        uint32_t viewHeapSize,
        uint32_t samplerHeapSize,
        CommandQueueImpl** outQueue);

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
        m_resourceCommandQueue->createCommandBuffer(info.commandBuffer.writeRef());
        info.d3dCommandList = static_cast<CommandBufferImpl*>(info.commandBuffer.get())->m_cmdList;
        return info;
    }
    void submitResourceCommandsAndWait(const ResourceCommandRecordInfo& info)
    {
        info.commandBuffer->close();
        m_resourceCommandQueue->executeCommandBuffer(info.commandBuffer);
        m_resourceCommandQueue->wait();
    }

    // D3D12Device members.

    Desc m_desc;

    gfx::DeviceInfo m_info;
    String m_adapterName;

    bool m_isInitialized = false;

    ComPtr<ID3D12Debug> m_dxDebug;

    DeviceInfo m_deviceInfo;
    ID3D12Device* m_device = nullptr;

    RefPtr<CommandQueueImpl> m_resourceCommandQueue;

    // Dll entry points
    PFN_D3D12_GET_DEBUG_INTERFACE m_D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE m_D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE m_D3D12SerializeRootSignature = nullptr;

    bool m_nvapi = false;
};


Result D3D12Device::CommandBufferImpl::PipelineCommandEncoder::_bindRenderState(
    PipelineStateImpl* pipelineStateImpl,
    Submitter* submitter)
{
    auto commandList = m_commandBuffer->m_cmdList;
    // TODO: we should only set some of this state as needed...

    auto pipelineTypeIndex = (int)pipelineStateImpl->desc.type;
    auto pipelineLayout = static_cast<PipelineLayoutImpl*>(pipelineStateImpl->m_pipelineLayout.get());

    submitter->setRootSignature(pipelineLayout->m_rootSignature);
    commandList->SetPipelineState(pipelineStateImpl->m_pipelineState);

    ID3D12DescriptorHeap* heaps[] = {
        m_frame->m_viewHeap.getHeap(),
        m_frame->m_samplerHeap.getHeap(),
    };
    commandList->SetDescriptorHeaps(SLANG_COUNT_OF(heaps), heaps);

    // We need to copy descriptors over from the descriptor sets
    // (where they are stored in CPU-visible heaps) to the GPU-visible
    // heaps so that they can be accessed by shader code.

    Int descriptorSetCount = pipelineLayout->m_descriptorSetCount;
    Int rootParameterIndex = 0;
    for (Int dd = 0; dd < descriptorSetCount; ++dd)
    {
        auto descriptorSet = m_boundDescriptorSets[pipelineTypeIndex][dd];
        auto descriptorSetLayout = descriptorSet->m_layout;

        // TODO: require that `descriptorSetLayout` is compatible with
        // `pipelineLayout->descriptorSetlayouts[dd]`.

        {
            if (auto descriptorCount = descriptorSetLayout->m_resourceCount)
            {
                auto& gpuHeap = m_frame->m_viewHeap;
                auto gpuDescriptorTable = gpuHeap.allocate(int(descriptorCount));

                auto& cpuHeap = *descriptorSet->m_resourceHeap;
                auto cpuDescriptorTable = descriptorSet->m_resourceTable;

                m_device->CopyDescriptorsSimple(
                    UINT(descriptorCount),
                    gpuHeap.getCpuHandle(gpuDescriptorTable),
                    cpuHeap.getCpuHandle(int(cpuDescriptorTable)),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

                submitter->setRootDescriptorTable(
                    int(rootParameterIndex++), gpuHeap.getGpuHandle(gpuDescriptorTable));
            }
        }
        {
            if (auto descriptorCount = descriptorSetLayout->m_samplerCount)
            {
                auto& gpuHeap = m_frame->m_samplerHeap;
                auto gpuDescriptorTable = gpuHeap.allocate(int(descriptorCount));

                auto& cpuHeap = *descriptorSet->m_samplerHeap;
                auto cpuDescriptorTable = descriptorSet->m_samplerTable;

                m_device->CopyDescriptorsSimple(
                    UINT(descriptorCount),
                    gpuHeap.getCpuHandle(gpuDescriptorTable),
                    cpuHeap.getCpuHandle(int(cpuDescriptorTable)),
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

                submitter->setRootDescriptorTable(
                    int(rootParameterIndex++), gpuHeap.getGpuHandle(gpuDescriptorTable));
            }
        }
        if (auto rootConstantRangeCount = descriptorSetLayout->m_rootConstantRanges.getCount())
        {
            auto srcData = descriptorSet->m_rootConstantData.getBuffer();

            for (auto& rootConstantRangeInfo : descriptorSetLayout->m_rootConstantRanges)
            {
                auto countOf32bitValues = rootConstantRangeInfo.size / sizeof(uint32_t);
                submitter->setRootConstants(
                    rootConstantRangeInfo.rootParamIndex,
                    0,
                    countOf32bitValues,
                    srcData + rootConstantRangeInfo.offset);
            }
        }
    }

    return SLANG_OK;
}

Result D3D12Device::createCommandQueueImpl(
    uint32_t frameCount,
    uint32_t viewHeapSize,
    uint32_t samplerHeapSize,
    D3D12Device::CommandQueueImpl** outQueue)
{
    RefPtr<D3D12Device::CommandQueueImpl> queue = new D3D12Device::CommandQueueImpl();
    SLANG_RETURN_ON_FAIL(queue->init(this, frameCount, viewHeapSize, samplerHeapSize));
    *outQueue = queue.detach();
    return SLANG_OK;
}

SlangResult SLANG_MCALL createD3D12Device(const IDevice::Desc* desc, IDevice** outDevice)
{
    RefPtr<D3D12Device> result = new D3D12Device();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    *outDevice = result.detach();
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

D3D12Device::~D3D12Device()
{
}

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
        *outBlob = resultBlob.detach();
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
    SLANG_RETURN_ON_FAIL(slangContext.initialize(desc.slang, SLANG_DXBC, "sm_5_1"));

    SLANG_RETURN_ON_FAIL(GraphicsAPIRenderer::initialize(desc));

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
    }

    m_desc = desc;

    // Create a command queue for internal resource transfer operations.
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(1, 32, 4, m_resourceCommandQueue.writeRef()));

    SLANG_RETURN_ON_FAIL(m_cpuViewHeap.init   (m_device, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    SLANG_RETURN_ON_FAIL(m_cpuSamplerHeap.init(m_device, 64,   D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

    SLANG_RETURN_ON_FAIL(m_rtvAllocator.init    (m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
    SLANG_RETURN_ON_FAIL(m_dsvAllocator.init    (m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
    SLANG_RETURN_ON_FAIL(m_viewAllocator.init   (m_device, 64, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    SLANG_RETURN_ON_FAIL(m_samplerAllocator.init(m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

    ComPtr<IDXGIDevice> dxgiDevice;
    if (m_deviceInfo.m_adapter)
    {
        DXGI_ADAPTER_DESC adapterDesc;
        m_deviceInfo.m_adapter->GetDesc(&adapterDesc);
        m_adapterName = String::fromWString(adapterDesc.Description);
        m_info.adapterName = m_adapterName.begin();
    }

    m_isInitialized = true;
    return SLANG_OK;
}

Result D3D12Device::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    RefPtr<CommandQueueImpl> queue;
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(8, 4096, 1024, queue.writeRef()));
    *outQueue = queue.detach();
    return SLANG_OK;
}

SLANG_NO_THROW Result SLANG_MCALL D3D12Device::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> swapchain = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(swapchain->init(this, desc, window));
    *outSwapchain = swapchain.detach();
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

static D3D12_RESOURCE_STATES _calcResourceState(IResource::Usage usage)
{
    typedef IResource::Usage Usage;
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
        case Usage::ShaderResource:         return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                                                   D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        case Usage::GenericRead:            return D3D12_RESOURCE_STATE_GENERIC_READ;
        default: return D3D12_RESOURCE_STATES(0);
    }
}

static D3D12_RESOURCE_FLAGS _calcResourceFlag(IResource::BindFlag::Enum bindFlag)
{
    typedef IResource::BindFlag BindFlag;
    switch (bindFlag)
    {
        case BindFlag::RenderTarget:        return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        case BindFlag::DepthStencil:        return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        case BindFlag::UnorderedAccess:     return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        default:                            return D3D12_RESOURCE_FLAG_NONE;
    }
}

static D3D12_RESOURCE_FLAGS _calcResourceBindFlags(IResource::Usage initialUsage, int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;

        dstFlags |= _calcResourceFlag(IResource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return D3D12_RESOURCE_FLAGS(dstFlags);
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

Result D3D12Device::createTextureResource(IResource::Usage initialUsage, const ITextureResource::Desc& descIn, const ITextureResource::SubresourceData* initData, ITextureResource** outResource)
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

    switch (initialUsage)
    {
    case IResource::Usage::RenderTarget:
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        break;
    case IResource::Usage::DepthWrite:
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        break;
    case IResource::Usage::UnorderedAccess:
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        break;
    default:
        break;
    }

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

                const TextureResource::Size mipSize = srcDesc.size.calcMipSize(j);

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
                        ::memcpy(dstRow, srcRow, mipRowSize);

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
        const D3D12_RESOURCE_STATES finalState = _calcResourceState(initialUsage);
        {
            D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
            texture->m_resource.transition(D3D12_RESOURCE_STATE_COPY_DEST, finalState, submitter);
        }
        submitResourceCommandsAndWait(encodeInfo);
    }

    *outResource = texture.detach();
    return SLANG_OK;
}

Result D3D12Device::createBufferResource(IResource::Usage initialUsage, const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    // Always align up to 256 bytes, since that is required for constant buffers.
    //
    // TODO: only do this for buffers that could potentially be bound as constant buffers...
    //
    const size_t alignedSizeInBytes = D3DUtil::calcAligned(srcDesc.sizeInBytes, 256);

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(initialUsage, srcDesc));

    D3D12_RESOURCE_DESC bufferDesc;
    _initBufferResourceDesc(alignedSizeInBytes, bufferDesc);

    bufferDesc.Flags = _calcResourceBindFlags(initialUsage, srcDesc.bindFlags);

    const D3D12_RESOURCE_STATES initialState = _calcResourceState(initialUsage);
    SLANG_RETURN_ON_FAIL(createBuffer(bufferDesc, initData, srcDesc.sizeInBytes, buffer->m_uploadResource, initialState, buffer->m_resource));

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

    auto samplerHeap = &m_cpuSamplerHeap;

    D3D12HostVisibleDescriptor cpuDescriptor;
    samplerHeap->allocate(&cpuDescriptor);
    m_device->CreateSampler(&dxDesc, cpuDescriptor.cpuHandle);

    // TODO: We really ought to have a free-list of sampler-heap
    // entries that we check before we go to the heap, and then
    // when we are done with a sampler we simply add it to the free list.
    //
    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_renderer = this;
    samplerImpl->m_descriptor = cpuDescriptor;
    *outSampler = samplerImpl.detach();
    return SLANG_OK;
}

Result D3D12Device::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (TextureResourceImpl*) texture;

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::RenderTarget:
        {
            SLANG_RETURN_ON_FAIL(m_rtvAllocator.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_rtvAllocator;
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
            SLANG_RETURN_ON_FAIL(m_dsvAllocator.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_dsvAllocator;
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

            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_viewAllocator;
            m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_viewAllocator;

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

    *outView = viewImpl.detach();
    return SLANG_OK;
}

Result D3D12Device::createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;
    auto resourceDesc = *resourceImpl->getDesc();

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;

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

            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_viewAllocator;
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

            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_viewAllocator;
            m_device->CreateShaderResourceView(resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;
    }

    *outView = viewImpl.detach();
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
        framebuffer->renderTargetViews[i] = desc.renderTargetViews[i];
        framebuffer->renderTargetDescriptors[i] =
            static_cast<ResourceViewImpl*>(desc.renderTargetViews[i])->m_descriptor.cpuHandle;
        auto clearValue =
            static_cast<TextureResourceImpl*>(
                static_cast<ResourceViewImpl*>(desc.renderTargetViews[i])->m_resource.Ptr())
                ->getDesc()
                ->optimalClearValue.color;
        memcpy(&framebuffer->renderTargetClearValues[i], &clearValue, sizeof(ColorClearValue));
    }
    framebuffer->depthStencilView = desc.depthStencilView;
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
    *outFb = framebuffer.detach();
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
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result D3D12Device::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc,
    IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl();
    result->init(desc);
    *outRenderPassLayout = result.detach();
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

    *outLayout = layout.detach();
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
    _initBufferResourceDesc(bufferSize, stagingDesc);

    D3D12Resource stageBuf;
    SLANG_RETURN_ON_FAIL(stageBuf.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, stagingDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr));

    // Do the copy
    encodeInfo.d3dCommandList->CopyBufferRegion(stageBuf, 0, resource, 0, bufferSize);

    // Wait until complete
    submitResourceCommandsAndWait(encodeInfo);

    // Map and copy
    RefPtr<ListBlob> blob = new ListBlob();
    {
        UINT8* data;
        D3D12_RANGE readRange = { 0, bufferSize };

        SLANG_RETURN_ON_FAIL(stageBuf.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        // Copy to memory buffer
        blob->m_data.setCount(bufferSize);
        ::memcpy(blob->m_data.getBuffer(), data, bufferSize);

        stageBuf.getResource()->Unmap(0, nullptr);
    }
    *outBlob = blob.detach();
    return SLANG_OK;
}

void D3D12Device::DescriptorSetImpl::setConstantBuffer(UInt range, UInt index, IBufferResource* buffer)
{
    auto dxDevice = m_renderer->m_device;

    auto resourceImpl = (BufferResourceImpl*) buffer;
    auto resourceDesc = resourceImpl->getDesc();

    // Constant buffer view size must be a multiple of 256 bytes, so we round it up here.
    const size_t alignedSizeInBytes = D3DUtil::calcAligned(resourceDesc->sizeInBytes, 256);

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

void D3D12Device::DescriptorSetImpl::setResource(UInt range, UInt index, IResourceView* view)
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

void D3D12Device::DescriptorSetImpl::setSampler(UInt range, UInt index, ISamplerState* sampler)
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
    auto descriptorIndex = m_samplerTable + arrayIndex;

    m_samplerObjects[arrayIndex] = samplerImpl;
    dxDevice->CopyDescriptorsSimple(
        1,
        m_samplerHeap->getCpuHandle(int(descriptorIndex)),
        samplerImpl->m_descriptor.cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void D3D12Device::DescriptorSetImpl::setCombinedTextureSampler(
    UInt range,
    UInt index,
    IResourceView*   textureView,
    ISamplerState*   sampler)
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
        samplerImpl->m_descriptor.cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void D3D12Device::DescriptorSetImpl::setRootConstants(
    UInt range,
    UInt offset,
    UInt size,
    void const* data)
{
    // The `range` parameter is the index of the range in
    // the original `DescriptorSetLayout::Desc`, which must
    // have been a root-constant range for this call to be
    // valid.
    //
    SLANG_ASSERT(range < UInt(m_layout->m_ranges.getCount()));
    auto& rangeInfo = m_layout->m_ranges[range];
    SLANG_ASSERT(rangeInfo.type == DescriptorSlotType::RootConstant);

    // The `arrayIndex` in that descriptor slot range is the "flat"
    // index of the root constant range that the user is trying
    // to write into. The root constant range represents a range
    // of bytes in the `m_rootConstantData` buffer.
    //
    auto rootConstantIndex = rangeInfo.arrayIndex;
    SLANG_ASSERT(rootConstantIndex >= 0);
    SLANG_ASSERT(rootConstantIndex < m_layout->m_rootConstantRanges.getCount());
    auto& rootConstantRangeInfo = m_layout->m_rootConstantRanges[rootConstantIndex];
    SLANG_ASSERT(offset + size <= UInt(rootConstantRangeInfo.size));

    memcpy((char*)m_rootConstantData.getBuffer() + rootConstantRangeInfo.offset + offset, data, size);
}

Result D3D12Device::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    if (desc.slangProgram && desc.slangProgram->getSpecializationParamCount() != 0)
    {
        // For a specializable program, we don't invoke any actual slang compilation yet.
        RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
        initProgramCommon(shaderProgram, desc);
        *outProgram = shaderProgram.detach();
        return SLANG_OK;
    }

    if( desc.kernelCount == 0 )
    {
        return createProgramFromSlang(this, desc, outProgram);
    }

    RefPtr<ShaderProgramImpl> program(new ShaderProgramImpl());
    program->m_pipelineType = desc.pipelineType;

    if (desc.pipelineType == PipelineType::Compute)
    {
        auto computeKernel = desc.findKernel(StageType::Compute);
        program->m_computeShader.insertRange(0, (const uint8_t*) computeKernel->codeBegin, computeKernel->getCodeSize());
    }
    else
    {
        auto vertexKernel = desc.findKernel(StageType::Vertex);
        auto fragmentKernel = desc.findKernel(StageType::Fragment);

        program->m_vertexShader.insertRange(0, (const uint8_t*) vertexKernel->codeBegin, vertexKernel->getCodeSize());
        program->m_pixelShader.insertRange(0, (const uint8_t*) fragmentKernel->codeBegin, fragmentKernel->getCodeSize());
    }
    initProgramCommon(program, desc);

    *outProgram = program.detach();
    return SLANG_OK;
}

Result D3D12Device::createDescriptorSetLayout(const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout)
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

        case DescriptorSlotType::RootConstant:
            // A root constant slot range doesn't contribute
            // to the toal number of resources or samplers.
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
        descriptorSetLayoutImpl->m_dxRootParameters.add(dxRootParameter);
    }
    if( totalSamplerRangeCount )
    {
        D3D12_ROOT_PARAMETER dxRootParameter = {};
        dxRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        dxRootParameter.DescriptorTable.NumDescriptorRanges = UINT(totalSamplerRangeCount);
        descriptorSetLayoutImpl->m_dxRootParameters.add(dxRootParameter);
    }

    // Next we can allocate space for all the D3D register ranges we need,
    // again based on totals that we can compute easily:
    //
    Int totalRangeCount = totalResourceRangeCount + totalSamplerRangeCount;
    descriptorSetLayoutImpl->m_dxRanges.setCount(totalRangeCount);

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

            case DescriptorSlotType::RootConstant:
                {
                    // A root constant range is a bit different than
                    // the other cases because it does *not* introduce
                    // any descriptor rangess into D3D12 descriptor tables,
                    // while it *does* introduce a distinct root parameter.
                    //
                    D3D12_ROOT_PARAMETER dxRootParameter = {};
                    dxRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                    dxRootParameter.Constants.Num32BitValues = UINT(rangeInfo.count) / UINT(sizeof(uint32_t));

                    // When binding the data for the range to the pipeline,
                    // we will need to know the "root parameter index" in
                    // order to identify the range to D3D12.
                    //
                    auto rootParameterIndex = descriptorSetLayoutImpl->m_dxRootParameters.getCount();
                    descriptorSetLayoutImpl->m_dxRootParameters.add(dxRootParameter);

                    // We need to create and store additional tracking data
                    // to remember this root constant range and how to set it.
                    //
                    // The additional data includes the D3D12 root parameter index,
                    // and the size of the range (in bytes).
                    //
                    DescriptorSetLayoutImpl::RootConstantRangeInfo rootConstantRangeInfo;
                    rootConstantRangeInfo.rootParamIndex = rootParameterIndex;
                    rootConstantRangeInfo.size = rangeDesc.count;
                    //
                    // We also need to compute an offset for the data in the backing
                    // storage of a particular descriptor set; we also use this as
                    // a place to update the  total size of the root constant data.
                    //
                    // Note: We don't deal with alignment issues here. D3D12 requires
                    // all root-constant data to be in multiples of 4 bytes and to be
                    // 4-byte aligned, and that should mean that alignment works
                    // out without extra effort on our part.
                    //
                    rootConstantRangeInfo.offset = descriptorSetLayoutImpl->m_rootConstantDataSize;
                    descriptorSetLayoutImpl->m_rootConstantDataSize += rootConstantRangeInfo.size;

                    auto rootConstantIndex = descriptorSetLayoutImpl->m_rootConstantRanges.getCount();
                    descriptorSetLayoutImpl->m_rootConstantRanges.add(rootConstantRangeInfo);

                    rangeInfo.arrayIndex = rootConstantIndex;
                    rangeInfo.count = 1;
                }
                break;
            }

            descriptorSetLayoutImpl->m_ranges.add(rangeInfo);
        }
    }

    // Finally, we will go through and fill in ready-to-go D3D
    // register range information.
    {
        UInt cbvRegisterCounter = 0;
        UInt srvRegisterCounter = 0;
        UInt uavRegisterCounter = 0;
        UInt samplerRegisterCounter = 0;

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


            case DescriptorSlotType::RootConstant:
                {
                    // A root constant range consumes a `b` register binding
                    // under the D3D12 rules, because it is represented as
                    // a `cbuffer` or `ConstantBuffer` declaration in HLSL.
                    //
                    // We need to allocate a register for the root constant
                    // buffer here to make the bindings line up, but we
                    // will skip out of the rest of the logic (via a `continue`
                    // so that this range doesn't turn into a descriptor
                    // range in one of the D3D12 descriptor tables.
                    //
                    Int dxRegister = rangeDesc.binding;
                    if( dxRegister < 0 )
                    {
                        dxRegister = cbvRegisterCounter;
                    }
                    cbvRegisterCounter = dxRegister + bindingCount;

                    auto rootConstantRangeIndex = descriptorSetLayoutImpl->m_ranges[rr].arrayIndex;
                    auto rootParamIndex = descriptorSetLayoutImpl->m_rootConstantRanges[rootConstantRangeIndex].rootParamIndex;

                    // The root constant range is represented in the D3D12
                    // root signature as its own root parameter (not in any
                    // table), and that root parameter needs to be set up
                    // to reference the correct binding space and index.
                    //
                    auto& dxRootParam = descriptorSetLayoutImpl->m_dxRootParameters[rootParamIndex];
                    dxRootParam.Constants.RegisterSpace = UINT(bindingSpace);
                    dxRootParam.Constants.ShaderRegister = UINT(dxRegister);
                    continue;
                }
                break;
            }

            D3D12_DESCRIPTOR_RANGE& dxRange = descriptorSetLayoutImpl->m_dxRanges[dxRangeIndex];
            memset(&dxRange, 0, sizeof(dxRange));

            Int dxRegister = rangeDesc.binding;

            switch(rangeDesc.type)
            {
            default:
                // ERROR: unsupported slot type.
                break;

            case DescriptorSlotType::Sampler:
                {
                    if( dxRegister < 0 )
                    {
                        dxRegister = samplerRegisterCounter;
                    }
                    samplerRegisterCounter = dxRegister + bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(dxRegister);
                    dxRange.RegisterSpace = UINT(bindingSpace);
                    dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                }
                break;

            case DescriptorSlotType::SampledImage:
            case DescriptorSlotType::UniformTexelBuffer:
                {
                    if( dxRegister < 0 )
                    {
                        dxRegister = srvRegisterCounter;
                    }
                    srvRegisterCounter = dxRegister + bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(dxRegister);
                    dxRange.RegisterSpace = UINT(bindingSpace);
                    dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                }
                break;

            case DescriptorSlotType::CombinedImageSampler:
                {
                    // The combined texture/sampler case basically just
                    // does the work of both the SRV and sampler cases above.
                    //
                    // TODO(tfoley): The current API for passing down an
                    // explicit register/binding can't handle the requirement
                    // that we specify *two* registers/bindings for the
                    // combined image/sampler case.
                    //
                    // Realistically, the `Renderer` implementation for
                    // targes that don't support combined texture/sampler
                    // bindings should just error out when a client attempts
                    // to create a descriptor set that uses them (rather than
                    // the current behavior which adds a lot of complexity
                    // in the name of trying to make them work).

                    {
                        // Here's the SRV logic:
                        Int srvRegister = dxRegister;
                        if( srvRegister < 0 )
                        {
                            srvRegister = srvRegisterCounter;
                        }
                        srvRegisterCounter = srvRegister + bindingCount;

                        dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                        dxRange.NumDescriptors = UINT(bindingCount);
                        dxRange.BaseShaderRegister = UINT(srvRegister);
                        dxRange.RegisterSpace = UINT(bindingSpace);
                        dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                    }

                    {
                        // And here we do the sampler logic at the "paired" index.
                        D3D12_DESCRIPTOR_RANGE& dxPairedSamplerRange = descriptorSetLayoutImpl->m_dxRanges[dxPairedSamplerRangeIndex];
                        memset(&dxPairedSamplerRange, 0, sizeof(dxPairedSamplerRange));

                        Int samplerRegister = dxRegister;
                        if( samplerRegister < 0 )
                        {
                            samplerRegister = samplerRegisterCounter;
                        }
                        samplerRegisterCounter = samplerRegister + bindingCount;

                        dxPairedSamplerRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                        dxPairedSamplerRange.NumDescriptors = UINT(bindingCount);
                        dxPairedSamplerRange.BaseShaderRegister = UINT(samplerRegister);
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
                    if( dxRegister < 0 )
                    {
                        dxRegister = uavRegisterCounter;
                    }
                    uavRegisterCounter = dxRegister + bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(dxRegister);
                    dxRange.RegisterSpace = UINT(bindingSpace);
                    dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                }
                break;
            case DescriptorSlotType::ReadOnlyStorageBuffer:
            {
                if (dxRegister < 0)
                {
                    dxRegister = srvRegisterCounter;
                }
                srvRegisterCounter = dxRegister + bindingCount;

                dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                dxRange.NumDescriptors = UINT(bindingCount);
                dxRange.BaseShaderRegister = UINT(dxRegister);
                dxRange.RegisterSpace = UINT(bindingSpace);
                dxRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            }
            break;
            case DescriptorSlotType::UniformBuffer:
            case DescriptorSlotType::DynamicUniformBuffer:
                {
                    if( dxRegister < 0 )
                    {
                        dxRegister = cbvRegisterCounter;
                    }
                    cbvRegisterCounter = dxRegister + bindingCount;

                    dxRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                    dxRange.NumDescriptors = UINT(bindingCount);
                    dxRange.BaseShaderRegister = UINT(dxRegister);
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

Result D3D12Device::createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout)
{
    static const UInt kMaxRanges = 16;
    static const UInt kMaxRootParameters = 32;

    D3D12_DESCRIPTOR_RANGE ranges[kMaxRanges];
    D3D12_ROOT_PARAMETER rootParameters[kMaxRootParameters];

    UInt rangeCount = 0;
    UInt rootParameterCount = 0;

    auto descriptorSetCount = desc.descriptorSetCount;

    Int spaceCounter = 0;

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
        Int space = descriptorSetInfo.space;
        if( space < 0 )
        {
            space = spaceCounter;
        }
        spaceCounter = space+1;

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
            range.RegisterSpace = UINT(space);

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

        // For now we assume that the register space used for
        // logical descriptor set #N will be space N.
        //
        // Note: this is the same assumption made in the first
        // loop, and any change/fix will need to be made to
        // both places consistently.
        //
        UInt bindingSpace   = dd;

        // Copy root parameter information from the set layout to our
        // overall pipeline layout.
        for( auto setRootParameter : descriptorSetLayout->m_dxRootParameters )
        {
            auto& rootParameter = rootParameters[rootParameterCount++];
            rootParameter = setRootParameter;

            switch( rootParameter.ParameterType )
            {
            default:
                break;

            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                // In the case where this parameter is a descriptor table, it
                // needs to point into our array of ranges (with offsets applied),
                // so we will fix up those pointers here.
                //
                rootParameter.DescriptorTable.pDescriptorRanges = rangePtr;
                rangePtr += rootParameter.DescriptorTable.NumDescriptorRanges;
                break;

            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
                // In the case where the parameter is a root constant range,
                // it needs to reflect the register space for the descriptor
                // set, as computed based on sets specified.
                //
                rootParameter.Constants.RegisterSpace = UINT(bindingSpace);
                break;
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

Result D3D12Device::createDescriptorSet(
    IDescriptorSetLayout* layout,
    IDescriptorSet::Flag::Enum flag,
    IDescriptorSet** outDescriptorSet)
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
        descriptorSetImpl->m_resourceObjects.setCount(resourceCount);
    }

    Int samplerCount = layoutImpl->m_samplerCount;
    if( samplerCount )
    {
        auto samplerHeap = &m_cpuSamplerHeap;
        descriptorSetImpl->m_samplerHeap = samplerHeap;
        descriptorSetImpl->m_samplerTable = samplerHeap->allocate(int(samplerCount));
        descriptorSetImpl->m_samplerObjects.setCount(samplerCount);
    }

    descriptorSetImpl->m_rootConstantData.setCount(layoutImpl->m_rootConstantDataSize);

    *outDescriptorSet = descriptorSetImpl.detach();
    return SLANG_OK;
}

Result D3D12Device::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto programImpl = (ShaderProgramImpl*) desc.program;
    auto inputLayoutImpl = (InputLayoutImpl*) desc.inputLayout;

    // Describe and create the graphics pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.pRootSignature = pipelineLayoutImpl->m_rootSignature;

    psoDesc.VS = { programImpl->m_vertexShader.getBuffer(), SIZE_T(programImpl->m_vertexShader.getCount()) };
    psoDesc.PS = { programImpl->m_pixelShader .getBuffer(), SIZE_T(programImpl->m_pixelShader .getCount()) };

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
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result D3D12Device::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto programImpl = (ShaderProgramImpl*) desc.program;

    // Only actually create a D3D12 pipeline state if the pipeline is fully specialized.
    ComPtr<ID3D12PipelineState> pipelineState;
    if (!programImpl->slangProgram || programImpl->slangProgram->getSpecializationParamCount() == 0)
    {
        // Describe and create the compute pipeline state object
        D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
        computeDesc.pRootSignature = pipelineLayoutImpl->m_rootSignature;
        computeDesc.CS = {
            programImpl->m_computeShader.getBuffer(),
            SIZE_T(programImpl->m_computeShader.getCount())};

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
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

} // renderer_test
