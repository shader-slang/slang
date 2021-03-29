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
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        clearFrame(uint32_t colorBufferMask, bool clearDepth, bool clearStencil) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createFramebufferLayout(
        const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setFramebuffer(IFramebuffer* frameBuffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) override;

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
        ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override;

    virtual Result createShaderObjectLayout(
        slang::TypeLayoutReflection* typeLayout,
        ShaderObjectLayoutBase** outLayout) override;
    virtual Result createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override;
    virtual void bindRootShaderObject(PipelineType pipelineType, IShaderObject* shaderObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc, IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc, IPipelineState** outState) override;

    virtual void* map(IBufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(IBufferResource* buffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* texture, ResourceState state, ISlangBlob** outBlob, size_t* outRowPitch, size_t* outPixelSize) override;

    virtual SLANG_NO_THROW void SLANG_MCALL
        setPrimitiveTopology(PrimitiveTopology topology) override;

    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setViewports(UInt count, Viewport const* viewports) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        setScissorRects(UInt count, ScissorRect const* rects) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setPipelineState(IPipelineState* state) override;
    virtual SLANG_NO_THROW void SLANG_MCALL draw(UInt vertexCount, UInt startVertex) override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex) override;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override;
    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() override {}
    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() override {}
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
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

        BufferResourceImpl(const IBufferResource::Desc& desc, IResource::Usage initialUsage):
            Parent(desc),
            m_initialUsage(initialUsage)
        {
        }

        MapFlavor m_mapFlavor;
        Usage m_initialUsage;
        ComPtr<ID3D11Buffer> m_buffer;
        ComPtr<ID3D11Buffer> m_staging;
    };
    class TextureResourceImpl : public TextureResource
    {
    public:
        typedef TextureResource Parent;

        TextureResourceImpl(const Desc& desc, Usage initialUsage) :
            Parent(desc),
            m_initialUsage(initialUsage)
        {
        }
        Usage m_initialUsage;
        ComPtr<ID3D11Resource> m_resource;

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
        ComPtr<ID3D11SamplerState> m_sampler;
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
        D3D11Device* m_renderer;
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
            imageDesc.init2D(
                IResource::Type::Texture2D, m_desc.format, m_desc.width, m_desc.height, 0);
            RefPtr<TextureResourceImpl> image =
                new TextureResourceImpl(imageDesc, IResource::Usage::RenderTarget);
            image->m_resource = d3dResource;
            ComPtr<ITextureResource> imageResourcePtr;
            imageResourcePtr = image.Ptr();
            for (uint32_t i = 0; i < m_desc.imageCount; i++)
            {
                m_images.add(imageResourcePtr);
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
		ComPtr<ID3D11InputLayout> m_layout;
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

    struct RootBindingState
    {
        List<ID3D11ShaderResourceView*> srvBindings;
        List<ID3D11UnorderedAccessView*> uavBindings;
        List<ID3D11SamplerState*> samplerBindings;
        List<ID3D11Buffer*> constantBuffers;
    };

    class ShaderObjectLayoutImpl : public ShaderObjectLayoutBase
    {
    public:
        struct BindingRangeInfo
        {
            slang::BindingType bindingType;
            Index count;
            Index baseIndex;
            // baseIndex2 is used to specify samplers in a CombinedTextureSampler binding.
            Index baseIndex2;

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

            Index m_srvCount = 0;
            Index m_samplerCount = 0;
            Index m_uavCount = 0;
            Index m_subObjectCount = 0;
            Index m_varyingInputCount = 0;
            Index m_varyingOutputCount = 0;

            Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout)
            {
                typeLayout = _unwrapParameterGroups(typeLayout);

                m_elementTypeLayout = typeLayout;

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
                        m_subObjectCount += count;
                        break;

                    case slang::BindingType::Sampler:
                        bindingRangeInfo.baseIndex = m_samplerCount;
                        m_samplerCount += count;
                        break;

                    case slang::BindingType::CombinedTextureSampler:
                        bindingRangeInfo.baseIndex = m_srvCount;
                        bindingRangeInfo.baseIndex2 = m_samplerCount;
                        m_srvCount += count;
                        m_samplerCount += count;
                        break;

                    case slang::BindingType::MutableRawBuffer:
                    case slang::BindingType::MutableTexture:
                    case slang::BindingType::MutableTypedBuffer:
                        bindingRangeInfo.baseIndex = m_uavCount;
                        m_uavCount += count;
                        break;

                    case slang::BindingType::VaryingInput:
                        bindingRangeInfo.baseIndex = m_varyingInputCount;
                        m_varyingInputCount += count;
                        break;

                    case slang::BindingType::VaryingOutput:
                        bindingRangeInfo.baseIndex = m_varyingOutputCount;
                        m_varyingOutputCount += count;
                        break;

                    default:
                        bindingRangeInfo.baseIndex = m_srvCount;
                        m_srvCount += count;
                        break;
                    }


                    m_bindingRanges.add(bindingRangeInfo);
                }

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

                    m_subObjectRanges.add(subObjectRange);
                }
                return SLANG_OK;
            }

            SlangResult build(ShaderObjectLayoutImpl** outLayout)
            {
                auto layout =
                    RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
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

        slang::TypeLayoutReflection* getElementTypeLayout() { return m_elementTypeLayout; }

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
    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            initBase(renderer, builder->m_elementTypeLayout);

            m_bindingRanges = builder->m_bindingRanges;

            m_srvCount = builder->m_srvCount;
            m_samplerCount = builder->m_samplerCount;
            m_uavCount = builder->m_uavCount;
            m_subObjectCount = builder->m_subObjectCount;
            m_subObjectRanges = builder->m_subObjectRanges;
            m_varyingInputCount = builder->m_varyingInputCount;
            m_varyingOutputCount = builder->m_varyingOutputCount;
            return SLANG_OK;
        }

        List<BindingRangeInfo> m_bindingRanges;
        Index m_srvCount = 0;
        Index m_samplerCount = 0;
        Index m_uavCount = 0;
        Index m_subObjectCount = 0;
        Index m_varyingInputCount = 0;
        Index m_varyingOutputCount = 0;
        List<SubObjectRangeInfo> m_subObjectRanges;
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
                builder.addEntryPoint(slangEntryPoint->getStage(), entryPointLayout);
            }

            SLANG_RETURN_ON_FAIL(builder.build(outLayout));

            return SLANG_OK;
        }

        slang::IComponentType* getSlangProgram() const { return m_program; }
        slang::ProgramLayout* getSlangProgramLayout() const { return m_programLayout; }

    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            SLANG_RETURN_ON_FAIL(Super::_init(builder));

            m_program = builder->m_program;
            m_programLayout = builder->m_programLayout;
            m_entryPoints = builder->m_entryPoints;
            return SLANG_OK;
        }

        ComPtr<slang::IComponentType>   m_program;
        slang::ProgramLayout* m_programLayout = nullptr;

        List<EntryPointInfo> m_entryPoints;
    };

    class ShaderObjectImpl : public ShaderObjectBase
    {
    public:
        static Result create(
            IDevice* device,
            ShaderObjectLayoutImpl* layout,
            ShaderObjectImpl** outShaderObject)
        {
            auto object = ComPtr<ShaderObjectImpl>(new ShaderObjectImpl());
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            *outShaderObject = object.detach();
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

        ShaderObjectLayoutImpl* getLayout()
        {
            return static_cast<ShaderObjectLayoutImpl*>(m_layout.Ptr());
        }

        SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() SLANG_OVERRIDE
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
            setObject(ShaderOffset const& offset, IShaderObject* object)
            SLANG_OVERRIDE
        {
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;

            auto subObject = static_cast<ShaderObjectImpl*>(object);

            auto bindingRangeIndex = offset.bindingRangeIndex;
            auto& bindingRange = layout->getBindingRange(bindingRangeIndex);

            m_objects[bindingRange.baseIndex + offset.bindingArrayIndex] = subObject;

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
                auto existentialTypeLayout = layout->getElementTypeLayout()->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                auto existentialType = existentialTypeLayout->getType();

                // The first field of the tuple (offset zero) is the run-time type information (RTTI)
                // ID for the concrete type being stored into the field.
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
                SLANG_RETURN_ON_FAIL(setData(witnessTableOffset, &conformanceID, sizeof(conformanceID)));

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
                    setData(payloadOffset, subObject->m_ordinaryData.getBuffer(), subObject->m_ordinaryData.getCount());
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
                }
            }

            return SLANG_E_NOT_IMPLEMENTED;
        }

        virtual SLANG_NO_THROW Result SLANG_MCALL
            getObject(ShaderOffset const& offset, IShaderObject** outObject)
            SLANG_OVERRIDE
        {
            SLANG_ASSERT(outObject);
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

            auto object = m_objects[bindingRange.baseIndex + offset.bindingArrayIndex].Ptr();
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
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
            m_srvs[bindingRange.baseIndex + offset.bindingArrayIndex] = static_cast<ShaderResourceViewImpl*>(textureView);
            m_samplers[bindingRange.baseIndex2 + offset.bindingArrayIndex] = static_cast<SamplerStateImpl*>(sampler);
            return SLANG_OK;
        }

    public:
        // Appends all types that are used to specialize the element type of this shader object in `args` list.
        virtual Result collectSpecializationArgs(ExtendedShaderObjectTypeList& args) override
        {
            auto& subObjectRanges = getLayout()->getSubObjectRanges();
            // The following logic is built on the assumption that all fields that involve existential types (and
            // therefore require specialization) will results in a sub-object range in the type layout.
            // This allows us to simply scan the sub-object ranges to find out all specialization arguments.
            Index subObjectRangeCount = subObjectRanges.getCount();
            for (Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount; subObjectRangeIndex++)
            {
                auto const& subObjectRange = subObjectRanges[subObjectRangeIndex];
                auto const& bindingRange = getLayout()->getBindingRange(subObjectRange.bindingRangeIndex);

                Index count = bindingRange.count;
                SLANG_ASSERT(count == 1);

                Index subObjectIndexInRange = 0;
                auto subObject = m_objects[bindingRange.baseIndex + subObjectIndexInRange];

                switch (bindingRange.bindingType)
                {
                case slang::BindingType::ExistentialValue:
                {
                    // A binding type of `ExistentialValue` means the sub-object represents a interface-typed field.
                    // In this case the specialization argument for this field is the actual specialized type of the bound
                    // shader object. If the shader object's type is an ordinary type without existential fields, then the
                    // type argument will simply be the ordinary type. But if the sub object's type is itself a specialized
                    // type, we need to make sure to use that type as the specialization argument.

                    ExtendedShaderObjectType specializedSubObjType;
                    SLANG_RETURN_ON_FAIL(subObject->getSpecializedShaderObjectType(&specializedSubObjType));
                    args.add(specializedSubObjType);
                    break;
                }
                case slang::BindingType::ParameterBlock:
                case slang::BindingType::ConstantBuffer:
                    // Currently we only handle the case where the field's type is
                    // `ParameterBlock<SomeStruct>` or `ConstantBuffer<SomeStruct>`, where `SomeStruct` is a struct type
                    // (not directly an interface type). In this case, we just recursively collect the specialization arguments
                    // from the bound sub object.
                    SLANG_RETURN_ON_FAIL(subObject->collectSpecializationArgs(args));
                    // TODO: we need to handle the case where the field is of the form `ParameterBlock<IFoo>`. We should treat
                    // this case the same way as the `ExistentialValue` case here, but currently we lack a mechanism to distinguish
                    // the two scenarios.
                    break;
                }
                // TODO: need to handle another case where specialization happens on resources fields e.g. `StructuredBuffer<IFoo>`.
            }
            return SLANG_OK;
        }


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
                m_ordinaryData.setCount(uniformSize);
                memset(m_ordinaryData.getBuffer(), 0, uniformSize);
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
                    m_objects[bindingRangeInfo.baseIndex + i] = subObject;
                }
            }

            return SLANG_OK;
        }

        /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given `offset`
        Result _writeOrdinaryData(
            D3D11Device* device,
            BufferResourceImpl* buffer,
            size_t offset,
            size_t                              destSize,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            auto src = m_ordinaryData.getBuffer();
            auto srcSize = size_t(m_ordinaryData.getCount());

            SLANG_ASSERT(srcSize <= destSize);

            device->uploadBufferData(buffer, offset, srcSize, src);

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
                size_t subObjectRangePendingDataOffset = _getSubObjectRangePendingDataOffset(specializedLayout, subObjectRangeIndex);
                size_t subObjectRangePendingDataStride = _getSubObjectRangePendingDataStride(specializedLayout, subObjectRangeIndex);

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
                    auto subObject = m_objects[bindingRangeInfo.baseIndex + i];

                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    SLANG_RETURN_ON_FAIL(subObject->_getSpecializedLayout(subObjectLayout.writeRef()));

                    auto subObjectOffset = subObjectRangePendingDataOffset + i * subObjectRangePendingDataStride;

                    subObject->_writeOrdinaryData(device, buffer, offset + subObjectOffset, destSize - subObjectOffset, subObjectLayout);
                }
            }

            return SLANG_OK;
        }

        // As discussed in `_writeOrdinaryData()`, these methods are just stubs waiting for
        // the "flat" Slang refelction information to provide access to the relevant data.
        //
        size_t _getSubObjectRangePendingDataOffset(ShaderObjectLayoutImpl* specializedLayout, Index subObjectRangeIndex) { return 0; }
        size_t _getSubObjectRangePendingDataStride(ShaderObjectLayoutImpl* specializedLayout, Index subObjectRangeIndex) { return 0; }

        /// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed
        Result _ensureOrdinaryDataBufferCreatedIfNeeded(D3D11Device* device)
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
            SLANG_RETURN_ON_FAIL(_getSpecializedLayout(specializedLayout.writeRef()));

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
            SLANG_RETURN_ON_FAIL(device->createBufferResource(
                IResource::Usage::ConstantBuffer, bufferDesc, nullptr, bufferResourcePtr.writeRef()));
            m_ordinaryDataBuffer = static_cast<BufferResourceImpl*>(bufferResourcePtr.get());

            // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
            //
            // Note that `_writeOrdinaryData` is potentially recursive in the case
            // where this object contains interface/existential-type fields, so we
            // don't need or want to inline it into this call site.
            //
            SLANG_RETURN_ON_FAIL(_writeOrdinaryData(device, m_ordinaryDataBuffer, 0, specializedOrdinaryDataSize, specializedLayout));

            return SLANG_OK;
        }

        /// Bind the buffer for ordinary/uniform data, if needed
        Result _bindOrdinaryDataBufferIfNeeded(
            D3D11Device* device,
            RootBindingState* bindingState)
        {
            // We start by ensuring that the buffer is created, if it is needed.
            //
            SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(device));

            // If we did indeed need/create a buffer, then we must bind it
            // into root binding state.
            //
            if (m_ordinaryDataBuffer)
            {
                bindingState->constantBuffers.add(m_ordinaryDataBuffer->m_buffer);
            }

            return SLANG_OK;
        }
    public:
        virtual Result bindObject(D3D11Device* device, RootBindingState* bindingState)
        {
            ShaderObjectLayoutImpl* layout = getLayout();

            Index baseRangeIndex = 0;
            SLANG_RETURN_ON_FAIL(_bindOrdinaryDataBufferIfNeeded(device, bindingState));

            for (auto sampler : m_samplers)
                bindingState->samplerBindings.add(sampler ? sampler->m_sampler.get() : nullptr);
            
            for (auto srv : m_srvs)
                bindingState->srvBindings.add(srv ? srv->m_srv : nullptr);

            for (auto uav : m_uavs)
                bindingState->uavBindings.add(uav ? uav->m_uav : nullptr);

            for (auto subObject : m_objects)
                subObject->bindObject(device, bindingState);
            
            return SLANG_OK;
        }

        /// Any "ordinary" / uniform data for this object
        List<char> m_ordinaryData;

        List<RefPtr<ShaderResourceViewImpl>> m_srvs;

        List<RefPtr<SamplerStateImpl>> m_samplers;

        List<RefPtr<UnorderedAccessViewImpl>> m_uavs;

        List<RefPtr<ShaderObjectImpl>> m_objects;

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
            *outLayout = RefPtr<ShaderObjectLayoutImpl>(m_specializedLayout).detach();
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
            RefPtr<ShaderObjectLayoutBase> layout;
            SLANG_RETURN_ON_FAIL(renderer->getShaderObjectLayout(extendedType.slangType, layout.writeRef()));

            *outLayout = static_cast<ShaderObjectLayoutImpl*>(layout.detach());
            return SLANG_OK;
        }

        RefPtr<ShaderObjectLayoutImpl> m_specializedLayout;
    };

    class RootShaderObjectImpl : public ShaderObjectImpl
    {
        typedef ShaderObjectImpl Super;

    public:
        static Result create(IDevice* device, RootShaderObjectLayoutImpl* layout, RootShaderObjectImpl** outShaderObject)
        {
            RefPtr<RootShaderObjectImpl> object = new RootShaderObjectImpl();
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            *outShaderObject = object.detach();
            return SLANG_OK;
        }

        RootShaderObjectLayoutImpl* getLayout() { return static_cast<RootShaderObjectLayoutImpl*>(m_layout.Ptr()); }

        UInt SLANG_MCALL getEntryPointCount() SLANG_OVERRIDE { return (UInt)m_entryPoints.getCount(); }
        SlangResult SLANG_MCALL getEntryPoint(UInt index, IShaderObject** outEntryPoint) SLANG_OVERRIDE
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

    protected:
        virtual Result bindObject(D3D11Device* device, RootBindingState* bindingState) override
        {
            SLANG_RETURN_ON_FAIL(Super::bindObject(device, bindingState));

            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPoint = m_entryPoints[i];
                SLANG_RETURN_ON_FAIL(entryPoint->bindObject(device, bindingState));
            }

            return SLANG_OK;
        }

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

            *outLayout = specializedLayout.detach();
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

    ComPtr<PipelineStateImpl> m_currentPipelineState;

    RootBindingState m_rootBindingState;

    bool m_framebufferBindingDirty = true;
    bool m_shaderBindingDirty = true;

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
    *outDevice = result.detach();
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
    SLANG_RETURN_ON_FAIL(slangContext.initialize(desc.slang, SLANG_DXBC, "sm_5_0"));

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
    *outSwapchain = swapchain.detach();
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
    *outLayout = layout.detach();
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
    *outFramebuffer = framebuffer.detach();
    return SLANG_OK;
}

