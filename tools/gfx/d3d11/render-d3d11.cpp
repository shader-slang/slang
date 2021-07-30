// render-d3d11.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "render-d3d11.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"

//WORKING: #include "options.h"
#include "../immediate-renderer-base.h"
#include "../d3d/d3d-util.h"
#include "../d3d/d3d-swapchain.h"
#include "../nvapi/nvapi-util.h"

// In order to use the Slang API, we need to include its header

//#include <slang.h>

#include "slang-com-ptr.h"
#include "../flag-combiner.h"

// We will be rendering with Direct3D 11, so we need to include
// the Windows and D3D11 headers

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <d3d11_2.h>
#include <d3dcompiler.h>

#ifdef GFX_NVAPI
// NVAPI integration is desribed here
// https://developer.nvidia.com/unlocking-gpu-intrinsics-hlsl

#   include "../nvapi/nvapi-include.h"
#endif

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

namespace gfx {

class D3D11Device : public ImmediateRendererBase
{
public:
    enum
    {
        kMaxUAVs = 64,
        kMaxRTVs = 8,
    };

    ~D3D11Device() {}

    // Renderer    implementation
    virtual SLANG_NO_THROW Result SLANG_MCALL initialize(const Desc& desc) override;
    virtual void clearFrame(uint32_t colorBufferMask, bool clearDepth, bool clearStencil) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer) override;
    virtual void setFramebuffer(IFramebuffer* frameBuffer) override;
    virtual void setStencilReference(uint32_t referenceValue) override;

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
        IBufferResource* buffer,
        IResourceView::Desc const& desc,
        IResourceView** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc, IQueryPool** outPool) override;

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout,
        ShaderObjectLayoutBase** outLayout) override;
    virtual Result createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
        override;
    virtual Result createRootShaderObject(IShaderProgram* program, ShaderObjectBase** outObject)
        override;
    virtual void bindRootShaderObject(IShaderObject* shaderObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

    virtual void* map(IBufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(IBufferResource* buffer, size_t offsetWritten, size_t sizeWritten) override;
    virtual void copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* texture, ResourceState state, ISlangBlob** outBlob, size_t* outRowPitch, size_t* outPixelSize) override;

    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual void setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override;
    virtual void setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual void setViewports(UInt count, Viewport const* viewports) override;
    virtual void setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual void setPipelineState(IPipelineState* state) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override {}
    virtual void waitForGpu() override
    {

    }
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
    }
    virtual void beginCommandBuffer(const CommandBufferInfo& info) override
    {
        if (info.hasWriteTimestamps)
        {
            m_immediateContext->Begin(m_disjointQuery);
        }
    }
    virtual void endCommandBuffer(const CommandBufferInfo& info) override
    {
        if (info.hasWriteTimestamps)
        {
            m_immediateContext->End(m_disjointQuery);
        }
    }
    virtual void writeTimestamp(IQueryPool* pool, SlangInt index) override
    {
        auto poolImpl = static_cast<QueryPoolImpl*>(pool);
        m_immediateContext->End(poolImpl->getQuery(index));
    }

