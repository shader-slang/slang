// render-d3d12.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "render-d3d12.h"

//WORKING:#include "options.h"
#include "../renderer-shared.h"
#include "../render-graphics-common.h"

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

class D3D12Renderer : public GraphicsAPIRenderer
{
public:
    // Renderer    implementation
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc, void* inWindowHandle) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setClearColor(const float color[4]) override;
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame() override;
    virtual SLANG_NO_THROW void SLANG_MCALL presentFrame() override;
    virtual SLANG_NO_THROW ITextureResource::Desc SLANG_MCALL getSwapChainTextureDesc() override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        IResource::Usage initialUsage,
        const ITextureResource::Desc& desc,
        const ITextureResource::Data* initData,
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

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createPipelineLayout(
        const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSet(
        IDescriptorSetLayout* layout, IDescriptorSet** outDescriptorSet) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL captureScreenSurface(
        void* buffer, size_t* inOutBufferSize, size_t* outRowPitch, size_t* outPixelSize) override;

    virtual SLANG_NO_THROW void* SLANG_MCALL
        map(IBufferResource* buffer, MapFlavor flavor) override;
    virtual SLANG_NO_THROW void SLANG_MCALL unmap(IBufferResource* buffer) override;
    //    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setDescriptorSet(
        PipelineType pipelineType,
        IPipelineLayout* layout,
        UInt index,
        IDescriptorSet* descriptorSet) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual void SLANG_MCALL setDepthStencilTarget(IResourceView* depthStencilView) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(UInt count, Viewport const* viewports) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setPipelineState(PipelineType pipelineType, IPipelineState* state) override;
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;
    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() override;
    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() override;
    virtual SLANG_NO_THROW RendererType SLANG_MCALL getRendererType() const override
    {
        return RendererType::DirectX12;
    }

    ~D3D12Renderer();

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
    };

    class ShaderProgramImpl : public IShaderProgram, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IShaderProgram* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IShaderProgram)
                return static_cast<IShaderProgram*>(this);
            return nullptr;
        }
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
                    const size_t bufferSize = m_memory.getCount();
                    D3D12CircularResourceHeap::Cursor cursor = circularHeap.allocateConstantBuffer(bufferSize);
                    ::memcpy(cursor.m_position, m_memory.getBuffer(), bufferSize);
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

        BufferResourceImpl(IResource::Usage initialUsage, const Desc& desc):
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
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
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

            /// Backing storage for root constant ranges in this descriptor set.
        List<char>                      m_rootConstantData;
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

    class PipelineStateImpl : public IPipelineState, public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IPipelineState* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IPipelineState)
                return static_cast<IPipelineState*>(this);
            return nullptr;
        }
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

    static PROC loadProc(HMODULE module, char const* name);
    Result createFrameResources();
        /// Blocks until gpu has completed all work
    void releaseFrameResources();

    Result createBuffer(const D3D12_RESOURCE_DESC& resourceDesc, const void* srcData, size_t srcDataSize, D3D12Resource& uploadResource, D3D12_RESOURCE_STATES finalState, D3D12Resource& resourceOut);

    void beginRender();

    void endRender();

    void submitGpuWorkAndWait();
    void _resetCommandList();

    Result captureTextureToSurface(
        D3D12Resource& resource,
        void* buffer,
        size_t* inOutBufferSize,
        size_t* outRowPitch,
        size_t* outPixelSize);

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

    Result _createDevice(DeviceCheckFlags deviceCheckFlags, const UnownedStringSlice& nameMatch, D3D_FEATURE_LEVEL featureLevel, DeviceInfo& outDeviceInfo);
    
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

    DeviceInfo m_deviceInfo;
    ID3D12Device* m_device = nullptr;

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

    // Dll entry points
    PFN_D3D12_GET_DEBUG_INTERFACE m_D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE m_D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE m_D3D12SerializeRootSignature = nullptr;

    HWND m_hwnd = nullptr;

    List<String> m_features;

    bool m_nvapi = false;
};