void D3D11Device::setFramebuffer(IFramebuffer* frameBuffer)
{
    m_framebufferBindingDirty = true;
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
        *outBlob = blob.detach();
        return SLANG_OK;
    }
}

static D3D11_BIND_FLAG _calcResourceFlag(IResource::BindFlag::Enum bindFlag)
{
    typedef IResource::BindFlag BindFlag;
    switch (bindFlag)
    {
        case BindFlag::VertexBuffer:            return D3D11_BIND_VERTEX_BUFFER;
        case BindFlag::IndexBuffer:             return D3D11_BIND_INDEX_BUFFER;
        case BindFlag::ConstantBuffer:          return D3D11_BIND_CONSTANT_BUFFER;
        case BindFlag::StreamOutput:            return D3D11_BIND_STREAM_OUTPUT;
        case BindFlag::RenderTarget:            return D3D11_BIND_RENDER_TARGET;
        case BindFlag::DepthStencil:            return D3D11_BIND_DEPTH_STENCIL;
        case BindFlag::UnorderedAccess:         return D3D11_BIND_UNORDERED_ACCESS;
        case BindFlag::PixelShaderResource:     return D3D11_BIND_SHADER_RESOURCE;
        case BindFlag::NonPixelShaderResource:  return D3D11_BIND_SHADER_RESOURCE;
        default:                                return D3D11_BIND_FLAG(0);
    }
}