protected:

    class ScopeNVAPI
    {
    public:
        ScopeNVAPI() : m_renderer(nullptr) {}
        SlangResult init(D3D11Device* renderer, Index regIndex);
        ~ScopeNVAPI();

    protected:
        D3D11Device* m_renderer;
    };

    class ShaderProgramImpl : public ShaderProgramBase
    {
    public:
        ComPtr<ID3D11VertexShader> m_vertexShader;
        ComPtr<ID3D11PixelShader> m_pixelShader;
        ComPtr<ID3D11ComputeShader> m_computeShader;
    };

    class BufferResourceImpl: public BufferResource
    {
	public:
        typedef BufferResource Parent;

        BufferResourceImpl(const IBufferResource::Desc& desc):
            Parent(desc)
        {
        }

        MapFlavor m_mapFlavor;
        D3D11_USAGE m_d3dUsage;
        ComPtr<ID3D11Buffer> m_buffer;
        ComPtr<ID3D11Buffer> m_staging;
        List<uint8_t> m_uploadStagingBuffer;

        virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
        {
            return 0;
        }
    };
    class TextureResourceImpl : public TextureResource
    {
    public:
        typedef TextureResource Parent;

        TextureResourceImpl(const Desc& desc)
            : Parent(desc)
        {
        }
        ComPtr<ID3D11Resource> m_resource;

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
        ComPtr<ID3D11SamplerState> m_sampler;
    };


    class ResourceViewImpl : public ResourceViewBase
    {
    public:
        enum class Type
        {
            SRV,
            UAV,
            DSV,
            RTV,
        };
        Type m_type;
    };

    class ShaderResourceViewImpl : public ResourceViewImpl
    {
    public:
        ComPtr<ID3D11ShaderResourceView>    m_srv;
    };

    class UnorderedAccessViewImpl : public ResourceViewImpl
    {
    public:
        ComPtr<ID3D11UnorderedAccessView>   m_uav;
    };

    class DepthStencilViewImpl : public ResourceViewImpl
    {
    public:
        ComPtr<ID3D11DepthStencilView>      m_dsv;
        DepthStencilClearValue m_clearValue;
    };

    class RenderTargetViewImpl : public ResourceViewImpl
    {
    public:
        ComPtr<ID3D11RenderTargetView>      m_rtv;
        float m_clearValue[4];
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
        ShortList<RefPtr<RenderTargetViewImpl>, kMaxRTVs> renderTargetViews;
        ShortList<ID3D11RenderTargetView*, kMaxRTVs> d3dRenderTargetViews;
        RefPtr<DepthStencilViewImpl> depthStencilView;
        ID3D11DepthStencilView* d3dDepthStencilView;
    };

    class SwapchainImpl : public D3DSwapchainBase
    {
    public:
        ComPtr<ID3D11Device> m_device;
        ComPtr<IDXGIFactory> m_dxgiFactory;
        RefPtr<D3D11Device> m_renderer;
        Result init(D3D11Device* renderer, const ISwapchain::Desc& swapchainDesc, WindowHandle window)
        {
            m_renderer = renderer;
            m_device = renderer->m_device;
            m_dxgiFactory = renderer->m_dxgiFactory;
            return D3DSwapchainBase::init(swapchainDesc, window, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL);
        }
        virtual void createSwapchainBufferImages() override
        {
            m_images.clear();
            // D3D11 implements automatic back buffer rotation, so the application
            // always render to buffer 0.
            ComPtr<ID3D11Resource> d3dResource;
            m_swapChain->GetBuffer(0, IID_PPV_ARGS(d3dResource.writeRef()));
            ITextureResource::Desc imageDesc = {};
            imageDesc.type = IResource::Type::Texture2D;
            imageDesc.arraySize = 0;
            imageDesc.numMipLevels = 1;
            imageDesc.size.width = m_desc.width;
            imageDesc.size.height = m_desc.height;
            imageDesc.size.depth = 1;
            imageDesc.format = m_desc.format;
            imageDesc.defaultState = ResourceState::Present;
            imageDesc.allowedStates = ResourceStateSet(
                ResourceState::Present,
                ResourceState::CopyDestination,
                ResourceState::RenderTarget);
            RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc);
            image->m_resource = d3dResource;
            for (uint32_t i = 0; i < m_desc.imageCount; i++)
            {
                m_images.add(image);
            }
        }
        virtual IDXGIFactory* getDXGIFactory() override { return m_dxgiFactory; }
        virtual IUnknown* getOwningDevice() override { return m_device; }
        virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) override
        {
            m_renderer->m_currentFramebuffer = nullptr;
            m_renderer->m_immediateContext->ClearState();
            return D3DSwapchainBase::resize(width, height);
        }
    };

    class InputLayoutImpl: public InputLayoutBase
	{
    public:
		ComPtr<ID3D11InputLayout> m_layout;
	};

    class QueryPoolImpl : public IQueryPool, public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL;
        IQueryPool* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IQueryPool)
                return static_cast<IQueryPool*>(this);
            return nullptr;
        }
    public:
        List<ComPtr<ID3D11Query>> m_queries;
        RefPtr<D3D11Device> m_device;
        D3D11_QUERY_DESC m_queryDesc;
        Result init(const IQueryPool::Desc& desc, D3D11Device* device)
        {
            m_device = device;
            m_queryDesc.MiscFlags = 0;
            switch (desc.type)
            {
            case QueryType::Timestamp:
                m_queryDesc.Query = D3D11_QUERY_TIMESTAMP;
                break;
            default:
                return SLANG_E_INVALID_ARG;
            }
            m_queries.setCount(desc.count);
            return SLANG_OK;
        }
        ID3D11Query* getQuery(SlangInt index)
        {
            if (!m_queries[index])
                m_device->m_device->CreateQuery(&m_queryDesc, m_queries[index].writeRef());
            return m_queries[index].get();
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL getResult(
            SlangInt queryIndex, SlangInt count, uint64_t* data) override
        {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
            while (S_OK != m_device->m_immediateContext->GetData(
                m_device->m_disjointQuery, &disjointData, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0))
            {
                Sleep(1);
            }
            m_device->m_info.timestampFrequency = disjointData.Frequency;

            for (SlangInt i = 0; i < count; i++)
            {
                SLANG_RETURN_ON_FAIL(m_device->m_immediateContext->GetData(
                    m_queries[queryIndex + i], data + i, sizeof(uint64_t), 0));
            }
            return SLANG_OK;
        }
    };

    class PipelineStateImpl : public PipelineStateBase
    {
    public:
    };


    class GraphicsPipelineStateImpl : public PipelineStateImpl
    {
    public:
        UINT                            m_rtvCount;

        RefPtr<InputLayoutImpl>         m_inputLayout;
        ComPtr<ID3D11DepthStencilState> m_depthStencilState;
        ComPtr<ID3D11RasterizerState>   m_rasterizerState;
        ComPtr<ID3D11BlendState>        m_blendState;

        float                           m_blendColor[4];
        UINT                            m_sampleMask;

        void init(const GraphicsPipelineStateDesc& inDesc)
        {
            PipelineStateBase::PipelineStateDesc pipelineDesc;
            pipelineDesc.graphics = inDesc;
            pipelineDesc.type = PipelineType::Graphics;
            initializeBase(pipelineDesc);
        }
    };

    class ComputePipelineStateImpl : public PipelineStateImpl
    {
    public:
        void init(const ComputePipelineStateDesc& inDesc)
        {
            PipelineStateBase::PipelineStateDesc pipelineDesc;
            pipelineDesc.compute = inDesc;
            pipelineDesc.type = PipelineType::Compute;
            initializeBase(pipelineDesc);
        }
    };

        /// Contextual data and operations required when binding shader objects to the pipeline state
    struct BindingContext
    {
        // One key service that the `BindingContext` provides is abstracting over
        // the difference between the D3D11 compute and graphics/rasteriation pipelines.
        // D3D11 has distinct operations for, e.g., `CSSetShaderResources`
        // for compute vs. `VSSetShaderResources` and `PSSetShaderResources`
        // for rasterization.
        //
        // The context type provides simple operations for setting each class
        // of resource/sampler, which will be overridden in derived types.
        //
        // TODO: These operations should really support binding multiple resources/samplers
        // in one call, so that we can eventually make more efficient use of the API.
        //
        // TODO: We could reasonably also just store the bound resources into
        // lcoal arrays like we are doing for UAVs, and remove the pipeline-specific
        // virtual functions. However, doing so would seemingly eliminate any
        // chance of avoiding redundant binding work when binding changes are
        // made for a root shader object.
        //
        virtual void setCBV(UINT index, ID3D11Buffer* buffer) = 0;
        virtual void setSRV(UINT index, ID3D11ShaderResourceView* srv) = 0;
        virtual void setSampler(UINT index, ID3D11SamplerState* sampler) = 0;

        // Unordered Access Views (UAVs) are a somewhat special case in that
        // the D3D11 API requires them to all be set at once, rather than one
        // at a time. To support this, we will keep a local array of the UAVs
        // that have been bound (up to the maximum supported by D3D 11.0)
        //
        void setUAV(UINT index, ID3D11UnorderedAccessView* uav)
        {
            uavs[index] = uav;

            // We will also track the total number of UAV slots that will
            // need to be bound (including any gaps that might occur due
            // to either explicit bindings or RTV bindings that conflict
            // with the `u` registers for fragment shaders).
            //
            if(uavCount <= index)
            {
                uavCount = index+1;
            }
        }

            /// The values bound for any UAVs
        ID3D11UnorderedAccessView* uavs[D3D11_PS_CS_UAV_REGISTER_COUNT];

            /// The number of entries in `uavs` that need to be considered when binding to the pipeline
        UINT uavCount = 0;

            /// The D3D11 device that we are using for binding
        D3D11Device* device = nullptr;

            /// The D3D11 device context that we are using for binding
        ID3D11DeviceContext* context = nullptr;

            /// Initialize a binding context for binding to the given `device` and `context`
        BindingContext(
            D3D11Device*            device,
            ID3D11DeviceContext*    context)
            : device(device)
            , context(context)
        {
            memset(uavs, 0, sizeof(uavs));
        }
    };

        /// A `BindingContext` for binding to the compute pipeline
    struct ComputeBindingContext : BindingContext
    {
            /// Initialize a binding context for binding to the given `device` and `context`
        ComputeBindingContext(
            D3D11Device*            device,
            ID3D11DeviceContext*    context)
            : BindingContext(device, context)
        {}

        void setCBV(UINT index, ID3D11Buffer* buffer) SLANG_OVERRIDE
        {
            context->CSSetConstantBuffers(index, 1, &buffer);
        }

        void setSRV(UINT index, ID3D11ShaderResourceView* srv) SLANG_OVERRIDE
        {
            context->CSSetShaderResources(index, 1, &srv);
        }

        void setSampler(UINT index, ID3D11SamplerState* sampler) SLANG_OVERRIDE
        {
            context->CSSetSamplers(index, 1, &sampler);
        }
    };

        /// A `BindingContext` for binding to the graphics/rasterization pipeline
    struct GraphicsBindingContext : BindingContext
    {
            /// Initialize a binding context for binding to the given `device` and `context`
        GraphicsBindingContext(
            D3D11Device*            device,
            ID3D11DeviceContext*    context)
            : BindingContext(device, context)
        {}

        // TODO: The operations here are only dealing with vertex and fragment
        // shaders for now. We should eventually extend them to handle HS/DS/GS
        // bindings. (We might want to skip those stages depending on whether
        // the associated program uses them at all).
        //
        // TODO: If we support cases where different stages might use distinct
        // entry-point parameters, we might need to support some modes where
        // a "stage mask" is passed in that applies to the bindings.
        //
        void setCBV(UINT index, ID3D11Buffer* buffer) SLANG_OVERRIDE
        {
            context->VSSetConstantBuffers(index, 1, &buffer);
            context->PSSetConstantBuffers(index, 1, &buffer);
        }

        void setSRV(UINT index, ID3D11ShaderResourceView* srv) SLANG_OVERRIDE
        {
            context->VSSetShaderResources(index, 1, &srv);
            context->PSSetShaderResources(index, 1, &srv);
        }

        void setSampler(UINT index, ID3D11SamplerState* sampler) SLANG_OVERRIDE
        {
            context->VSSetSamplers(index, 1, &sampler);
            context->PSSetSamplers(index, 1, &sampler);
        }
    };

    // In order to bind shader parameters to the correct locations, we need to
    // be able to describe those locations. Most shader parameters will
    // only consume a single type of D3D11-visible regsiter (e.g., a `t`
    // register for a txture, or an `s` register for a sampler), and scalar
    // integers suffice for these cases.
    //
    // In more complex cases we might be binding an entire "sub-object" like
    // a parameter block, an entry point, etc. For the general case, we need
    // to be able to represent a composite offset that includes offsets for
    // each of the register classes known to D3D11.

        /// A "simple" binding offset that records an offset in CBV/SRV/UAV/Sampler slots
    struct SimpleBindingOffset
    {
        uint32_t cbv    = 0;
        uint32_t srv    = 0;
        uint32_t uav    = 0;
        uint32_t sampler = 0;

            /// Create a default (zero) offset
        SimpleBindingOffset()
        {}

            /// Create an offset based on offset information in the given Slang `varLayout`
        SimpleBindingOffset(slang::VariableLayoutReflection* varLayout)
        {
            if(varLayout)
            {
                cbv     = (uint32_t) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
                srv     = (uint32_t) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE);
                uav     = (uint32_t) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS);
                sampler = (uint32_t) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_SAMPLER_STATE);
            }
        }

            /// Create an offset based on size/stride information in the given Slang `typeLayout`
        SimpleBindingOffset(slang::TypeLayoutReflection* typeLayout)
        {
            if(typeLayout)
            {
                cbv     = (uint32_t) typeLayout->getSize(SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER);
                srv     = (uint32_t) typeLayout->getSize(SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE);
                uav     = (uint32_t) typeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS);
                sampler = (uint32_t) typeLayout->getSize(SLANG_PARAMETER_CATEGORY_SAMPLER_STATE);
            }
        }

            /// Add any values in the given `offset`
        void operator+=(SimpleBindingOffset const& offset)
        {
            cbv += offset.cbv;
            srv += offset.srv;
            uav += offset.uav;
            sampler += offset.sampler;
        }
    };

    // While a "simple" binding offset representation will work in many cases,
    // once we need to deal with layout for programs with interface-type parameters
    // that have been statically specialized, we also need to track the offset
    // for where to bind any "pending" data that arises from the process of static
    // specialization.
    //
    // In order to conveniently track both the "primary" and "pending" offset information,
    // we will define a more complete `BindingOffset` type that combines simple
    // binding offsets for the primary and pending parts.

        /// A representation of the offset at which to bind a shader parameter or sub-object
    struct BindingOffset : SimpleBindingOffset
    {
        // Offsets for "primary" data are stored directly in the `BindingOffset`
        // via the inheritance from `SimpleBindingOffset`.

            /// Offset for any "pending" data
        SimpleBindingOffset pending;

            /// Create a default (zero) offset
        BindingOffset()
        {}

            /// Create an offset from a simple offset
        explicit BindingOffset(SimpleBindingOffset const& offset)
            : SimpleBindingOffset(offset)
        {}

            /// Create an offset based on offset information in the given Slang `varLayout`
        BindingOffset(slang::VariableLayoutReflection* varLayout)
            : SimpleBindingOffset(varLayout)
            , pending(varLayout->getPendingDataLayout())
        {}

            /// Create an offset based on size/stride information in the given Slang `typeLayout`
        BindingOffset(slang::TypeLayoutReflection* typeLayout)
            : SimpleBindingOffset(typeLayout)
            , pending(typeLayout->getPendingDataTypeLayout())
        {}

            /// Add any values in the given `offset`
        void operator+=(SimpleBindingOffset const& offset)
        {
            SimpleBindingOffset::operator+=(offset);
        }

            /// Add any values in the given `offset`
        void operator+=(BindingOffset const& offset)
        {
            SimpleBindingOffset::operator+=(offset);
            pending += offset.pending;
        }
    };

    class ShaderObjectLayoutImpl : public ShaderObjectLayoutBase
    {
    public:
        // A shader object comprises three main kinds of state:
        //
        // * Zero or more bytes of ordinary ("uniform") data
        // * Zero or more *bindings* for textures, buffers, and samplers
        // * Zero or more *sub-objects* representing nested parameter blocks, etc.
        //
        // A shader object *layout* stores information that can be used to
        // organize these different kinds of state and optimize access to them.
        //
        // For example, both texture/buffer/sampler bindings and sub-objects
        // are organized into logical *binding ranges* by the Slang reflection
        // API, and a shader object layout will store information about those
        // ranges in a form that is usable for the D3D11 API:

            /// Information about a logical binding range as reported by Slang reflection
        struct BindingRangeInfo
        {
                /// The type of bindings in this range
            slang::BindingType bindingType;

                /// The number of bindings in this range
            Index count;

                /// The starting index for this range in the appropriate "flat" array in a shader object.
                /// E.g., for a shader resourve view range, this would be an index into the `m_srvs` array.
            Index baseIndex;

                /// The offset of this binding range from the start of the sub-object
                /// in terms of whatever D3D11 register class it consumes. E.g., for
                /// a `Texture2D` binding range this will represent an offset in
                /// `t` registers.
                ///
            uint32_t registerOffset;

                /// An index into the sub-object array if this binding range is treated
                /// as a sub-object.
            Index subObjectIndex;
        };

        // Sometimes we just want to iterate over the ranges that represnet
        // sub-objects while skipping over the others, because sub-object
        // ranges often require extra handling or more state.
        //
        // For that reason we also store pre-computed information about each
        // sub-object range.

            /// Offset information for a sub-object range
        struct SubObjectRangeOffset : BindingOffset
        {
            SubObjectRangeOffset()
            {}

            SubObjectRangeOffset(slang::VariableLayoutReflection* varLayout)
                : BindingOffset(varLayout)
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
                : BindingOffset(typeLayout)
            {
                if(auto pendingLayout = typeLayout->getPendingDataTypeLayout())
                {
                    pendingOrdinaryData = (uint32_t) typeLayout->getStride();
                }
            }

                /// The strid for "pending" ordinary data related to this range
            uint32_t pendingOrdinaryData = 0;
        };

            /// Information about a logical binding range as reported by Slang reflection
        struct SubObjectRangeInfo
        {
                /// The index of the binding range that corresponds to this sub-object range
            Index bindingRangeIndex;

                /// The layout expected for objects bound to this range (if known)
            RefPtr<ShaderObjectLayoutImpl> layout;

                /// The offset to use when binding the first object in this range
            SubObjectRangeOffset offset;

                /// Stride between consecutive objects in this range
            SubObjectRangeStride stride;
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

                /// The indices of the binding ranges that represent SRVs
            List<Index> m_srvRanges;

                /// The indices of the binding ranges that represent UAVs
            List<Index> m_uavRanges;

                /// The indices of the binding ranges that represent samplers
            List<Index> m_samplerRanges;

            Index m_srvCount = 0;
            Index m_samplerCount = 0;
            Index m_uavCount = 0;
            Index m_subObjectCount = 0;

            uint32_t m_totalOrdinaryDataSize = 0;
            
                /// The container type of this shader object. When `m_containerType` is
                /// `StructuredBuffer` or `UnsizedArray`, this shader object represents a collection
                /// instead of a single object.
            ShaderObjectContainerType m_containerType = ShaderObjectContainerType::None;

            Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout)
            {
                typeLayout = _unwrapParameterGroups(typeLayout, m_containerType);

                m_elementTypeLayout = typeLayout;

                m_totalOrdinaryDataSize = (uint32_t) typeLayout->getSize();

                // Compute the binding ranges that are used to store
                // the logical contents of the object in memory.

                SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
                for (SlangInt r = 0; r < bindingRangeCount; ++r)
                {
                    slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                    SlangInt count = typeLayout->getBindingRangeBindingCount(r);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(r);

                    BindingRangeInfo bindingRangeInfo;
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
                        if (slangBindingType == slang::BindingType::RawBuffer)
                        {
                            bindingRangeInfo.baseIndex = m_srvCount;
                            m_srvCount += count;
                            m_srvRanges.add(r);
                        }
                        else
                        {
                            bindingRangeInfo.baseIndex = m_uavCount;
                            m_uavCount += count;
                            m_uavRanges.add(r);
                        }
                        break;
                    case slang::BindingType::Sampler:
                        bindingRangeInfo.baseIndex = m_samplerCount;
                        m_samplerCount += count;
                        m_samplerRanges.add(r);
                        break;

                    case slang::BindingType::CombinedTextureSampler:
                        break;
                    case slang::BindingType::MutableTexture:
                    case slang::BindingType::MutableTypedBuffer:
                        bindingRangeInfo.baseIndex = m_uavCount;
                        m_uavCount += count;
                        m_uavRanges.add(r);
                        break;

                    case slang::BindingType::VaryingInput:
                        break;

                    case slang::BindingType::VaryingOutput:
                        break;

                    default:
                        bindingRangeInfo.baseIndex = m_srvCount;
                        m_srvCount += count;
                        m_srvRanges.add(r);
                        break;
                    }

                    // We'd like to extract the information on the D3D11 shader
                    // register that this range should bind into.
                    //
                    // A binding range represents a logical member of the shader
                    // object type, and it may encompass zero or more *descriptor
                    // ranges* that describe how it is physically bound to pipeline
                    // state.
                    //
                    // If the current bindign range is backed by at least one descriptor
                    // range then we can query the register offset of that descriptor
                    // range. We expect that in the common case there will be exactly
                    // one descriptor range, and we can extract the information easily.
                    //
                    // TODO: we might eventually need to special-case our handling
                    // of combined texture-sampler ranges since they will need to
                    // store two different offsets.
                    //
                    if(typeLayout->getBindingRangeDescriptorRangeCount(r) != 0)
                    {
                        // The Slang reflection information organizes the descriptor ranges
                        // into "descriptor sets" but D3D11 has no notion like that so we
                        // expect all ranges belong to a single set.
                        //
                        SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
                        SLANG_ASSERT(descriptorSetIndex == 0);

                        SlangInt descriptorRangeIndex = typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);
                        auto registerOffset = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(descriptorSetIndex, descriptorRangeIndex);

                        bindingRangeInfo.registerOffset = (uint32_t) registerOffset;
                    }

                    m_bindingRanges.add(bindingRangeInfo);
                }

                SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
                for (SlangInt r = 0; r < subObjectRangeCount; ++r)
                {
                    SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
                    auto& bindingRange = m_bindingRanges[bindingRangeIndex];

                    auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

                    SubObjectRangeInfo subObjectRange;
                    subObjectRange.bindingRangeIndex = bindingRangeIndex;

                    // We will use Slang reflection information to extract the offset and stride
                    // information for each sub-object range.
                    //
                    subObjectRange.offset = SubObjectRangeOffset(typeLayout->getSubObjectRangeOffset(r));
                    subObjectRange.stride = SubObjectRangeStride(slangLeafTypeLayout);

                    // A sub-object range can either represent a sub-object of a known
                    // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
                    // *or* it can represent a sub-object of some existential type (e.g., `IBar`).
                    //
                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    switch(slangBindingType)
                    {
                    default:
                        {
                            // In the case of `ConstantBuffer<X>` or `ParameterBlock<X>`
                            // we can construct a layout from the element type directly.
                            //
                            auto elementTypeLayout = slangLeafTypeLayout->getElementTypeLayout();
                            createForElementType(
                                m_renderer,
                                elementTypeLayout,
                                subObjectLayout.writeRef());
                        }
                        break;

                    case slang::BindingType::ExistentialValue:
                        // In the case of an interface-type sub-object range, we can only
                        // construct a layout if we have static specialization information
                        // that tells us what type we expect to find in that range.
                        //
                        // The static specialization information is expected to take the
                        // form of a "pending" type layotu attached to the interface type
                        // of the leaf type layout.
                        //
                        if(auto pendingTypeLayout = slangLeafTypeLayout->getPendingDataTypeLayout())
                        {
                            createForElementType(
                                m_renderer,
                                pendingTypeLayout,
                                subObjectLayout.writeRef());

                            // An interface-type range that includes ordinary data can
                            // increase the size of the ordinary data buffer we need to
                            // allocate for the parent object.
                            //
                            uint32_t ordinaryDataEnd = subObjectRange.offset.pendingOrdinaryData
                                + (uint32_t) bindingRange.count * subObjectRange.stride.pendingOrdinaryData;

                            if(ordinaryDataEnd > m_totalOrdinaryDataSize)
                            {
                                m_totalOrdinaryDataSize = ordinaryDataEnd;
                            }
                        }
                    }
                    subObjectRange.layout = subObjectLayout;

                    m_subObjectRanges.add(subObjectRange);
                }
                return SLANG_OK;
            }

            SlangResult build(ShaderObjectLayoutImpl** outLayout)
            {
                auto layout =
                    RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
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

        Index getSRVCount() { return m_srvCount; }
        Index getSamplerCount() { return m_samplerCount; }
        Index getUAVCount() { return m_uavCount; }
        Index getSubObjectCount() { return m_subObjectCount; }
        Index getVaryingOutputCount() { return m_varyingOutputCount; }

        SubObjectRangeInfo const& getSubObjectRange(Index index) { return m_subObjectRanges[index]; }
        List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

        RendererBase* getRenderer() { return m_renderer; }

        slang::TypeReflection* getType()
        {
            return m_elementTypeLayout->getType();
        }

            /// Get the indices that represent all the SRV ranges in this type
        List<Index> const& getSRVRanges() const { return m_srvRanges; }

            /// Get the indices that reprsent all the UAV ranges in this type
        List<Index> const& getUAVRanges() const { return m_uavRanges; }

            /// Get the indices that represnet all the sampler ranges in this type
        List<Index> const& getSamplerRanges() const { return m_samplerRanges; }

        uint32_t getTotalOrdinaryDataSize() const { return m_totalOrdinaryDataSize; }

    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            initBase(renderer, builder->m_elementTypeLayout);

            m_bindingRanges = builder->m_bindingRanges;
            m_srvRanges = builder->m_srvRanges;
            m_uavRanges = builder->m_uavRanges;
            m_samplerRanges = builder->m_samplerRanges;

            m_srvCount = builder->m_srvCount;
            m_samplerCount = builder->m_samplerCount;
            m_uavCount = builder->m_uavCount;
            m_subObjectCount = builder->m_subObjectCount;
            m_subObjectRanges = builder->m_subObjectRanges;

            m_totalOrdinaryDataSize = builder->m_totalOrdinaryDataSize;

            m_containerType = builder->m_containerType;
            return SLANG_OK;
        }

        List<BindingRangeInfo> m_bindingRanges;
        List<Index> m_srvRanges;
        List<Index> m_uavRanges;
        List<Index> m_samplerRanges;
        Index m_srvCount = 0;
        Index m_samplerCount = 0;
        Index m_uavCount = 0;
        Index m_subObjectCount = 0;
        Index m_varyingInputCount = 0;
        Index m_varyingOutputCount = 0;
        uint32_t m_totalOrdinaryDataSize = 0;
        List<SubObjectRangeInfo> m_subObjectRanges;
    };

    class RootShaderObjectLayoutImpl : public ShaderObjectLayoutImpl
    {
        typedef ShaderObjectLayoutImpl Super;

    public:
        struct EntryPointInfo
        {
            RefPtr<ShaderObjectLayoutImpl>  layout;

                /// The offset for this entry point's parameters, relative to the starting offset for the program
            BindingOffset                   offset;
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
                m_pendingDataOffset = BindingOffset(globalsLayout).pending;
            }

            void addEntryPoint(SlangStage stage, ShaderObjectLayoutImpl* entryPointLayout, slang::EntryPointLayout* slangEntryPoint)
            {
                EntryPointInfo info;
                info.layout = entryPointLayout;
                info.offset = BindingOffset(slangEntryPoint->getVarLayout());
                m_entryPoints.add(info);
            }

            slang::IComponentType*  m_program;
            slang::ProgramLayout*   m_programLayout;
            List<EntryPointInfo>    m_entryPoints;
            SimpleBindingOffset     m_pendingDataOffset;
        };

        EntryPointInfo& getEntryPoint(Index index) { return m_entryPoints[index]; }

        List<EntryPointInfo>& getEntryPoints() { return m_entryPoints; }

        static Result create(
            RendererBase* renderer,
            slang::IComponentType* program,
            slang::ProgramLayout* programLayout,
            RootShaderObjectLayoutImpl** outLayout)
        {
            RootShaderObjectLayoutImpl::Builder builder(renderer, program, programLayout);
            builder.addGlobalParams(programLayout->getGlobalParamsVarLayout());

            SlangInt entryPointCount = programLayout->getEntryPointCount();
            for (SlangInt e = 0; e < entryPointCount; ++e)
            {
                auto slangEntryPoint = programLayout->getEntryPointByIndex(e);
                RefPtr<ShaderObjectLayoutImpl> entryPointLayout;
                SLANG_RETURN_ON_FAIL(ShaderObjectLayoutImpl::createForElementType(
                    renderer, slangEntryPoint->getTypeLayout(), entryPointLayout.writeRef()));
                builder.addEntryPoint(slangEntryPoint->getStage(), entryPointLayout, slangEntryPoint);
            }

            SLANG_RETURN_ON_FAIL(builder.build(outLayout));

            return SLANG_OK;
        }

        slang::IComponentType* getSlangProgram() const { return m_program; }
        slang::ProgramLayout* getSlangProgramLayout() const { return m_programLayout; }

            /// Get the offset at which "pending" shader parameters for this program start
        SimpleBindingOffset const& getPendingDataOffset() const { return m_pendingDataOffset; }

    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            SLANG_RETURN_ON_FAIL(Super::_init(builder));

            m_program = builder->m_program;
            m_programLayout = builder->m_programLayout;
            m_entryPoints = builder->m_entryPoints;
            m_pendingDataOffset = builder->m_pendingDataOffset;
            return SLANG_OK;
        }

        ComPtr<slang::IComponentType>   m_program;
        slang::ProgramLayout* m_programLayout = nullptr;

        List<EntryPointInfo> m_entryPoints;
        SimpleBindingOffset m_pendingDataOffset;
    };

    class ShaderObjectImpl
        : public ShaderObjectBaseImpl<
              ShaderObjectImpl,
              ShaderObjectLayoutImpl,
              SimpleShaderObjectData>
    {
    public:
        static Result create(
            IDevice* device,
            ShaderObjectLayoutImpl* layout,
            ShaderObjectImpl** outShaderObject)
        {
            auto object = RefPtr<ShaderObjectImpl>(new ShaderObjectImpl());
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            returnRefPtrMove(outShaderObject, object);
            return SLANG_OK;
        }

        RendererBase* getDevice() { return m_layout->getDevice(); }

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
            setResource(ShaderOffset const& offset, IResourceView* resourceView) SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

            auto resourceViewImpl = static_cast<ResourceViewImpl*>(resourceView);
            if (D3DUtil::isUAVBinding(bindingRange.bindingType))
            {
                SLANG_ASSERT(resourceViewImpl->m_type == ResourceViewImpl::Type::UAV);
                m_uavs[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<UnorderedAccessViewImpl*>(resourceView);
            }
            else
            {
                SLANG_ASSERT(resourceViewImpl->m_type == ResourceViewImpl::Type::SRV);
                m_srvs[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<ShaderResourceViewImpl*>(resourceView);
            }
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL setSampler(ShaderOffset const& offset, ISamplerState* sampler)
            SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

            m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<SamplerStateImpl*>(sampler);
            return SLANG_OK;
        }

        SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
            ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) SLANG_OVERRIDE
        {
            return SLANG_E_NOT_IMPLEMENTED;
        }

    public:


    protected:
        friend class ProgramVars;

        Result init(IDevice* device, ShaderObjectLayoutImpl* layout)
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
                m_data.setCount(uniformSize);
                memset(m_data.getBuffer(), 0, uniformSize);
            }

            m_srvs.setCount(layout->getSRVCount());
            m_samplers.setCount(layout->getSamplerCount());
            m_uavs.setCount(layout->getUAVCount());

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

                auto& bindingRangeInfo = layout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);
                for (Index i = 0; i < bindingRangeInfo.count; ++i)
                {
                    RefPtr<ShaderObjectImpl> subObject;
                    SLANG_RETURN_ON_FAIL(
                        ShaderObjectImpl::create(device, subObjectLayout, subObject.writeRef()));
                    m_objects[bindingRangeInfo.subObjectIndex + i] = subObject;
                }
            }

            return SLANG_OK;
        }

        /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given `offset`
        Result _writeOrdinaryData(
            void*                   dest,
            size_t                  destSize,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // We start by simply writing in the ordinary data contained directly in this object.
            //
            auto src = m_data.getBuffer();
            auto srcSize = size_t(m_data.getCount());
            SLANG_ASSERT(srcSize <= destSize);
            memcpy(dest, src, srcSize);

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
                auto const& bindingRangeInfo = specializedLayout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);

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
                // offset and stride for the location of the pending data allocation in the specialized
                // type layout, which will store the values for this sub-object range.
                //
                // TODO: The reflection API functions we are assuming here haven't been implemented
                // yet, so the functions being called here are stubs.
                //
                // TODO: It might not be that a single sub-object range can reliably map to a single
                // contiguous array with a single stride; we need to carefully consider what the layout
                // logic does for complex cases with multiple layers of nested arrays and structures.
                //
                size_t subObjectRangePendingDataOffset = subObjectRangeInfo.offset.pendingOrdinaryData;
                size_t subObjectRangePendingDataStride = subObjectRangeInfo.stride.pendingOrdinaryData;

                // If the range doesn't actually need/use the "pending" allocation at all, then
                // we need to detect that case and skip such ranges.
                //
                // TODO: This should probably be handled on a per-object basis by caching a "does it fit?"
                // bit as part of the information for bound sub-objects, given that we already
                // compute the "does it fit?" status as part of `setObject()`.
                //
                if (subObjectRangePendingDataOffset == 0)
                    continue;

                for (Slang::Index i = 0; i < count; ++i)
                {
                    auto subObject = m_objects[bindingRangeInfo.subObjectIndex + i];

                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    SLANG_RETURN_ON_FAIL(subObject->_getSpecializedLayout(subObjectLayout.writeRef()));

                    auto subObjectOffset = subObjectRangePendingDataOffset + i * subObjectRangePendingDataStride;

                    auto subObjectDest = (char*)dest + subObjectOffset;

                    subObject->_writeOrdinaryData(subObjectDest, destSize - subObjectOffset, subObjectLayout);
                }
            }

            return SLANG_OK;
        }

            /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
            ///
            /// The `specializedLayout` type must represent a specialized layout for this
            /// type that includes any "pending" data.
            ///
        Result _ensureOrdinaryDataBufferCreatedIfNeeded(
            D3D11Device*            device,
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
            if (m_ordinaryDataBuffer)
                return SLANG_OK;

            auto specializedOrdinaryDataSize = specializedLayout->getTotalOrdinaryDataSize();
            if (specializedOrdinaryDataSize == 0)
                return SLANG_OK;

            // Once we have computed how large the buffer should be, we can allocate
            // it using the existing public `IDevice` API.
            //

            ComPtr<IBufferResource> bufferResourcePtr;
            IBufferResource::Desc bufferDesc = {};
            bufferDesc.type = IResource::Type::Buffer;
            bufferDesc.sizeInBytes = specializedOrdinaryDataSize;
            bufferDesc.defaultState = ResourceState::ConstantBuffer;
            bufferDesc.allowedStates =
                ResourceStateSet(ResourceState::ConstantBuffer, ResourceState::CopyDestination);
            bufferDesc.cpuAccessFlags |= AccessFlag::Write;
            SLANG_RETURN_ON_FAIL(
                device->createBufferResource(bufferDesc, nullptr, bufferResourcePtr.writeRef()));
            m_ordinaryDataBuffer = static_cast<BufferResourceImpl*>(bufferResourcePtr.get());

            // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
            //
            // Note that `_writeOrdinaryData` is potentially recursive in the case
            // where this object contains interface/existential-type fields, so we
            // don't need or want to inline it into this call site.
            //

            auto ordinaryData = device->map(m_ordinaryDataBuffer, gfx::MapFlavor::WriteDiscard);
            auto result = _writeOrdinaryData(ordinaryData, specializedOrdinaryDataSize, specializedLayout);
            device->unmap(m_ordinaryDataBuffer, 0, specializedOrdinaryDataSize);
            
            return result;
        }

            /// Bind the buffer for ordinary/uniform data, if needed
            ///
            /// The `ioOffset` parameter will be updated to reflect the constant buffer
            /// register consumed by the ordinary data buffer, if one was bound.
            ///
        Result _bindOrdinaryDataBufferIfNeeded(
            BindingContext*         context,
            BindingOffset&          ioOffset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // We start by ensuring that the buffer is created, if it is needed.
            //
            SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(context->device, specializedLayout));

            // If we did indeed need/create a buffer, then we must bind it
            // into root binding state.
            //
            if (m_ordinaryDataBuffer)
            {
                context->setCBV(ioOffset.cbv, m_ordinaryDataBuffer->m_buffer);
                ioOffset.cbv++;
            }

            return SLANG_OK;
        }
    public:
            /// Bind this object as if it was declared as a `ConstantBuffer<T>` in Slang
        Result bindAsConstantBuffer(
            BindingContext*         context,
            BindingOffset const&    inOffset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // When binding a `ConstantBuffer<X>` we need to first bind a constant
            // buffer for any "ordinary" data in `X`, and then bind the remaining
            // resources and sub-objets.
            //
            // The one important detail to keep track of its that *if* we bind
            // a constant buffer for ordinary data we will need to account for
            // it in the offset we use for binding the remaining data. That
            // detail is dealt with here by the way that `_bindOrdinaryDataBufferIfNeeded`
            // will modify the `offset` parameter if it binds anything.
            //
            BindingOffset offset = inOffset;
            SLANG_RETURN_ON_FAIL(_bindOrdinaryDataBufferIfNeeded(context, /*inout*/ offset, specializedLayout));

            // Once the ordinary data buffer is bound, we can move on to binding
            // the rest of the state, which can use logic shared with the case
            // for interface-type sub-object ranges.
            //
            // Note that this call will use the `offset` value that might have
            // been modified during `_bindOrindaryDataBufferIfNeeded`.
            //
            SLANG_RETURN_ON_FAIL(bindAsValue(context, offset, specializedLayout));

            return SLANG_OK;
        }

            /// Bind this object as a value that appears in the body of another object.
            ///
            /// This case is directly used when binding an object for an interface-type
            /// sub-object range when static specialization is used. It is also used
            /// indirectly when binding sub-objects to constant buffer or parameter
            /// block ranges.
            ///
        Result bindAsValue(
            BindingContext*         context,
            BindingOffset const&    offset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // We start by iterating over the binding ranges in this type, isolating
            // just those ranges that represent SRVs, UAVs, and samplers.
            // In each loop we will bind the values stored for those binding ranges
            // to the correct D3D11 register (based on the `registerOffset` field
            // stored in the bindinge range).
            //
            // TODO: These loops could be optimized if we stored parallel arrays
            // for things like `m_srvs` so that we directly store an array of
            // `ID3D11ShaderResourceView*` where each entry matches the `gfx`-level
            // object that was bound (or holds null if nothing is bound).
            // In that case, we could perform a single `setSRVs()` call for each
            // binding range.
            //
            // TODO: More ambitiously, if the Slang layout algorithm could be modified
            // so that non-sub-object binding ranges are guaranteed to be contiguous
            // then a *single* `setSRVs()` call could set all of the SRVs for an object
            // at once.

            for(auto bindingRangeIndex : specializedLayout->getSRVRanges())
            {
                auto const& bindingRange = specializedLayout->getBindingRange(bindingRangeIndex);
                auto count = (uint32_t) bindingRange.count;
                auto baseIndex = (uint32_t) bindingRange.baseIndex;
                auto registerOffset = bindingRange.registerOffset + offset.srv;
                for(uint32_t i = 0; i < count; ++i)
                {
                    auto srv = m_srvs[baseIndex + i];
                    context->setSRV(registerOffset + i, srv ? srv->m_srv : nullptr);
                }
            }

            for(auto bindingRangeIndex : specializedLayout->getUAVRanges())
            {
                auto const& bindingRange = specializedLayout->getBindingRange(bindingRangeIndex);
                auto count = (uint32_t) bindingRange.count;
                auto baseIndex = (uint32_t) bindingRange.baseIndex;
                auto registerOffset = bindingRange.registerOffset + offset.uav;
                for(uint32_t i = 0; i < count; ++i)
                {
                    auto uav = m_uavs[baseIndex + i];
                    context->setUAV(registerOffset + i, uav ? uav->m_uav : nullptr);
                }
            }

            for(auto bindingRangeIndex : specializedLayout->getSamplerRanges())
            {
                auto const& bindingRange = specializedLayout->getBindingRange(bindingRangeIndex);
                auto count = (uint32_t) bindingRange.count;
                auto baseIndex = (uint32_t) bindingRange.baseIndex;
                auto registerOffset = bindingRange.registerOffset + offset.sampler;
                for(uint32_t i = 0; i < count; ++i)
                {
                    auto sampler = m_samplers[baseIndex + i];
                    context->setSampler(registerOffset + i, sampler ? sampler->m_sampler.get() : nullptr);
                }
            }

            // Once all the simple binding ranges are dealt with, we will bind
            // all of the sub-objects in sub-object ranges.
            //
            for(auto const& subObjectRange : specializedLayout->getSubObjectRanges())
            {
                auto subObjectLayout = subObjectRange.layout;
                auto const& bindingRange = specializedLayout->getBindingRange(subObjectRange.bindingRangeIndex);
                Index count = bindingRange.count;
                Index subObjectIndex = bindingRange.subObjectIndex;

                // The starting offset for a sub-object range was computed
                // from Slang reflection information, so we can apply it here.
                //
                BindingOffset rangeOffset = offset;
                rangeOffset += subObjectRange.offset;

                // Similarly, the "stride" between consecutive objects in
                // the range was also pre-computed.
                //
                BindingOffset rangeStride = subObjectRange.stride;

                switch(bindingRange.bindingType)
                {
                // For D3D11-compatible compilation targets, the Slang compiler
                // treats the `ConstantBuffer<T>` and `ParameterBlock<T>` types the same.
                //
                case slang::BindingType::ConstantBuffer:
                case slang::BindingType::ParameterBlock:
                    {
                        BindingOffset objOffset = rangeOffset;
                        for(Index i = 0; i < count; ++i)
                        {
                            auto subObject = m_objects[subObjectIndex + i];

                            // Unsurprisingly, we bind each object in the range as
                            // a constant buffer.
                            //
                            subObject->bindAsConstantBuffer(context, objOffset, subObjectLayout);

                            objOffset += rangeStride;
                        }
                    }
                    break;

                case slang::BindingType::ExistentialValue:
                    // We can only bind information for existential-typed sub-object
                    // ranges if we have a static type that we are able to specialize to.
                    //
                    if(subObjectLayout)
                    {
                        // The data for objects in this range will always be bound into
                        // the "pending" allocation for the parent block/buffer/object.
                        // As a result, the offset for the first object in the range
                        // will come from the `pending` part of the range's offset.
                        //
                        SimpleBindingOffset objOffset = rangeOffset.pending;
                        SimpleBindingOffset objStride = rangeStride.pending;

                        for(Index i = 0; i < count; ++i)
                        {
                            auto subObject = m_objects[subObjectIndex + i];
                            subObject->bindAsValue(context, BindingOffset(objOffset), subObjectLayout);

                            objOffset += objStride;
                        }
                    }
                    break;

                default:
                    break;
                }
            }

            return SLANG_OK;
        }

        // Because the binding ranges have already been reflected
        // and organized as part of each shader object layout,
        // the object itself can store its data in a small number
        // of simple arrays.
            /// The shader resource views (SRVs) that are part of the state of this object
        List<RefPtr<ShaderResourceViewImpl>> m_srvs;

            /// The unordered access views (UAVs) that are part of the state of this object
        List<RefPtr<UnorderedAccessViewImpl>> m_uavs;

            /// The samplers that are part of the state of this object
        List<RefPtr<SamplerStateImpl>> m_samplers;

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
        Result _getSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
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
        /// This operation is virtual so that it can be customized by `ProgramVars`.
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
        static Result create(IDevice* device, RootShaderObjectLayoutImpl* layout, RootShaderObjectImpl** outShaderObject)
        {
            RefPtr<RootShaderObjectImpl> object = new RootShaderObjectImpl();
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            returnRefPtrMove(outShaderObject, object);
            return SLANG_OK;
        }

        RootShaderObjectLayoutImpl* getLayout() { return static_cast<RootShaderObjectLayoutImpl*>(m_layout.Ptr()); }

        UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return (UInt)m_entryPoints.getCount(); }
        SlangResult SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint) SLANG_OVERRIDE
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

            /// Bind this object as a root shader object
        Result bindAsRoot(
            BindingContext*             context,
            RootShaderObjectLayoutImpl* specializedLayout)
        {
            // When binding an entire root shader object, we need to deal with
            // the way that specialization might have allocated space for "pending"
            // parameter data after all the primary parameters.
            //
            // We start by initializing an offset that will store zeros for the
            // primary data, an the computed offset from the specialized layout
            // for pending data.
            //
            BindingOffset offset;
            offset.pending = specializedLayout->getPendingDataOffset();

            // Note: We could *almost* call `bindAsConstantBuffer()` here to bind
            // the state of the root object itself, but there is an important
            // detail that means we can't:
            //
            // The `_bindOrdinaryDataBufferIfNeeded` operation automatically
            // increments the offset parameter if it binds a buffer, so that
            // subsequently bindings will be adjusted. However, the reflection
            // information computed for root shader parameters is absolute rather
            // than relative to the default constant buffer (if any).
            //
            // TODO: Quite technically, the ordinary data buffer for the global
            // scope is *not* guaranteed to be at offset zero, so this logic should
            // really be querying an appropriate absolute offset from `specializedLayout`.
            //
            BindingOffset ordinaryDataBufferOffset = offset;
            SLANG_RETURN_ON_FAIL(_bindOrdinaryDataBufferIfNeeded(context, /*inout*/ ordinaryDataBufferOffset, specializedLayout));
            SLANG_RETURN_ON_FAIL(bindAsValue(context, offset, specializedLayout));

            // Once the state stored in the root shader object itself has been bound,
            // we turn our attention to the entry points and their parameters.
            //
            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPoint = m_entryPoints[i];
                auto const& entryPointInfo = specializedLayout->getEntryPoint(i);

                // Each entry point will be bound at some offset relative to where
                // the root shader parameters start.
                //
                BindingOffset entryPointOffset = offset;
                entryPointOffset += entryPointInfo.offset;

                // An entry point can simply be bound as a constant buffer, because
                // the absolute offsets as are used for the global scope do not apply
                // (because entry points don't need to deal with explicit bindings).
                //
                SLANG_RETURN_ON_FAIL(entryPoint->bindAsConstantBuffer(context, entryPointOffset, entryPointInfo.layout));
            }

            return SLANG_OK;
        }


    protected:
        Result init(IDevice* device, RootShaderObjectLayoutImpl* layout)
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
            // rquire re-computing the layouts (or generated kernel code) for any of the entry points
            // that had already been loaded (in contrast to a compose-then-specialize approach).
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
            RootShaderObjectLayoutImpl::create(getRenderer(), specializedComponentType, slangSpecializedLayout, specializedLayout.writeRef());

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
    };

    void _flushGraphicsState();

    // D3D11Device members.

    DeviceInfo m_info;
    String m_adapterName;

    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_immediateContext;
    ComPtr<ID3D11Texture2D> m_backBufferTexture;
    ComPtr<IDXGIFactory> m_dxgiFactory;
    RefPtr<FramebufferImpl> m_currentFramebuffer;

    RefPtr<PipelineStateImpl> m_currentPipelineState;

    ComPtr<ID3D11Query> m_disjointQuery;

    uint32_t m_stencilRef = 0;
    bool m_depthStencilStateDirty = true;

    Desc m_desc;

    float m_clearColor[4] = { 0, 0, 0, 0 };

    bool m_nvapi = false;
};