SlangResult SLANG_MCALL createD3D12Renderer(IRenderer** outRenderer)
{
    *outRenderer = new D3D12Renderer();
    (*outRenderer)->addRef();
    return SLANG_OK;
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

Result D3D12Renderer::createBuffer(const D3D12_RESOURCE_DESC& resourceDesc, const void* srcData, size_t srcDataSize, D3D12Resource& uploadResource, D3D12_RESOURCE_STATES finalState, D3D12Resource& resourceOut)
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

Result D3D12Renderer::captureTextureToSurface(
    D3D12Resource& resource,
    void* buffer,
    size_t* inOutBufferSize,
    size_t* outRowPitch,
    size_t* outPixelSize)
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
    if (outRowPitch)
        *outRowPitch = rowPitch;
    if (outPixelSize)
        *outPixelSize = bytesPerPixel;
    if (!buffer || *inOutBufferSize == 0)
    {
        *inOutBufferSize = bufferSize;
        return SLANG_OK;
    }
    if (*inOutBufferSize < bufferSize)
        return SLANG_ERROR_INSUFFICIENT_BUFFER;
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

        memcpy(buffer, data, bufferSize);

        dxResource->Unmap(0, nullptr);
        return SLANG_OK;
    }
}

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
        if(auto rootConstantRangeCount = descriptorSetLayout->m_rootConstantRanges.getCount())
        {
            auto srcData = descriptorSet->m_rootConstantData.getBuffer();

            for(auto& rootConstantRangeInfo : descriptorSetLayout->m_rootConstantRanges)
            {
                auto countOf32bitValues = rootConstantRangeInfo.size / sizeof(uint32_t);
                submitter->setRootConstants(rootConstantRangeInfo.rootParamIndex, 0, countOf32bitValues, srcData + rootConstantRangeInfo.offset);
            }
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

Result D3D12Renderer::_createDevice(DeviceCheckFlags deviceCheckFlags, const UnownedStringSlice& nameMatch, D3D_FEATURE_LEVEL featureLevel, DeviceInfo& outDeviceInfo)
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
    SLANG_RETURN_ON_FAIL(m_deviceInfo.m_dxgiFactory->CreateSwapChain(m_commandQueue, &swapChainDesc, swapChain.writeRef()));
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
    SLANG_RETURN_ON_FAIL(m_deviceInfo.m_dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

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
    desc.init2D(IResource::Type::Texture2D, Format::Unknown, m_desc.width, m_desc.height, 1);

    return desc;
}

SlangResult D3D12Renderer::captureScreenSurface(
    void* buffer, size_t* inOutBufferSize, size_t* outRowPitch, size_t* outPixelSize)
{
    return captureTextureToSurface(*m_renderTargets[m_renderTargetIndex], buffer, inOutBufferSize, outRowPitch, outPixelSize);
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

Result D3D12Renderer::createTextureResource(IResource::Usage initialUsage, const ITextureResource::Desc& descIn, const ITextureResource::Data* initData, ITextureResource** outResource)
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
    layouts.setCount(numMipMaps);
    List<UInt64> mipRowSizeInBytes;
    mipRowSizeInBytes.setCount(numMipMaps);
    List<UInt32> mipNumRows;
    mipNumRows.setCount(numMipMaps);

    // Since textures are effectively immutable currently initData must be set
    assert(initData);
    // We should have this many sub resources
    assert(initData->numSubResources == numMipMaps * srcDesc.size.depth * arraySize);

    // NOTE! This is just the size for one array upload -> not for the whole texture
    UInt64 requiredSize = 0;
    m_device->GetCopyableFootprints(&resourceDesc, 0, numMipMaps, 0, layouts.begin(), mipNumRows.begin(), mipRowSizeInBytes.begin(), &requiredSize);

    // Sub resource indexing
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx#subresource_indexing
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

                //assert(srcRow == (const uint8_t*)(srcMip.getBuffer() + srcMip.getCount()));
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

Result D3D12Renderer::createBufferResource(IResource::Usage initialUsage, const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
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
            buffer->m_memory.setCount(UInt(alignedSizeInBytes));
            // Initialize
            if (initData)
            {
                ::memcpy(buffer->m_memory.getBuffer(), initData, srcDesc.sizeInBytes);
            }
            break;
        }
        case Style::ResourceBacked:
        {
            const D3D12_RESOURCE_STATES initialState = _calcResourceState(initialUsage);
            SLANG_RETURN_ON_FAIL(createBuffer(bufferDesc, initData, srcDesc.sizeInBytes, buffer->m_uploadResource, initialState, buffer->m_resource));
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

Result D3D12Renderer::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
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

Result D3D12Renderer::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
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
            m_device->CreateRenderTargetView(resourceImpl->m_resource, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::DepthStencil:
        {
            SLANG_RETURN_ON_FAIL(m_dsvAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateDepthStencilView(resourceImpl->m_resource, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::UnorderedAccess:
        {
            // TODO: need to support the separate "counter resource" for the case
            // of append/consume buffers with attached counters.

            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));
            m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, nullptr, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            SLANG_RETURN_ON_FAIL(m_viewAllocator.allocate(&viewImpl->m_descriptor));

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

Result D3D12Renderer::createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
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
            m_device->CreateShaderResourceView(resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;
    }

    *outView = viewImpl.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount, IInputLayout** outLayout)
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

void* D3D12Renderer::map(IBufferResource* bufferIn, MapFlavor flavor)
{
    typedef BufferResourceImpl::BackingStyle Style;

    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    buffer->m_mapFlavor = flavor;

    const size_t bufferSize = buffer->getDesc()->sizeInBytes;

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
                        buffer->m_memory.setCount(bufferSize);
                        ::memcpy(buffer->m_memory.getBuffer(), data, bufferSize);

                        stageBuf.getResource()->Unmap(0, nullptr);
                    }

                    return buffer->m_memory.getBuffer();
                }
            }
            break;
        }
        case Style::MemoryBacked:
        {
            return buffer->m_memory.getBuffer();
        }
        default: return nullptr;
    }

    return nullptr;
}