static int _calcResourceBindFlags(int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;

        dstFlags |= _calcResourceFlag(IResource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return dstFlags;
}

static int _calcResourceAccessFlags(int accessFlags)
{
    switch (accessFlags)
    {
        case 0:         return 0;
        case IResource::AccessFlag::Read:            return D3D11_CPU_ACCESS_READ;
        case IResource::AccessFlag::Write:           return D3D11_CPU_ACCESS_WRITE;
        case IResource::AccessFlag::Read |
             IResource::AccessFlag::Write:           return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        default: assert(!"Invalid flags"); return 0;
    }
}

Result D3D11Device::createTextureResource(IResource::Usage initialUsage, const ITextureResource::Desc& descIn, const ITextureResource::SubresourceData* initData, ITextureResource** outResource)
{
    TextureResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    const int effectiveArraySize = srcDesc.calcEffectiveArraySize();

    const DXGI_FORMAT format = D3DUtil::getMapFormat(srcDesc.format);
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        return SLANG_FAIL;
    }

    const int bindFlags = _calcResourceBindFlags(srcDesc.bindFlags);

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
                    const int mipHeight = ITextureResource::Size::calcMipSize(srcDesc.size.height, j);

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

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(srcDesc, initialUsage));
    
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

    *outResource = texture.detach();
    return SLANG_OK;
}