SlangResult SLANG_MCALL createD3D11Device(const IDevice::Desc* desc, IDevice** outDevice)
{
    RefPtr<D3D11Device> result = new D3D11Device();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outDevice, result);
    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ScopeNVAPI !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult D3D11Device::ScopeNVAPI::init(D3D11Device* device, Index regIndex)
{
    if (!device->m_nvapi)
    {
        // There is nothing to set as nvapi is not set
        return SLANG_OK;
    }

#ifdef GFX_NVAPI
    NvAPI_Status nvapiStatus = NvAPI_D3D11_SetNvShaderExtnSlot(renderer->m_device, NvU32(regIndex));
    if (nvapiStatus != NVAPI_OK)
    {
        return SLANG_FAIL;
    }
#endif

    // Record the renderer so it can be freed
    m_renderer = device;
    return SLANG_OK;
}

D3D11Device::ScopeNVAPI::~ScopeNVAPI()
{
    // If the m_renderer is not set, it must not have been set up
    if (m_renderer)
    {
#ifdef GFX_NVAPI
        // Disable the slot used
        NvAPI_Status nvapiStatus = NvAPI_D3D11_SetNvShaderExtnSlot(m_renderer->m_device, ~0);
        SLANG_ASSERT(nvapiStatus == NVAPI_OK);
#endif
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!D3D11Device !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

static bool _isSupportedNVAPIOp(IUnknown* dev, uint32_t op)
{
#ifdef GFX_NVAPI
    {
        bool isSupported;
        NvAPI_Status status = NvAPI_D3D11_IsNvShaderExtnOpCodeSupported(dev, NvU32(op), &isSupported);
        return status == NVAPI_OK && isSupported;
    }
#else
    return false;
#endif
}

SlangResult D3D11Device::initialize(const Desc& desc)
{
    SLANG_RETURN_ON_FAIL(slangContext.initialize(
        desc.slang,
        SLANG_DXBC,
        "sm_5_0",
        makeArray(slang::PreprocessorMacroDesc{ "__D3D11__", "1" }).getView()));

    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

    // Initialize DeviceInfo
    {
        m_info.deviceType = DeviceType::DirectX11;
        m_info.bindingStyle = BindingStyle::DirectX;
        m_info.projectionStyle = ProjectionStyle::DirectX;
        m_info.apiName = "Direct3D 11";
        static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    m_desc = desc;

    // Rather than statically link against D3D, we load it dynamically.
    HMODULE d3dModule = LoadLibraryA("d3d11.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d11.dll'\n");
        return SLANG_FAIL;
    }

    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN D3D11CreateDeviceAndSwapChain_ =
        (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(d3dModule, "D3D11CreateDeviceAndSwapChain");
    if (!D3D11CreateDeviceAndSwapChain_)
    {
        fprintf(stderr,
            "error: failed load symbol 'D3D11CreateDeviceAndSwapChain'\n");
        return SLANG_FAIL;
    }

    PFN_D3D11_CREATE_DEVICE D3D11CreateDevice_ =
        (PFN_D3D11_CREATE_DEVICE)GetProcAddress(d3dModule, "D3D11CreateDevice");
    if (!D3D11CreateDevice_)
    {
        fprintf(stderr,
            "error: failed load symbol 'D3D11CreateDevice'\n");
        return SLANG_FAIL;
    }

    // We will ask for the highest feature level that can be supported.
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;
    const int totalNumFeatureLevels = SLANG_COUNT_OF(featureLevels);

    {
        // On a machine that does not have an up-to-date version of D3D installed,
        // the `D3D11CreateDeviceAndSwapChain` call will fail with `E_INVALIDARG`
        // if you ask for feature level 11_1 (DeviceCheckFlag::UseFullFeatureLevel).
        // The workaround is to call `D3D11CreateDeviceAndSwapChain` the first time
        // with 11_1 and then back off to 11_0 if that fails.

        FlagCombiner combiner;
        // TODO: we should probably provide a command-line option
        // to override UseDebug of default rather than leave it
        // up to each back-end to specify.

#if _DEBUG
        combiner.add(DeviceCheckFlag::UseDebug, ChangeType::OnOff);                 ///< First try debug then non debug
#else
        combiner.add(DeviceCheckFlag::UseDebug, ChangeType::Off);                   ///< Don't bother with debug
#endif
        combiner.add(DeviceCheckFlag::UseHardwareDevice, ChangeType::OnOff);        ///< First try hardware, then reference
        combiner.add(DeviceCheckFlag::UseFullFeatureLevel, ChangeType::OnOff);      ///< First try fully featured, then degrade features


        const int numCombinations = combiner.getNumCombinations();
        Result res = SLANG_FAIL;
        for (int i = 0; i < numCombinations; ++i)
        {
            const auto deviceCheckFlags = combiner.getCombination(i);
            D3DUtil::createFactory(deviceCheckFlags, m_dxgiFactory);

            // If we have an adapter set on the desc, look it up. We only need to do so for hardware
            ComPtr<IDXGIAdapter> adapter;
            if (desc.adapter &&  (deviceCheckFlags & DeviceCheckFlag::UseHardwareDevice))
            {
                List<ComPtr<IDXGIAdapter>> dxgiAdapters;
                D3DUtil::findAdapters(deviceCheckFlags, Slang::UnownedStringSlice(desc.adapter), dxgiAdapters);
                if (dxgiAdapters.getCount() == 0)
                {
                    continue;
                }
                adapter = dxgiAdapters[0];
            }

            // The adapter can be nullptr - that just means 'default', but when so we need to select the driver type
            D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN;
            if (adapter == nullptr)
            {
                // If we don't have an adapter, select directly
                driverType = (deviceCheckFlags & DeviceCheckFlag::UseHardwareDevice) ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_REFERENCE;
            }

            const int startFeatureIndex = (deviceCheckFlags & DeviceCheckFlag::UseFullFeatureLevel) ? 0 : 1; 
            const UINT deviceFlags = (deviceCheckFlags & DeviceCheckFlag::UseDebug) ? D3D11_CREATE_DEVICE_DEBUG : 0;

            res = D3D11CreateDevice_(
                adapter,
                driverType,
                nullptr,
                deviceFlags,
                &featureLevels[startFeatureIndex],
                totalNumFeatureLevels - startFeatureIndex,
                D3D11_SDK_VERSION,
                m_device.writeRef(),
                &featureLevel,
                m_immediateContext.writeRef());
            // Check if successfully constructed - if so we are done. 
            if (SLANG_SUCCEEDED(res))
            {
                break;
            }
        }
        // If res is failure, means all styles have have failed, and so initialization fails.
        if (SLANG_FAILED(res))
        {
            return res;
        }
        // Check we have a swap chain, context and device
        SLANG_ASSERT(m_immediateContext && m_device);

        ComPtr<IDXGIDevice> dxgiDevice;
        if (m_device->QueryInterface(dxgiDevice.writeRef()) == 0)
        {
            ComPtr<IDXGIAdapter> dxgiAdapter;
            dxgiDevice->GetAdapter(dxgiAdapter.writeRef());
            DXGI_ADAPTER_DESC adapterDesc;
            dxgiAdapter->GetDesc(&adapterDesc);
            m_adapterName = String::fromWString(adapterDesc.Description);
            m_info.adapterName = m_adapterName.begin();
        }
    }

    // NVAPI
    if (desc.nvapiExtnSlot >= 0)
    {
        if (SLANG_FAILED(NVAPIUtil::initialize()))
        {
            return SLANG_E_NOT_AVAILABLE;
        }

#ifdef GFX_NVAPI
        if (NvAPI_D3D11_SetNvShaderExtnSlot(m_device, NvU32(desc.nvapiExtnSlot)) != NVAPI_OK)
        {
            return SLANG_E_NOT_AVAILABLE;
        }

        if (_isSupportedNVAPIOp(m_device, NV_EXTN_OP_UINT64_ATOMIC ))
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

    {
        // Create a TIMESTAMP_DISJOINT query object to query/update frequency info.
        D3D11_QUERY_DESC disjointQueryDesc = {};
        disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        SLANG_RETURN_ON_FAIL(m_device->CreateQuery(
            &disjointQueryDesc, m_disjointQuery.writeRef()));
        m_immediateContext->Begin(m_disjointQuery);
        m_immediateContext->End(m_disjointQuery);
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData = {};
        m_immediateContext->GetData(m_disjointQuery, &disjointData, sizeof(disjointData), 0);
        m_info.timestampFrequency = disjointData.Frequency;
    }
    return SLANG_OK;
}

void D3D11Device::clearFrame(uint32_t colorBufferMask, bool clearDepth, bool clearStencil)
{
    uint32_t mask = 1;
    for (auto rtv : m_currentFramebuffer->renderTargetViews)
    {
        if (colorBufferMask & mask)
            m_immediateContext->ClearRenderTargetView(rtv->m_rtv, rtv->m_clearValue);
        mask <<= 1;
    }

    if (m_currentFramebuffer->depthStencilView)
    {
        UINT clearFlags = 0;
        if (clearDepth)
            clearFlags = D3D11_CLEAR_DEPTH;
        if (clearStencil)
            clearFlags |= D3D11_CLEAR_STENCIL;
        if (clearFlags)
        {
            m_immediateContext->ClearDepthStencilView(
                m_currentFramebuffer->depthStencilView->m_dsv,
                clearFlags,
                m_currentFramebuffer->depthStencilView->m_clearValue.depth,
                m_currentFramebuffer->depthStencilView->m_clearValue.stencil);
        }
    }
}

Result D3D11Device::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> swapchain = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(swapchain->init(this, desc, window));
    returnComPtr(outSwapchain, swapchain);
    return SLANG_OK;
}

Result D3D11Device::createFramebufferLayout(
    const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout)
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

Result D3D11Device::createFramebuffer(
    const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer)
{
    RefPtr<FramebufferImpl> framebuffer = new FramebufferImpl();
    framebuffer->renderTargetViews.setCount(desc.renderTargetCount);
    framebuffer->d3dRenderTargetViews.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        framebuffer->renderTargetViews[i] = static_cast<RenderTargetViewImpl*>(desc.renderTargetViews[i]);
        framebuffer->d3dRenderTargetViews[i] = framebuffer->renderTargetViews[i]->m_rtv;
    }
    framebuffer->depthStencilView = static_cast<DepthStencilViewImpl*>(desc.depthStencilView);
    framebuffer->d3dDepthStencilView = framebuffer->depthStencilView->m_dsv;
    returnComPtr(outFramebuffer, framebuffer);
    return SLANG_OK;
}

void D3D11Device::setFramebuffer(IFramebuffer* frameBuffer)
{
    // Note: the framebuffer state will be flushed to the pipeline as part
    // of binding the root shader object.
    //
    // TODO: alternatively we could call `OMSetRenderTargets` here and then
    // call `OMSetRenderTargetsAndUnorderedAccessViews` later with the option
    // that preserves the existing RTV/DSV bindings.
    //
    m_currentFramebuffer = static_cast<FramebufferImpl*>(frameBuffer);
}

void D3D11Device::setStencilReference(uint32_t referenceValue)
{
    m_stencilRef = referenceValue;
    m_depthStencilStateDirty = true;
}

SlangResult D3D11Device::readTextureResource(
    ITextureResource* resource,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    SLANG_UNUSED(state);

    auto texture = static_cast<TextureResourceImpl*>(resource);
    // Don't bother supporting MSAA for right now
    if (texture->getDesc()->sampleDesc.numSamples > 1)
    {
        fprintf(stderr, "ERROR: cannot capture multi-sample texture\n");
        return E_INVALIDARG;
    }

    size_t bytesPerPixel = sizeof(uint32_t);
    size_t rowPitch = int(texture->getDesc()->size.width) * bytesPerPixel;
    size_t bufferSize = rowPitch * int(texture->getDesc()->size.height);
    if (outRowPitch)
        *outRowPitch = rowPitch;
    if (outPixelSize)
        *outPixelSize = bytesPerPixel;

    D3D11_TEXTURE2D_DESC textureDesc;
    auto d3d11Texture = ((ID3D11Texture2D*)texture->m_resource.get());
    d3d11Texture->GetDesc(&textureDesc);

    HRESULT hr = S_OK;
    ComPtr<ID3D11Texture2D> stagingTexture;

    if (textureDesc.Usage == D3D11_USAGE_STAGING &&
        (textureDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
    {
        stagingTexture = d3d11Texture;
    }
    else
    {
        // Modify the descriptor to give us a staging texture
        textureDesc.BindFlags = 0;
        textureDesc.MiscFlags &= ~D3D11_RESOURCE_MISC_TEXTURECUBE;
        textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        textureDesc.Usage = D3D11_USAGE_STAGING;

        hr = m_device->CreateTexture2D(&textureDesc, 0, stagingTexture.writeRef());
        if (FAILED(hr))
        {
            fprintf(stderr, "ERROR: failed to create staging texture\n");
            return hr;
        }

        m_immediateContext->CopyResource(stagingTexture, d3d11Texture);
    }

    // Now just read back texels from the staging textures
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        SLANG_RETURN_ON_FAIL(m_immediateContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource));
        RefPtr<ListBlob> blob = new ListBlob();
        blob->m_data.setCount(bufferSize);
        char* buffer = (char*)blob->m_data.begin();
        for (size_t y = 0; y < textureDesc.Height; y++)
        {
            memcpy(
                (char*)buffer + y * (*outRowPitch),
                (char*)mappedResource.pData + y * mappedResource.RowPitch,
                *outRowPitch);
        }
        // Make sure to unmap
        m_immediateContext->Unmap(stagingTexture, 0);
        returnComPtr(outBlob, blob);
        return SLANG_OK;
    }
}

static D3D11_BIND_FLAG _calcResourceFlag(ResourceState state)
{
    switch (state)
    {
    case ResourceState::VertexBuffer:
        return D3D11_BIND_VERTEX_BUFFER;
    case ResourceState::IndexBuffer:
        return D3D11_BIND_INDEX_BUFFER;
    case ResourceState::ConstantBuffer:
        return D3D11_BIND_CONSTANT_BUFFER;
    case ResourceState::StreamOutput:
        return D3D11_BIND_STREAM_OUTPUT;
    case ResourceState::RenderTarget:
        return D3D11_BIND_RENDER_TARGET;
    case ResourceState::DepthRead:
    case ResourceState::DepthWrite:
        return D3D11_BIND_DEPTH_STENCIL;
    case ResourceState::UnorderedAccess:
        return D3D11_BIND_UNORDERED_ACCESS;
    case ResourceState::ShaderResource:
        return D3D11_BIND_SHADER_RESOURCE;
    default:
        return D3D11_BIND_FLAG(0);
    }
}

static int _calcResourceBindFlags(ResourceStateSet allowedStates)
{
    int dstFlags = 0;
    for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; i++)
    {
        auto state = (ResourceState)i;
        if (allowedStates.contains(state))
            dstFlags |= _calcResourceFlag(state);
    }
    return dstFlags;
}

static int _calcResourceAccessFlags(int accessFlags)
{
    switch (accessFlags)
    {
        case 0:         return 0;
        case AccessFlag::Read:            return D3D11_CPU_ACCESS_READ;
        case AccessFlag::Write:           return D3D11_CPU_ACCESS_WRITE;
        case AccessFlag::Read |
             AccessFlag::Write:           return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        default: assert(!"Invalid flags"); return 0;
    }
}

Result D3D11Device::createTextureResource(const ITextureResource::Desc& descIn, const ITextureResource::SubresourceData* initData, ITextureResource** outResource)
{
    TextureResource::Desc srcDesc = fixupTextureDesc(descIn);

    const int effectiveArraySize = calcEffectiveArraySize(srcDesc);

    const DXGI_FORMAT format = D3DUtil::getMapFormat(srcDesc.format);
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        return SLANG_FAIL;
    }

    const int bindFlags = _calcResourceBindFlags(srcDesc.allowedStates);

    // Set up the initialize data
    List<D3D11_SUBRESOURCE_DATA> subRes;
    D3D11_SUBRESOURCE_DATA* subResourcesPtr = nullptr;
    if(initData)
    {
        subRes.setCount(srcDesc.numMipLevels * effectiveArraySize);
        {
            int subResourceIndex = 0;
            for (int i = 0; i < effectiveArraySize; i++)
            {
                for (int j = 0; j < srcDesc.numMipLevels; j++)
                {
                    const int mipHeight = calcMipSize(srcDesc.size.height, j);

                    D3D11_SUBRESOURCE_DATA& data = subRes[subResourceIndex];
                    auto& srcData = initData[subResourceIndex];

                    data.pSysMem = srcData.data;
                    data.SysMemPitch = UINT(srcData.strideY);
                    data.SysMemSlicePitch = UINT(srcData.strideZ);

                    subResourceIndex++;
                }
            }
        }
        subResourcesPtr = subRes.getBuffer();
    }

    const int accessFlags = _calcResourceAccessFlags(srcDesc.cpuAccessFlags);

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(srcDesc));
    
    switch (srcDesc.type)
    {
        case IResource::Type::Texture1D:
        {
            D3D11_TEXTURE1D_DESC desc = { 0 };
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = accessFlags;
            desc.Format = format;
            desc.MiscFlags = 0;
            desc.MipLevels = srcDesc.numMipLevels;
            desc.ArraySize = effectiveArraySize;
            desc.Width = srcDesc.size.width;
            desc.Usage = D3D11_USAGE_DEFAULT;

            ComPtr<ID3D11Texture1D> texture1D;
            SLANG_RETURN_ON_FAIL(m_device->CreateTexture1D(&desc, subResourcesPtr, texture1D.writeRef()));

            texture->m_resource = texture1D;
            break;
        }
        case IResource::Type::TextureCube:
        case IResource::Type::Texture2D:
        {
            D3D11_TEXTURE2D_DESC desc = { 0 };
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = accessFlags;
            desc.Format = format;
            desc.MiscFlags = 0;
            desc.MipLevels = srcDesc.numMipLevels;
            desc.ArraySize = effectiveArraySize;

            desc.Width = srcDesc.size.width;
            desc.Height = srcDesc.size.height;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.SampleDesc.Count = srcDesc.sampleDesc.numSamples;
            desc.SampleDesc.Quality = srcDesc.sampleDesc.quality;

            if (srcDesc.type == IResource::Type::TextureCube)
            {
                desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
            }

            ComPtr<ID3D11Texture2D> texture2D;
            SLANG_RETURN_ON_FAIL(m_device->CreateTexture2D(&desc, subResourcesPtr, texture2D.writeRef()));

            texture->m_resource = texture2D;
            break;
        }
        case IResource::Type::Texture3D:
        {
            D3D11_TEXTURE3D_DESC desc = { 0 };
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = accessFlags;
            desc.Format = format;
            desc.MiscFlags = 0;
            desc.MipLevels = srcDesc.numMipLevels;
            desc.Width = srcDesc.size.width;
            desc.Height = srcDesc.size.height;
            desc.Depth = srcDesc.size.depth;
            desc.Usage = D3D11_USAGE_DEFAULT;

            ComPtr<ID3D11Texture3D> texture3D;
            SLANG_RETURN_ON_FAIL(m_device->CreateTexture3D(&desc, subResourcesPtr, texture3D.writeRef()));

            texture->m_resource = texture3D;
            break;
        }
        default:
            return SLANG_FAIL;
    }

    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result D3D11Device::createBufferResource(const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    IBufferResource::Desc srcDesc = fixupBufferDesc(descIn);

    auto d3dBindFlags = _calcResourceBindFlags(srcDesc.allowedStates);

    size_t alignedSizeInBytes = srcDesc.sizeInBytes;

    if(d3dBindFlags & D3D11_BIND_CONSTANT_BUFFER)
    {
        // Make aligned to 256 bytes... not sure why, but if you remove this the tests do fail.
        alignedSizeInBytes = D3DUtil::calcAligned(alignedSizeInBytes, 256);
    }

    // Hack to make the initialization never read from out of bounds memory, by copying into a buffer
    List<uint8_t> initDataBuffer;
    if (initData && alignedSizeInBytes > srcDesc.sizeInBytes)
    {
        initDataBuffer.setCount(alignedSizeInBytes);
        ::memcpy(initDataBuffer.getBuffer(), initData, srcDesc.sizeInBytes);
        initData = initDataBuffer.getBuffer();
    }

    D3D11_BUFFER_DESC bufferDesc = { 0 };
    bufferDesc.ByteWidth = UINT(alignedSizeInBytes);
    bufferDesc.BindFlags = d3dBindFlags;
    // For read we'll need to do some staging
    bufferDesc.CPUAccessFlags =
        _calcResourceAccessFlags(descIn.cpuAccessFlags & AccessFlag::Write);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;

    // If written by CPU, make it dynamic
    if ((descIn.cpuAccessFlags & AccessFlag::Write) &&
        !descIn.allowedStates.contains(ResourceState::UnorderedAccess))
    {
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    }

    switch (descIn.defaultState)
    {
    case ResourceState::ConstantBuffer:
        {
            // We'll just assume ConstantBuffers are dynamic for now
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            break;
        }
        default: break;
    }

    if (bufferDesc.BindFlags & (D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE))
    {
        //desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        if (srcDesc.elementSize != 0)
        {
            bufferDesc.StructureByteStride = srcDesc.elementSize;
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        }
        else
        {
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        }
    }

    if (srcDesc.cpuAccessFlags & AccessFlag::Write)
    {
        bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    }

    D3D11_SUBRESOURCE_DATA subResourceData = { 0 };
    subResourceData.pSysMem = initData;

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc));

    SLANG_RETURN_ON_FAIL(m_device->CreateBuffer(&bufferDesc, initData ? &subResourceData : nullptr, buffer->m_buffer.writeRef()));
    buffer->m_d3dUsage = bufferDesc.Usage;

    if ((srcDesc.cpuAccessFlags & AccessFlag::Read) ||
        ((srcDesc.cpuAccessFlags & AccessFlag::Write) && bufferDesc.Usage != D3D11_USAGE_DYNAMIC))
    {
        D3D11_BUFFER_DESC bufDesc = {};
        bufDesc.BindFlags = 0;
        bufDesc.ByteWidth = (UINT)alignedSizeInBytes;
        bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        bufDesc.Usage = D3D11_USAGE_STAGING;

        SLANG_RETURN_ON_FAIL(m_device->CreateBuffer(&bufDesc, nullptr, buffer->m_staging.writeRef()));
    }
    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

