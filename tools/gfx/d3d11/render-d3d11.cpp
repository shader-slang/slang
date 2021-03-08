// render-d3d11.cpp
#define _CRT_SECURE_NO_WARNINGS

#include "render-d3d11.h"
#include "core/slang-basic.h"
#include "core/slang-blob.h"

//WORKING: #include "options.h"
#include "../immediate-renderer-base.h"
#include "../d3d/d3d-util.h"
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

class D3D11Renderer : public ImmediateRendererBase
{
public:
    enum
    {
        kMaxUAVs = 64,
        kMaxRTVs = 8,
    };

    ~D3D11Renderer() {}

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
        ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        const InputElementDesc* inputElements,
        UInt inputElementCount,
        IInputLayout** outLayout) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createDescriptorSetLayout(
        const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) override;
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
    virtual SLANG_NO_THROW RendererType SLANG_MCALL getRendererType() const override
    {
        return RendererType::DirectX11;
    }

protected:

    class ScopeNVAPI
    {
    public:
        ScopeNVAPI() : m_renderer(nullptr) {}
        SlangResult init(D3D11Renderer* renderer, Index regIndex);
        ~ScopeNVAPI();

    protected:
        D3D11Renderer* m_renderer;
    };

#if 0
    struct BindingDetail
    {
        ComPtr<ID3D11ShaderResourceView>    m_srv;
        ComPtr<ID3D11UnorderedAccessView>   m_uav;
        ComPtr<ID3D11SamplerState>          m_samplerState;
    };

    class BindingStateImpl: public BindingState
    {
		public:
        typedef BindingState Parent;

            /// Ctor
        BindingStateImpl(const Desc& desc):
            Parent(desc)
        {}

        List<BindingDetail> m_bindingDetails;
    };