void D3D12Renderer::unmap(IBufferResource* bufferIn)
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

                    m_commandList->CopyBufferRegion(resource, 0, uploadResource, 0, buffer->getDesc()->sizeInBytes);

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

void D3D12Renderer::setVertexBuffers(UInt startSlot, UInt slotCount, IBufferResource*const* buffers, const UInt* strides, const UInt* offsets)
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

void D3D12Renderer::setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
{
    m_boundIndexBuffer = (BufferResourceImpl*) buffer;
    m_boundIndexFormat = D3DUtil::getMapFormat(indexFormat);
    m_boundIndexOffset = UINT(offset);
}

void D3D12Renderer::setDepthStencilTarget(IResourceView* depthStencilView)
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

void D3D12Renderer::setPipelineState(PipelineType pipelineType, IPipelineState* state)
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
        for (Index i = 0; i < m_boundVertexBuffers.getCount(); i++)
        {
            const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[i];
            BufferResourceImpl* buffer = boundVertexBuffer.m_buffer;
            if (buffer)
            {
                D3D12_VERTEX_BUFFER_VIEW& vertexView = vertexViews[numVertexViews++];
                vertexView.BufferLocation = buffer->m_resource.getResource()->GetGPUVirtualAddress()
                    + boundVertexBuffer.m_offset;
                vertexView.SizeInBytes = UINT(buffer->getDesc()->sizeInBytes - boundVertexBuffer.m_offset);
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
        indexBufferView.SizeInBytes = UINT(m_boundIndexBuffer->getDesc()->sizeInBytes - m_boundIndexOffset);
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

void D3D12Renderer::DescriptorSetImpl::setConstantBuffer(UInt range, UInt index, IBufferResource* buffer)
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

void D3D12Renderer::DescriptorSetImpl::setResource(UInt range, UInt index, IResourceView* view)
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

void D3D12Renderer::DescriptorSetImpl::setSampler(UInt range, UInt index, ISamplerState* sampler)
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
        samplerImpl->m_cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void D3D12Renderer::DescriptorSetImpl::setRootConstants(
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

void D3D12Renderer::setDescriptorSet(PipelineType pipelineType, IPipelineLayout* layout, UInt index, IDescriptorSet* descriptorSet)
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

Result D3D12Renderer::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
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

    *outProgram = program.detach();
    return SLANG_OK;
}

Result D3D12Renderer::createDescriptorSetLayout(const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout)
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

Result D3D12Renderer::createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout)
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

Result D3D12Renderer::createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet** outDescriptorSet)
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

Result D3D12Renderer::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
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

Result D3D12Renderer::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto programImpl = (ShaderProgramImpl*) desc.program;

    // Describe and create the compute pipeline state object
    D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
    computeDesc.pRootSignature = pipelineLayoutImpl->m_rootSignature;
    computeDesc.CS = { programImpl->m_computeShader.getBuffer(), SIZE_T(programImpl->m_computeShader.getCount()) };

    ComPtr<ID3D12PipelineState> pipelineState;

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

        // Put the pointer to the extension into an array - there can be multiple extensions enabled at once.
        const NVAPI_D3D12_PSO_EXTENSION_DESC* extensions[] = { &extensionDesc };

        // Now create the PSO.
        const NvAPI_Status nvapiStatus = NvAPI_D3D12_CreateComputePipelineState(m_device, &computeDesc, SLANG_COUNT_OF(extensions), extensions, pipelineState.writeRef());

        if (nvapiStatus != NVAPI_OK)
        {
            return SLANG_FAIL;
        }
    }
    else
#endif
    {
        SLANG_RETURN_ON_FAIL(m_device->CreateComputePipelineState(&computeDesc, IID_PPV_ARGS(pipelineState.writeRef())));
    }

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl();
    pipelineStateImpl->m_pipelineType = PipelineType::Compute;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_pipelineState = pipelineState;
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

} // renderer_test