D3D11_FILTER_TYPE translateFilterMode(TextureFilteringMode mode)
{
    switch (mode)
    {
    default:
        return D3D11_FILTER_TYPE(0);

#define CASE(SRC, DST) \
    case TextureFilteringMode::SRC: return D3D11_FILTER_TYPE_##DST

        CASE(Point, POINT);
        CASE(Linear, LINEAR);

#undef CASE
    }
}

D3D11_FILTER_REDUCTION_TYPE translateFilterReduction(TextureReductionOp op)
{
    switch (op)
    {
    default:
        return D3D11_FILTER_REDUCTION_TYPE(0);

#define CASE(SRC, DST) \
    case TextureReductionOp::SRC: return D3D11_FILTER_REDUCTION_TYPE_##DST

        CASE(Average, STANDARD);
        CASE(Comparison, COMPARISON);
        CASE(Minimum, MINIMUM);
        CASE(Maximum, MAXIMUM);

#undef CASE
    }
}

D3D11_TEXTURE_ADDRESS_MODE translateAddressingMode(TextureAddressingMode mode)
{
    switch (mode)
    {
    default:
        return D3D11_TEXTURE_ADDRESS_MODE(0);

#define CASE(SRC, DST) \
    case TextureAddressingMode::SRC: return D3D11_TEXTURE_ADDRESS_##DST

    CASE(Wrap,          WRAP);
    CASE(ClampToEdge,   CLAMP);
    CASE(ClampToBorder, BORDER);
    CASE(MirrorRepeat,  MIRROR);
    CASE(MirrorOnce,    MIRROR_ONCE);

#undef CASE
    }
}

