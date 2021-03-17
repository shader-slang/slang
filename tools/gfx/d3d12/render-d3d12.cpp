// render-d3d12.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "render-d3d12.h"

//WORKING:#include "options.h"
#include "../renderer-shared.h"
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

class D3D12Device : public RendererBase
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

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout,
        ShaderObjectLayoutBase** outLayout) override;
    virtual Result createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
        override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override;

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
            m_renderer->m_cpuSamplerHeap.free(m_descriptor);
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

    class CommandBufferImpl;

    class PipelineCommandEncoder
    {
    public:
        bool m_isOpen = false;
        bool m_bindingDirty = true;
        CommandBufferImpl* m_commandBuffer;
        ExecutionFrameResources* m_frame;
        D3D12Device* m_renderer;
        ID3D12Device* m_device;
        ID3D12GraphicsCommandList* m_d3dCmdList;
        ID3D12GraphicsCommandList* m_preCmdList = nullptr;

        RefPtr<PipelineStateImpl> m_currentPipeline;
        RefPtr<ShaderObjectBase> m_rootShaderObject;

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
            m_frame = commandBuffer->m_frame;
        }

        void endEncodingImpl() { m_isOpen = false; }

        void bindRootShaderObjectImpl(IShaderObject* object)
        {
            m_rootShaderObject = static_cast<RootShaderObjectImpl*>(object);
            m_bindingDirty = true;
        }

        void setPipelineStateImpl(IPipelineState* pipelineState)
        {
            m_currentPipeline = static_cast<PipelineStateImpl*>(pipelineState);
            m_bindingDirty = true;
        }

        Result _bindRenderState(Submitter* submitter);
    };

    struct DescriptorHeapReference
    {
        bool isCpuHeap;
        union Reference
        {
            D3D12DescriptorHeap* gpuHeap;
            D3D12HostVisibleDescriptorAllocator* cpuHeap;
        } ptr;
        DescriptorHeapReference& operator=(D3D12DescriptorHeap* gpuHeap)
        {
            ptr.gpuHeap = gpuHeap;
            isCpuHeap = false;
            return *this;
        }
        DescriptorHeapReference& operator=(D3D12HostVisibleDescriptorAllocator* cpuHeap)
        {
            ptr.cpuHeap = cpuHeap;
            isCpuHeap = true;
            return *this;
        }
        SLANG_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle(int index) const
        {
            if (isCpuHeap)
                return ptr.cpuHeap->getCpuHandle(index);
            else
                return ptr.gpuHeap->getCpuHandle(index);
        }
        SLANG_FORCE_INLINE D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle(int index) const
        {
            SLANG_ASSERT(!isCpuHeap);
            return ptr.gpuHeap->getGpuHandle(index);
        }
    };

    struct DescriptorTable
    {
        DescriptorHeapReference heap;
        uint32_t table;
    };

    struct BindingOffset
    {
        int32_t resource;
        int32_t sampler;
    };

    struct RootBindingState
    {
        ExecutionFrameResources* frame;
        D3D12Device* device;
        ArrayView<DescriptorTable> descriptorTables;
        BindingOffset offset;
        uint32_t rootParamIndex; // The root parameter index of this object.
        uint32_t futureRootParamOffset; // The starting offset of additional sub-object descriptor tables.
    };

    struct DescriptorSetInfo
    {
        uint32_t resourceDescriptorCount = 0;
        uint32_t samplerDescriptorCount = 0;
    };

    struct BindingLocation
    {
        int32_t index;
        BindingOffset offsetInDescriptorTable;
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
        struct BindingRangeInfo
        {
            slang::BindingType bindingType;
            uint32_t count;
            uint32_t spaceIndex;
            uint32_t flatResourceOffset; // Offset in flattend array of resource binding slots.
            BindingLocation binding;

            // Returns true if this binding range consumes a specialization argument slot.
            bool isSpecializationArg() const
            {
                return bindingType == slang::BindingType::ExistentialValue;
            }
        };
        struct SubObjectRangeInfo
        {
            RefPtr<ShaderObjectLayoutImpl> layout;
            Index bindingRangeIndex;
            slang::BindingType bindingType;

            // The offset for the constant buffer descriptor if this
            // sub-object is referenced as a `ConstantBuffer<T>`.
            // For a `ParameterBlock` binding range, this is always 0 since
            // parameter blocks start in a fresh descriptor table.
            BindingOffset descriptorOffset;
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
            DescriptorSetInfo m_descriptorSetInfo;
            uint32_t m_subObjectCount = 0;
            uint32_t m_flatResourceCount = 0;

            void addBindingRangesOfType(slang::TypeLayoutReflection* typeLayout)
            {
                SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();

                // Reserve CBV slot for the implicit constant buffer if the type contains
                // ordinary uniform data fields.
                if (typeLayout->getSize(slang::ParameterCategory::Uniform) != 0)
                {
                    m_descriptorSetInfo.resourceDescriptorCount = 1;
                }

                for (SlangInt r = 0; r < bindingRangeCount; ++r)
                {
                    slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                    uint32_t count = (uint32_t)typeLayout->getBindingRangeBindingCount(r);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(r);
                    BindingRangeInfo bindingRangeInfo = {};
                    bindingRangeInfo.bindingType = slangBindingType;
                    bindingRangeInfo.count = count;
                    bindingRangeInfo.flatResourceOffset = m_flatResourceCount;
                    bindingRangeInfo.spaceIndex =
                        (uint32_t)typeLayout->getBindingRangeDescriptorSetIndex(r);

                    switch (slangBindingType)
                    {
                    case slang::BindingType::ConstantBuffer:
                    case slang::BindingType::ParameterBlock:
                    case slang::BindingType::ExistentialValue:
                        bindingRangeInfo.binding.index = m_subObjectCount;
                        m_subObjectCount += count;
                        break;

                    case slang::BindingType::Sampler:
                        bindingRangeInfo.binding.offsetInDescriptorTable.sampler =
                            m_descriptorSetInfo.samplerDescriptorCount;
                        m_descriptorSetInfo.samplerDescriptorCount += count;
                        break;

                    case slang::BindingType::CombinedTextureSampler:
                        bindingRangeInfo.binding.offsetInDescriptorTable.sampler =
                            m_descriptorSetInfo.samplerDescriptorCount;
                        bindingRangeInfo.binding.offsetInDescriptorTable.resource =
                            m_descriptorSetInfo.resourceDescriptorCount;
                        m_descriptorSetInfo.samplerDescriptorCount += count;
                        m_descriptorSetInfo.resourceDescriptorCount += count;
                        m_flatResourceCount += count;
                        break;

                    case slang::BindingType::MutableRawBuffer:
                    case slang::BindingType::MutableTexture:
                    case slang::BindingType::MutableTypedBuffer:
                        bindingRangeInfo.binding.offsetInDescriptorTable.resource =
                            m_descriptorSetInfo.resourceDescriptorCount;
                        m_descriptorSetInfo.resourceDescriptorCount += count;
                        m_flatResourceCount += count;
                        break;

                    case slang::BindingType::VaryingInput:
                    case slang::BindingType::VaryingOutput:
                        break;

                    default:
                        bindingRangeInfo.binding.offsetInDescriptorTable.resource =
                            m_descriptorSetInfo.resourceDescriptorCount;
                        m_descriptorSetInfo.resourceDescriptorCount += count;
                        m_flatResourceCount += count;
                        break;
                    }
                    m_bindingRanges.add(bindingRangeInfo);
                }
            }

            Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout)
            {
                typeLayout = _unwrapParameterGroups(typeLayout);

                m_elementTypeLayout = typeLayout;

                // Compute the binding ranges that are used to store
                // the logical contents of the object in memory.

                addBindingRangesOfType(typeLayout);

                SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
                for (SlangInt r = 0; r < subObjectRangeCount; ++r)
                {
                    SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
                    auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
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
                    if (slangBindingType != slang::BindingType::ExistentialValue)
                    {
                        createForElementType(
                            m_renderer,
                            slangLeafTypeLayout->getElementTypeLayout(),
                            subObjectLayout.writeRef());
                    }

                    SubObjectRangeInfo subObjectRange;
                    subObjectRange.bindingRangeIndex = bindingRangeIndex;
                    subObjectRange.layout = subObjectLayout;
                    subObjectRange.bindingType = slangBindingType;
                    subObjectRange.descriptorOffset.resource =
                        m_descriptorSetInfo.resourceDescriptorCount;
                    subObjectRange.descriptorOffset.sampler =
                        m_descriptorSetInfo.samplerDescriptorCount;
                    m_subObjectRanges.add(subObjectRange);
                }

                return SLANG_OK;
            }

            SlangResult build(ShaderObjectLayoutImpl** outLayout)
            {
                auto layout = RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
                SLANG_RETURN_ON_FAIL(layout->_init(this));

                *outLayout = layout.detach();
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

        DescriptorSetInfo getDescriptorSetInfo() { return m_descriptorSetInfo; }

        slang::TypeLayoutReflection* getElementTypeLayout() { return m_elementTypeLayout; }

        uint32_t getResourceCount() { return m_resourceSlotCount; }

        Index getSubObjectCount() { return m_subObjectCount; }

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

            m_descriptorSetInfo = builder->m_descriptorSetInfo;
            m_bindingRanges = _Move(builder->m_bindingRanges);
            m_subObjectCount = builder->m_subObjectCount;
            m_subObjectRanges = builder->m_subObjectRanges;
            m_resourceSlotCount = builder->m_flatResourceCount;
            return SLANG_OK;
        }

        List<BindingRangeInfo> m_bindingRanges;
        DescriptorSetInfo m_descriptorSetInfo;
        Index m_subObjectCount = 0;
        List<SubObjectRangeInfo> m_subObjectRanges;
        uint32_t m_resourceSlotCount;
    };

    class RootShaderObjectLayoutImpl : public ShaderObjectLayoutImpl
    {
        typedef ShaderObjectLayoutImpl Super;

    public:
        struct EntryPointInfo
        {
            RefPtr<ShaderObjectLayoutImpl> layout;
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

                *outLayout = layout.detach();
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

            struct BindingRegisterOffset
            {
                // The index to the physical descriptor set that stores the binding.
                uint32_t descriptorSetIndex;

                uint32_t spaceOffset; // The `space` index as specified in shader.
                uint32_t textureOffset; // `t` registers
                uint32_t samplerOffset; // `s` registers
                uint32_t constantBufferOffset; // `b` registers
                uint32_t uavOffset; // `u` registers
                void set(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t value)
                {
                    switch (type)
                    {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                        constantBufferOffset = value;
                        return;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                        uavOffset = value;
                        return;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                        textureOffset = value;
                        return;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                        samplerOffset = value;
                        return;
                    default:
                        break;
                    }
                }
                uint32_t get(D3D12_DESCRIPTOR_RANGE_TYPE type)
                {
                    switch (type)
                    {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                        return constantBufferOffset;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                        return uavOffset;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                        return textureOffset;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                        return samplerOffset;
                    default:
                        return 0;
                    }
                }
            };

            void addDescriptorRange(
                slang::TypeLayoutReflection* typeLayout,
                D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
                Index bindingRangeIndex,
                BindingRegisterOffset* offset,
                BindingRegisterOffset* newOffset)
            {
                D3D12_DESCRIPTOR_RANGE range = {};
                range.RangeType = rangeType;
                auto descriptorRangeIndex =
                    typeLayout->getBindingRangeFirstDescriptorRangeIndex(bindingRangeIndex);
                auto relativeSpaceIndex =
                    (uint32_t)typeLayout->getBindingRangeDescriptorSetIndex(bindingRangeIndex);
                auto space = offset->spaceOffset + relativeSpaceIndex;
                // Update descriptor range descs in current descriptor set.
                auto& descriptorSet = m_descriptorSets[offset->descriptorSetIndex];
                range.NumDescriptors =
                    (UINT)typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(
                        relativeSpaceIndex, descriptorRangeIndex);
                range.BaseShaderRegister =
                    (UINT)typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
                        relativeSpaceIndex, descriptorRangeIndex) +
                    offset->get(range.RangeType);
                newOffset->set(
                    range.RangeType,
                    Math::Max(range.BaseShaderRegister + 1, newOffset->get(range.RangeType)));
                range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                range.RegisterSpace = space;
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
            }

            void addObject(slang::TypeLayoutReflection* typeLayout, BindingRegisterOffset* offset)
            {
                typeLayout = _unwrapParameterGroups(typeLayout);
                SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
                // `register` and `space` index offset of future sub-objects.
                BindingRegisterOffset subObjectOffset = *offset;
                for (SlangInt i = 0; i < bindingRangeCount; i++)
                {
                    auto bindingType = typeLayout->getBindingRangeType(i);
                    D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
                    if (translateDescriptorRangeType(bindingType, &rangeType) != SLANG_OK)
                    {
                        // Ignore all descriptor ranges that does not map directly into a
                        // d3d descriptor.
                        continue;
                    }
                    // The CBV descriptor range, along with any additional descriptor ranges associated
                    // with the constant buffer binding range, will be appended to the end of this object's
                    // descriptor table, so we skip them now.
                    if (bindingType == slang::BindingType::ConstantBuffer)
                        continue;
                    addDescriptorRange(typeLayout, rangeType, i, offset, &subObjectOffset);
                }
                auto subObjectCount = typeLayout->getSubObjectRangeCount();
                for (SlangInt i = 0; i < subObjectCount; i++)
                {
                    auto rangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(i);
                    switch (typeLayout->getBindingRangeType(rangeIndex))
                    {
                    case slang::BindingType::ConstantBuffer:
                        {
                            auto subObjectType = typeLayout->getBindingRangeLeafTypeLayout(rangeIndex);
                            auto subObjectElementType = _unwrapParameterGroups(subObjectType);
                            if (subObjectElementType->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
                            {
                                addDescriptorRange(
                                    typeLayout,
                                    D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                                    rangeIndex,
                                    offset,
                                    &subObjectOffset);
                            }
                            addObject(subObjectType, &subObjectOffset);
                        }
                        break;
                    case slang::BindingType::ParameterBlock:
                        {
                            BindingRegisterOffset newOffset = {};
                            newOffset.descriptorSetIndex = (uint32_t)m_descriptorSets.getCount();
                            m_descriptorSets.add(DescriptorSetLayout{});
                            newOffset.spaceOffset =
                                offset->spaceOffset +
                                (uint32_t)typeLayout->getBindingRangeDescriptorSetIndex(rangeIndex);
                            auto subObjectType =
                                typeLayout->getBindingRangeLeafTypeLayout(rangeIndex);
                            addObject(subObjectType, &newOffset);
                        }
                        break;
                    }
                }
                *offset = subObjectOffset;
            }

            static BindingRegisterOffset getOffsetFromVarLayout(
                slang::VariableLayoutReflection* varLayout)
            {
                BindingRegisterOffset offset;
                offset.descriptorSetIndex = 0;
                offset.spaceOffset =
                    (uint32_t)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_REGISTER_SPACE);
                offset.samplerOffset =
                    (uint32_t)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SAMPLER_STATE);
                offset.textureOffset =
                    (uint32_t)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE);
                offset.constantBufferOffset =
                    (uint32_t)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
                offset.uavOffset =
                    (uint32_t)varLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS);
                return offset;
            }

            void addObject(
                slang::TypeLayoutReflection* typeLayout,
                slang::VariableLayoutReflection* varLayout)
            {
                auto offset = getOffsetFromVarLayout(varLayout);
                addObject(typeLayout, &offset);
            }

            void addEntryPoint(slang::EntryPointReflection* entryPoint)
            {
                BindingRegisterOffset offset = getOffsetFromVarLayout(entryPoint->getVarLayout());
                if (entryPoint->hasDefaultConstantBuffer())
                {
                    addDescriptorRange(
                        entryPoint->getTypeLayout(),
                        D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                        0,
                        &offset,
                        &offset);
                }
                addObject(entryPoint->getTypeLayout(), &offset);
            }

            D3D12_ROOT_SIGNATURE_DESC& build(
                List<D3D12Device::DescriptorSetInfo>& outRootDescriptorSetInfos)
            {
                for (Index i = 0; i < m_descriptorSets.getCount(); i++)
                {
                    auto& descriptorSet = m_descriptorSets[i];
                    D3D12Device::DescriptorSetInfo setInfo;
                    setInfo.resourceDescriptorCount = descriptorSet.m_resourceCount;
                    setInfo.samplerDescriptorCount = descriptorSet.m_samplerCount;
                    outRootDescriptorSetInfos.add(setInfo);
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
            slang::IComponentType* program,
            ID3D12RootSignature** outRootSignature,
            List<DescriptorSetInfo>& outRootDescriptorSetInfos)
        {
            RootSignatureDescBuilder builder;
            builder.m_descriptorSets.add(DescriptorSetLayout{});

            auto layout = program->getLayout();
            auto globalParamLayout = layout->getGlobalParamsTypeLayout();
            auto globalVarLayout = layout->getGlobalParamsVarLayout();

            builder.addObject(globalParamLayout, globalVarLayout);

            for (SlangUInt i = 0; i < layout->getEntryPointCount(); i++)
            {
                auto entryPoint = layout->getEntryPointByIndex(i);
                builder.addEntryPoint(entryPoint);
            }

            auto& rootSignatureDesc = builder.build(outRootDescriptorSetInfos);

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

            SLANG_RETURN_ON_FAIL(builder.build(outLayout));

            if (program->getSpecializationParamCount() == 0)
            {
                // For root object, we would like know the union of all binding slots
                // including all sub-objects in the shader-object hierarchy, so at
                // parameter binding time we can easily know how many GPU descriptor tables
                // to create without walking throught the shader-object hierarchy again.
                // We build out this array along with root signature construction.
                List<DescriptorSetInfo> outRootDescriptorSetInfos;
                SLANG_RETURN_ON_FAIL(createRootSignatureFromSlang(
                    device,
                    program,
                    (*outLayout)->m_rootSignature.writeRef(),
                    (*outLayout)->m_gpuDescriptorSetInfos));
            }
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
        List<DescriptorSetInfo> m_gpuDescriptorSetInfos;
    };

    class ShaderProgramImpl : public ShaderProgramBase
    {
    public:
        PipelineType m_pipelineType;
        List<uint8_t> m_vertexShader;
        List<uint8_t> m_pixelShader;
        List<uint8_t> m_computeShader;
        RefPtr<RootShaderObjectLayoutImpl> m_rootObjectLayout;
    };

    class ShaderObjectImpl : public ShaderObjectBase
    {
    public:
        static Result create(
            D3D12Device* device,
            ShaderObjectLayoutImpl* layout,
            ShaderObjectImpl** outShaderObject)
        {
            auto object = ComPtr<ShaderObjectImpl>(new ShaderObjectImpl());
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            *outShaderObject = object.detach();
            return SLANG_OK;
        }

        ~ShaderObjectImpl()
        {
            auto layoutImpl = static_cast<ShaderObjectLayoutImpl*>(m_layout.Ptr());
            if (m_descriptorSet.m_resourceCount)
            {
                m_resourceHeap->free(
                    m_descriptorSet.m_resourceTable, m_descriptorSet.m_resourceCount);
            }
            if (m_descriptorSet.m_samplerCount)
            {
                m_samplerHeap->free(m_descriptorSet.m_samplerTable, m_descriptorSet.m_samplerCount);
            }
        }

        RendererBase* getDevice() { return m_layout->getDevice(); }

        SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return 0; }

        SLANG_NO_THROW Result SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint)
            SLANG_OVERRIDE
        {
            *outEntryPoint = nullptr;
            return SLANG_OK;
        }

        ShaderObjectLayoutImpl* getLayout()
        {
            return static_cast<ShaderObjectLayoutImpl*>(m_layout.Ptr());
        }

        SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout()
            SLANG_OVERRIDE
        {
            return m_layout->getElementTypeLayout();
        }

        SLANG_NO_THROW Result SLANG_MCALL
            setData(ShaderOffset const& inOffset, void const* data, size_t inSize) SLANG_OVERRIDE
        {
            Index offset = inOffset.uniformOffset;
            Index size = inSize;

            char* dest = m_ordinaryData.getBuffer();
            Index availableSize = m_ordinaryData.getCount();

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

        virtual SLANG_NO_THROW Result SLANG_MCALL
            setObject(ShaderOffset const& offset, IShaderObject* object) SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;

            auto subObject = static_cast<ShaderObjectImpl*>(object);

            auto bindingRangeIndex = offset.bindingRangeIndex;
            auto& bindingRange = layout->getBindingRange(bindingRangeIndex);

            m_objects[bindingRange.binding.index + offset.bindingArrayIndex] = subObject;

            // If the range being assigned into represents an interface/existential-type leaf field,
            // then we need to consider how the `object` being assigned here affects specialization.
            // We may also need to assign some data from the sub-object into the ordinary data
            // buffer for the parent object.
            //
            if (bindingRange.bindingType == slang::BindingType::ExistentialValue)
            {
                // A leaf field of interface type is laid out inside of the parent object
                // as a tuple of `(RTTI, WitnessTable, Payload)`. The layout of these fields
                // is a contract between the compiler and any runtime system, so we will
                // need to rely on details of the binary layout.

                // We start by querying the layout/type of the concrete value that the application
                // is trying to store into the field, and also the layout/type of the leaf
                // existential-type field itself.
                //
                auto concreteTypeLayout = subObject->getElementTypeLayout();
                auto concreteType = concreteTypeLayout->getType();
                //
                auto existentialTypeLayout =
                    layout->getElementTypeLayout()->getBindingRangeLeafTypeLayout(
                        bindingRangeIndex);
                auto existentialType = existentialTypeLayout->getType();

                // The first field of the tuple (offset zero) is the run-time type information
                // (RTTI) ID for the concrete type being stored into the field.
                //
                // TODO: We need to be able to gather the RTTI type ID from `object` and then
                // use `setData(offset, &TypeID, sizeof(TypeID))`.

                // The second field of the tuple (offset 8) is the ID of the "witness" for the
                // conformance of the concrete type to the interface used by this field.
                //
                auto witnessTableOffset = offset;
                witnessTableOffset.uniformOffset += 8;
                //
                // Conformances of a type to an interface are computed and then stored by the
                // Slang runtime, so we can look up the ID for this particular conformance (which
                // will create it on demand).
                //
                ComPtr<slang::ISession> slangSession;
                SLANG_RETURN_ON_FAIL(getRenderer()->getSlangSession(slangSession.writeRef()));
                //
                // Note: If the type doesn't actually conform to the required interface for
                // this sub-object range, then this is the point where we will detect that
                // fact and error out.
                //
                uint32_t conformanceID = 0xFFFFFFFF;
                SLANG_RETURN_ON_FAIL(slangSession->getTypeConformanceWitnessSequentialID(
                    concreteType, existentialType, &conformanceID));
                //
                // Once we have the conformance ID, then we can write it into the object
                // at the required offset.
                //
                SLANG_RETURN_ON_FAIL(
                    setData(witnessTableOffset, &conformanceID, sizeof(conformanceID)));

                // The third field of the tuple (offset 16) is the "payload" that is supposed to
                // hold the data for a value of the given concrete type.
                //
                auto payloadOffset = offset;
                payloadOffset.uniformOffset += 16;

                // There are two cases we need to consider here for how the payload might be used:
                //
                // * If the concrete type of the value being bound is one that can "fit" into the
                //   available payload space,  then it should be stored in the payload.
                //
                // * If the concrete type of the value cannot fit in the payload space, then it
                //   will need to be stored somewhere else.
                //
                if (_doesValueFitInExistentialPayload(concreteTypeLayout, existentialTypeLayout))
                {
                    // If the value can fit in the payload area, then we will go ahead and copy
                    // its bytes into that area.
                    //
                    setData(
                        payloadOffset,
                        subObject->m_ordinaryData.getBuffer(),
                        subObject->m_ordinaryData.getCount());
                }
                else
                {
                    // If the value does *not *fit in the payload area, then there is nothing
                    // we can do at this point (beyond saving a reference to the sub-object, which
                    // was handled above).
                    //
                    // Once all the sub-objects have been set into the parent object, we can
                    // compute a specialized layout for it, and that specialized layout can tell
                    // us where the data for these sub-objects has been laid out.
                    return SLANG_E_NOT_IMPLEMENTED;
                }
            }
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            getObject(ShaderOffset const& offset, IShaderObject** outObject) SLANG_OVERRIDE
        {
            SLANG_ASSERT(outObject);
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

            auto object = m_objects[bindingRange.binding.index + offset.bindingArrayIndex].Ptr();
            object->addRef();
            *outObject = object;
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL
            setResource(ShaderOffset const& offset, IResourceView* resourceView) SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;

            auto resourceViewImpl = static_cast<ResourceViewImpl*>(resourceView);

            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
            auto descriptorSlotIndex = bindingRange.binding.offsetInDescriptorTable.resource +
                                       (int32_t)offset.bindingArrayIndex;
            // Hold a reference to the resource to prevent its destruction.
            m_boundResources[bindingRange.flatResourceOffset + offset.bindingArrayIndex] =
                resourceViewImpl->m_resource;
            ID3D12Device* d3dDevice = static_cast<D3D12Device*>(getDevice())->m_device;
            d3dDevice->CopyDescriptorsSimple(
                1,
                m_resourceHeap->getCpuHandle(
                    m_descriptorSet.m_resourceTable +
                    bindingRange.binding.offsetInDescriptorTable.resource +
                    (int32_t)offset.bindingArrayIndex),
                resourceViewImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            return SLANG_OK;
        }

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
                m_samplerHeap->getCpuHandle(
                    m_descriptorSet.m_samplerTable +
                    bindingRange.binding.offsetInDescriptorTable.sampler +
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
                m_resourceHeap->getCpuHandle(
                    m_descriptorSet.m_resourceTable +
                    bindingRange.binding.offsetInDescriptorTable.resource +
                    (int32_t)offset.bindingArrayIndex),
                resourceViewImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            auto samplerImpl = static_cast<SamplerStateImpl*>(sampler);
            d3dDevice->CopyDescriptorsSimple(
                1,
                m_samplerHeap->getCpuHandle(
                    m_descriptorSet.m_samplerTable +
                    bindingRange.binding.offsetInDescriptorTable.sampler +
                    (int32_t)offset.bindingArrayIndex),
                samplerImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            return SLANG_OK;
        }

    public:
        // Appends all types that are used to specialize the element type of this shader object in
        // `args` list.
        virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
        {
            auto& subObjectRanges = getLayout()->getSubObjectRanges();
            // The following logic is built on the assumption that all fields that involve
            // existential types (and therefore require specialization) will results in a sub-object
            // range in the type layout. This allows us to simply scan the sub-object ranges to find
            // out all specialization arguments.
            Index subObjectRangeCount = subObjectRanges.getCount();
            for (Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount;
                 subObjectRangeIndex++)
            {
                auto const& subObjectRange = subObjectRanges[subObjectRangeIndex];
                auto const& bindingRange =
                    getLayout()->getBindingRange(subObjectRange.bindingRangeIndex);

                Index count = bindingRange.count;
                SLANG_ASSERT(count == 1);

                Index subObjectIndexInRange = 0;
                auto subObject = m_objects[bindingRange.binding.index + subObjectIndexInRange];

                switch (bindingRange.bindingType)
                {
                case slang::BindingType::ExistentialValue:
                    {
                        // A binding type of `ExistentialValue` means the sub-object represents a
                        // interface-typed field. In this case the specialization argument for this
                        // field is the actual specialized type of the bound shader object. If the
                        // shader object's type is an ordinary type without existential fields, then
                        // the type argument will simply be the ordinary type. But if the sub
                        // object's type is itself a specialized type, we need to make sure to use
                        // that type as the specialization argument.

                        ExtendedShaderObjectType specializedSubObjType;
                        SLANG_RETURN_ON_FAIL(
                            subObject->getSpecializedShaderObjectType(&specializedSubObjType));
                        args.add(specializedSubObjType);
                        break;
                    }
                case slang::BindingType::ParameterBlock:
                case slang::BindingType::ConstantBuffer:
                    // Currently we only handle the case where the field's type is
                    // `ParameterBlock<SomeStruct>` or `ConstantBuffer<SomeStruct>`, where
                    // `SomeStruct` is a struct type (not directly an interface type). In this case,
                    // we just recursively collect the specialization arguments from the bound sub
                    // object.
                    SLANG_RETURN_ON_FAIL(subObject->collectSpecializationArgs(args));
                    // TODO: we need to handle the case where the field is of the form
                    // `ParameterBlock<IFoo>`. We should treat this case the same way as the
                    // `ExistentialValue` case here, but currently we lack a mechanism to
                    // distinguish the two scenarios.
                    break;
                }
                // TODO: need to handle another case where specialization happens on resources
                // fields e.g. `StructuredBuffer<IFoo>`.
            }
            return SLANG_OK;
        }

    protected:
        Result init(D3D12Device* device, ShaderObjectLayoutImpl* layout)
        {
            m_layout = layout;

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
                m_ordinaryData.setCount(uniformSize);
                memset(m_ordinaryData.getBuffer(), 0, uniformSize);
            }

            // Allocate descriptor tables for this shader object.
            m_resourceHeap = &device->m_cpuViewHeap;
            m_samplerHeap = &device->m_cpuSamplerHeap;
            auto descSetInfo = layout->getDescriptorSetInfo();
            m_descriptorSet.m_resourceCount = descSetInfo.resourceDescriptorCount;
            if (descSetInfo.resourceDescriptorCount)
            {
                m_descriptorSet.m_resourceTable =
                    m_resourceHeap->allocate(descSetInfo.resourceDescriptorCount);
            }
            m_descriptorSet.m_samplerCount = descSetInfo.samplerDescriptorCount;
            if (descSetInfo.samplerDescriptorCount)
            {
                m_descriptorSet.m_samplerTable =
                    m_samplerHeap->allocate(descSetInfo.samplerDescriptorCount);
            }

            m_boundResources.setCount(layout->getResourceCount());

            // If the layout specifies that we have any sub-objects, then
            // we need to size the array to account for them.
            //
            Index subObjectCount = layout->getSubObjectCount();
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
                    m_objects[bindingRangeInfo.binding.index + i] = subObject;
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
            auto src = m_ordinaryData.getBuffer();
            auto srcSize = size_t(m_ordinaryData.getCount());

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
                size_t subObjectRangePendingDataOffset =
                    _getSubObjectRangePendingDataOffset(specializedLayout, subObjectRangeIndex);
                size_t subObjectRangePendingDataStride =
                    _getSubObjectRangePendingDataStride(specializedLayout, subObjectRangeIndex);

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
                    auto subObject = m_objects[bindingRangeInfo.binding.index + i];

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

        // As discussed in `_writeOrdinaryData()`, these methods are just stubs waiting for
        // the "flat" Slang refelction information to provide access to the relevant data.
        //
        size_t _getSubObjectRangePendingDataOffset(
            ShaderObjectLayoutImpl* specializedLayout,
            Index subObjectRangeIndex)
        {
            return 0;
        }
        size_t _getSubObjectRangePendingDataStride(
            ShaderObjectLayoutImpl* specializedLayout,
            Index subObjectRangeIndex)
        {
            return 0;
        }

        /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
        Result _ensureOrdinaryDataBufferCreatedIfNeeded(PipelineCommandEncoder* encoder)
        {
            // If we have already created a buffer to hold ordinary data, then we should
            // simply re-use that buffer rather than re-create it.
            //
            // TODO: Simply re-using the buffer without any kind of validation checks
            // means that we are assuming that users cannot or will not perform any `set`
            // operations on a shader object once an operation has requested this buffer
            // be created. We need to enforce that rule if we want to rely on it.
            //
            if (m_ordinaryDataBuffer)
                return SLANG_OK;

            // Computing the size of the ordinary data buffer is *not* just as simple
            // as using the size of the `m_ordinayData` array that we store. The reason
            // for the added complexity is that interface-type fields may lead to the
            // storage being specialized such that it needs extra appended data to
            // store the concrete values that logically belong in those interface-type
            // fields but wouldn't fit in the fixed-size allocation we gave them.
            //
            // TODO: We need to actually implement that logic by using reflection
            // data computed for the specialized type of this shader object.
            // For now we just make the simple assumption described above despite
            // knowing that it is false.
            //
            RefPtr<ShaderObjectLayoutImpl> specializedLayout;
            SLANG_RETURN_ON_FAIL(getSpecializedLayout(specializedLayout.writeRef()));

            auto specializedOrdinaryDataSize = specializedLayout->getElementTypeLayout()->getSize();
            if (specializedOrdinaryDataSize == 0)
                return SLANG_OK;

            // Once we have computed how large the buffer should be, we can allocate
            // it using the existing public `IDevice` API.
            //

            ComPtr<IBufferResource> bufferResourcePtr;
            IBufferResource::Desc bufferDesc;
            bufferDesc.init(specializedOrdinaryDataSize);
            bufferDesc.cpuAccessFlags |= IResource::AccessFlag::Write;
            SLANG_RETURN_ON_FAIL(encoder->m_renderer->createBufferResource(
                IResource::Usage::ConstantBuffer,
                bufferDesc,
                nullptr,
                bufferResourcePtr.writeRef()));
            m_ordinaryDataBuffer = static_cast<BufferResourceImpl*>(bufferResourcePtr.get());

            // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
            //
            // Note that `_writeOrdinaryData` is potentially recursive in the case
            // where this object contains interface/existential-type fields, so we
            // don't need or want to inline it into this call site.
            //
            SLANG_RETURN_ON_FAIL(_writeOrdinaryData(
                encoder, m_ordinaryDataBuffer, 0, specializedOrdinaryDataSize, specializedLayout));

            return SLANG_OK;
        }

        /// Bind the buffer for ordinary/uniform data, if needed
        Result _bindOrdinaryDataBufferIfNeeded(PipelineCommandEncoder* encoder)
        {
            // We start by ensuring that the buffer is created, if it is needed.
            //
            SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(encoder));

            // If we did indeed need/create a buffer, then we must bind it
            // into root binding state.
            //
            if (m_ordinaryDataBuffer)
            {
                auto descriptorTable = m_descriptorSet.m_resourceTable;
                D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
                viewDesc.BufferLocation =
                    m_ordinaryDataBuffer->m_resource.getResource()->GetGPUVirtualAddress();
                viewDesc.SizeInBytes =
                    (UINT)D3DUtil::calcAligned((UInt)m_ordinaryData.getCount(), 256);
                encoder->m_device->CreateConstantBufferView(
                    &viewDesc,
                    m_resourceHeap->getCpuHandle(descriptorTable));
            }

            return SLANG_OK;
        }

    public:
        virtual Result bindObject(PipelineCommandEncoder* encoder, RootBindingState* bindingState)
        {
            ShaderObjectLayoutImpl* layout = getLayout();
            SLANG_RETURN_ON_FAIL(_bindOrdinaryDataBufferIfNeeded(encoder));
            uint32_t descTableIndex = bindingState->rootParamIndex;
            auto& descSet = m_descriptorSet;
            if (descSet.m_resourceCount)
            {
                auto gpuDescriptorTable = bindingState->descriptorTables[descTableIndex];
                auto& gpuHeap = gpuDescriptorTable.heap;
                auto& cpuHeap = *m_resourceHeap;
                auto cpuDescriptorTable = descSet.m_resourceTable;

                bindingState->device->m_device->CopyDescriptorsSimple(
                    UINT(descSet.m_resourceCount),
                    gpuHeap.getCpuHandle(gpuDescriptorTable.table + bindingState->offset.resource),
                    cpuHeap.getCpuHandle(cpuDescriptorTable),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                bindingState->offset.resource += descSet.m_resourceCount;
                descTableIndex++;
            }
            if (descSet.m_samplerCount)
            {
                auto gpuDescriptorTable = bindingState->descriptorTables[descTableIndex];
                auto& gpuHeap = gpuDescriptorTable.heap;
                auto& cpuHeap = *m_samplerHeap;
                auto cpuDescriptorTable = (int)descSet.m_samplerTable;

                bindingState->device->m_device->CopyDescriptorsSimple(
                    UINT(descSet.m_samplerCount),
                    gpuHeap.getCpuHandle(gpuDescriptorTable.table + bindingState->offset.sampler),
                    cpuHeap.getCpuHandle(cpuDescriptorTable),
                    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
                bindingState->offset.sampler += descSet.m_samplerCount;
                descTableIndex++;
            }
            bindingState->futureRootParamOffset =
                Math::Max(descTableIndex, bindingState->futureRootParamOffset);
            auto& subObjectRanges = layout->getSubObjectRanges();
            for (Index i = 0; i < subObjectRanges.getCount(); i++)
            {
                auto bindingRange =
                    layout->getBindingRange(layout->getSubObjectRange(i).bindingRangeIndex);
                switch (layout->getSubObjectRange(i).bindingType)
                {
                case slang::BindingType::ParameterBlock:
                    {
                        auto baseIndex = bindingRange.binding.index;
                        for (uint32_t j = 0; j < bindingRange.count; j++)
                        {
                            auto newBindingState = *bindingState;
                            newBindingState.offset.resource = 0;
                            newBindingState.offset.sampler = 0;
                            newBindingState.rootParamIndex = bindingState->futureRootParamOffset;
                            newBindingState.futureRootParamOffset = newBindingState.rootParamIndex;
                            m_objects[baseIndex + j]->bindObject(encoder, &newBindingState);
                            bindingState->futureRootParamOffset =
                                newBindingState.futureRootParamOffset;
                        }
                    }
                    break;
                case slang::BindingType::ConstantBuffer:
                    {
                        auto baseIndex = bindingRange.binding.index;
                        for (uint32_t j = 0; j < bindingRange.count; j++)
                        {
                            m_objects[baseIndex + j]->bindObject(encoder, bindingState);
                        }
                    }
                    break;
                case slang::BindingType::ExistentialValue:
                    // If the existential object contains only ordinary data fields,
                    // the data is already written into m_ordinaryDataBuffer during `setObject`,
                    // so we don't need to do anything here.
                    // If the existential object has resource fields, this is the time to set
                    // those fields as in the "pendingLayout" section.
                    // TODO: implement resource fields binding for inline existential values.
                default:
                    break;
                }
            }
            return SLANG_OK;
        }

        /// Any "ordinary" / uniform data for this object
        List<char> m_ordinaryData;

        List<RefPtr<ShaderObjectImpl>> m_objects;

        D3D12HostVisibleDescriptorAllocator* m_resourceHeap = nullptr;
        D3D12HostVisibleDescriptorAllocator* m_samplerHeap = nullptr;

        struct DescriptorSet
        {
            int32_t m_resourceTable = 0;
            int32_t m_samplerTable = 0;
            uint32_t m_resourceCount = 0;
            uint32_t m_samplerCount = 0;
        };
        DescriptorSet m_descriptorSet;

        ShortList<RefPtr<Resource>, 8> m_boundResources;

        /// A constant buffer used to stored ordinary data for this object
        /// and existential-type sub-objects.
        ///
        /// Created on demand with `_createOrdinaryDataBufferIfNeeded()`
        RefPtr<BufferResourceImpl> m_ordinaryDataBuffer;

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
            *outLayout = RefPtr<ShaderObjectLayoutImpl>(m_specializedLayout).detach();
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
            RefPtr<ShaderObjectLayoutBase> layout;
            SLANG_RETURN_ON_FAIL(
                renderer->getShaderObjectLayout(extendedType.slangType, layout.writeRef()));

            *outLayout = static_cast<ShaderObjectLayoutImpl*>(layout.detach());
            return SLANG_OK;
        }

        RefPtr<ShaderObjectLayoutImpl> m_specializedLayout;
    };

    class RootShaderObjectImpl : public ShaderObjectImpl
    {
        typedef ShaderObjectImpl Super;

    public:
        static Result create(
            D3D12Device* device,
            RootShaderObjectLayoutImpl* layout,
            RootShaderObjectImpl** outShaderObject)
        {
            RefPtr<RootShaderObjectImpl> object = new RootShaderObjectImpl();
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            *outShaderObject = object.detach();
            return SLANG_OK;
        }

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
            *outEntryPoint = m_entryPoints[index];
            m_entryPoints[index]->addRef();
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
        virtual Result bindObject(PipelineCommandEncoder* encoder, RootBindingState* bindingState) override
        {
            SLANG_RETURN_ON_FAIL(Super::bindObject(encoder, bindingState));

            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPoint = m_entryPoints[i];
                SLANG_RETURN_ON_FAIL(entryPoint->bindObject(encoder, bindingState));
            }

            return SLANG_OK;
        }
    protected:

        Result init(D3D12Device* device, RootShaderObjectLayoutImpl* layout)
        {
            SLANG_RETURN_ON_FAIL(Super::init(device, layout));

            for (auto entryPointInfo : layout->getEntryPoints())
            {
                RefPtr<ShaderObjectImpl> entryPoint;
                SLANG_RETURN_ON_FAIL(
                    ShaderObjectImpl::create(device, entryPointInfo.layout, entryPoint.writeRef()));
                m_entryPoints.add(entryPoint);
            }

            return SLANG_OK;
        }

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

            *outLayout = specializedLayout.detach();
            return SLANG_OK;
        }

        List<RefPtr<ShaderObjectImpl>> m_entryPoints;
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

            ID3D12DescriptorHeap* heaps[] = {
                m_frame->m_viewHeap.getHeap(),
                m_frame->m_samplerHeap.getHeap(),
            };
            m_cmdList->SetDescriptorHeaps(SLANG_COUNT_OF(heaps), heaps);
        }

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
                PipelineCommandEncoder::init(cmdBuffer);
                m_preCmdList = nullptr;
                m_device = renderer->m_device;
                m_renderPass = renderPass;
                m_framebuffer = framebuffer;
                m_frame = frame;
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
                bindRootShaderObjectImpl(object);
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
                    _bindRenderState(&submitter);
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
                PipelineCommandEncoder::init(cmdBuffer);
                m_preCmdList = nullptr;
                m_device = renderer->m_device;
                m_frame = frame;
                m_currentPipeline = nullptr;
            }

            virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) override
            {
                setPipelineStateImpl(state);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                bindRootShaderObject(IShaderObject* object) override
            {
                bindRootShaderObjectImpl(object);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
            {
                // Submit binding for compute
                {
                    ComputeSubmitter submitter(m_d3dCmdList);
                    _bindRenderState(&submitter);
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

    D3D12HostVisibleDescriptorAllocator m_rtvAllocator;
    D3D12HostVisibleDescriptorAllocator m_dsvAllocator;
    // Space in the GPU-visible heaps is precious, so we will also keep
    // around CPU-visible heaps for storing descriptors in a format
    // that is ready for copying into the GPU-visible heaps as needed.
    //
    D3D12HostVisibleDescriptorAllocator m_cpuViewHeap; ///< Cbv, Srv, Uav
    D3D12HostVisibleDescriptorAllocator m_cpuSamplerHeap; ///< Heap for samplers

    // Dll entry points
    PFN_D3D12_GET_DEBUG_INTERFACE m_D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE m_D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE m_D3D12SerializeRootSignature = nullptr;

    bool m_nvapi = false;
};


Result D3D12Device::PipelineCommandEncoder::_bindRenderState(Submitter* submitter)
{
    RefPtr<PipelineStateBase> newPipeline;
    m_renderer->maybeSpecializePipeline(
        m_currentPipeline, m_rootShaderObject, newPipeline);
    RootShaderObjectImpl* rootObjectImpl =
        static_cast<RootShaderObjectImpl*>(m_rootShaderObject.Ptr());
    PipelineStateImpl* newPipelineImpl = static_cast<PipelineStateImpl*>(newPipeline.Ptr());
    auto commandList = m_d3dCmdList;
    auto pipelineTypeIndex = (int)newPipelineImpl->desc.type;
    auto programImpl = static_cast<ShaderProgramImpl*>(newPipelineImpl->m_program.get());
    commandList->SetPipelineState(newPipelineImpl->m_pipelineState);
    submitter->setRootSignature(programImpl->m_rootObjectLayout->m_rootSignature);
    ShortList<DescriptorTable, kMaxDescriptorSetCount> descriptorTables;
    RefPtr<ShaderObjectLayoutImpl> specializedRootLayout;
    rootObjectImpl->getSpecializedLayout(specializedRootLayout.writeRef());
    RootShaderObjectLayoutImpl* rootLayoutImpl =
        static_cast<RootShaderObjectLayoutImpl*>(specializedRootLayout.Ptr());
    for (auto& descSet : rootLayoutImpl->m_gpuDescriptorSetInfos)
    {
        if (descSet.resourceDescriptorCount)
        {
            DescriptorTable table;
            table.heap = &m_frame->m_viewHeap;
            table.table = m_frame->m_viewHeap.allocate((int)descSet.resourceDescriptorCount);
            descriptorTables.add(table);
        }
        if (descSet.samplerDescriptorCount)
        {
            DescriptorTable table;
            table.heap = &m_frame->m_samplerHeap;
            table.table = m_frame->m_samplerHeap.allocate((int)descSet.samplerDescriptorCount);
            descriptorTables.add(table);
        }
    }
    RootBindingState bindState = {};
    bindState.device = m_renderer;
    bindState.frame = m_frame;
    auto descTablesView = descriptorTables.getArrayView();
    bindState.descriptorTables = descTablesView.arrayView;
    SLANG_RETURN_ON_FAIL(rootObjectImpl->bindObject(this, &bindState));
    
    for (Index i = 0; i < descriptorTables.getCount(); i++)
    {
        submitter->setRootDescriptorTable(
            (int)i, descriptorTables[i].heap.getGpuHandle(descriptorTables[i].table));
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

    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

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

    SLANG_RETURN_ON_FAIL(m_cpuViewHeap.init   (m_device, 8192, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
    SLANG_RETURN_ON_FAIL(m_cpuSamplerHeap.init(m_device, 1024,   D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

    SLANG_RETURN_ON_FAIL(m_rtvAllocator.init    (m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
    SLANG_RETURN_ON_FAIL(m_dsvAllocator.init    (m_device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

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
        case Usage::CopySource:             return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case Usage::CopyDest:               return D3D12_RESOURCE_STATE_COPY_DEST;
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

            SLANG_RETURN_ON_FAIL(m_cpuViewHeap.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_cpuViewHeap;
            m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            SLANG_RETURN_ON_FAIL(m_cpuViewHeap.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_cpuViewHeap;

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

            SLANG_RETURN_ON_FAIL(m_cpuViewHeap.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_cpuViewHeap;
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

            SLANG_RETURN_ON_FAIL(m_cpuViewHeap.allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = &m_cpuViewHeap;
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
        *outProgram = shaderProgram.detach();
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
            // TODO: report compile error.
        }
        SLANG_RETURN_ON_FAIL(compileResult);
        List<uint8_t>* shaderCodeDestBuffer = nullptr;
        switch (stage)
        {
        case SLANG_STAGE_COMPUTE:
            shaderCodeDestBuffer = &shaderProgram->m_computeShader;
            break;
        case SLANG_STAGE_VERTEX:
            shaderCodeDestBuffer = &shaderProgram->m_vertexShader;
            break;
        case SLANG_STAGE_FRAGMENT:
            shaderCodeDestBuffer = &shaderProgram->m_pixelShader;
            break;
        default:
            SLANG_ASSERT(!"unsupported shader stage.");
            return SLANG_FAIL;
        }
        shaderCodeDestBuffer->addRange(
            reinterpret_cast<const uint8_t*>(kernelCode->getBufferPointer()),
            (Index)kernelCode->getBufferSize());
    }
    *outProgram = shaderProgram.detach();
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
    *outLayout = layout.detach();
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
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result SLANG_MCALL
    D3D12Device::createRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    auto programImpl = dynamic_cast<ShaderProgramImpl*>(program);
    RefPtr<RootShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(RootShaderObjectImpl::create(
        this, programImpl->m_rootObjectLayout, shaderObject.writeRef()));
    *outObject = shaderObject.detach();
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
        *outState = pipelineStateImpl.detach();
        return SLANG_OK;
    }

    // Only actually create a D3D12 pipeline state if the pipeline is fully specialized.
    auto inputLayoutImpl = (InputLayoutImpl*) desc.inputLayout;

    // Describe and create the graphics pipeline state object (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.pRootSignature = programImpl->m_rootObjectLayout->m_rootSignature;

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

    auto programImpl = (ShaderProgramImpl*) desc.program;
    if (!programImpl->m_rootObjectLayout->m_rootSignature)
    {
        RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
        pipelineStateImpl->init(desc);
        *outState = pipelineStateImpl.detach();
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