Result D3D11Device::createBufferResource(IResource::Usage initialUsage, const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    IBufferResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    auto d3dBindFlags = _calcResourceBindFlags(srcDesc.bindFlags);

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
        _calcResourceAccessFlags(descIn.cpuAccessFlags & IResource::AccessFlag::Write);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;

    // If written by CPU, make it dynamic
    if ((descIn.cpuAccessFlags & IResource::AccessFlag::Write) &&
        ((descIn.bindFlags & IResource::BindFlag::UnorderedAccess) == 0))
    {
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    }

    switch (initialUsage)
    {
    case IResource::Usage::ConstantBuffer:
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

    if (srcDesc.cpuAccessFlags & IResource::AccessFlag::Write)
    {
        bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    }

    D3D11_SUBRESOURCE_DATA subResourceData = { 0 };
    subResourceData.pSysMem = initData;

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc, initialUsage));

    SLANG_RETURN_ON_FAIL(m_device->CreateBuffer(&bufferDesc, initData ? &subResourceData : nullptr, buffer->m_buffer.writeRef()));

    if (srcDesc.cpuAccessFlags & IResource::AccessFlag::Read)
    {
        D3D11_BUFFER_DESC bufDesc = {};
        bufDesc.BindFlags = 0;
        bufDesc.ByteWidth = (UINT)alignedSizeInBytes;
        bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        bufDesc.Usage = D3D11_USAGE_STAGING;

        SLANG_RETURN_ON_FAIL(m_device->CreateBuffer(&bufDesc, nullptr, buffer->m_staging.writeRef()));
    }

    *outResource = buffer.detach();
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
    *outSampler = samplerImpl.detach();
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
            memcpy(
                viewImpl->m_clearValue,
                &resourceImpl->getDesc()->optimalClearValue.color,
                sizeof(float) * 4);
            *outView = viewImpl.detach();
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
            *outView = viewImpl.detach();
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
            *outView = viewImpl.detach();
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
            *outView = viewImpl.detach();
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
            *outView = viewImpl.detach();
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
            *outView = viewImpl.detach();
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

    *outLayout = impl.detach();
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

            buffer = bufferResource->m_staging;
            if (!buffer)
            {
                return nullptr;
            }

            // Okay copy the data over
            m_immediateContext->CopyResource(buffer, bufferResource->m_buffer);

            break;
        default:
            return nullptr;
    }

    // We update our constant buffer per-frame, just for the purposes
    // of the example, but we don't actually load different data
    // per-frame (we always use an identity projection).
    D3D11_MAPPED_SUBRESOURCE mappedSub;
    SLANG_RETURN_NULL_ON_FAIL(m_immediateContext->Map(buffer, 0, mapType, 0, &mappedSub));

    bufferResource->m_mapFlavor = flavor;

    return mappedSub.pData;
}