static D3D11_COMPARISON_FUNC translateComparisonFunc(ComparisonFunc func)
{
    switch (func)
    {
    default:
        // TODO: need to report failures
        return D3D11_COMPARISON_ALWAYS;

#define CASE(FROM, TO) \
    case ComparisonFunc::FROM: return D3D11_COMPARISON_##TO

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

Result D3D11Device::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    D3D11_FILTER_REDUCTION_TYPE dxReduction = translateFilterReduction(desc.reductionOp);
    D3D11_FILTER dxFilter;
    if (desc.maxAnisotropy > 1)
    {
        dxFilter = D3D11_ENCODE_ANISOTROPIC_FILTER(dxReduction);
    }
    else
    {
        D3D11_FILTER_TYPE dxMin = translateFilterMode(desc.minFilter);
        D3D11_FILTER_TYPE dxMag = translateFilterMode(desc.magFilter);
        D3D11_FILTER_TYPE dxMip = translateFilterMode(desc.mipFilter);

        dxFilter = D3D11_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, dxReduction);
    }

    D3D11_SAMPLER_DESC dxDesc = {};
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

    ComPtr<ID3D11SamplerState> sampler;
    SLANG_RETURN_ON_FAIL(m_device->CreateSamplerState(
        &dxDesc,
        sampler.writeRef()));

    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_sampler = sampler;
    returnComPtr(outSampler, samplerImpl);
    return SLANG_OK;
}