#endif

    
    enum class D3D11DescriptorSlotType
    {
        ConstantBuffer,
        ShaderResourceView,
        UnorderedAccessView,
        Sampler,

        CombinedTextureSampler,

        CountOf,
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
        // Each descriptor set for the D3D11 renderer stores distinct
        // arrays for each kind of shader-visible entity D3D11 understands:
        // shader resource views (SRVs), unordered access views (UAVs),
        // constant buffers (CBs), and samplers.
        //
        // (This description will ignore compiled image/sampler pairs,
        // since they aren't really well supported at present)
        //
        // Each descriptor range in an input `DescriptorSetLayout::Desc`
        // will map to a range of entries in one of those arrays, but
        // in general there can be multiple `DescriptorSlotType`s that
        // map to the same `D3D11DescriptorSlotType`.
        //
        // Each `RangeInfo` in a D3D11 descriptor set layout represents
        // of of the descriptor slot ranges in the original `Desc`,
        // and stores the information that is relevant to its layout
        // in our D3D11 implementation.

        struct RangeInfo
        {
                /// The type of descriptors in the range, in D3D11 terms (SRVs, UAVs, etc.)
            D3D11DescriptorSlotType type;

                /// The start index of this range in the relevant descriptor-type-specific array.
                ///
                /// Note: This is *not* the same as the index of the range, both because multiple
                /// `DescriptorSlotType`s might map to the same array in the D3D11 implementation,
                /// and also because a given range might store multiple descriptors (so a 3-texture
                /// range that comes after a 5-texture range will have an `arrayIndex` of 5 but
                /// a range index of 1).
                ///
            UInt                    arrayIndex;

                /// For the case of a combined image/sampler pair, the `arrayIndex` is an index
                /// into the array of SRVs, and we store a separate index into the array of
                /// samplers.
                ///
            UInt                    pairedSamplerArrayIndex;
        };
        List<RangeInfo> m_ranges;

        // Because D3D11 does not support root constants as they appear in
        // D3D12 and Vulkan, we need to map root-constant ranges in the original `Desc`
        // over to ordinary constant buffers. Each root-constant range (of whatever
        // size) will map to a constant-buffer range of a single buffer.
        //
        // In order to be able to properly allocate/initialize these root constant
        // buffers, we store additional information about them in a flattened array
        // that only stores information for root constant ranges.

        struct RootConstantRangeInfo
        {
                /// Index of the `RangeInfo` corresponding to this root-constant range
            Index rangeIndex;

                /// Size of the original root-constant range, in bytes.
            UInt  size;
        };
        List<RootConstantRangeInfo> m_rootConstantRanges;

        UInt m_counts[int(D3D11DescriptorSlotType::CountOf)];
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
        struct DescriptorSetInfo
        {
            RefPtr<DescriptorSetLayoutImpl> layout;
            UInt                            baseIndices[int(D3D11DescriptorSlotType::CountOf)];
        };

        List<DescriptorSetInfo>     m_descriptorSets;
        UINT                        m_uavCount;
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

        D3D11Renderer*                          m_renderer = nullptr;

        RefPtr<DescriptorSetLayoutImpl>         m_layout;

        List<ComPtr<ID3D11Buffer>>              m_cbs;
        List<ComPtr<ID3D11ShaderResourceView>>  m_srvs;
        List<ComPtr<ID3D11UnorderedAccessView>> m_uavs;
        List<ComPtr<ID3D11SamplerState>>        m_samplers;
    };

    class ShaderProgramImpl : public GraphicsCommonShaderProgram
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

    class SwapchainImpl
        : public ISwapchain
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISwapchain* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISwapchain)
                return static_cast<ISwapchain*>(this);
            return nullptr;
        }

    public:
        Result init(D3D11Renderer* renderer, const ISwapchain::Desc& desc, WindowHandle window)
        {
            // Return fail on non-supported platforms.
            switch (window.type)
            {
            case WindowHandle::Type::Win32Handle:
                break;
            default:
                return SLANG_FAIL;
            }

            m_renderer = renderer;
            m_desc = desc;

            // Describe the swap chain.
            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            swapChainDesc.BufferCount = desc.imageCount;
            swapChainDesc.BufferDesc.Width = desc.width;
            swapChainDesc.BufferDesc.Height = desc.height;
            swapChainDesc.BufferDesc.Format = D3DUtil::getMapFormat(desc.format);
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            swapChainDesc.OutputWindow = (HWND)window.handleValues[0];
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.Windowed = TRUE;

            if (!desc.enableVSync)
            {
                swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
            }

            // Swap chain needs the queue so that it can force a flush on it.
            ComPtr<IDXGISwapChain> swapChain;
            SLANG_RETURN_ON_FAIL(m_renderer->m_dxgiFactory->CreateSwapChain(
                m_renderer->m_device, &swapChainDesc, swapChain.writeRef()));
            SLANG_RETURN_ON_FAIL(swapChain->QueryInterface(m_swapChain.writeRef()));

            if (!desc.enableVSync)
            {
                m_swapChainWaitableObject = m_swapChain->GetFrameLatencyWaitableObject();

                int maxLatency = desc.imageCount - 2;

                // Make sure the maximum latency is in the range required by dx12 runtime
                maxLatency = (maxLatency < 1) ? 1 : maxLatency;
                maxLatency = (maxLatency > DXGI_MAX_SWAP_CHAIN_BUFFERS)
                                 ? DXGI_MAX_SWAP_CHAIN_BUFFERS
                                 : maxLatency;

                m_swapChain->SetMaximumFrameLatency(maxLatency);
            }

            SLANG_RETURN_ON_FAIL(m_renderer->m_dxgiFactory->MakeWindowAssociation(
                (HWND)window.handleValues[0], DXGI_MWA_NO_ALT_ENTER));

            for (uint32_t i = 0; i < desc.imageCount; i++)
            {
                ComPtr<ID3D11Resource> d3dResource;
                m_swapChain->GetBuffer(0, IID_PPV_ARGS(d3dResource.writeRef()));
                ITextureResource::Desc imageDesc = {};
                imageDesc.init2D(
                    IResource::Type::Texture2D, desc.format, desc.width, desc.height, 0);
                RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc, IResource::Usage::RenderTarget);
                image->m_resource = d3dResource;
                m_images.add(image);
            }
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }
        virtual SLANG_NO_THROW Result
            getImage(uint32_t index, ITextureResource** outResource) override
        {
            m_images[index]->addRef();
            *outResource = m_images[index].Ptr();
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW Result present() override
        {
            if (m_swapChainWaitableObject)
            {
                // check if now is good time to present
                // This doesn't wait - because the wait time is 0. If it returns WAIT_TIMEOUT it
                // means that no frame is waiting to be be displayed so there is no point doing a
                // present.
                const bool shouldPresent =
                    (WaitForSingleObjectEx(m_swapChainWaitableObject, 0, TRUE) != WAIT_TIMEOUT);
                if (shouldPresent)
                {
                    m_swapChain->Present(0, 0);
                }
            }
            else
            {
                if (SLANG_FAILED(m_swapChain->Present(1, 0)))
                {
                    return SLANG_FAIL;
                }
            }
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW uint32_t acquireNextImage() override
        {
            uint32_t count;
            m_swapChain->GetLastPresentCount(&count);
            return count % m_images.getCount();
        }

    public:
        D3D11Renderer* m_renderer = nullptr;
        ISwapchain::Desc m_desc;
        HANDLE m_swapChainWaitableObject = nullptr;
        ComPtr<IDXGISwapChain2> m_swapChain;
        ShortList<RefPtr<TextureResourceImpl>> m_images;
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

    void _flushGraphicsState();
    void _flushComputeState();

    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_immediateContext;
    ComPtr<ID3D11Texture2D> m_backBufferTexture;
    ComPtr<IDXGIFactory> m_dxgiFactory;

    RefPtr<FramebufferImpl> m_currentFramebuffer;

    ComPtr<PipelineStateImpl> m_currentPipelineState;

    ComPtr<ID3D11UnorderedAccessView>   m_uavBindings[int(PipelineType::CountOf)][kMaxUAVs];

    bool m_framebufferBindingDirty = true;
    bool m_shaderBindingDirty = true;

    uint32_t m_stencilRef = 0;
    bool m_depthStencilStateDirty = true;

    Desc m_desc;

    float m_clearColor[4] = { 0, 0, 0, 0 };

    bool m_nvapi = false;
};

SlangResult SLANG_MCALL createD3D11Renderer(const IRenderer::Desc* desc, IRenderer** outRenderer)
{
    RefPtr<D3D11Renderer> result = new D3D11Renderer();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    *outRenderer = result.detach();
    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ScopeNVAPI !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult D3D11Renderer::ScopeNVAPI::init(D3D11Renderer* renderer, Index regIndex)
{
    if (!renderer->m_nvapi)
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
    m_renderer = renderer;
    return SLANG_OK;
}

D3D11Renderer::ScopeNVAPI::~ScopeNVAPI()
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!D3D11Renderer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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

SlangResult D3D11Renderer::initialize(const Desc& desc)
{
    SLANG_RETURN_ON_FAIL(slangContext.initialize(desc.slang, SLANG_DXBC, "sm_5_0"));

    SLANG_RETURN_ON_FAIL(GraphicsAPIRenderer::initialize(desc));

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

void D3D11Renderer::clearFrame(uint32_t colorBufferMask, bool clearDepth, bool clearStencil)
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

Result D3D11Renderer::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> swapchain = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(swapchain->init(this, desc, window));
    *outSwapchain = swapchain.detach();
    return SLANG_OK;
}

Result D3D11Renderer::createFramebufferLayout(
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

Result D3D11Renderer::createFramebuffer(
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

void D3D11Renderer::setFramebuffer(IFramebuffer* frameBuffer)
{
    m_framebufferBindingDirty = true;
    m_currentFramebuffer = static_cast<FramebufferImpl*>(frameBuffer);
}

void D3D11Renderer::setStencilReference(uint32_t referenceValue)
{
    m_stencilRef = referenceValue;
    m_depthStencilStateDirty = true;
}

SlangResult D3D11Renderer::readTextureResource(
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

Result D3D11Renderer::createTextureResource(IResource::Usage initialUsage, const ITextureResource::Desc& descIn, const ITextureResource::Data* initData, ITextureResource** outResource)
{
    TextureResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    const int effectiveArraySize = srcDesc.calcEffectiveArraySize();

    if(initData)
    {
        assert(initData->numSubResources == srcDesc.numMipLevels * effectiveArraySize * srcDesc.size.depth);
    }

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

                    data.pSysMem = initData->subResources[subResourceIndex];

                    data.SysMemPitch = UINT(initData->mipRowStrides[j]);
                    data.SysMemSlicePitch = UINT(initData->mipRowStrides[j] * mipHeight);

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

Result D3D11Renderer::createBufferResource(IResource::Usage initialUsage, const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
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

Result D3D11Renderer::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
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

Result D3D11Renderer::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
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

Result D3D11Renderer::createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
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

Result D3D11Renderer::createInputLayout(const InputElementDesc* inputElementsIn, UInt inputElementCount, IInputLayout** outLayout)
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

void* D3D11Renderer::map(IBufferResource* bufferIn, MapFlavor flavor)
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

void D3D11Renderer::unmap(IBufferResource* bufferIn)
{
    BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bufferIn);
    ID3D11Buffer* buffer = (bufferResource->m_mapFlavor == MapFlavor::HostRead) ? bufferResource->m_staging : bufferResource->m_buffer;
    m_immediateContext->Unmap(buffer, 0);
}

#if 0
void D3D11Renderer::setInputLayout(InputLayout* inputLayoutIn)
{
    auto inputLayout = static_cast<InputLayoutImpl*>(inputLayoutIn);
    m_immediateContext->IASetInputLayout(inputLayout->m_layout);
}
#endif

void D3D11Renderer::setPrimitiveTopology(PrimitiveTopology topology)
{
    m_immediateContext->IASetPrimitiveTopology(D3DUtil::getPrimitiveTopology(topology));
}

void D3D11Renderer::setVertexBuffers(UInt startSlot, UInt slotCount, IBufferResource*const* buffersIn, const UInt* stridesIn, const UInt* offsetsIn)
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

void D3D11Renderer::setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
{
    DXGI_FORMAT dxFormat = D3DUtil::getMapFormat(indexFormat);
    m_immediateContext->IASetIndexBuffer(((BufferResourceImpl*)buffer)->m_buffer, dxFormat, UINT(offset));
}

void D3D11Renderer::setViewports(UInt count, Viewport const* viewports)
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

void D3D11Renderer::setScissorRects(UInt count, ScissorRect const* rects)
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


void D3D11Renderer::setPipelineState(IPipelineState* state)
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

void D3D11Renderer::draw(UInt vertexCount, UInt startVertex)
{
    _flushGraphicsState();
    m_immediateContext->Draw((UINT)vertexCount, (UINT)startVertex);
}

void D3D11Renderer::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
    _flushGraphicsState();
    m_immediateContext->DrawIndexed((UINT)indexCount, (UINT)startIndex, (INT)baseVertex);
}

Result D3D11Renderer::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
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

    if (desc.pipelineType == PipelineType::Compute)
    {
        auto computeKernel = desc.findKernel(StageType::Compute);

        ComPtr<ID3D11ComputeShader> computeShader;

        {
            ScopeNVAPI scopeNVAPI;
            SLANG_RETURN_ON_FAIL(scopeNVAPI.init(this, 0));
            SLANG_RETURN_ON_FAIL(m_device->CreateComputeShader(computeKernel->codeBegin, computeKernel->getCodeSize(), nullptr, computeShader.writeRef()));
        }

        RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
        shaderProgram->m_computeShader.swap(computeShader);
        initProgramCommon(shaderProgram, desc);

        *outProgram = shaderProgram.detach();
        return SLANG_OK;
    }
    else
    {
        auto vertexKernel = desc.findKernel(StageType::Vertex);
        auto fragmentKernel = desc.findKernel(StageType::Fragment);

        ComPtr<ID3D11VertexShader> vertexShader;
        ComPtr<ID3D11PixelShader> pixelShader;

        {
            ScopeNVAPI scopeNVAPI;
            SLANG_RETURN_ON_FAIL(scopeNVAPI.init(this, 0));

            SLANG_RETURN_ON_FAIL(m_device->CreateVertexShader(vertexKernel->codeBegin, vertexKernel->getCodeSize(), nullptr, vertexShader.writeRef()));
            SLANG_RETURN_ON_FAIL(m_device->CreatePixelShader(fragmentKernel->codeBegin, fragmentKernel->getCodeSize(), nullptr, pixelShader.writeRef()));
        }

        RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
        shaderProgram->m_vertexShader.swap(vertexShader);
        shaderProgram->m_pixelShader.swap(pixelShader);
        initProgramCommon(shaderProgram, desc);

        *outProgram = shaderProgram.detach();
        return SLANG_OK;
    }
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

Result D3D11Renderer::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

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

Result D3D11Renderer::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    RefPtr<ComputePipelineStateImpl> state = new ComputePipelineStateImpl();
    state->init(desc);
    *outState = state.detach();
    return SLANG_OK;
}

void D3D11Renderer::copyBuffer(
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

void D3D11Renderer::dispatchCompute(int x, int y, int z)
{
    _flushComputeState();
    m_immediateContext->Dispatch(x, y, z);
}

Result D3D11Renderer::createDescriptorSetLayout(const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout)
{
    RefPtr<DescriptorSetLayoutImpl> descriptorSetLayoutImpl = new DescriptorSetLayoutImpl();

    UInt counts[int(D3D11DescriptorSlotType::CountOf)] = { 0, };

    UInt rangeCount = desc.slotRangeCount;
    for(UInt rr = 0; rr < rangeCount; ++rr)
    {
        auto rangeDesc = desc.slotRanges[rr];

        DescriptorSetLayoutImpl::RangeInfo rangeInfo;

        UInt slotCount = rangeDesc.count;
        switch(rangeDesc.type)
        {
        default:
            assert(!"invalid slot type");
            return SLANG_FAIL;

        case DescriptorSlotType::Sampler:
            rangeInfo.type = D3D11DescriptorSlotType::Sampler;
            break;

        case DescriptorSlotType::CombinedImageSampler:
            rangeInfo.type = D3D11DescriptorSlotType::CombinedTextureSampler;
            break;

        case DescriptorSlotType::RootConstant:
            // A root-constant range will be treated as if it were
            // a constant-buffer range with a single buffer in it.
            //
            slotCount = 1;
        case DescriptorSlotType::UniformBuffer:
        case DescriptorSlotType::DynamicUniformBuffer:
            rangeInfo.type = D3D11DescriptorSlotType::ConstantBuffer;
            break;

        case DescriptorSlotType::SampledImage:
        case DescriptorSlotType::UniformTexelBuffer:
        case DescriptorSlotType::InputAttachment:
        case DescriptorSlotType::ReadOnlyStorageBuffer:
            rangeInfo.type = D3D11DescriptorSlotType::ShaderResourceView;
            break;

        case DescriptorSlotType::StorageImage:
        case DescriptorSlotType::StorageTexelBuffer:
        case DescriptorSlotType::StorageBuffer:
        case DescriptorSlotType::DynamicStorageBuffer:
            rangeInfo.type = D3D11DescriptorSlotType::UnorderedAccessView;
            break;
        }

        if(rangeInfo.type == D3D11DescriptorSlotType::CombinedTextureSampler)
        {
            auto srvTypeIndex = int(D3D11DescriptorSlotType::ShaderResourceView);
            auto samplerTypeIndex = int(D3D11DescriptorSlotType::Sampler);

            rangeInfo.arrayIndex = counts[srvTypeIndex];
            rangeInfo.pairedSamplerArrayIndex = counts[samplerTypeIndex];

            counts[srvTypeIndex] += slotCount;
            counts[samplerTypeIndex] += slotCount;
        }
        else
        {
            auto typeIndex = int(rangeInfo.type);

            rangeInfo.arrayIndex = counts[typeIndex];
            counts[typeIndex] += slotCount;
        }
        Index rangeIndex = descriptorSetLayoutImpl->m_ranges.getCount();
        descriptorSetLayoutImpl->m_ranges.add(rangeInfo);

        if(rangeDesc.type == DescriptorSlotType::RootConstant)
        {
            // If the range represents a root constant range, then
            // we need to also store the information we will need when
            // allocating a constant buffer to provide backing storage
            // for the range.
            //
            DescriptorSetLayoutImpl::RootConstantRangeInfo rootConstantRangeInfo;
            rootConstantRangeInfo.rangeIndex = rangeIndex;
            rootConstantRangeInfo.size = rangeDesc.count;
            descriptorSetLayoutImpl->m_rootConstantRanges.add(rootConstantRangeInfo);
        }
    }

    for(int ii = 0; ii < int(D3D11DescriptorSlotType::CountOf); ++ii)
    {
        descriptorSetLayoutImpl->m_counts[ii] = counts[ii];
    }

    *outLayout = descriptorSetLayoutImpl.detach();
    return SLANG_OK;
}

Result D3D11Renderer::createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout)
{
    RefPtr<PipelineLayoutImpl> pipelineLayoutImpl = new PipelineLayoutImpl();

    UInt counts[int(D3D11DescriptorSlotType::CountOf)] = { 0, };

    UInt setCount = desc.descriptorSetCount;
    for(UInt ii = 0; ii < setCount; ++ii)
    {
        auto setDesc = desc.descriptorSets[ii];
        PipelineLayoutImpl::DescriptorSetInfo setInfo;

        setInfo.layout = (DescriptorSetLayoutImpl*) setDesc.layout;

        for(int jj = 0; jj < int(D3D11DescriptorSlotType::CountOf); ++jj)
        {
            setInfo.baseIndices[jj] = counts[jj];
            counts[jj] += setInfo.layout->m_counts[jj];
        }

        pipelineLayoutImpl->m_descriptorSets.add(setInfo);
    }

    pipelineLayoutImpl->m_uavCount = UINT(counts[int(D3D11DescriptorSlotType::UnorderedAccessView)]);

    *outLayout = pipelineLayoutImpl.detach();
    return SLANG_OK;
}

Result D3D11Renderer::createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet::Flag::Enum flag, IDescriptorSet** outDescriptorSet)
{
    SLANG_UNUSED(flag);

    auto layoutImpl = (DescriptorSetLayoutImpl*)layout;

    RefPtr<DescriptorSetImpl> descriptorSetImpl = new DescriptorSetImpl();

    descriptorSetImpl->m_renderer = this;
    descriptorSetImpl->m_layout = layoutImpl;
    descriptorSetImpl->m_cbs     .setCount(layoutImpl->m_counts[int(D3D11DescriptorSlotType::ConstantBuffer)]);
    descriptorSetImpl->m_srvs    .setCount(layoutImpl->m_counts[int(D3D11DescriptorSlotType::ShaderResourceView)]);
    descriptorSetImpl->m_uavs    .setCount(layoutImpl->m_counts[int(D3D11DescriptorSlotType::UnorderedAccessView)]);
    descriptorSetImpl->m_samplers.setCount(layoutImpl->m_counts[int(D3D11DescriptorSlotType::Sampler)]);

    // If the layout includes any root constant ranges, then
    // we will need to allocate a constant buffer for each
    // range to provide "backing storage" for its data.
    //
    for(auto rootConstantRange : layoutImpl->m_rootConstantRanges)
    {
        // The root constant range will refer to a descriptor slot
        // range that represents a range with a single constant
        // buffer in it. We need to grab that range so that we
        // know what constant-buffer bindign slot to fill in.
        //
        auto rangeIndex = rootConstantRange.rangeIndex;
        auto bufferRange = layoutImpl->m_ranges[rangeIndex];

        // We will allocate the constant buffer that provides
        // backing storage directly using D3D11 API calls,
        // rather than allocate it as a buffer resource.
        //
        // TODO: We could revisit that decision if allocating
        // a buffer resource proves easier down the line.

        // Note: A D3D11 constant buffer must be a multiple of 16 bytes
        // in size, so we will round up the allocation size to match
        // the requirement.
        //
        UINT size = (UINT) rootConstantRange.size;
        size = (size + 15) & ~15;

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.ByteWidth = size;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

        Slang::ComPtr<ID3D11Buffer> buffer;
        SLANG_RETURN_ON_FAIL(m_device->CreateBuffer(&bufferDesc, nullptr, buffer.writeRef()));

        descriptorSetImpl->m_cbs[bufferRange.arrayIndex] = buffer;
    }

    *outDescriptorSet = descriptorSetImpl.detach();
    return SLANG_OK;
}


void D3D11Renderer::_flushGraphicsState()
{
    auto pipelineType = int(PipelineType::Graphics);
    if (m_framebufferBindingDirty || m_shaderBindingDirty)
    {
        m_framebufferBindingDirty = false;
        m_shaderBindingDirty = false;

        auto pipelineState = static_cast<GraphicsPipelineStateImpl*>(m_currentPipelineState.get());
        auto pipelineLayout =
            static_cast<PipelineLayoutImpl*>(pipelineState->m_pipelineLayout.get());
        auto rtvCount = (UINT)m_currentFramebuffer->renderTargetViews.getCount();
        auto uavCount = pipelineLayout->m_uavCount;
        m_immediateContext->OMSetRenderTargetsAndUnorderedAccessViews(
            rtvCount,
            m_currentFramebuffer->d3dRenderTargetViews.getArrayView().getBuffer(),
            m_currentFramebuffer->d3dDepthStencilView,
            rtvCount,
            uavCount,
            m_uavBindings[pipelineType][0].readRef(),
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

void D3D11Renderer::_flushComputeState()
{
    auto pipelineType = int(PipelineType::Compute);
    if (m_shaderBindingDirty)
    {
        m_shaderBindingDirty = false;

        auto pipelineState = static_cast<ComputePipelineStateImpl*>(m_currentPipelineState.get());
        auto pipelineLayout =
            static_cast<PipelineLayoutImpl*>(pipelineState->m_pipelineLayout.get());

        auto uavCount = pipelineLayout->m_uavCount;

        m_immediateContext->CSSetUnorderedAccessViews(
            0,
            uavCount,
            m_uavBindings[pipelineType][0].readRef(),
            nullptr);
    }
}

void D3D11Renderer::DescriptorSetImpl::setConstantBuffer(UInt range, UInt index, IBufferResource* buffer)
{
    auto bufferImpl = (BufferResourceImpl*) buffer;
    auto& rangeInfo = m_layout->m_ranges[range];

    assert(rangeInfo.type == D3D11DescriptorSlotType::ConstantBuffer);

    m_cbs[rangeInfo.arrayIndex + index] = bufferImpl->m_buffer;
}

void D3D11Renderer::DescriptorSetImpl::setResource(UInt range, UInt index, IResourceView* view)
{
    auto viewImpl = (ResourceViewImpl*)view;
    auto& rangeInfo = m_layout->m_ranges[range];
    auto flatIndex = rangeInfo.arrayIndex + index;

    switch (rangeInfo.type)
    {
    case D3D11DescriptorSlotType::ShaderResourceView:
        {
            if( viewImpl )
            {
                assert(viewImpl->m_type == ResourceViewImpl::Type::SRV);
                auto srvImpl = (ShaderResourceViewImpl*)viewImpl;
                m_srvs[flatIndex] = srvImpl->m_srv;
            }
            else
            {
                m_srvs[flatIndex] = nullptr;
            }
        }
        break;

    case D3D11DescriptorSlotType::UnorderedAccessView:
        {
            if( viewImpl )
            {
                assert(viewImpl->m_type == ResourceViewImpl::Type::UAV);
                auto uavImpl = (UnorderedAccessViewImpl*)viewImpl;
                m_uavs[flatIndex] = uavImpl->m_uav;
            }
            else
            {
                m_uavs[flatIndex] = nullptr;
            }
        }
        break;

    default:
        assert(!"invalid to bind a resource view to this descriptor range");
        break;
    }
}

void D3D11Renderer::DescriptorSetImpl::setSampler(UInt range, UInt index, ISamplerState* sampler)
{
    auto samplerImpl = (SamplerStateImpl*) sampler;
    auto& rangeInfo = m_layout->m_ranges[range];

    assert(rangeInfo.type == D3D11DescriptorSlotType::Sampler);

    m_samplers[rangeInfo.arrayIndex + index] = samplerImpl->m_sampler;
}

void D3D11Renderer::DescriptorSetImpl::setCombinedTextureSampler(
    UInt            range,
    UInt            index,
    IResourceView*   textureView,
    ISamplerState*   sampler)
{
    auto viewImpl = (ResourceViewImpl*) textureView;
    auto samplerImpl = (SamplerStateImpl*)sampler;

    auto& rangeInfo = m_layout->m_ranges[range];
    assert(rangeInfo.type == D3D11DescriptorSlotType::CombinedTextureSampler);

    assert(viewImpl->m_type == ResourceViewImpl::Type::SRV);
    auto srvImpl = (ShaderResourceViewImpl*)viewImpl;
    m_srvs[rangeInfo.arrayIndex + index] = srvImpl->m_srv;

    m_samplers[rangeInfo.arrayIndex + index] = samplerImpl->m_sampler;

    // TODO: need a place to bind the matching sampler...
    m_srvs[rangeInfo.pairedSamplerArrayIndex + index] = srvImpl->m_srv;
}

void D3D11Renderer::DescriptorSetImpl::setRootConstants(
    UInt range,
    UInt offset,
    UInt size,
    void const* data)
{
    // The `range` parameter represents the index of a descriptor
    // slot range in the layout of this descriptor set.
    //
    // A root constant range will have been translated into
    // a constnat buffer range at creation time for the layout.
    //
    auto& rangeInfo = m_layout->m_ranges[range];
    assert(rangeInfo.type == D3D11DescriptorSlotType::ConstantBuffer);

    // At the time the descriptor set was allocated, a
    // constant buffer will have been created and bound
    // into `m_cbs` to provide backing storage for the
    // root constant range.
    //
    auto dxBuffer = m_cbs[rangeInfo.arrayIndex];
    auto dxContext = m_renderer->m_immediateContext;

    // Once we have the buffer that provides backing
    // storage we simply need to map it and write
    // the user-provided data into it.
    //
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = dxContext->Map(dxBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped);
    if( FAILED(hr) )
    {
        SLANG_ASSERT(!"failed to map backing storage for root constant range");
        return;
    }
    memcpy((char*)mapped.pData + offset, data, size);
    dxContext->Unmap(dxBuffer, 0);
}

void D3D11Renderer::setDescriptorSet(PipelineType pipelineType, IPipelineLayout* layout, UInt index, IDescriptorSet* descriptorSet)
{
    auto pipelineLayoutImpl = (PipelineLayoutImpl*)layout;
    auto descriptorSetImpl = (DescriptorSetImpl*) descriptorSet;

    auto descriptorSetLayoutImpl = descriptorSetImpl->m_layout;
    auto& setInfo = pipelineLayoutImpl->m_descriptorSets[index];

    // Note: `setInfo->layout` and `descriptorSetLayoutImpl` need to be compatible

    // TODO: If/when we add per-stage visibility masks, it would be best to organize
    // this as a loop over stages, so that we only do the binding that is required
    // for each stage.

    {
        const int slotType = int(D3D11DescriptorSlotType::ConstantBuffer);
        const UINT slotCount = UINT(setInfo.layout->m_counts[slotType]);
        if(slotCount)
        {
            const UINT startSlot = UINT(setInfo.baseIndices[slotType]);

            auto cbs = descriptorSetImpl->m_cbs[0].readRef();

            m_immediateContext->VSSetConstantBuffers(startSlot, slotCount, cbs);
            // ...
            m_immediateContext->PSSetConstantBuffers(startSlot, slotCount, cbs);

            m_immediateContext->CSSetConstantBuffers(startSlot, slotCount, cbs);
        }
    }

    {
        const int slotType = int(D3D11DescriptorSlotType::ShaderResourceView);
        const UINT slotCount = UINT(setInfo.layout->m_counts[slotType]);
        if(slotCount)
        {
            const UINT startSlot = UINT(setInfo.baseIndices[slotType]);

            auto srvs = descriptorSetImpl->m_srvs[0].readRef();

            m_immediateContext->VSSetShaderResources(startSlot, slotCount, srvs);
            // ...
            m_immediateContext->PSSetShaderResources(startSlot, slotCount, srvs);

            m_immediateContext->CSSetShaderResources(startSlot, slotCount, srvs);
        }
    }

    {
        const int slotType = int(D3D11DescriptorSlotType::Sampler);
        const UINT slotCount = UINT(setInfo.layout->m_counts[slotType]);
        if(slotCount)
        {
            const UINT startSlot = UINT(setInfo.baseIndices[slotType]);

            auto samplers = descriptorSetImpl->m_samplers[0].readRef();

            m_immediateContext->VSSetSamplers(startSlot, slotCount, samplers);
            // ...
            m_immediateContext->PSSetSamplers(startSlot, slotCount, samplers);

            m_immediateContext->CSSetSamplers(startSlot, slotCount, samplers);
        }
    }

    {
        // Note: UAVs are handled differently from other bindings, because
        // D3D11 requires all UAVs to be set with a single call, rather
        // than allowing incremental updates. We will therefore shadow
        // the UAV bindings with `m_uavBindings` and then flush them
        // as needed right before a draw/dispatch.
        //
        const int slotType = int(D3D11DescriptorSlotType::UnorderedAccessView);
        const UInt slotCount = setInfo.layout->m_counts[slotType];
        if(slotCount)
        {
            UInt startSlot = setInfo.baseIndices[slotType];

            auto uavs = descriptorSetImpl->m_uavs[0].readRef();

            for(UINT ii = 0; ii < slotCount; ++ii)
            {
                m_uavBindings[int(pipelineType)][startSlot + ii] = uavs[ii];
            }
            m_shaderBindingDirty = true;
        }
    }
}

}