void D3D11Device::unmap(IBufferResource* bufferIn)
{
    BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bufferIn);
    ID3D11Buffer* buffer = (bufferResource->m_mapFlavor == MapFlavor::HostRead) ? bufferResource->m_staging : bufferResource->m_buffer;
    m_immediateContext->Unmap(buffer, 0);
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
            auto programImpl = static_cast<ShaderProgramImpl*>(stateImpl->m_program.get());

            // TODO: We could conceivably do some lightweight state
            // differencing here (e.g., check if `programImpl` is the
            // same as the program that is currently bound).
            //
            // It isn't clear how much that would pay off given that
            // the D3D11 runtime seems to do its own state diffing.

            // IA

            m_immediateContext->IASetInputLayout(stateImpl->m_inputLayout->m_layout);

            // VS
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
            auto programImpl = static_cast<ShaderProgramImpl*>(stateImpl->m_program.get());

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
        *outProgram = shaderProgram.detach();
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
            // TODO: dump compiler output.
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
    *outProgram = shaderProgram.detach();
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
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result D3D11Device::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(this,
        static_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result D3D11Device::createRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    auto programImpl = static_cast<ShaderProgramImpl*>(program);
    RefPtr<RootShaderObjectImpl> shaderObject;
    RefPtr<RootShaderObjectLayoutImpl> rootLayout;
    SLANG_RETURN_ON_FAIL(RootShaderObjectLayoutImpl::create(
        this, programImpl->slangProgram, programImpl->slangProgram->getLayout(), rootLayout.writeRef()));
    SLANG_RETURN_ON_FAIL(RootShaderObjectImpl::create(
        this, rootLayout.Ptr(), shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

void D3D11Device::bindRootShaderObject(PipelineType pipelineType, IShaderObject* shaderObject)
{
    RootShaderObjectImpl* rootShaderObjectImpl = static_cast<RootShaderObjectImpl*>(shaderObject);
    RefPtr<PipelineStateBase> specializedPipeline;
    maybeSpecializePipeline(m_currentPipelineState, rootShaderObjectImpl, specializedPipeline);
    setPipelineState(specializedPipeline.Ptr());

    m_rootBindingState.samplerBindings.clear();
    m_rootBindingState.srvBindings.clear();
    m_rootBindingState.uavBindings.clear();
    m_rootBindingState.constantBuffers.clear();
    static_cast<ShaderObjectImpl*>(shaderObject)->bindObject(this, &m_rootBindingState);
    switch (pipelineType)
    {
    case PipelineType::Compute:
        m_immediateContext->CSSetShaderResources(0, (UINT)m_rootBindingState.srvBindings.getCount(), m_rootBindingState.srvBindings.getBuffer());
        m_immediateContext->CSSetUnorderedAccessViews(0, (UINT)m_rootBindingState.uavBindings.getCount(), m_rootBindingState.uavBindings.getBuffer(), nullptr);
        m_immediateContext->CSSetSamplers(0, (UINT)m_rootBindingState.samplerBindings.getCount(), m_rootBindingState.samplerBindings.getBuffer());
        m_immediateContext->CSSetConstantBuffers(0, (UINT)m_rootBindingState.constantBuffers.getCount(), m_rootBindingState.constantBuffers.getBuffer());
        break;
    default:
        m_immediateContext->VSSetShaderResources(0, (UINT)m_rootBindingState.srvBindings.getCount(), m_rootBindingState.srvBindings.getBuffer());
        m_immediateContext->PSSetShaderResources(0, (UINT)m_rootBindingState.srvBindings.getCount(), m_rootBindingState.srvBindings.getBuffer());
        m_immediateContext->VSSetSamplers(0, (UINT)m_rootBindingState.samplerBindings.getCount(), m_rootBindingState.samplerBindings.getBuffer());
        m_immediateContext->PSSetSamplers(0, (UINT)m_rootBindingState.samplerBindings.getCount(), m_rootBindingState.samplerBindings.getBuffer());
        m_immediateContext->VSSetConstantBuffers(0, (UINT)m_rootBindingState.constantBuffers.getCount(), m_rootBindingState.constantBuffers.getBuffer());
        m_immediateContext->PSSetConstantBuffers(0, (UINT)m_rootBindingState.constantBuffers.getCount(), m_rootBindingState.constantBuffers.getBuffer());
        m_shaderBindingDirty = true;
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
    *outState = state.detach();
    return SLANG_OK;
}

Result D3D11Device::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;

    RefPtr<ComputePipelineStateImpl> state = new ComputePipelineStateImpl();
    state->init(desc);
    *outState = state.detach();
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
    auto pipelineType = int(PipelineType::Graphics);
    if (m_framebufferBindingDirty || m_shaderBindingDirty)
    {
        m_framebufferBindingDirty = false;
        m_shaderBindingDirty = false;

        auto pipelineState = static_cast<GraphicsPipelineStateImpl*>(m_currentPipelineState.get());
        auto rtvCount = (UINT)m_currentFramebuffer->renderTargetViews.getCount();
        auto uavCount = (UINT)m_rootBindingState.uavBindings.getCount();
        m_immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
            rtvCount,
            m_currentFramebuffer->d3dRenderTargetViews.getArrayView().getBuffer(),
            m_currentFramebuffer->d3dDepthStencilView,
            rtvCount,
            uavCount,
            m_rootBindingState.uavBindings.getBuffer(),
            nullptr);
    }
    if (m_depthStencilStateDirty)
    {
        m_depthStencilStateDirty = false;
        auto pipelineState = static_cast<GraphicsPipelineStateImpl*>(m_currentPipelineState.get());
        m_immediateContext->OMSetDepthStencilState(
            pipelineState->m_depthStencilState, m_stencilRef);
    }
}

}