Result D3D11Device::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (TextureResourceImpl*) texture;

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::RenderTarget:
        {
            ComPtr<ID3D11RenderTargetView> rtv;
            SLANG_RETURN_ON_FAIL(m_device->CreateRenderTargetView(resourceImpl->m_resource, nullptr, rtv.writeRef()));

            RefPtr<RenderTargetViewImpl> viewImpl = new RenderTargetViewImpl();
            viewImpl->m_type = ResourceViewImpl::Type::RTV;
            viewImpl->m_rtv = rtv;
            viewImpl->m_desc = desc;

            memcpy(
                viewImpl->m_clearValue,
                &resourceImpl->getDesc()->optimalClearValue.color,
                sizeof(float) * 4);
            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;

    case IResourceView::Type::DepthStencil:
        {
            ComPtr<ID3D11DepthStencilView> dsv;
            SLANG_RETURN_ON_FAIL(m_device->CreateDepthStencilView(resourceImpl->m_resource, nullptr, dsv.writeRef()));

            RefPtr<DepthStencilViewImpl> viewImpl = new DepthStencilViewImpl();
            viewImpl->m_type = ResourceViewImpl::Type::DSV;
            viewImpl->m_dsv = dsv;
            viewImpl->m_clearValue = resourceImpl->getDesc()->optimalClearValue.depthStencil;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;

    case IResourceView::Type::UnorderedAccess:
        {
            ComPtr<ID3D11UnorderedAccessView> uav;
            SLANG_RETURN_ON_FAIL(m_device->CreateUnorderedAccessView(resourceImpl->m_resource, nullptr, uav.writeRef()));

            RefPtr<UnorderedAccessViewImpl> viewImpl = new UnorderedAccessViewImpl();
            viewImpl->m_type = ResourceViewImpl::Type::UAV;
            viewImpl->m_uav = uav;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            ComPtr<ID3D11ShaderResourceView> srv;
            SLANG_RETURN_ON_FAIL(m_device->CreateShaderResourceView(resourceImpl->m_resource, nullptr, srv.writeRef()));

            RefPtr<ShaderResourceViewImpl> viewImpl = new ShaderResourceViewImpl();
            viewImpl->m_type = ResourceViewImpl::Type::SRV;
            viewImpl->m_srv = srv;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;
    }
}

Result D3D11Device::createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;
    auto resourceDesc = *resourceImpl->getDesc();

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::UnorderedAccess:
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Format = D3DUtil::getMapFormat(desc.format);
            uavDesc.Buffer.FirstElement = 0;

            if(resourceDesc.elementSize)
            {
                uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / resourceDesc.elementSize);
            }
            else if(desc.format == Format::Unknown)
            {
                uavDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / 4);
            }
            else
            {
                uavDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / gfxGetFormatSize(desc.format));
            }

            ComPtr<ID3D11UnorderedAccessView> uav;
            SLANG_RETURN_ON_FAIL(m_device->CreateUnorderedAccessView(resourceImpl->m_buffer, &uavDesc, uav.writeRef()));

            RefPtr<UnorderedAccessViewImpl> viewImpl = new UnorderedAccessViewImpl();
            viewImpl->m_type = ResourceViewImpl::Type::UAV;
            viewImpl->m_uav = uav;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvDesc.Format = D3DUtil::getMapFormat(desc.format);
            srvDesc.Buffer.FirstElement = 0;

            if(resourceDesc.elementSize)
            {
                srvDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / resourceDesc.elementSize);
            }
            else if(desc.format == Format::Unknown)
            {
                // We need to switch to a different member of the `union`,
                // so that we can set the `BufferEx.Flags` member.
                //
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;

                // Because we've switched, we need to re-set the `FirstElement`
                // field to be valid, since we can't count on all compilers
                // to respect that `Buffer.FirstElement` and `BufferEx.FirstElement`
                // alias in memory.
                //
                srvDesc.BufferEx.FirstElement = 0;

                srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.BufferEx.NumElements = UINT(resourceDesc.sizeInBytes / 4);
            }
            else
            {
                srvDesc.Buffer.NumElements = UINT(resourceDesc.sizeInBytes / gfxGetFormatSize(desc.format));
            }

            ComPtr<ID3D11ShaderResourceView> srv;
            SLANG_RETURN_ON_FAIL(m_device->CreateShaderResourceView(resourceImpl->m_buffer, &srvDesc, srv.writeRef()));

            RefPtr<ShaderResourceViewImpl> viewImpl = new ShaderResourceViewImpl();
            viewImpl->m_type = ResourceViewImpl::Type::SRV;
            viewImpl->m_srv = srv;
            viewImpl->m_desc = desc;
            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;
    }
}

Result D3D11Device::createInputLayout(const InputElementDesc* inputElementsIn, UInt inputElementCount, IInputLayout** outLayout)
{
    D3D11_INPUT_ELEMENT_DESC inputElements[16] = {};

    char hlslBuffer[1024];
    char* hlslCursor = &hlslBuffer[0];

    hlslCursor += sprintf(hlslCursor, "float4 main(\n");

    for (UInt ii = 0; ii < inputElementCount; ++ii)
    {
        inputElements[ii].SemanticName = inputElementsIn[ii].semanticName;
        inputElements[ii].SemanticIndex = (UINT)inputElementsIn[ii].semanticIndex;
        inputElements[ii].Format = D3DUtil::getMapFormat(inputElementsIn[ii].format);
        inputElements[ii].InputSlot = 0;
        inputElements[ii].AlignedByteOffset = (UINT)inputElementsIn[ii].offset;
        inputElements[ii].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputElements[ii].InstanceDataStepRate = 0;

        if (ii != 0)
        {
            hlslCursor += sprintf(hlslCursor, ",\n");
        }

        char const* typeName = "Unknown";
        switch (inputElementsIn[ii].format)
        {
            case Format::RGBA_Float32:
            case Format::RGBA_Unorm_UInt8:
                typeName = "float4";
                break;
            case Format::RGB_Float32:
                typeName = "float3";
                break;
            case Format::RG_Float32:
                typeName = "float2";
                break;
            case Format::R_Float32:
                typeName = "float";
                break;
            default:
                return SLANG_FAIL;
        }

        hlslCursor += sprintf(hlslCursor, "%s a%d : %s%d",
            typeName,
            (int)ii,
            inputElementsIn[ii].semanticName,
            (int)inputElementsIn[ii].semanticIndex);
    }

    hlslCursor += sprintf(hlslCursor, "\n) : SV_Position { return 0; }");

    ComPtr<ID3DBlob> vertexShaderBlob;
    SLANG_RETURN_ON_FAIL(D3DUtil::compileHLSLShader("inputLayout", hlslBuffer, "main", "vs_5_0", vertexShaderBlob));

    ComPtr<ID3D11InputLayout> inputLayout;
    SLANG_RETURN_ON_FAIL(m_device->CreateInputLayout(&inputElements[0], (UINT)inputElementCount, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),
        inputLayout.writeRef()));

    RefPtr<InputLayoutImpl> impl = new InputLayoutImpl;
    impl->m_layout.swap(inputLayout);

    returnComPtr(outLayout, impl);
    return SLANG_OK;
}

Result D3D11Device::createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outPool)
{
    RefPtr<QueryPoolImpl> result = new QueryPoolImpl();
    SLANG_RETURN_ON_FAIL(result->init(desc, this));
    returnComPtr(outPool, result);
    return SLANG_OK;
}

void* D3D11Device::map(IBufferResource* bufferIn, MapFlavor flavor)
{
    BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bufferIn);

    D3D11_MAP mapType;
    ID3D11Buffer* buffer = bufferResource->m_buffer;

    switch (flavor)
    {
        case MapFlavor::WriteDiscard:
            mapType = D3D11_MAP_WRITE_DISCARD;
            break;
        case MapFlavor::HostWrite:
            mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
            break;
        case MapFlavor::HostRead:
            mapType = D3D11_MAP_READ;
            break;
        default:
            return nullptr;
    }

    bufferResource->m_mapFlavor = flavor;

    switch (flavor)
    {
    case MapFlavor::WriteDiscard:
    case MapFlavor::HostWrite:
        // If buffer is not dynamic, we need to use staging buffer.
        if (bufferResource->m_d3dUsage != D3D11_USAGE_DYNAMIC)
        {
            bufferResource->m_uploadStagingBuffer.setCount(bufferResource->getDesc()->sizeInBytes);
            return bufferResource->m_uploadStagingBuffer.getBuffer();
        }
        break;
    case MapFlavor::HostRead:
        buffer = bufferResource->m_staging;
        if (!buffer)
        {
            return nullptr;
        }

        // Okay copy the data over
        m_immediateContext->CopyResource(buffer, bufferResource->m_buffer);

    }

    // We update our constant buffer per-frame, just for the purposes
    // of the example, but we don't actually load different data
    // per-frame (we always use an identity projection).
    D3D11_MAPPED_SUBRESOURCE mappedSub;
    SLANG_RETURN_NULL_ON_FAIL(m_immediateContext->Map(buffer, 0, mapType, 0, &mappedSub));

    return mappedSub.pData;
}

void D3D11Device::unmap(IBufferResource* bufferIn, size_t offsetWritten, size_t sizeWritten)
{
    BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bufferIn);
    switch (bufferResource->m_mapFlavor)
    {
    case MapFlavor::WriteDiscard:
    case MapFlavor::HostWrite:
        // If buffer is not dynamic, the CPU has already written to the staging buffer,
        // and we need to copy the content over to the GPU buffer.
        if (bufferResource->m_d3dUsage != D3D11_USAGE_DYNAMIC && sizeWritten != 0)
        {
            D3D11_BOX dstBox = {};
            dstBox.left = (UINT)offsetWritten;
            dstBox.right = (UINT)(offsetWritten + sizeWritten);
            dstBox.back = 1;
            dstBox.bottom = 1;
            m_immediateContext->UpdateSubresource(
                bufferResource->m_buffer,
                0,
                &dstBox,
                bufferResource->m_uploadStagingBuffer.getBuffer() + offsetWritten,
                0,
                0);
            return;
        }
    }
    m_immediateContext->Unmap(bufferResource->m_mapFlavor == MapFlavor::HostRead ? bufferResource->m_staging : bufferResource->m_buffer, 0);
}

#if 0
void D3D11Device::setInputLayout(InputLayout* inputLayoutIn)
{
    auto inputLayout = static_cast<InputLayoutImpl*>(inputLayoutIn);
    m_immediateContext->IASetInputLayout(inputLayout->m_layout);
}
#endif

void D3D11Device::setPrimitiveTopology(PrimitiveTopology topology)
{
    m_immediateContext->IASetPrimitiveTopology(D3DUtil::getPrimitiveTopology(topology));
}

void D3D11Device::setVertexBuffers(UInt startSlot, UInt slotCount, IBufferResource*const* buffersIn, const UInt* stridesIn, const UInt* offsetsIn)
{
    static const int kMaxVertexBuffers = 16;
	assert(slotCount <= kMaxVertexBuffers);

    UINT vertexStrides[kMaxVertexBuffers];
    UINT vertexOffsets[kMaxVertexBuffers];
	ID3D11Buffer* dxBuffers[kMaxVertexBuffers];

	auto buffers = (BufferResourceImpl*const*)buffersIn;

    for (UInt ii = 0; ii < slotCount; ++ii)
    {
        vertexStrides[ii] = (UINT)stridesIn[ii];
        vertexOffsets[ii] = (UINT)offsetsIn[ii];
		dxBuffers[ii] = buffers[ii]->m_buffer;
	}

    m_immediateContext->IASetVertexBuffers((UINT)startSlot, (UINT)slotCount, dxBuffers, &vertexStrides[0], &vertexOffsets[0]);
}

void D3D11Device::setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
{
    DXGI_FORMAT dxFormat = D3DUtil::getMapFormat(indexFormat);
    m_immediateContext->IASetIndexBuffer(((BufferResourceImpl*)buffer)->m_buffer, dxFormat, UINT(offset));
}

void D3D11Device::setViewports(UInt count, Viewport const* viewports)
{
    static const int kMaxViewports = D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX + 1;
    assert(count <= kMaxViewports);

    D3D11_VIEWPORT dxViewports[kMaxViewports];
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

    m_immediateContext->RSSetViewports(UINT(count), dxViewports);
}

void D3D11Device::setScissorRects(UInt count, ScissorRect const* rects)
{
    static const int kMaxScissorRects = D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX + 1;
    assert(count <= kMaxScissorRects);

    D3D11_RECT dxRects[kMaxScissorRects];
    for(UInt ii = 0; ii < count; ++ii)
    {
        auto& inRect = rects[ii];
        auto& dxRect = dxRects[ii];

        dxRect.left     = LONG(inRect.minX);
        dxRect.top      = LONG(inRect.minY);
        dxRect.right    = LONG(inRect.maxX);
        dxRect.bottom   = LONG(inRect.maxY);
    }

    m_immediateContext->RSSetScissorRects(UINT(count), dxRects);
}


void D3D11Device::setPipelineState(IPipelineState* state)
{
    auto pipelineType = static_cast<PipelineStateBase*>(state)->desc.type;

    switch(pipelineType)
    {
    default:
        break;

    case PipelineType::Graphics:
        {
            auto stateImpl = (GraphicsPipelineStateImpl*) state;
            auto programImpl = static_cast<ShaderProgramImpl*>(stateImpl->m_program.Ptr());

            // TODO: We could conceivably do some lightweight state
            // differencing here (e.g., check if `programImpl` is the
            // same as the program that is currently bound).
            //
            // It isn't clear how much that would pay off given that
            // the D3D11 runtime seems to do its own state diffing.

            // IA

            m_immediateContext->IASetInputLayout(stateImpl->m_inputLayout->m_layout);

            // VS

            // TODO(tfoley): Why the conditional here? If somebody is trying to disable the VS or PS, shouldn't we respect that?
            if (programImpl->m_vertexShader)
                m_immediateContext->VSSetShader(programImpl->m_vertexShader, nullptr, 0);

            // HS

            // DS

            // GS

            // RS

            m_immediateContext->RSSetState(stateImpl->m_rasterizerState);

            // PS
            if (programImpl->m_pixelShader)
                m_immediateContext->PSSetShader(programImpl->m_pixelShader, nullptr, 0);

            // OM

            m_immediateContext->OMSetBlendState(stateImpl->m_blendState, stateImpl->m_blendColor, stateImpl->m_sampleMask);

            m_currentPipelineState = stateImpl;

            m_depthStencilStateDirty = true;
        }
        break;

    case PipelineType::Compute:
        {
            auto stateImpl = (ComputePipelineStateImpl*) state;
            auto programImpl = static_cast<ShaderProgramImpl*>(stateImpl->m_program.Ptr());

            // CS

            m_immediateContext->CSSetShader(programImpl->m_computeShader, nullptr, 0);
            m_currentPipelineState = stateImpl;
        }
        break;
    }

    /// ...
}

void D3D11Device::draw(UInt vertexCount, UInt startVertex)
{
    _flushGraphicsState();
    m_immediateContext->Draw((UINT)vertexCount, (UINT)startVertex);
}

void D3D11Device::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
    _flushGraphicsState();
    m_immediateContext->DrawIndexed((UINT)indexCount, (UINT)startIndex, (INT)baseVertex);
}

Result D3D11Device::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    SLANG_ASSERT(desc.slangProgram);

    if (desc.slangProgram->getSpecializationParamCount() != 0)
    {
        // For a specializable program, we don't invoke any actual slang compilation yet.
        RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
        shaderProgram->slangProgram = desc.slangProgram;
        returnComPtr(outProgram, shaderProgram);
        return SLANG_OK;
    }

    // If the program is already specialized, compile and create shader kernels now.
    SlangInt targetIndex = 0;
    auto slangProgram = desc.slangProgram;
    auto programLayout = slangProgram->getLayout(targetIndex);
    if (!programLayout)
        return SLANG_FAIL;
    SlangUInt entryPointCount = programLayout->getEntryPointCount();
    if (entryPointCount == 0)
        return SLANG_FAIL;

    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
    shaderProgram->slangProgram = desc.slangProgram;

    ScopeNVAPI scopeNVAPI;
    SLANG_RETURN_ON_FAIL(scopeNVAPI.init(this, 0));
    for (SlangUInt i = 0; i < entryPointCount; i++)
    {
        ComPtr<ISlangBlob> kernelCode;
        ComPtr<ISlangBlob> diagnostics;

        auto compileResult = slangProgram->getEntryPointCode(
            (SlangInt)i, 0, kernelCode.writeRef(), diagnostics.writeRef());

        if (diagnostics)
        {
            getDebugCallback()->handleMessage(
                compileResult == SLANG_OK ? DebugMessageType::Warning : DebugMessageType::Error,
                DebugMessageSource::Slang,
                (char*)diagnostics->getBufferPointer());
        }

        SLANG_RETURN_ON_FAIL(compileResult);

        auto entryPoint = programLayout->getEntryPointByIndex(i);
        switch (entryPoint->getStage())
        {
        case SLANG_STAGE_COMPUTE:
            SLANG_ASSERT(entryPointCount == 1);
            SLANG_RETURN_ON_FAIL(m_device->CreateComputeShader(
                kernelCode->getBufferPointer(),
                kernelCode->getBufferSize(),
                nullptr,
                shaderProgram->m_computeShader.writeRef()));
            break;
        case SLANG_STAGE_VERTEX:
            SLANG_RETURN_ON_FAIL(m_device->CreateVertexShader(
                kernelCode->getBufferPointer(),
                kernelCode->getBufferSize(),
                nullptr,
                shaderProgram->m_vertexShader.writeRef()));
            break;
        case SLANG_STAGE_FRAGMENT:
            SLANG_RETURN_ON_FAIL(m_device->CreatePixelShader(
                kernelCode->getBufferPointer(),
                kernelCode->getBufferSize(),
                nullptr,
                shaderProgram->m_pixelShader.writeRef()));
            break;
        default:
            SLANG_ASSERT(!"pipeline stage not implemented");
        }
    }
    returnComPtr(outProgram, shaderProgram);
    return SLANG_OK;
}

static D3D11_STENCIL_OP translateStencilOp(StencilOp op)
{
    switch(op)
    {
    default:
        // TODO: need to report failures
        return D3D11_STENCIL_OP_KEEP;

#define CASE(FROM, TO) \
    case StencilOp::FROM: return D3D11_STENCIL_OP_##TO

    CASE(Keep,              KEEP);
    CASE(Zero,              ZERO);
    CASE(Replace,           REPLACE);
    CASE(IncrementSaturate, INCR_SAT);
    CASE(DecrementSaturate, DECR_SAT);
    CASE(Invert,            INVERT);
    CASE(IncrementWrap,     INCR);
    CASE(DecrementWrap,     DECR);
#undef CASE

    }
}

static D3D11_FILL_MODE translateFillMode(FillMode mode)
{
    switch(mode)
    {
    default:
        // TODO: need to report failures
        return D3D11_FILL_SOLID;

    case FillMode::Solid:       return D3D11_FILL_SOLID;
    case FillMode::Wireframe:   return D3D11_FILL_WIREFRAME;
    }
}

static D3D11_CULL_MODE translateCullMode(CullMode mode)
{
    switch(mode)
    {
    default:
        // TODO: need to report failures
        return D3D11_CULL_NONE;

    case CullMode::None:    return D3D11_CULL_NONE;
    case CullMode::Back:    return D3D11_CULL_BACK;
    case CullMode::Front:   return D3D11_CULL_FRONT;
    }
}

bool isBlendDisabled(AspectBlendDesc const& desc)
{
    return desc.op == BlendOp::Add
        && desc.srcFactor == BlendFactor::One
        && desc.dstFactor == BlendFactor::Zero;
}


bool isBlendDisabled(TargetBlendDesc const& desc)
{
    return isBlendDisabled(desc.color)
        && isBlendDisabled(desc.alpha);
}

D3D11_BLEND_OP translateBlendOp(BlendOp op)
{
    switch(op)
    {
    default:
        assert(!"unimplemented");
        return (D3D11_BLEND_OP) -1;

#define CASE(FROM, TO) case BlendOp::FROM: return D3D11_BLEND_OP_##TO
    CASE(Add,               ADD);
    CASE(Subtract,          SUBTRACT);
    CASE(ReverseSubtract,   REV_SUBTRACT);
    CASE(Min,               MIN);
    CASE(Max,               MAX);
#undef CASE
    }
}

D3D11_BLEND translateBlendFactor(BlendFactor factor)
{
    switch(factor)
    {
    default:
        assert(!"unimplemented");
        return (D3D11_BLEND) -1;

#define CASE(FROM, TO) case BlendFactor::FROM: return D3D11_BLEND_##TO
    CASE(Zero,                  ZERO);
    CASE(One,                   ONE);
    CASE(SrcColor,              SRC_COLOR);
    CASE(InvSrcColor,           INV_SRC_COLOR);
    CASE(SrcAlpha,              SRC_ALPHA);
    CASE(InvSrcAlpha,           INV_SRC_ALPHA);
    CASE(DestAlpha,             DEST_ALPHA);
    CASE(InvDestAlpha,          INV_DEST_ALPHA);
    CASE(DestColor,             DEST_COLOR);
    CASE(InvDestColor,          INV_DEST_ALPHA);
    CASE(SrcAlphaSaturate,      SRC_ALPHA_SAT);
    CASE(BlendColor,            BLEND_FACTOR);
    CASE(InvBlendColor,         INV_BLEND_FACTOR);
    CASE(SecondarySrcColor,     SRC1_COLOR);
    CASE(InvSecondarySrcColor,  INV_SRC1_COLOR);
    CASE(SecondarySrcAlpha,     SRC1_ALPHA);
    CASE(InvSecondarySrcAlpha,  INV_SRC1_ALPHA);
#undef CASE
    }
}

D3D11_COLOR_WRITE_ENABLE translateRenderTargetWriteMask(RenderTargetWriteMaskT mask)
{
    UINT result = 0;
#define CASE(FROM, TO) if(mask & RenderTargetWriteMask::Enable##FROM) result |= D3D11_COLOR_WRITE_ENABLE_##TO

    CASE(Red,   RED);
    CASE(Green, GREEN);
    CASE(Blue,  BLUE);
    CASE(Alpha, ALPHA);

#undef CASE
    return D3D11_COLOR_WRITE_ENABLE(result);
}

Result D3D11Device::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout,
    ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(ShaderObjectLayoutImpl::createForElementType(
        this, typeLayout, layout.writeRef()));
    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result D3D11Device::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(this,
        static_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    returnComPtr(outObject, shaderObject);
    return SLANG_OK;
}

Result D3D11Device::createRootShaderObject(IShaderProgram* program, ShaderObjectBase** outObject)
{
    auto programImpl = static_cast<ShaderProgramImpl*>(program);
    RefPtr<RootShaderObjectImpl> shaderObject;
    RefPtr<RootShaderObjectLayoutImpl> rootLayout;
    SLANG_RETURN_ON_FAIL(RootShaderObjectLayoutImpl::create(
        this, programImpl->slangProgram, programImpl->slangProgram->getLayout(), rootLayout.writeRef()));
    SLANG_RETURN_ON_FAIL(RootShaderObjectImpl::create(
        this, rootLayout.Ptr(), shaderObject.writeRef()));
    returnRefPtrMove(outObject, shaderObject);
    return SLANG_OK;
}

void D3D11Device::bindRootShaderObject(IShaderObject* shaderObject)
{
    RootShaderObjectImpl* rootShaderObjectImpl = static_cast<RootShaderObjectImpl*>(shaderObject);
    RefPtr<PipelineStateBase> specializedPipeline;
    maybeSpecializePipeline(m_currentPipelineState, rootShaderObjectImpl, specializedPipeline);
    PipelineStateImpl* specializedPipelineImpl = static_cast<PipelineStateImpl*>(specializedPipeline.Ptr());
    setPipelineState(specializedPipelineImpl);

    // In order to bind the root object we must compute its specialized layout.
    //
    // TODO: This is in most ways redundant with `maybeSpecializePipeline` above,
    // and the two operations should really be one.
    //
    RefPtr<ShaderObjectLayoutImpl> specializedRootLayout;
    rootShaderObjectImpl->_getSpecializedLayout(specializedRootLayout.writeRef());
    RootShaderObjectLayoutImpl* specializedRootLayoutImpl = static_cast<RootShaderObjectLayoutImpl*>(specializedRootLayout.Ptr());

    // Depending on whether we are binding a compute or a graphics/rasterization
    // pipeline, we will need to bind any SRVs/UAVs/CBs/samplers using different
    // D3D11 calls. We deal with that distinction here by instantiating an
    // appropriate subtype of `BindingContext` based on the pipeline type.
    //
    switch (m_currentPipelineState->desc.type)
    {
    case PipelineType::Compute:
        {
            ComputeBindingContext context(this, m_immediateContext);
            rootShaderObjectImpl->bindAsRoot(&context, specializedRootLayoutImpl);

            // Because D3D11 requires all UAVs to be set at once, we did *not* issue
            // actual binding calls during the `bindAsRoot` step, and instead we
            // batch them up and set them here.
            //
            m_immediateContext->CSSetUnorderedAccessViews(0, context.uavCount, context.uavs, nullptr);
        }
        break;
    default:
        {
            GraphicsBindingContext context(this, m_immediateContext);
            rootShaderObjectImpl->bindAsRoot(&context, specializedRootLayoutImpl);

            // Similar to the compute case above, the rasteirzation case needs to
            // set the UAVs after the call to `bindAsRoot()` completes, but we
            // also have a few extra wrinkles here that are specific to the D3D 11.0
            // rasterization pipeline.
            //
            // In D3D 11.0, the RTV and UAV binding slots alias, so that a shader
            // that binds an RTV for `SV_Target0` cannot also bind a UAV for `u0`.
            // The Slang layout algorithm already accounts for this rule, and assigns
            // all UAVs to slots taht won't alias the RTVs it knows about.
            //
            // In order to account for the aliasing, we need to consider how many
            // RTVs are bound as part of the active framebuffer, and then adjust
            // the UAVs that we bind accordingly.
            //
            auto rtvCount = (UINT)m_currentFramebuffer->renderTargetViews.getCount();
            //
            // The `context` we are using will have computed the number of UAV registers
            // that might need to be bound, as a range from 0 to `context.uavCount`.
            // However we need to skip over the first `rtvCount` of those, so the
            // actual number of UAVs we wnat to bind is smaller:
            //
            // Note: As a result we expect that either there were no UAVs bound,
            // *or* the number of UAV slots bound is higher than the number of
            // RTVs so that there is something left to actually bind.
            //
            SLANG_ASSERT((context.uavCount == 0) || (context.uavCount >= rtvCount));
            auto bindableUAVCount = context.uavCount - rtvCount;
            //
            // Similarly, the actual UAVs we intend to bind will come after the first
            // `rtvCount` in the array.
            //
            auto bindableUAVs = context.uavs + rtvCount;

            // Once the offsetting is accounted for, we set all of the RTVs, DSV,
            // and UAVs with one call.
            //
            // TODO: We may want to use the capability for `OMSetRenderTargetsAnd...`
            // to only set the UAVs and leave the RTVs/UAVs alone, so that we don't
            // needlessly re-bind RTVs during a pass.
            //
            m_immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
                rtvCount,
                m_currentFramebuffer->d3dRenderTargetViews.getArrayView().getBuffer(),
                m_currentFramebuffer->d3dDepthStencilView,
                rtvCount,
                bindableUAVCount,
                bindableUAVs,
                nullptr);
        }
        break;
    }
}

Result D3D11Device::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;

    auto programImpl = (ShaderProgramImpl*) desc.program;

    ComPtr<ID3D11DepthStencilState> depthStencilState;
    {
        D3D11_DEPTH_STENCIL_DESC dsDesc;
        dsDesc.DepthEnable      = desc.depthStencil.depthTestEnable;
        dsDesc.DepthWriteMask   = desc.depthStencil.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc        = translateComparisonFunc(desc.depthStencil.depthFunc);
        dsDesc.StencilEnable    = desc.depthStencil.stencilEnable;
        dsDesc.StencilReadMask  = desc.depthStencil.stencilReadMask;
        dsDesc.StencilWriteMask = desc.depthStencil.stencilWriteMask;

    #define FACE(DST, SRC) \
        dsDesc.DST.StencilFailOp      = translateStencilOp(     desc.depthStencil.SRC.stencilFailOp);       \
        dsDesc.DST.StencilDepthFailOp = translateStencilOp(     desc.depthStencil.SRC.stencilDepthFailOp);  \
        dsDesc.DST.StencilPassOp      = translateStencilOp(     desc.depthStencil.SRC.stencilPassOp);       \
        dsDesc.DST.StencilFunc        = translateComparisonFunc(desc.depthStencil.SRC.stencilFunc);         \
        /* end */

        FACE(FrontFace, frontFace);
        FACE(BackFace,  backFace);

        SLANG_RETURN_ON_FAIL(m_device->CreateDepthStencilState(
            &dsDesc,
            depthStencilState.writeRef()));
    }

    ComPtr<ID3D11RasterizerState> rasterizerState;
    {
        D3D11_RASTERIZER_DESC rsDesc;
        rsDesc.FillMode                 = translateFillMode(desc.rasterizer.fillMode);
        rsDesc.CullMode                 = translateCullMode(desc.rasterizer.cullMode);
        rsDesc.FrontCounterClockwise    = desc.rasterizer.frontFace == FrontFaceMode::Clockwise;
        rsDesc.DepthBias                = desc.rasterizer.depthBias;
        rsDesc.DepthBiasClamp           = desc.rasterizer.depthBiasClamp;
        rsDesc.SlopeScaledDepthBias     = desc.rasterizer.slopeScaledDepthBias;
        rsDesc.DepthClipEnable          = desc.rasterizer.depthClipEnable;
        rsDesc.ScissorEnable            = desc.rasterizer.scissorEnable;
        rsDesc.MultisampleEnable        = desc.rasterizer.multisampleEnable;
        rsDesc.AntialiasedLineEnable    = desc.rasterizer.antialiasedLineEnable;

        SLANG_RETURN_ON_FAIL(m_device->CreateRasterizerState(
            &rsDesc,
            rasterizerState.writeRef()));

    }

    ComPtr<ID3D11BlendState> blendState;
    {
        auto& srcDesc = desc.blend;
        D3D11_BLEND_DESC dstDesc = {};

        TargetBlendDesc defaultTargetBlendDesc;

        static const UInt kMaxTargets = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
        if(srcDesc.targetCount > kMaxTargets) return SLANG_FAIL;

        for(UInt ii = 0; ii < kMaxTargets; ++ii)
        {
            TargetBlendDesc const* srcTargetBlendDescPtr = nullptr;
            if(ii < srcDesc.targetCount)
            {
                srcTargetBlendDescPtr = &srcDesc.targets[ii];
            }
            else if(srcDesc.targetCount == 0)
            {
                srcTargetBlendDescPtr = &defaultTargetBlendDesc;
            }
            else
            {
                srcTargetBlendDescPtr = &srcDesc.targets[srcDesc.targetCount-1];
            }

            auto& srcTargetBlendDesc = *srcTargetBlendDescPtr;
            auto& dstTargetBlendDesc = dstDesc.RenderTarget[ii];

            if(isBlendDisabled(srcTargetBlendDesc))
            {
                dstTargetBlendDesc.BlendEnable      = false;
                dstTargetBlendDesc.BlendOp          = D3D11_BLEND_OP_ADD;
                dstTargetBlendDesc.BlendOpAlpha     = D3D11_BLEND_OP_ADD;
                dstTargetBlendDesc.SrcBlend         = D3D11_BLEND_ONE;
                dstTargetBlendDesc.SrcBlendAlpha    = D3D11_BLEND_ONE;
                dstTargetBlendDesc.DestBlend        = D3D11_BLEND_ZERO;
                dstTargetBlendDesc.DestBlendAlpha   = D3D11_BLEND_ZERO;
            }
            else
            {
                dstTargetBlendDesc.BlendEnable = true;
                dstTargetBlendDesc.BlendOp          = translateBlendOp(srcTargetBlendDesc.color.op);
                dstTargetBlendDesc.BlendOpAlpha     = translateBlendOp(srcTargetBlendDesc.alpha.op);
                dstTargetBlendDesc.SrcBlend         = translateBlendFactor(srcTargetBlendDesc.color.srcFactor);
                dstTargetBlendDesc.SrcBlendAlpha    = translateBlendFactor(srcTargetBlendDesc.alpha.srcFactor);
                dstTargetBlendDesc.DestBlend        = translateBlendFactor(srcTargetBlendDesc.color.dstFactor);
                dstTargetBlendDesc.DestBlendAlpha   = translateBlendFactor(srcTargetBlendDesc.alpha.dstFactor);
            }

            dstTargetBlendDesc.RenderTargetWriteMask = translateRenderTargetWriteMask(srcTargetBlendDesc.writeMask);
        }

        dstDesc.IndependentBlendEnable = srcDesc.targetCount > 1;
        dstDesc.AlphaToCoverageEnable = srcDesc.alphaToCoverateEnable;

        SLANG_RETURN_ON_FAIL(m_device->CreateBlendState(
            &dstDesc,
            blendState.writeRef()));
    }

    RefPtr<GraphicsPipelineStateImpl> state = new GraphicsPipelineStateImpl();
    state->m_depthStencilState = depthStencilState;
    state->m_rasterizerState = rasterizerState;
    state->m_blendState = blendState;
    state->m_inputLayout = static_cast<InputLayoutImpl*>(desc.inputLayout);
    state->m_rtvCount = (UINT) static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout)
                            ->m_renderTargets.getCount();
    state->m_blendColor[0] = 0;
    state->m_blendColor[1] = 0;
    state->m_blendColor[2] = 0;
    state->m_blendColor[3] = 0;
    state->m_sampleMask = 0xFFFFFFFF;
    state->init(desc);
    returnComPtr(outState, state);
    return SLANG_OK;
}

Result D3D11Device::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;

    RefPtr<ComputePipelineStateImpl> state = new ComputePipelineStateImpl();
    state->init(desc);
    returnComPtr(outState, state);
    return SLANG_OK;
}

void D3D11Device::copyBuffer(
    IBufferResource* dst,
    size_t dstOffset,
    IBufferResource* src,
    size_t srcOffset,
    size_t size)
{
    auto dstImpl = static_cast<BufferResourceImpl*>(dst);
    auto srcImpl = static_cast<BufferResourceImpl*>(src);
    D3D11_BOX srcBox = {};
    srcBox.left = (UINT)srcOffset;
    srcBox.right = (UINT)(srcOffset + size);
    srcBox.bottom = srcBox.back = 1;
    m_immediateContext->CopySubresourceRegion(
        dstImpl->m_buffer, 0, (UINT)dstOffset, 0, 0, srcImpl->m_buffer, 0, &srcBox);
}

void D3D11Device::dispatchCompute(int x, int y, int z)
{
    m_immediateContext->Dispatch(x, y, z);
}

void D3D11Device::_flushGraphicsState()
{
    if (m_depthStencilStateDirty)
    {
        m_depthStencilStateDirty = false;
        auto pipelineState = static_cast<GraphicsPipelineStateImpl*>(m_currentPipelineState.Ptr());
        m_immediateContext->OMSetDepthStencilState(
            pipelineState->m_depthStencilState, m_stencilRef);
    }
}

}
