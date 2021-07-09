// render-vk.cpp
#include "render-vk.h"

//WORKING:#include "options.h"
#include "../renderer-shared.h"
#include "../transient-resource-heap-base.h"

#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-chunked-list.h"

#include "vk-api.h"
#include "vk-util.h"
#include "vk-device-queue.h"

// Vulkan has a different coordinate system to ogl
// http://anki3d.org/vulkan-coordinate-system/
#ifndef ENABLE_VALIDATION_LAYER
#if _DEBUG
#define ENABLE_VALIDATION_LAYER 1
#else
#define ENABLE_VALIDATION_LAYER 0
#endif
#endif

#ifdef _MSC_VER
#   include <stddef.h>
#   pragma warning(disable: 4996)
#   if (_MSC_VER < 1900)
#       define snprintf sprintf_s
#   endif
#endif

// Undef xlib macros
#ifdef Always
#undef Always
#endif
#ifdef None
#undef None
#endif

namespace gfx {
using namespace Slang;

class VKDevice : public RendererBase
{
public:
    enum
    {
        kMaxRenderTargets = 8,
        kMaxAttachments = kMaxRenderTargets + 1,
        kMaxPushConstantSize = 256,
        kMaxDescriptorSets = 8,
    };
    // Renderer    implementation
    Result initVulkanInstanceAndDevice(bool useValidationLayer);
    virtual SLANG_NO_THROW Result SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc,
        ITransientResourceHeap** outHeap) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebufferLayout(const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc,
        IRenderPassLayout** outRenderPassLayout) override;
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
    virtual Result createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
        override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc,
        IQueryPool** outPool) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* texture,
        ResourceState state,
        ISlangBlob** outBlob,
        size_t* outRowPitch,
        size_t* outPixelSize) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* buffer,
        size_t offset,
        size_t size,
        ISlangBlob** outBlob) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc,
        IAccelerationStructure** outView) override;

    void waitForGpu();
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
    }
        /// Dtor
    ~VKDevice();

public:

    class Buffer
    {
        public:
            /// Initialize a buffer with specified size, and memory props
        Result init(const VulkanApi& api, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqMemoryProperties);

            /// Returns true if has been initialized
        bool isInitialized() const { return m_api != nullptr; }

            // Default Ctor
        Buffer():
            m_api(nullptr)
        {}

            /// Dtor
        ~Buffer()
        {
            if (m_api)
            {
                m_api->vkDestroyBuffer(m_api->m_device, m_buffer, nullptr);
                m_api->vkFreeMemory(m_api->m_device, m_memory, nullptr);
            }
        }

        VkBuffer m_buffer;
        VkDeviceMemory m_memory;
        const VulkanApi* m_api;
    };

    class InputLayoutImpl : public InputLayoutBase
    {
    public:
        List<VkVertexInputAttributeDescription> m_vertexDescs;
        int m_vertexSize;
    };

    class BufferResourceImpl: public BufferResource
    {
    public:
        typedef BufferResource Parent;

        BufferResourceImpl(const IBufferResource::Desc& desc, VKDevice* renderer)
            : Parent(desc)
            , m_renderer(renderer)
        {
            assert(renderer);
        }

        RefPtr<VKDevice> m_renderer;
        Buffer m_buffer;
        Buffer m_uploadBuffer;

        virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
        {
            if (!m_buffer.m_api->vkGetBufferDeviceAddress)
                return 0;
            VkBufferDeviceAddressInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            info.buffer = m_buffer.m_buffer;
            return (DeviceAddress)m_buffer.m_api->vkGetBufferDeviceAddress(
                m_buffer.m_api->m_device, &info);
        }
    };

    class TextureResourceImpl : public TextureResource
    {
    public:
        typedef TextureResource Parent;
        TextureResourceImpl(const Desc& desc, VKDevice* device)
            : Parent(desc)
            , m_device(device)
        {
        }
        ~TextureResourceImpl()
        {
            auto& vkAPI = m_device->m_api;
            if (!m_isWeakImageReference)
            {
                vkAPI.vkFreeMemory(vkAPI.m_device, m_imageMemory, nullptr);
                vkAPI.vkDestroyImage(vkAPI.m_device, m_image, nullptr);
            }
        }

        VkImage m_image = VK_NULL_HANDLE;
        VkFormat m_vkformat = VK_FORMAT_R8G8B8A8_UNORM;
        VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
        bool m_isWeakImageReference = false;
        RefPtr<VKDevice> m_device;
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
        VkSampler m_sampler;
        RefPtr<VKDevice> m_device;
        SamplerStateImpl(VKDevice* device)
            : m_device(device)
        {
        }
        ~SamplerStateImpl()
        {
            m_device->m_api.vkDestroySampler(m_device->m_api.m_device, m_sampler, nullptr);
        }
    };

    class ResourceViewImpl : public ResourceViewBase
    {
    public:
        enum class ViewType
        {
            Texture,
            TexelBuffer,
            PlainBuffer,
        };
    public:
        ResourceViewImpl(ViewType viewType, VKDevice* device)
            : m_type(viewType)
            , m_device(device)
        {
        }
        ViewType            m_type;
        RefPtr<VKDevice> m_device;
    };

    class TextureResourceViewImpl : public ResourceViewImpl
    {
    public:
        TextureResourceViewImpl(VKDevice* device)
            : ResourceViewImpl(ViewType::Texture, device)
        {
        }
        ~TextureResourceViewImpl()
        {
            m_device->m_api.vkDestroyImageView(m_device->m_api.m_device, m_view, nullptr);
        }
        RefPtr<TextureResourceImpl> m_texture;
        VkImageView                 m_view;
        VkImageLayout               m_layout;
    };

    class TexelBufferResourceViewImpl : public ResourceViewImpl
    {
    public:
        TexelBufferResourceViewImpl(VKDevice* device)
            : ResourceViewImpl(ViewType::TexelBuffer, device)
        {
        }
        ~TexelBufferResourceViewImpl()
        {
            m_device->m_api.vkDestroyBufferView(m_device->m_api.m_device, m_view, nullptr);
        }
        RefPtr<BufferResourceImpl>  m_buffer;
        VkBufferView m_view;
    };

    class PlainBufferResourceViewImpl : public ResourceViewImpl
    {
    public:
        PlainBufferResourceViewImpl(VKDevice* device)
            : ResourceViewImpl(ViewType::PlainBuffer, device)
        {
        }
        RefPtr<BufferResourceImpl>  m_buffer;
        VkDeviceSize                offset;
        VkDeviceSize                size;
    };

    class AccelerationStructureImpl : public AccelerationStructureBase
    {
    public:
        VkAccelerationStructureKHR m_vkHandle = VK_NULL_HANDLE;
        RefPtr<BufferResourceImpl> m_buffer;
        VkDeviceSize m_offset;
        VkDeviceSize m_size;
        RefPtr<VKDevice> m_device;
    public:
        virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() override
        {
            return m_buffer->getDeviceAddress() + m_offset;
        }
        ~AccelerationStructureImpl()
        {
            if (m_device)
            {
                m_device->m_api.vkDestroyAccelerationStructureKHR(m_device->m_api.m_device, m_vkHandle, nullptr);
            }
        }
    };

    class FramebufferLayoutImpl : public FramebufferLayoutBase
    {
    public:
        VkRenderPass m_renderPass;
        BreakableReference<VKDevice> m_renderer;
        Array<VkAttachmentDescription, kMaxAttachments> m_attachmentDescs;
        Array<VkAttachmentReference, kMaxRenderTargets> m_colorReferences;
        VkAttachmentReference m_depthReference;
        bool m_hasDepthStencilAttachment;
        uint32_t m_renderTargetCount;

    public:
        ~FramebufferLayoutImpl()
        {
            m_renderer->m_api.vkDestroyRenderPass(m_renderer->m_api.m_device, m_renderPass, nullptr);
        }
        virtual void comFree() override { m_renderer.breakStrongReference(); }
        Result init(VKDevice* renderer, const IFramebufferLayout::Desc& desc)
        {
            m_renderer = renderer;
            m_renderTargetCount = desc.renderTargetCount;
            // Create render pass.
            int numAttachments = m_renderTargetCount;
            m_hasDepthStencilAttachment = (desc.depthStencil!=nullptr);
            if (m_hasDepthStencilAttachment)
            {
                numAttachments++;
            }
            // We need extra space if we have depth buffer
            m_attachmentDescs.setCount(numAttachments);
            for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
            {
                auto& renderTarget = desc.renderTargets[i];
                VkAttachmentDescription& dst = m_attachmentDescs[i];

                dst.flags = 0;
                dst.format = VulkanUtil::getVkFormat(renderTarget.format);
                dst.samples = (VkSampleCountFlagBits)renderTarget.sampleCount;

                // The following load/store/layout settings does not matter.
                // In FramebufferLayout we just need a "compatible" render pass that
                // can be used to create a framebuffer. A framebuffer created
                // with this render pass setting can be used with actual render passes
                // that has a different loadOp/storeOp/layout setting.
                dst.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                dst.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                dst.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            if (desc.depthStencil)
            {
                VkAttachmentDescription& dst = m_attachmentDescs[desc.renderTargetCount];
                dst.flags = 0;
                dst.format = VulkanUtil::getVkFormat(desc.depthStencil->format);
                dst.samples = (VkSampleCountFlagBits)desc.depthStencil->sampleCount;
                dst.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                dst.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                dst.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            Array<VkAttachmentReference, kMaxRenderTargets>& colorReferences = m_colorReferences;
            colorReferences.setCount(desc.renderTargetCount);
            for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
            {
                VkAttachmentReference& dst = colorReferences[i];
                dst.attachment = i;
                dst.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            m_depthReference = VkAttachmentReference{};
            m_depthReference.attachment = desc.renderTargetCount;
            m_depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpassDesc = {};
            subpassDesc.flags = 0;
            subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDesc.inputAttachmentCount = 0u;
            subpassDesc.pInputAttachments = nullptr;
            subpassDesc.colorAttachmentCount = desc.renderTargetCount;
            subpassDesc.pColorAttachments = colorReferences.getBuffer();
            subpassDesc.pResolveAttachments = nullptr;
            subpassDesc.pDepthStencilAttachment =
                m_hasDepthStencilAttachment ? &m_depthReference : nullptr;
            subpassDesc.preserveAttachmentCount = 0u;
            subpassDesc.pPreserveAttachments = nullptr;

            VkRenderPassCreateInfo renderPassCreateInfo = {};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassCreateInfo.attachmentCount = numAttachments;
            renderPassCreateInfo.pAttachments = m_attachmentDescs.getBuffer();
            renderPassCreateInfo.subpassCount = 1;
            renderPassCreateInfo.pSubpasses = &subpassDesc;
            SLANG_VK_RETURN_ON_FAIL(m_renderer->m_api.vkCreateRenderPass(
                m_renderer->m_api.m_device, &renderPassCreateInfo, nullptr, &m_renderPass));
            return SLANG_OK;
        }
    };

    class RenderPassLayoutImpl
        : public IRenderPassLayout
        , public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        IRenderPassLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IRenderPassLayout)
                return static_cast<IRenderPassLayout*>(this);
            return nullptr;
        }

    public:
        VkRenderPass m_renderPass;
        RefPtr<VKDevice> m_renderer;
        ~RenderPassLayoutImpl()
        {
            m_renderer->m_api.vkDestroyRenderPass(
                m_renderer->m_api.m_device, m_renderPass, nullptr);
        }

        static VkAttachmentLoadOp translateLoadOp(IRenderPassLayout::AttachmentLoadOp loadOp)
        {
            switch (loadOp)
            {
            case IRenderPassLayout::AttachmentLoadOp::Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case IRenderPassLayout::AttachmentLoadOp::Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            default:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }
        }

        static VkAttachmentStoreOp translateStoreOp(IRenderPassLayout::AttachmentStoreOp storeOp)
        {
            switch (storeOp)
            {
            case IRenderPassLayout::AttachmentStoreOp::Store:
                return VK_ATTACHMENT_STORE_OP_STORE;
            default:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
        }

        Result init(VKDevice* renderer, const IRenderPassLayout::Desc& desc)
        {
            m_renderer = renderer;

            // Create render pass using load/storeOp and layouts info from `desc`.
            auto framebufferLayout = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
            assert(desc.renderTargetCount == framebufferLayout->m_renderTargetCount);

            // We need extra space if we have depth buffer
            Array<VkAttachmentDescription, kMaxAttachments> attachmentDescs;
            attachmentDescs = framebufferLayout->m_attachmentDescs;
            for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
            {
                VkAttachmentDescription& dst = attachmentDescs[i];
                auto access = desc.renderTargetAccess[i];
                // Fill in loadOp/storeOp and layout from desc.
                dst.loadOp = translateLoadOp(access.loadOp);
                dst.storeOp = translateStoreOp(access.storeOp);
                dst.stencilLoadOp = translateLoadOp(access.stencilLoadOp);
                dst.stencilStoreOp = translateStoreOp(access.stencilStoreOp);
                dst.initialLayout = VulkanUtil::mapResourceStateToLayout(access.initialState);
                dst.finalLayout = VulkanUtil::mapResourceStateToLayout(access.finalState);
            }

            if (framebufferLayout->m_hasDepthStencilAttachment)
            {
                VkAttachmentDescription& dst = attachmentDescs[desc.renderTargetCount];
                auto access = *desc.depthStencilAccess;
                dst.loadOp = translateLoadOp(access.loadOp);
                dst.storeOp = translateStoreOp(access.storeOp);
                dst.stencilLoadOp = translateLoadOp(access.stencilLoadOp);
                dst.stencilStoreOp = translateStoreOp(access.stencilStoreOp);
                dst.initialLayout = VulkanUtil::mapResourceStateToLayout(access.initialState);
                dst.finalLayout = VulkanUtil::mapResourceStateToLayout(access.finalState);
            }

            VkSubpassDescription subpassDesc = {};
            subpassDesc.flags = 0;
            subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDesc.inputAttachmentCount = 0u;
            subpassDesc.pInputAttachments = nullptr;
            subpassDesc.colorAttachmentCount = desc.renderTargetCount;
            subpassDesc.pColorAttachments = framebufferLayout->m_colorReferences.getBuffer();
            subpassDesc.pResolveAttachments = nullptr;
            subpassDesc.pDepthStencilAttachment = framebufferLayout->m_hasDepthStencilAttachment
                                                      ? &framebufferLayout->m_depthReference
                                                      : nullptr;
            subpassDesc.preserveAttachmentCount = 0u;
            subpassDesc.pPreserveAttachments = nullptr;

            VkRenderPassCreateInfo renderPassCreateInfo = {};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassCreateInfo.attachmentCount = (uint32_t)attachmentDescs.getCount();
            renderPassCreateInfo.pAttachments = attachmentDescs.getBuffer();
            renderPassCreateInfo.subpassCount = 1;
            renderPassCreateInfo.pSubpasses = &subpassDesc;
            SLANG_VK_RETURN_ON_FAIL(m_renderer->m_api.vkCreateRenderPass(
                m_renderer->m_api.m_device, &renderPassCreateInfo, nullptr, &m_renderPass));
            return SLANG_OK;
        }
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
        VkFramebuffer m_handle;
        ShortList<ComPtr<IResourceView>> renderTargetViews;
        ComPtr<IResourceView> depthStencilView;
        uint32_t m_width;
        uint32_t m_height;
        RefPtr<VKDevice> m_renderer;
        VkClearValue m_clearValues[kMaxAttachments];
        RefPtr<FramebufferLayoutImpl> m_layout;
    public:
        ~FramebufferImpl()
        {
            m_renderer->m_api.vkDestroyFramebuffer(m_renderer->m_api.m_device, m_handle, nullptr);
        }
        Result init(VKDevice* renderer, const IFramebuffer::Desc& desc)
        {
            m_renderer = renderer;
            auto dsv = desc.depthStencilView
                           ? static_cast<TextureResourceViewImpl*>(desc.depthStencilView)
                           : nullptr;
            // Get frame dimensions from attachments.
            if (dsv)
            {
                // If we have a depth attachment, get frame size from there.
                auto size = dsv->m_texture->getDesc()->size;
                m_width = size.width;
                m_height = size.height;
            }
            else
            {
                // If we don't have a depth attachment, then we must have at least
                // one color attachment. Get frame dimension from there.
                auto size = static_cast<TextureResourceViewImpl*>(desc.renderTargetViews[0])
                                ->m_texture->getDesc()
                                ->size;
                m_width = size.width;
                m_height = size.height;
            }

            // Create render pass.
            int numAttachments = desc.renderTargetCount;
            if (desc.depthStencilView)
                numAttachments++;
            Array<VkImageView, kMaxAttachments> imageViews;
            imageViews.setCount(numAttachments);
            renderTargetViews.setCount(desc.renderTargetCount);
            for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
            {
                auto resourceView =
                    static_cast<TextureResourceViewImpl*>(desc.renderTargetViews[i]);
                renderTargetViews[i] = resourceView;
                imageViews[i] = resourceView->m_view;
                memcpy(
                    &m_clearValues[i],
                    &resourceView->m_texture->getDesc()->optimalClearValue.color,
                    sizeof(gfx::ColorClearValue));
            }

            if (dsv)
            {
                imageViews[desc.renderTargetCount] = dsv->m_view;
                depthStencilView = dsv;
                memcpy(
                    &m_clearValues[desc.renderTargetCount],
                    &dsv->m_texture->getDesc()->optimalClearValue.depthStencil,
                    sizeof(gfx::DepthStencilClearValue));
            }


            // Create framebuffer.
            m_layout = static_cast<FramebufferLayoutImpl*>(desc.layout);
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_layout->m_renderPass;
            framebufferInfo.attachmentCount = numAttachments;
            framebufferInfo.pAttachments = imageViews.getBuffer();
            framebufferInfo.width = m_width;
            framebufferInfo.height = m_height;
            framebufferInfo.layers = 1;

            SLANG_VK_RETURN_ON_FAIL(m_renderer->m_api.vkCreateFramebuffer(
                m_renderer->m_api.m_device, &framebufferInfo, nullptr, &m_handle));
            return SLANG_OK;
        }
    };

    struct BoundVertexBuffer
    {
        RefPtr<BufferResourceImpl> m_buffer;
        int m_stride;
        int m_offset;
    };

    class PipelineStateImpl : public PipelineStateBase
    {
    public:
        PipelineStateImpl(VKDevice* device)
        {
            // Only weakly reference `device` at start.
            // We make it a strong reference only when the pipeline state is exposed to the user.
            // Note that `PipelineState`s may also be created via implicit specialization that
            // happens behind the scenes, and the user will not have access to those specialized
            // pipeline states. Only those pipeline states that are returned to the user needs to
            // hold a strong reference to `device`.
            m_device.setWeakReference(device);
        }
        ~PipelineStateImpl()
        {
            if (m_pipeline != VK_NULL_HANDLE)
            {
                m_device->m_api.vkDestroyPipeline(m_device->m_api.m_device, m_pipeline, nullptr);
            }
        }

        // Turns `m_device` into a strong reference.
        // This method should be called before returning the pipeline state object to
        // external users (i.e. via an `IPipelineState` pointer).
        void establishStrongDeviceReference() { m_device.establishStrongReference(); }

        virtual void comFree() override { m_device.breakStrongReference(); }

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

        BreakableReference<VKDevice> m_device;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
    };

    // In order to bind shader parameters to the correct locations, we need to
    // be able to describe those locations. Most shader parameters in Vulkan
    // simply consume a single `binding`, but we also need to deal with
    // parameters that represent push-constant ranges.
    //
    // In more complex cases we might be binding an entire "sub-object" like
    // a parameter block, an entry point, etc. For the general case, we need
    // to be able to represent a composite offset that includes offsets for
    // each of the cases that Vulkan supports.

        /// A "simple" binding offset that records `binding`, `set`, etc. offsets
    struct SimpleBindingOffset
    {
            /// An offset in GLSL/SPIR-V `binding`s
        uint32_t binding = 0;

            /// The descriptor `set` that the `binding` field should be understood as an index into
        uint32_t bindingSet = 0;

            /// The starting index for any "child" descriptor sets to start at
        uint32_t childSet = 0;

        // The distinction between `bindingSet` and `childSet` above is subtle, but
        // potentially very important when objects contain nested parameter blocks.
        // Consider:
        //
        //      struct Stuff { ... }
        //      struct Things
        //      {
        //          Texture2D t;
        //          ParameterBlock<Stuff> stuff;
        //      }
        //
        //      ParameterBlock<Stuff> gStuff;
        //      Texture2D gTex;
        //      ConstantBuffer<Things> gThings;
        //
        // In this example, the global-scope parameters like `gTex` and `gThings`
        // are expected to be laid out in `set=0`, and we also expect `gStuff`
        // to be laid out as `set=1`. As a result we expect that `gThings.t`
        // will be laid out as `binding=1,set=0` (right after `gTex`), but
        // `gThings.stuff` should be laid out as `set=2`.
        //
        // In this case, when binding `gThings` we would want a binding offset
        // that has a `binding` or 1, a `bindingSet` of 0, and a `childSet` of 2.
        //
        // TODO: Validate that any of this works as intended.

            /// The offset in push-constant ranges (not bytes)
        uint32_t pushConstantRange = 0;

            /// Create a default (zero) offset
        SimpleBindingOffset()
        {}

            /// Create an offset based on offset information in the given Slang `varLayout`
        SimpleBindingOffset(slang::VariableLayoutReflection* varLayout)
        {
            if(varLayout)
            {
                bindingSet = (uint32_t) varLayout->getBindingSpace(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
                binding = (uint32_t) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);

                childSet = (uint32_t) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_REGISTER_SPACE);

                pushConstantRange = (uint32_t) varLayout->getOffset(SLANG_PARAMETER_CATEGORY_PUSH_CONSTANT_BUFFER);
            }
        }

            /// Add any values in the given `offset`
        void operator+=(SimpleBindingOffset const& offset)
        {
            binding += offset.binding;
            bindingSet += offset.bindingSet;
            childSet += offset.childSet;
            pushConstantRange += offset.pushConstantRange;
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
        // ranges in a form that is usable for the Vulkan API:

        struct BindingRangeInfo
        {
            slang::BindingType bindingType;
            Index count;
            Index baseIndex;

                /// An index into the sub-object array if this binding range is treated
                /// as a sub-object.
            Index subObjectIndex;

                /// The `binding` offset to apply for this range
            uint32_t bindingOffset;

                /// The `set` offset to apply for this range
            uint32_t setOffset;

            // Note: The 99% case is that `setOffset` will be zero. For any shader object
            // that was allocated from an ordinary Slang type (anything other than a root
            // shader object in fact), all of the bindings will have been allocated into
            // a single logical descriptor set.
            //
            // TODO: Ideally we could refactor so that only the root shader object layout
            // stores a set offset for its binding ranges, and all other objects skip
            // storing a field that never actually matters.
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
            {
                if(auto pendingLayout = typeLayout->getPendingDataTypeLayout())
                {
                    pendingOrdinaryData = (uint32_t) pendingLayout->getStride();
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

        struct DescriptorSetInfo
        {
            List<VkDescriptorSetLayoutBinding> vkBindings;
            Slang::Int space = -1;
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        };

        struct Builder
        {
        public:
            Builder(VKDevice* renderer)
                : m_renderer(renderer)
            {}

            VKDevice* m_renderer;
            slang::TypeLayoutReflection* m_elementTypeLayout;

            /// The container type of this shader object. When `m_containerType` is
            /// `StructuredBuffer` or `UnsizedArray`, this shader object represents a collection
            /// instead of a single object.
            ShaderObjectContainerType m_containerType = ShaderObjectContainerType::None;

            List<BindingRangeInfo> m_bindingRanges;
            List<SubObjectRangeInfo> m_subObjectRanges;

            Index m_resourceViewCount = 0;
            Index m_samplerCount = 0;
            Index m_combinedTextureSamplerCount = 0;
            Index m_subObjectCount = 0;
            Index m_varyingInputCount = 0;
            Index m_varyingOutputCount = 0;
            List<DescriptorSetInfo> m_descriptorSetBuildInfos;
            Dictionary<Index, Index> m_mapSpaceToDescriptorSetIndex;

                /// The number of descriptor sets allocated by child/descendent objects
            uint32_t m_childDescriptorSetCount = 0;

                /// The total number of `binding`s consumed by this object and its children/descendents
            uint32_t m_totalBindingCount = 0;

                /// The push-constant ranges that belong to this object itself (if any)
            List<VkPushConstantRange> m_ownPushConstantRanges;

                /// The number of push-constant ranges owned by child/descendent objects
            uint32_t m_childPushConstantRangeCount = 0;

            uint32_t m_totalOrdinaryDataSize = 0;

            Index findOrAddDescriptorSet(Index space)
            {
                Index index;
                if (m_mapSpaceToDescriptorSetIndex.TryGetValue(space, index))
                    return index;

                DescriptorSetInfo info = {};
                info.space = space;

                index = m_descriptorSetBuildInfos.getCount();
                m_descriptorSetBuildInfos.add(info);

                m_mapSpaceToDescriptorSetIndex.Add(space, index);
                return index;
            }

            static VkDescriptorType _mapDescriptorType(slang::BindingType slangBindingType)
            {
                switch (slangBindingType)
                {
                case slang::BindingType::PushConstant:
                default:
                    SLANG_ASSERT("unsupported binding type");
                    return VK_DESCRIPTOR_TYPE_MAX_ENUM;

                case slang::BindingType::Sampler:
                    return VK_DESCRIPTOR_TYPE_SAMPLER;
                case slang::BindingType::CombinedTextureSampler:
                    return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                case slang::BindingType::Texture:
                    return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                case slang::BindingType::MutableTexture:
                    return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                case slang::BindingType::TypedBuffer:
                    return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                case slang::BindingType::MutableTypedBuffer:
                    return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                case slang::BindingType::RawBuffer:
                case slang::BindingType::MutableRawBuffer:
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                case slang::BindingType::InputRenderTarget:
                    return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                case slang::BindingType::InlineUniformData:
                    return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
                case slang::BindingType::RayTracingAccelerationStructure:
                    return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                case slang::BindingType::ConstantBuffer:
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                }
            }

                /// Add any descriptor ranges implied by this object containing a leaf
                /// sub-object described by `typeLayout`, at the given `offset`.
            void _addDescriptorRangesAsValue(
                slang::TypeLayoutReflection*    typeLayout,
                BindingOffset const&            offset)
            {
                // First we will scan through all the descriptor sets that the Slang reflection
                // information believes go into making up the given type.
                //
                // Note: We are initializing the sets in order so that their order in our
                // internal data structures should be deterministically based on the order
                // in which they are listed in Slang's reflection information.
                //
                Index descriptorSetCount = typeLayout->getDescriptorSetCount();
                for (Index i = 0; i < descriptorSetCount; ++i)
                {
                    SlangInt descriptorRangeCount = typeLayout->getDescriptorSetDescriptorRangeCount(i);
                    if (descriptorRangeCount == 0)
                        continue;
                    auto descriptorSetIndex = findOrAddDescriptorSet(offset.bindingSet + typeLayout->getDescriptorSetSpaceOffset(i));
                }

                // For actually populating the descriptor sets we prefer to enumerate
                // the binding ranges of the type instead of the descriptor sets.
                //
                Index bindRangeCount = typeLayout->getBindingRangeCount();
                for( Index i = 0; i < bindRangeCount; ++i )
                {
                    auto bindingRangeIndex = i;
                    auto bindingRangeType = typeLayout->getBindingRangeType(bindingRangeIndex);
                    switch(bindingRangeType)
                    {
                    default:
                        break;

                    // We will skip over ranges that represent sub-objects for now, and handle
                    // them in a separate pass.
                    //
                    case slang::BindingType::ParameterBlock:
                    case slang::BindingType::ConstantBuffer:
                    case slang::BindingType::ExistentialValue:
                    case slang::BindingType::PushConstant:
                        continue;
                    }

                    // Given a binding range we are interested in, we will then enumerate
                    // its contained descriptor ranges.

                    Index descriptorRangeCount = typeLayout->getBindingRangeDescriptorRangeCount(bindingRangeIndex);
                    if (descriptorRangeCount == 0)
                        continue;
                    auto slangDescriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(bindingRangeIndex);
                    auto descriptorSetIndex = findOrAddDescriptorSet(offset.bindingSet + typeLayout->getDescriptorSetSpaceOffset(slangDescriptorSetIndex));
                    auto& descriptorSetInfo = m_descriptorSetBuildInfos[descriptorSetIndex];

                    Index firstDescriptorRangeIndex = typeLayout->getBindingRangeFirstDescriptorRangeIndex(bindingRangeIndex);
                    for(Index j = 0; j < descriptorRangeCount; ++j)
                    {
                        Index descriptorRangeIndex = firstDescriptorRangeIndex + j;
                        auto slangDescriptorType = typeLayout->getDescriptorSetDescriptorRangeType(slangDescriptorSetIndex, descriptorRangeIndex);

                        // Certain kinds of descriptor ranges reflected by Slang do not
                        // manifest as descriptors at the Vulkan level, so we will skip those.
                        //
                        switch (slangDescriptorType)
                        {
                        case slang::BindingType::ExistentialValue:
                        case slang::BindingType::InlineUniformData:
                        case slang::BindingType::PushConstant:
                            continue;
                        default:
                            break;
                        }

                        auto vkDescriptorType = _mapDescriptorType(slangDescriptorType);
                        VkDescriptorSetLayoutBinding vkBindingRangeDesc = {};
                        vkBindingRangeDesc.binding = offset.binding + (uint32_t)typeLayout->getDescriptorSetDescriptorRangeIndexOffset(slangDescriptorSetIndex, descriptorRangeIndex);
                        vkBindingRangeDesc.descriptorCount = (uint32_t)typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(slangDescriptorSetIndex, descriptorRangeIndex);
                        vkBindingRangeDesc.descriptorType = vkDescriptorType;
                        vkBindingRangeDesc.stageFlags = VK_SHADER_STAGE_ALL;

                        descriptorSetInfo.vkBindings.add(vkBindingRangeDesc);
                    }
                }

                // We skipped over the sub-object ranges when adding descriptors above,
                // and now we will address that oversight by iterating over just
                // the sub-object ranges.
                //
                Index subObjectRangeCount = typeLayout->getSubObjectRangeCount();
                for(Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount; ++subObjectRangeIndex)
                {
                    auto bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(subObjectRangeIndex);
                    auto bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);

                    auto subObjectTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                    SLANG_ASSERT(subObjectTypeLayout);

                    BindingOffset subObjectRangeOffset = offset;
                    subObjectRangeOffset += BindingOffset(typeLayout->getSubObjectRangeOffset(subObjectRangeIndex));

                    switch(bindingType)
                    {
                    // A `ParameterBlock<X>` never contributes descripto ranges to the
                    // decriptor sets of a parent object.
                    //
                    case slang::BindingType::ParameterBlock:
                    default:
                        break;

                    case slang::BindingType::ExistentialValue:
                        // An interest/existential-typed sub-object range will only contribute descriptor
                        // ranges to a parent object in the case where it has been specialied, which
                        // is precisely the case where the Slang reflection information will tell us
                        // about its "pending" layout.
                        //
                        if(auto pendingTypeLayout = subObjectTypeLayout->getPendingDataTypeLayout())
                        {
                            BindingOffset pendingOffset = BindingOffset(subObjectRangeOffset.pending);
                            _addDescriptorRangesAsValue(pendingTypeLayout, pendingOffset);
                        }
                        break;

                    case slang::BindingType::ConstantBuffer:
                        {
                            // A `ConstantBuffer<X>` range will contribute any nested descriptor
                            // ranges in `X`, along with a leading descriptor range for a
                            // uniform buffer to hold ordinary/uniform data, if there is any.

                            SLANG_ASSERT(subObjectTypeLayout);

                            auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                            SLANG_ASSERT(containerVarLayout);

                            auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                            SLANG_ASSERT(elementVarLayout);

                            auto elementTypeLayout = elementVarLayout->getTypeLayout();
                            SLANG_ASSERT(elementTypeLayout);

                            BindingOffset containerOffset = subObjectRangeOffset;
                            containerOffset += BindingOffset(subObjectTypeLayout->getContainerVarLayout());

                            BindingOffset elementOffset = subObjectRangeOffset;
                            elementOffset += BindingOffset(elementVarLayout);

                            _addDescriptorRangesAsConstantBuffer(elementTypeLayout, containerOffset, elementOffset);
                        }
                        break;

                    case slang::BindingType::PushConstant:
                        {
                            // This case indicates a `ConstantBuffer<X>` that was marked as being
                            // used for push constants.
                            //
                            // Much of the handling is the same as for an ordinary `ConstantBuffer<X>`,
                            // but of course we need to handle the ordinary data part differently.

                            SLANG_ASSERT(subObjectTypeLayout);

                            auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                            SLANG_ASSERT(containerVarLayout);

                            auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                            SLANG_ASSERT(elementVarLayout);

                            auto elementTypeLayout = elementVarLayout->getTypeLayout();
                            SLANG_ASSERT(elementTypeLayout);

                            BindingOffset containerOffset = subObjectRangeOffset;
                            containerOffset += BindingOffset(subObjectTypeLayout->getContainerVarLayout());

                            BindingOffset elementOffset = subObjectRangeOffset;
                            elementOffset += BindingOffset(elementVarLayout);

                            _addDescriptorRangesAsPushConstantBuffer(elementTypeLayout, containerOffset, elementOffset);
                        }
                        break;
                    }

                }
            }

                /// Add the descriptor ranges implied by a `ConstantBuffer<X>` where `X` is
                /// described by `elementTypeLayout`.
                ///
                /// The `containerOffset` and `elementOffset` are the binding offsets that
                /// should apply to the buffer itself and the contents of the buffer, respectively.
                ///
            void _addDescriptorRangesAsConstantBuffer(
                slang::TypeLayoutReflection*    elementTypeLayout,
                BindingOffset const&            containerOffset,
                BindingOffset const&            elementOffset)
            {
                // If the type has ordinary uniform data fields, we need to make sure to create
                // a descriptor set with a constant buffer binding in the case that the shader
                // object is bound as a stand alone parameter block.
                if (elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
                {
                    auto descriptorSetIndex = findOrAddDescriptorSet(containerOffset.bindingSet);
                    auto& descriptorSetInfo = m_descriptorSetBuildInfos[descriptorSetIndex];
                    VkDescriptorSetLayoutBinding vkBindingRangeDesc = {};
                    vkBindingRangeDesc.binding = containerOffset.binding;
                    vkBindingRangeDesc.descriptorCount = 1;
                    vkBindingRangeDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    vkBindingRangeDesc.stageFlags = VK_SHADER_STAGE_ALL;
                    descriptorSetInfo.vkBindings.add(vkBindingRangeDesc);
                }

                _addDescriptorRangesAsValue(elementTypeLayout, elementOffset);
            }

                /// Add the descriptor ranges implied by a `PushConstantBuffer<X>` where `X` is
                /// described by `elementTypeLayout`.
                ///
                /// The `containerOffset` and `elementOffset` are the binding offsets that
                /// should apply to the buffer itself and the contents of the buffer, respectively.
                ///
            void _addDescriptorRangesAsPushConstantBuffer(
                slang::TypeLayoutReflection*    elementTypeLayout,
                BindingOffset const&            containerOffset,
                BindingOffset const&            elementOffset)
            {
                // If the type has ordinary uniform data fields, we need to make sure to create
                // a descriptor set with a constant buffer binding in the case that the shader
                // object is bound as a stand alone parameter block.
                auto ordinaryDataSize = (uint32_t) elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
                if (ordinaryDataSize != 0)
                {
                    auto pushConstantRangeIndex = containerOffset.pushConstantRange;

                    VkPushConstantRange vkPushConstantRange = {};
                    vkPushConstantRange.size = ordinaryDataSize;
                    vkPushConstantRange.stageFlags = VK_SHADER_STAGE_ALL; // TODO: be more clever

                    while(m_ownPushConstantRanges.getCount() <= pushConstantRangeIndex)
                    {
                        VkPushConstantRange emptyRange = { 0 };
                        m_ownPushConstantRanges.add(emptyRange);
                    }

                    m_ownPushConstantRanges[pushConstantRangeIndex] = vkPushConstantRange;
                }

                _addDescriptorRangesAsValue(elementTypeLayout, elementOffset);
            }

                /// Add binding ranges to this shader object layout, as implied by the given `typeLayout`
            void addBindingRanges(
                slang::TypeLayoutReflection* typeLayout)
            {
                SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
                for (SlangInt r = 0; r < bindingRangeCount; ++r)
                {
                    slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                    uint32_t count = (uint32_t)typeLayout->getBindingRangeBindingCount(r);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(r);

                    Index baseIndex = 0;
                    Index subObjectIndex = 0;
                    switch (slangBindingType)
                    {
                    case slang::BindingType::ConstantBuffer:
                    case slang::BindingType::ParameterBlock:
                    case slang::BindingType::ExistentialValue:
                        baseIndex = m_subObjectCount;
                        subObjectIndex = baseIndex;
                        m_subObjectCount += count;
                        break;
                    case slang::BindingType::RawBuffer:
                    case slang::BindingType::MutableRawBuffer:
                        if (slangLeafTypeLayout->getType()->getElementType() != nullptr)
                        {
                            // A structured buffer occupies both a resource slot and
                            // a sub-object slot.
                            subObjectIndex = m_subObjectCount;
                            m_subObjectCount += count;
                        }
                        baseIndex = m_resourceViewCount;
                        m_resourceViewCount += count;
                        break;
                    case slang::BindingType::Sampler:
                        baseIndex = m_samplerCount;
                        m_samplerCount += count;
                        m_totalBindingCount += 1;
                        break;

                    case slang::BindingType::CombinedTextureSampler:
                        baseIndex = m_combinedTextureSamplerCount;
                        m_combinedTextureSamplerCount += count;
                        m_totalBindingCount += 1;
                        break;

                    case slang::BindingType::VaryingInput:
                        baseIndex = m_varyingInputCount;
                        m_varyingInputCount += count;
                        break;

                    case slang::BindingType::VaryingOutput:
                        baseIndex = m_varyingOutputCount;
                        m_varyingOutputCount += count;
                        break;
                    default:
                        baseIndex = m_resourceViewCount;
                        m_resourceViewCount += count;
                        m_totalBindingCount += 1;
                        break;
                    }

                    BindingRangeInfo bindingRangeInfo;
                    bindingRangeInfo.bindingType = slangBindingType;
                    bindingRangeInfo.count = count;
                    bindingRangeInfo.baseIndex = baseIndex;
                    bindingRangeInfo.subObjectIndex = subObjectIndex;

                    // We'd like to extract the information on the GLSL/SPIR-V
                    // `binding` that this range should bind into (or whatever
                    // other specific kind of offset/index is appropriate to it).
                    //
                    // A binding range represents a logical member of the shader
                    // object type, and it may encompass zero or more *descriptor
                    // ranges* that describe how it is physically bound to pipeline
                    // state.
                    //
                    // If the current bindign range is backed by at least one descriptor
                    // range then we can query the binding offset of that descriptor
                    // range. We expect that in the common case there will be exactly
                    // one descriptor range, and we can extract the information easily.
                    //
                    if(typeLayout->getBindingRangeDescriptorRangeCount(r) != 0)
                    {
                        SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
                        SlangInt descriptorRangeIndex = typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

                        auto set = typeLayout->getDescriptorSetSpaceOffset(descriptorSetIndex);
                        auto bindingOffset = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(descriptorSetIndex, descriptorRangeIndex);

                        bindingRangeInfo.setOffset = uint32_t(set);
                        bindingRangeInfo.bindingOffset = uint32_t(bindingOffset);
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

                    // A sub-object range can either represent a sub-object of a known
                    // type, like a `ConstantBuffer<Foo>` or `ParameterBlock<Foo>`
                    // (in which case we can pre-compute a layout to use, based on
                    // the type `Foo`) *or* it can represent a sub-object of some
                    // existential type (e.g., `IBar`) in which case we cannot
                    // know the appropraite type/layout of sub-object to allocate.
                    //
                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    switch(slangBindingType)
                    {
                    default:
                        {
                            auto elementTypeLayout = slangLeafTypeLayout->getElementTypeLayout();
                            ShaderObjectLayoutImpl::createForElementType(
                                m_renderer,
                                elementTypeLayout,
                                subObjectLayout.writeRef());
                        }
                        break;

                    case slang::BindingType::ExistentialValue:
                        if(auto pendingTypeLayout = slangLeafTypeLayout->getPendingDataTypeLayout())
                        {
                            ShaderObjectLayoutImpl::createForElementType(
                                m_renderer,
                                pendingTypeLayout,
                                subObjectLayout.writeRef());
                        }
                        break;
                    }

                    SubObjectRangeInfo subObjectRange;
                    subObjectRange.bindingRangeIndex = bindingRangeIndex;
                    subObjectRange.layout = subObjectLayout;

                    // We will use Slang reflection infromation to extract the offset information
                    // for each sub-object range.
                    //
                    // TODO: We should also be extracting the uniform offset here.
                    //
                    subObjectRange.offset = SubObjectRangeOffset(typeLayout->getSubObjectRangeOffset(r));
                    subObjectRange.stride = SubObjectRangeStride(slangLeafTypeLayout);

                    switch(slangBindingType)
                    {
                    case slang::BindingType::ParameterBlock:
                        m_childDescriptorSetCount += subObjectLayout->getTotalDescriptorSetCount();
                        m_childPushConstantRangeCount += subObjectLayout->getTotalPushConstantRangeCount();
                        break;

                    case slang::BindingType::ConstantBuffer:
                        m_childDescriptorSetCount += subObjectLayout->getChildDescriptorSetCount();
                        m_totalBindingCount += subObjectLayout->getTotalBindingCount();
                        m_childPushConstantRangeCount += subObjectLayout->getTotalPushConstantRangeCount();
                        break;

                    case slang::BindingType::ExistentialValue:
                        if(subObjectLayout)
                        {
                            m_childDescriptorSetCount += subObjectLayout->getChildDescriptorSetCount();
                            m_totalBindingCount += subObjectLayout->getTotalBindingCount();
                            m_childPushConstantRangeCount += subObjectLayout->getTotalPushConstantRangeCount();

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
                        break;

                    default:
                        break;
                    }

                    m_subObjectRanges.add(subObjectRange);
                }
            }

            Result setElementTypeLayout(
                slang::TypeLayoutReflection* typeLayout)
            {
                typeLayout = _unwrapParameterGroups(typeLayout, m_containerType);
                m_elementTypeLayout = typeLayout;

                m_totalOrdinaryDataSize = (uint32_t) typeLayout->getSize();

                // Next we will compute the binding ranges that are used to store
                // the logical contents of the object in memory. These will relate
                // to the descriptor ranges in the various sets, but not always
                // in a one-to-one fashion.

                addBindingRanges(typeLayout);

                // Note: This routine does not take responsibility for
                // adding descriptor ranges at all, because the exact way
                // that descriptor ranges need to be added varies between
                // ordinary shader objects, root shader objects, and entry points.

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
            VKDevice* renderer,
            slang::TypeLayoutReflection* elementType,
            ShaderObjectLayoutImpl** outLayout)
        {
            Builder builder(renderer);
            builder.setElementTypeLayout(elementType);

            // When constructing a shader object layout directly from a reflected
            // type in Slang, we want to compute the descriptor sets and ranges
            // that would be used if this object were bound as a parameter block.
            //
            // It might seem like we need to deal with the other cases for how
            // the shader object might be bound, but the descriptor ranges we
            // compute here will only ever be used in parameter-block case.
            //
            // One important wrinkle is that we know that the parameter block
            // allocated for `elementType` will potentially need a buffer `binding`
            // for any ordinary data it contains.

            bool needsOrdinaryDataBuffer = elementType->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0;
            uint32_t ordinaryDataBufferCount = needsOrdinaryDataBuffer ? 1 : 0;

            // When binding the object, we know that the ordinary data buffer will
            // always use a the first available `binding`, so its offset will be
            // all zeroes.
            //
            BindingOffset containerOffset;

            // In contrast, the `binding`s used by all the other entries in the
            // parameter block will need to be offset by one if there was
            // an ordinary data buffer.
            //
            BindingOffset elementOffset;
            elementOffset.binding = ordinaryDataBufferCount;

            // Furthermore, any `binding`s that arise due to "pending" data
            // in the type of the object (due to specialization for existential types)
            // will need to come after all the other `binding`s that were
            // part of the "primary" (unspecialized) data.
            //
            uint32_t primaryDescriptorCount = ordinaryDataBufferCount
                + (uint32_t) elementType->getSize(SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);
            elementOffset.pending.binding = primaryDescriptorCount;

            // Once we've computed the offset information, we simply add the
            // descriptor ranges as if things were declared as a `ConstantBuffer<X>`,
            // since that is how things will be laid out inside the parameter block.
            //
            builder._addDescriptorRangesAsConstantBuffer(elementType, containerOffset, elementOffset);
            return builder.build(outLayout);
        }

        ~ShaderObjectLayoutImpl()
        {
            for (auto& descSetInfo : m_descriptorSetInfos)
            {
                getDevice()->m_api.vkDestroyDescriptorSetLayout(
                    getDevice()->m_api.m_device, descSetInfo.descriptorSetLayout, nullptr);
            }
        }

            /// Get the number of descriptor sets that are allocated for this object itself
            /// (if it needed to be bound as a parameter block).
            ///
        uint32_t getOwnDescriptorSetCount() { return uint32_t(m_descriptorSetInfos.getCount()); }

            /// Get information about the descriptor sets that would be allocated to
            /// represent this object itself as a parameter block.
            ///
        List<DescriptorSetInfo> const& getOwnDescriptorSets() { return m_descriptorSetInfos; }

            /// Get the number of descriptor sets that would need to be allocated and bound
            /// to represent the children of this object if it were bound as a parameter
            /// block.
            ///
            /// To a first approximation, this is the number of (transitive) children
            /// that are declared as `ParameterBlock<X>`.
            ///
        uint32_t getChildDescriptorSetCount() { return m_childDescriptorSetCount; }

            /// Get the total number of descriptor sets that would need to be allocated and bound
            /// to represent this object and its children (transitively) as a parameter block.
            ///
        uint32_t getTotalDescriptorSetCount() { return getOwnDescriptorSetCount() + getChildDescriptorSetCount(); }

            /// Get the total number of `binding`s required to represent this type and its
            /// (transitive) children.
            ///
            /// Note that this count does *not* include bindings that would be part of child
            /// parameter blocks, nor does it include the binding for an ordinary data buffer,
            /// if one is needed.
            ///
        uint32_t getTotalBindingCount() { return m_totalBindingCount; }


            /// Get the list of push constant ranges required to bind the state of this object itself.
        List<VkPushConstantRange> const& getOwnPushConstantRanges() const { return m_ownPushConstantRanges; }

            /// Get the number of push constant ranges required to bind the state of this object itself.
        uint32_t getOwnPushConstantRangeCount() { return (uint32_t) m_ownPushConstantRanges.getCount(); }

            /// Get the number of push constant ranges required to bind the state of the (transitive)
            /// children of this object.
        uint32_t getChildPushConstantRangeCount() { return m_childPushConstantRangeCount; }

            /// Get the total number of push constant ranges required to bind the state of this object
            /// and its (transitive) children.
        uint32_t getTotalPushConstantRangeCount() { return getOwnPushConstantRangeCount() + getChildPushConstantRangeCount(); }

        uint32_t getTotalOrdinaryDataSize() const { return m_totalOrdinaryDataSize; }

        List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

        Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

        BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

        Index getResourceViewCount() { return m_resourceViewCount; }
        Index getSamplerCount() { return m_samplerCount; }
        Index getCombinedTextureSamplerCount() { return m_combinedTextureSamplerCount; }
        Index getSubObjectCount() { return m_subObjectCount; }

        SubObjectRangeInfo const& getSubObjectRange(Index index)
        {
            return m_subObjectRanges[index];
        }
        List<SubObjectRangeInfo> const& getSubObjectRanges() { return m_subObjectRanges; }

        VKDevice* getDevice() { return static_cast<VKDevice*>(m_renderer); }

        slang::TypeReflection* getType() { return m_elementTypeLayout->getType(); }

    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            initBase(renderer, builder->m_elementTypeLayout);

            m_bindingRanges = builder->m_bindingRanges;

            m_descriptorSetInfos = _Move(builder->m_descriptorSetBuildInfos);
            m_ownPushConstantRanges = builder->m_ownPushConstantRanges;
            m_resourceViewCount = builder->m_resourceViewCount;
            m_samplerCount = builder->m_samplerCount;
            m_combinedTextureSamplerCount = builder->m_combinedTextureSamplerCount;
            m_childDescriptorSetCount = builder->m_childDescriptorSetCount;
            m_totalBindingCount = builder->m_totalBindingCount;
            m_subObjectCount = builder->m_subObjectCount;
            m_subObjectRanges = builder->m_subObjectRanges;
            m_totalOrdinaryDataSize = builder->m_totalOrdinaryDataSize;

            m_containerType = builder->m_containerType;

            // Create VkDescriptorSetLayout for all descriptor sets.
            for (auto& descriptorSetInfo : m_descriptorSetInfos)
            {
                VkDescriptorSetLayoutCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                createInfo.pBindings = descriptorSetInfo.vkBindings.getBuffer();
                createInfo.bindingCount = (uint32_t)descriptorSetInfo.vkBindings.getCount();
                VkDescriptorSetLayout vkDescSetLayout;
                SLANG_RETURN_ON_FAIL(renderer->m_api.vkCreateDescriptorSetLayout(
                    renderer->m_api.m_device, &createInfo, nullptr, &vkDescSetLayout));
                descriptorSetInfo.descriptorSetLayout = vkDescSetLayout;
            }
            return SLANG_OK;
        }

        List<DescriptorSetInfo> m_descriptorSetInfos;
        List<BindingRangeInfo> m_bindingRanges;
        Index m_resourceViewCount = 0;
        Index m_samplerCount = 0;
        Index m_combinedTextureSamplerCount = 0;
        Index m_subObjectCount = 0;
        List<VkPushConstantRange> m_ownPushConstantRanges;
        uint32_t m_childPushConstantRangeCount = 0;

        uint32_t m_childDescriptorSetCount = 0;
        uint32_t m_totalBindingCount = 0;
        uint32_t m_totalOrdinaryDataSize = 0;

        List<SubObjectRangeInfo> m_subObjectRanges;
    };

    class EntryPointLayout : public ShaderObjectLayoutImpl
    {
        typedef ShaderObjectLayoutImpl Super;

    public:
        struct Builder : Super::Builder
        {
            Builder(VKDevice* device)
                : Super::Builder(device)
            {}

            Result build(EntryPointLayout** outLayout)
            {
                RefPtr<EntryPointLayout> layout = new EntryPointLayout();
                SLANG_RETURN_ON_FAIL(layout->_init(this));

                returnRefPtrMove(outLayout, layout);
                return SLANG_OK;
            }

            void addEntryPointParams(slang::EntryPointLayout* entryPointLayout)
            {
                m_slangEntryPointLayout = entryPointLayout;
                setElementTypeLayout(entryPointLayout->getTypeLayout());
                m_shaderStageFlag = VulkanUtil::getShaderStage(entryPointLayout->getStage());

                // Note: we do not bother adding any descriptor sets/ranges here,
                // because the descriptor ranges of an entry point will simply
                // be allocated as part of the descriptor sets for the root
                // shader object.
            }

            slang::EntryPointLayout* m_slangEntryPointLayout = nullptr;

            VkShaderStageFlags m_shaderStageFlag;
        };

        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            SLANG_RETURN_ON_FAIL(Super::_init(builder));

            m_slangEntryPointLayout = builder->m_slangEntryPointLayout;
            m_shaderStageFlag = builder->m_shaderStageFlag;
            return SLANG_OK;
        }

        VkShaderStageFlags getShaderStageFlag() const { return m_shaderStageFlag; }

        slang::EntryPointLayout* getSlangLayout() const { return m_slangEntryPointLayout; };

        slang::EntryPointLayout* m_slangEntryPointLayout;
        VkShaderStageFlags m_shaderStageFlag;
    };

    class RootShaderObjectLayout : public ShaderObjectLayoutImpl
    {
        typedef ShaderObjectLayoutImpl Super;

    public:
        ~RootShaderObjectLayout()
        {
            if (m_pipelineLayout)
            {
                m_renderer->m_api.vkDestroyPipelineLayout(
                    m_renderer->m_api.m_device, m_pipelineLayout, nullptr);
            }
        }

            /// Information stored for each entry point of the program
        struct EntryPointInfo
        {
                /// Layout of the entry point
            RefPtr<EntryPointLayout> layout;

                /// Offset for binding the entry point, relative to the start of the program
            BindingOffset offset;
        };

        struct Builder : Super::Builder
        {
            Builder(
                VKDevice* renderer,
                slang::IComponentType* program,
                slang::ProgramLayout* programLayout)
                : Super::Builder(renderer)
                , m_program(program)
                , m_programLayout(programLayout)
            {}

            Result build(RootShaderObjectLayout** outLayout)
            {
                RefPtr<RootShaderObjectLayout> layout = new RootShaderObjectLayout();
                SLANG_RETURN_ON_FAIL(layout->_init(this));
                returnRefPtrMove(outLayout, layout);
                return SLANG_OK;
            }

            void addGlobalParams(slang::VariableLayoutReflection* globalsLayout)
            {
                setElementTypeLayout(globalsLayout->getTypeLayout());

                // We need to populate our descriptor sets/ranges with information
                // from the layout of the global scope.
                //
                // While we expect that the parameter in the global scope start
                // at an offset of zero, it is also worth querying the offset
                // information because it could impact the locations assigned
                // to "pending" data in the case of static specialization.
                //
                BindingOffset offset(globalsLayout);

                // Note: We are adding descriptor ranges here based directly on
                // the type of the global-scope layout. The type layout for the
                // global scope will either be something like a `struct GlobalParams`
                // that contains all the global-scope parameters or a `ConstantBuffer<GlobalParams>`
                // and in either case the `_addDescriptorRangesAsValue` can properly
                // add all the ranges implied.
                //
                // As a result we don't require any special-case logic here to
                // deal with the possibility of a "default" constant buffer allocated
                // for global-scope parameters of uniform/ordinary type.
                //
                _addDescriptorRangesAsValue(globalsLayout->getTypeLayout(), offset);

                // We want to keep track of the offset that was applied to "pending"
                // data because we will need it again later when it comes time to
                // actually bind things.
                //
                m_pendingDataOffset = offset.pending;
            }

            void addEntryPoint(EntryPointLayout* entryPointLayout)
            {
                auto slangEntryPointLayout = entryPointLayout->getSlangLayout();
                auto entryPointVarLayout = slangEntryPointLayout->getVarLayout();

                // The offset information for each entry point needs to
                // be adjusted by any offset for "pending" data that
                // was recorded in the global-scope layout.
                //
                // TODO(tfoley): Double-check that this is correct.

                BindingOffset entryPointOffset(entryPointVarLayout);
                entryPointOffset.pending += m_pendingDataOffset;

                EntryPointInfo info;
                info.layout = entryPointLayout;
                info.offset = entryPointOffset;

                // Similar to the case for the global scope, we expect the
                // type layout for the entry point parameters to be either
                // a `struct EntryPointParams` or a `PushConstantBuffer<EntryPointParams>`.
                // Rather than deal with the different cases here, we will
                // trust the `_addDescriptorRangesAsValue` code to handle
                // either case correctly.
                //
                _addDescriptorRangesAsValue(entryPointVarLayout->getTypeLayout(), entryPointOffset);

                m_entryPoints.add(info);
            }

            slang::IComponentType* m_program;
            slang::ProgramLayout* m_programLayout;
            List<EntryPointInfo> m_entryPoints;

                /// Offset to apply to "pending" data from this object, sub-objects, and entry points
            SimpleBindingOffset m_pendingDataOffset;
        };

        Index findEntryPointIndex(VkShaderStageFlags stage)
        {
            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPoint = m_entryPoints[i];
                if (entryPoint.layout->getShaderStageFlag() == stage)
                    return i;
            }
            return -1;
        }

        EntryPointInfo const& getEntryPoint(Index index) { return m_entryPoints[index]; }

        List<EntryPointInfo> const& getEntryPoints() const { return m_entryPoints; }

        static Result create(
            VKDevice* renderer,
            slang::IComponentType* program,
            slang::ProgramLayout* programLayout,
            RootShaderObjectLayout** outLayout)
        {
            RootShaderObjectLayout::Builder builder(renderer, program, programLayout);
            builder.addGlobalParams(programLayout->getGlobalParamsVarLayout());

            SlangInt entryPointCount = programLayout->getEntryPointCount();
            for (SlangInt e = 0; e < entryPointCount; ++e)
            {
                auto slangEntryPoint = programLayout->getEntryPointByIndex(e);

                EntryPointLayout::Builder entryPointBuilder(renderer);
                entryPointBuilder.addEntryPointParams(slangEntryPoint);

                RefPtr<EntryPointLayout> entryPointLayout;
                SLANG_RETURN_ON_FAIL(entryPointBuilder.build(entryPointLayout.writeRef()));

                builder.addEntryPoint(entryPointLayout);
            }

            SLANG_RETURN_ON_FAIL(builder.build(outLayout));

            return SLANG_OK;
        }

        SimpleBindingOffset const& getPendingDataOffset() const { return m_pendingDataOffset; }

        slang::IComponentType* getSlangProgram() const { return m_program; }
        slang::ProgramLayout* getSlangProgramLayout() const { return m_programLayout; }

            /// Get all of the push constant ranges that will be bound for this object and all (transitive) sub-objects
        List<VkPushConstantRange> const& getAllPushConstantRanges() { return m_allPushConstantRanges; }

    protected:
        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            SLANG_RETURN_ON_FAIL(Super::_init(builder));

            m_program = builder->m_program;
            m_programLayout = builder->m_programLayout;
            m_entryPoints = _Move(builder->m_entryPoints);
            m_pendingDataOffset = builder->m_pendingDataOffset;
            m_renderer = renderer;

            // If the program has unbound specialization parameters,
            // then we will avoid creating a final Vulkan pipeline layout.
            //
            // TODO: We should really create the information necessary
            // for binding as part of a separate object, so that we have
            // a clean seperation between what is needed for writing into
            // a shader object vs. what is needed for binding it to the
            // pipeline. We eventually need to be able to create bindable
            // state objects from unspecialized programs, in order to
            // support dynamic dispatch.
            //
            if (m_program->getSpecializationParamCount() != 0)
                return SLANG_OK;

            // Otherwise, we need to create a final (bindable) layout.
            //
            // We will use a recursive walk to collect all the `VkDescriptorSetLayout`s
            // that are required for the global scope, sub-objects, and entry points.
            //
            SLANG_RETURN_ON_FAIL(addAllDescriptorSets());

            // We will also use a recursive walk to collect all the push-constant
            // ranges needed for this object, sub-objects, and entry points.
            //
            SLANG_RETURN_ON_FAIL(addAllPushConstantRanges());

            // Once we've collected the information across the entire
            // tree of sub-objects

            // Now call Vulkan API to create a pipeline layout.
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)m_vkDescriptorSetLayouts.getCount();
            pipelineLayoutCreateInfo.pSetLayouts = m_vkDescriptorSetLayouts.getBuffer();
            if (m_allPushConstantRanges.getCount())
            {
                pipelineLayoutCreateInfo.pushConstantRangeCount =
                    (uint32_t)m_allPushConstantRanges.getCount();
                pipelineLayoutCreateInfo.pPushConstantRanges =
                    m_allPushConstantRanges.getBuffer();
            }
            SLANG_RETURN_ON_FAIL(m_renderer->m_api.vkCreatePipelineLayout(
                m_renderer->m_api.m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
            return SLANG_OK;
        }

            /// Add all the descriptor sets implied by this root object and sub-objects
        Result addAllDescriptorSets()
        {
            SLANG_RETURN_ON_FAIL(addAllDescriptorSetsRec(this));

            // Note: the descriptor ranges/sets for direct entry point parameters
            // were already enumerated into the ranges/sets of the root object itself,
            // so we don't wnat to add them again.
            //
            // We do however have to deal with the possibility that an entry
            // point could introduce "child" descriptor sets, e.g., because it
            // has a `ParameterBlock<X>` parameter.
            //
            for(auto& entryPoint : getEntryPoints())
            {
                SLANG_RETURN_ON_FAIL(addChildDescriptorSetsRec(entryPoint.layout));
            }

            return SLANG_OK;
        }

            /// Recurisvely add descriptor sets defined by `layout` and sub-objects
        Result addAllDescriptorSetsRec(ShaderObjectLayoutImpl* layout)
        {
            // TODO: This logic assumes that descriptor sets are all contiguous
            // and have been allocated in a global order that matches the order
            // of enumeration here.

            for (auto& descSetInfo : layout->getOwnDescriptorSets())
            {
                m_vkDescriptorSetLayouts.add(descSetInfo.descriptorSetLayout);
            }

            SLANG_RETURN_ON_FAIL(addChildDescriptorSetsRec(layout));
            return SLANG_OK;
        }

            /// Recurisvely add descriptor sets defined by sub-objects of `layout`
        Result addChildDescriptorSetsRec(ShaderObjectLayoutImpl* layout)
        {
            for (auto& subObject : layout->getSubObjectRanges())
            {
                auto bindingRange = layout->getBindingRange(subObject.bindingRangeIndex);
                switch(bindingRange.bindingType)
                {
                case slang::BindingType::ParameterBlock:
                    SLANG_RETURN_ON_FAIL(addAllDescriptorSetsRec(subObject.layout));
                    break;

                default:
                    if(auto subObjectLayout = subObject.layout)
                    {
                        SLANG_RETURN_ON_FAIL(addChildDescriptorSetsRec(subObject.layout));
                    }
                    break;
                }
            }

            return SLANG_OK;
        }

            /// Add all the push-constant ranges implied by this root object and sub-objects
        Result addAllPushConstantRanges()
        {
            SLANG_RETURN_ON_FAIL(addAllPushConstantRangesRec(this));

            for(auto& entryPoint : getEntryPoints())
            {
                SLANG_RETURN_ON_FAIL(addChildPushConstantRangesRec(entryPoint.layout));
            }

            return SLANG_OK;
        }

            /// Recurisvely add push-constant ranges defined by `layout` and sub-objects
        Result addAllPushConstantRangesRec(ShaderObjectLayoutImpl* layout)
        {
            // TODO: This logic assumes that push-constant ranges are all contiguous
            // and have been allocated in a global order that matches the order
            // of enumeration here.

            for (auto pushConstantRange : layout->getOwnPushConstantRanges())
            {
                pushConstantRange.offset = m_totalPushConstantSize;
                m_totalPushConstantSize += pushConstantRange.size;

                m_allPushConstantRanges.add(pushConstantRange);
            }

            SLANG_RETURN_ON_FAIL(addChildPushConstantRangesRec(layout));
            return SLANG_OK;
        }

            /// Recurisvely add push-constant ranges defined by sub-objects of `layout`
        Result addChildPushConstantRangesRec(ShaderObjectLayoutImpl* layout)
        {
            for (auto& subObject : layout->getSubObjectRanges())
            {
                if(auto subObjectLayout = subObject.layout)
                {
                    SLANG_RETURN_ON_FAIL(addAllPushConstantRangesRec(subObject.layout));
                }
            }

            return SLANG_OK;
        }

    public:
        ComPtr<slang::IComponentType> m_program;
        slang::ProgramLayout* m_programLayout = nullptr;
        List<EntryPointInfo> m_entryPoints;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        Array<VkDescriptorSetLayout, kMaxDescriptorSets> m_vkDescriptorSetLayouts;
        List<VkPushConstantRange> m_allPushConstantRanges;
        uint32_t m_totalPushConstantSize = 0;

        SimpleBindingOffset m_pendingDataOffset;
        VKDevice* m_renderer = nullptr;
    };
    
    class ShaderProgramImpl : public ShaderProgramBase
    {
    public:
        ShaderProgramImpl(VKDevice* device, PipelineType pipelineType)
            : m_device(device)
            , m_pipelineType(pipelineType)
        {
            for (auto& shaderModule : m_modules)
                shaderModule = VK_NULL_HANDLE;
        }

        ~ShaderProgramImpl()
        {
            for (auto shaderModule : m_modules)
            {
                if (shaderModule != VK_NULL_HANDLE)
                {
                    m_device->m_api.vkDestroyShaderModule(
                        m_device->m_api.m_device, shaderModule, nullptr);
                }
            }
        }

        virtual void comFree() override
        {
            m_device.breakStrongReference();
        }

        BreakableReference<VKDevice> m_device;

        PipelineType m_pipelineType;

        Array<VkPipelineShaderStageCreateInfo, 8> m_stageCreateInfos;
        Array<ComPtr<ISlangBlob>, 8> m_codeBlobs; //< To keep storage of code in scope
        Array<VkShaderModule, 8> m_modules;
        RefPtr<RootShaderObjectLayout> m_rootObjectLayout;
    };

    class CommandBufferImpl;

    class PipelineCommandEncoder : public RefObject
    {
    public:
        CommandBufferImpl* m_commandBuffer;
        VkCommandBuffer m_vkCommandBuffer;
        VkCommandBuffer m_vkPreCommandBuffer = VK_NULL_HANDLE;
        VkPipeline m_boundPipelines[3] = {};
        VKDevice* m_device = nullptr;
        RefPtr<PipelineStateImpl> m_currentPipeline;

        static int getBindPointIndex(VkPipelineBindPoint bindPoint)
        {
            switch (bindPoint)
            {
            case VK_PIPELINE_BIND_POINT_GRAPHICS:
                return 0;
            case VK_PIPELINE_BIND_POINT_COMPUTE:
                return 1;
            case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
                return 2;
            default:
                assert(!"unknown pipeline type.");
                return -1;
            }
        }
        VulkanApi* m_api;

        void init(CommandBufferImpl* commandBuffer);

        void endEncodingImpl()
        {
            for (auto& pipeline : m_boundPipelines)
                pipeline = VK_NULL_HANDLE;
        }

        static void _uploadBufferData(
            VkCommandBuffer commandBuffer,
            BufferResourceImpl* buffer,
            size_t offset,
            size_t size,
            void* data)
        {
            auto& api = buffer->m_renderer->m_api;

            assert(buffer->m_uploadBuffer.isInitialized());

            void* mappedData = nullptr;
            SLANG_VK_CHECK(api.vkMapMemory(
                api.m_device, buffer->m_uploadBuffer.m_memory, offset, size, 0, &mappedData));
            memcpy(mappedData, data, size);
            api.vkUnmapMemory(api.m_device, buffer->m_uploadBuffer.m_memory);

            // Copy from staging buffer to real buffer
            VkBufferCopy copyInfo = {};
            copyInfo.size = size;
            copyInfo.dstOffset = offset;
            copyInfo.srcOffset = offset;
            api.vkCmdCopyBuffer(
                commandBuffer,
                buffer->m_uploadBuffer.m_buffer,
                buffer->m_buffer.m_buffer,
                1,
                &copyInfo);
        }

        void uploadBufferDataImpl(IBufferResource* buffer, size_t offset, size_t size, void* data)
        {
            m_vkPreCommandBuffer = m_commandBuffer->getPreCommandBuffer();
            _uploadBufferData(
                m_vkPreCommandBuffer, static_cast<BufferResourceImpl*>(buffer), offset, size, data);
        }

        Result bindRootShaderObjectImpl(VkPipelineBindPoint bindPoint);

        Result setPipelineStateImpl(IPipelineState* state, IShaderObject** outRootObject)
        {
            m_currentPipeline = static_cast<PipelineStateImpl*>(state);
            SLANG_RETURN_ON_FAIL(m_commandBuffer->m_rootObject.init(
                m_commandBuffer->m_renderer,
                m_currentPipeline->getProgram<ShaderProgramImpl>()->m_rootObjectLayout));
            *outRootObject = &m_commandBuffer->m_rootObject;
            return SLANG_OK;
        }

        void flushBindingState(VkPipelineBindPoint pipelineBindPoint)
        {
            auto& api = *m_api;

            // Get specialized pipeline state and bind it.
            //
            RefPtr<PipelineStateBase> newPipeline;
            m_device->maybeSpecializePipeline(
                m_currentPipeline, &m_commandBuffer->m_rootObject, newPipeline);
            PipelineStateImpl* newPipelineImpl = static_cast<PipelineStateImpl*>(newPipeline.Ptr());

            bindRootShaderObjectImpl(pipelineBindPoint);

            auto pipelineBindPointId = getBindPointIndex(pipelineBindPoint);
            if (m_boundPipelines[pipelineBindPointId] != newPipelineImpl->m_pipeline)
            {
                api.vkCmdBindPipeline(
                    m_vkCommandBuffer, pipelineBindPoint, newPipelineImpl->m_pipeline);
                m_boundPipelines[pipelineBindPointId] = newPipelineImpl->m_pipeline;
            }
        }
    };

        /// Context information required when binding shader objects to the pipeline
    struct RootBindingContext
    {
            /// The pipeline layout being used for binding
        VkPipelineLayout pipelineLayout;

            /// An allocator to use for descriptor sets during binding
        DescriptorSetAllocator* descriptorSetAllocator;

            /// The dvice being used
        VKDevice* device;

            /// The descriptor sets that are being allocated and bound
        VkDescriptorSet* descriptorSets;

            /// Information about all the push-constant ranges that should be bound
        ConstArrayView<VkPushConstantRange> pushConstantRanges;
    };

    class ShaderObjectImpl : public ShaderObjectBaseImpl<ShaderObjectImpl, ShaderObjectLayoutImpl, SimpleShaderObjectData>
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
            if (resourceView->getViewDesc()->type == IResourceView::Type::AccelerationStructure)
            {
                m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] =
                    static_cast<AccelerationStructureImpl*>(resourceView);
            }
            else
            {
                m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] =
                    static_cast<ResourceViewImpl*>(resourceView);
            }
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

            m_samplers[bindingRange.baseIndex + offset.bindingArrayIndex] =
                static_cast<SamplerStateImpl*>(sampler);
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

            auto& slot =
                m_combinedTextureSamplers[bindingRange.baseIndex + offset.bindingArrayIndex];
            slot.textureView = static_cast<TextureResourceViewImpl*>(textureView);
            slot.sampler = static_cast<SamplerStateImpl*>(sampler);
            return SLANG_OK;
        }

    protected:
        friend class RootShaderObjectLayout;

        Result init(IDevice* device, ShaderObjectLayoutImpl* layout)
        {
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

#if 0
        // If the layout tells us there are any descriptor sets to
        // allocate, then we do so now.
        //
        for(auto descriptorSetInfo : layout->getDescriptorSets())
        {
            RefPtr<DescriptorSet> descriptorSet;
            SLANG_RETURN_ON_FAIL(renderer->createDescriptorSet(descriptorSetInfo->layout, descriptorSet.writeRef()));
            m_descriptorSets.add(descriptorSet);
        }
#endif

            m_resourceViews.setCount(layout->getResourceViewCount());
            m_samplers.setCount(layout->getSamplerCount());
            m_combinedTextureSamplers.setCount(layout->getCombinedTextureSamplerCount());

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

        /// Write the uniform/ordinary data of this object into the given `dest` buffer at the given
        /// `offset`
        Result _writeOrdinaryData(
            PipelineCommandEncoder* encoder,
            IBufferResource* buffer,
            size_t offset,
            size_t destSize,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            auto src = m_data.getBuffer();
            auto srcSize = size_t(m_data.getCount());

            SLANG_ASSERT(srcSize <= destSize);

            encoder->uploadBufferDataImpl(buffer, offset, srcSize, src);

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

                for (Slang::Index i = 0; i < count; ++i)
                {
                    auto subObject = m_objects[bindingRangeInfo.subObjectIndex + i];

                    RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
                    SLANG_RETURN_ON_FAIL(
                        subObject->_getSpecializedLayout(subObjectLayout.writeRef()));

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

    public:
        struct CombinedTextureSamplerSlot
        {
            RefPtr<TextureResourceViewImpl> textureView;
            RefPtr<SamplerStateImpl> sampler;
            operator bool() { return textureView && sampler; }
        };

            /// Write a single desriptor using the Vulkan API
        static inline void writeDescriptor(
            RootBindingContext&         context,
            VkWriteDescriptorSet const& write)
        {
            auto device = context.device;
            device->m_api.vkUpdateDescriptorSets(
                device->m_device,
                1,
                &write,
                0,
                nullptr);
        }

        static void writeBufferDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            BufferResourceImpl* buffer,
            size_t bufferOffset,
            size_t bufferSize)
        {
            auto descriptorSet = context.descriptorSets[offset.bindingSet];

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = buffer->m_buffer.m_buffer;
            bufferInfo.offset = bufferOffset;
            bufferInfo.range = bufferSize;

            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorCount = 1;
            write.descriptorType = descriptorType;
            write.dstArrayElement = 0;
            write.dstBinding = offset.binding;
            write.dstSet = descriptorSet;
            write.pBufferInfo = &bufferInfo;

            writeDescriptor(context, write);
        }

        static void writeBufferDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            BufferResourceImpl* buffer)
        {
            writeBufferDescriptor(
                context, offset, descriptorType, buffer, 0, buffer->getDesc()->sizeInBytes);
        }


        static void writePlainBufferDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
        {
            auto descriptorSet = context.descriptorSets[offset.bindingSet];

            Index count = resourceViews.getCount();
            for(Index i = 0; i < count; ++i)
            {
                auto bufferView = static_cast<PlainBufferResourceViewImpl*>(resourceViews[i].Ptr());

                VkDescriptorBufferInfo bufferInfo = {};

                if(bufferView)
                {
                    bufferInfo.buffer = bufferView->m_buffer->m_buffer.m_buffer;
                    bufferInfo.offset = 0;
                    bufferInfo.range = bufferView->m_buffer->getDesc()->sizeInBytes;
                }

                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorCount = 1;
                write.descriptorType = descriptorType;
                write.dstArrayElement = uint32_t(i);
                write.dstBinding = offset.binding;
                write.dstSet = descriptorSet;
                write.pBufferInfo = &bufferInfo;

                writeDescriptor(context, write);
            }
        }

        static void writeTexelBufferDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
        {
            auto descriptorSet = context.descriptorSets[offset.bindingSet];

            Index count = resourceViews.getCount();
            for(Index i = 0; i < count; ++i)
            {
                auto resourceView = static_cast<TexelBufferResourceViewImpl*>(resourceViews[i].Ptr());

                VkBufferView bufferView = resourceView->m_view;

                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorCount = 1;
                write.descriptorType = descriptorType;
                write.dstArrayElement = uint32_t(i);
                write.dstBinding = offset.binding;
                write.dstSet = descriptorSet;
                write.pTexelBufferView = &bufferView;

                writeDescriptor(context, write);
            }
        }

        static void writeTextureSamplerDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            ArrayView<CombinedTextureSamplerSlot> slots)
        {
            auto descriptorSet = context.descriptorSets[offset.bindingSet];

            Index count = slots.getCount();
            for(Index i = 0; i < count; ++i)
            {
                auto texture = slots[i].textureView;
                auto sampler = slots[i].sampler;

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageView = texture->m_view;
                imageInfo.imageLayout = texture->m_layout;
                imageInfo.sampler = sampler->m_sampler;

                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorCount = 1;
                write.descriptorType = descriptorType;
                write.dstArrayElement = uint32_t(i);
                write.dstBinding = offset.binding;
                write.dstSet = descriptorSet;
                write.pImageInfo = &imageInfo;

                writeDescriptor(context, write);
            }
        }

        static void writeAccelerationStructureDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
        {
            auto descriptorSet = context.descriptorSets[offset.bindingSet];

            Index count = resourceViews.getCount();
            for (Index i = 0; i < count; ++i)
            {
                auto accelerationStructure =
                    static_cast<AccelerationStructureImpl*>(resourceViews[i].Ptr());
                
                VkWriteDescriptorSetAccelerationStructureKHR writeAS = {};
                writeAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
                writeAS.accelerationStructureCount = 1;
                writeAS.pAccelerationStructures = &accelerationStructure->m_vkHandle;
                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorCount = 1;
                write.descriptorType = descriptorType;
                write.dstArrayElement = uint32_t(i);
                write.dstBinding = offset.binding;
                write.dstSet = descriptorSet;
                write.pNext = &writeAS;
                writeDescriptor(context, write);
            }
        }

        static void writeTextureDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<ResourceViewInternalBase>> resourceViews)
        {
            auto descriptorSet = context.descriptorSets[offset.bindingSet];

            Index count = resourceViews.getCount();
            for(Index i = 0; i < count; ++i)
            {
                auto texture = static_cast<TextureResourceViewImpl*>(resourceViews[i].Ptr());

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageView = texture->m_view;
                imageInfo.imageLayout = texture->m_layout;
                imageInfo.sampler = 0;

                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorCount = 1;
                write.descriptorType = descriptorType;
                write.dstArrayElement = uint32_t(i);
                write.dstBinding = offset.binding;
                write.dstSet = descriptorSet;
                write.pImageInfo = &imageInfo;

                writeDescriptor(context, write);
            }
        }

        static void writeSamplerDescriptor(
            RootBindingContext& context,
            BindingOffset const& offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<SamplerStateImpl>> samplers)
        {
            auto descriptorSet = context.descriptorSets[offset.bindingSet];

            Index count = samplers.getCount();
            for(Index i = 0; i < count; ++i)
            {
                auto sampler = samplers[i];

                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageView = 0;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                imageInfo.sampler = sampler->m_sampler;

                VkWriteDescriptorSet write = {};
                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write.descriptorCount = 1;
                write.descriptorType = descriptorType;
                write.dstArrayElement = uint32_t(i);
                write.dstBinding = offset.binding;
                write.dstSet = descriptorSet;
                write.pImageInfo = &imageInfo;

                writeDescriptor(context, write);
            }
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
            SLANG_RETURN_ON_FAIL(encoder->m_commandBuffer->m_transientHeap->allocateConstantBuffer(
                m_constantBufferSize, m_constantBuffer, m_constantBufferOffset));

            // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
            //
            // Note that `_writeOrdinaryData` is potentially recursive in the case
            // where this object contains interface/existential-type fields, so we
            // don't need or want to inline it into this call site.
            //
            SLANG_RETURN_ON_FAIL(_writeOrdinaryData(
                encoder,
                m_constantBuffer,
                m_constantBufferOffset,
                m_constantBufferSize,
                specializedLayout));

            // Update version tracker so that we don't redundantly alloc and fill in
            // constant buffers for the same transient heap.
            m_upToDateConstantBufferHeapVersion =
                encoder->m_commandBuffer->m_transientHeap->getVersion();
            return SLANG_OK;
        }

    public:

            /// Bind this shader object as a "value"
            ///
            /// This is the mode used for binding sub-objects for existential-type
            /// fields, and is also used as part of the implementation of the
            /// parameter-block and constant-buffer cases.
            ///
        Result bindAsValue(
            PipelineCommandEncoder* encoder,
            RootBindingContext&     context,
            BindingOffset const&    offset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // We start by iterating over the "simple" (non-sub-object) binding
            // ranges and writing them to the descriptor sets that are being
            // passed down.
            //
            for (auto bindingRangeInfo : specializedLayout->getBindingRanges())
            {
                BindingOffset rangeOffset = offset;

                auto baseIndex = bindingRangeInfo.baseIndex;
                auto count = (uint32_t)bindingRangeInfo.count;
                switch (bindingRangeInfo.bindingType)
                {
                case slang::BindingType::ConstantBuffer:
                case slang::BindingType::ParameterBlock:
                case slang::BindingType::ExistentialValue:
                    break;

                case slang::BindingType::Texture:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writeTextureDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                        m_resourceViews.getArrayView(baseIndex, count));
                    break;
                case slang::BindingType::MutableTexture:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writeTextureDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        m_resourceViews.getArrayView(baseIndex, count));
                    break;
                case slang::BindingType::CombinedTextureSampler:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writeTextureSamplerDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        m_combinedTextureSamplers.getArrayView(baseIndex, count));
                    break;

                case slang::BindingType::Sampler:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writeSamplerDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_SAMPLER,
                        m_samplers.getArrayView(baseIndex, count));
                    break;

                case slang::BindingType::RawBuffer:
                case slang::BindingType::MutableRawBuffer:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writePlainBufferDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        m_resourceViews.getArrayView(baseIndex, count));
                    break;

                case slang::BindingType::TypedBuffer:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writeTexelBufferDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
                        m_resourceViews.getArrayView(baseIndex, count));
                    break;
                case slang::BindingType::MutableTypedBuffer:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writeTexelBufferDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                        m_resourceViews.getArrayView(baseIndex, count));
                    break;
                case slang::BindingType::RayTracingAccelerationStructure:
                    rangeOffset.bindingSet += bindingRangeInfo.setOffset;
                    rangeOffset.binding += bindingRangeInfo.bindingOffset;
                    writeAccelerationStructureDescriptor(
                        context,
                        rangeOffset,
                        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                        m_resourceViews.getArrayView(baseIndex, count));
                    break;
                case slang::BindingType::VaryingInput:
                case slang::BindingType::VaryingOutput:
                    break;

                default:
                    SLANG_ASSERT(!"unsupported binding type");
                    return SLANG_FAIL;
                    break;
                }
            }

            // Once we've handled the simpel binding ranges, we move on to the
            // sub-object ranges, which are generally more involved.
            //
            for( auto const& subObjectRange : specializedLayout->getSubObjectRanges() )
            {
                auto const& bindingRangeInfo = specializedLayout->getBindingRange(subObjectRange.bindingRangeIndex);
                auto count = bindingRangeInfo.count;
                auto subObjectIndex = bindingRangeInfo.subObjectIndex;

                auto subObjectLayout = subObjectRange.layout;

                // The starting offset to use for the sub-object
                // has already been computed and stored as part
                // of the layout, so we can get to the starting
                // offset for the range easily.
                //
                BindingOffset rangeOffset = offset;
                rangeOffset += subObjectRange.offset;

                BindingOffset rangeStride = subObjectRange.stride;

                switch( bindingRangeInfo.bindingType )
                {
                case slang::BindingType::ConstantBuffer:
                    {
                        BindingOffset objOffset = rangeOffset;
                        for (uint32_t i = 0; i < count; ++i)
                        {
                            // Binding a constant buffer sub-object is simple enough:
                            // we just call `bindAsConstantBuffer` on it to bind
                            // the ordinary data buffer (if needed) and any other
                            // bindings it recursively contains.
                            //
                            ShaderObjectImpl* subObject = m_objects[subObjectIndex + i];
                            subObject->bindAsConstantBuffer(encoder, context, objOffset, subObjectLayout);

                            // When dealing with arrays of sub-objects, we need to make
                            // sure to increment the offset for each subsequent object
                            // by the appropriate stride.
                            //
                            objOffset += rangeStride;
                        }
                    }
                    break;
                case slang::BindingType::ParameterBlock:
                    {
                        BindingOffset objOffset = rangeOffset;
                        for (uint32_t i = 0; i < count; ++i)
                        {
                            // The case for `ParameterBlock<X>` is not that different
                            // from `ConstantBuffer<X>`, except that we call `bindAsParameterBlock`
                            // instead (understandably).
                            //
                            ShaderObjectImpl* subObject = m_objects[subObjectIndex + i];
                            subObject->bindAsParameterBlock(encoder, context, objOffset, subObjectLayout);

                            objOffset += rangeStride;
                        }
                    }
                    break;

                case slang::BindingType::ExistentialValue:
                    // Interface/existential-type sub-object ranges are the most complicated case.
                    //
                    // First, we can only bind things if we have static specialization information
                    // to work with, which is exactly the case where `subObjectLayout` will be non-null.
                    //
                    if( subObjectLayout )
                    {
                        // Second, the offset where we want to start binding for existential-type
                        // ranges is a bit different, because we don't wnat to bind at the "primary"
                        // offset that got passed down, but instead at the "pending" offset.
                        //
                        // For the purposes of nested binding, what used to be the pending offset
                        // will now be used as the primary offset.
                        //
                        SimpleBindingOffset objOffset = rangeOffset.pending;
                        SimpleBindingOffset objStride = rangeStride.pending;
                        for (uint32_t i = 0; i < count; ++i)
                        {
                            // An existential-type sub-object is always bound just as a value,
                            // which handles its nested bindings and descriptor sets, but
                            // does not deal with ordianry data. The ordinary data should
                            // have been handled as part of the buffer for a parent object
                            // already.
                            //
                            ShaderObjectImpl* subObject = m_objects[subObjectIndex + i];
                            subObject->bindAsValue(encoder, context, BindingOffset(objOffset), subObjectLayout);
                            objOffset += objStride;
                        }
                    }
                    break;
                case slang::BindingType::RawBuffer:
                case slang::BindingType::MutableRawBuffer:
                    // No action needed for sub-objects bound though a `StructuredBuffer`.
                    break;
                default:
                    SLANG_ASSERT(!"unsupported sub-object type");
                    return SLANG_FAIL;
                    break;
                }
            }

            return SLANG_OK;
        }

            /// Allocate the descriptor sets needed for binding this object (but not nested parameter blocks)
        Result allocateDescriptorSets(
            PipelineCommandEncoder* encoder,
            RootBindingContext&     context,
            BindingOffset const&    offset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            auto baseDescriptorSetIndex = offset.childSet;

            // The number of sets to allocate and their layouts was already pre-computed
            // as part of the shader object layout, so we use that information here.
            //
            for (auto descriptorSetInfo : specializedLayout->getOwnDescriptorSets())
            {
                auto descriptorSetHandle = context.descriptorSetAllocator->allocate(
                        descriptorSetInfo.descriptorSetLayout).handle;

                // For each set, we need to write it into the set of descriptor sets
                // being used for binding. This is done both so that other steps
                // in binding can find the set to fill it in, but also so that
                // we can bind all the descriptor sets to the pipeline when the
                // time comes.
                //
                auto descriptorSetIndex = baseDescriptorSetIndex + descriptorSetInfo.space;
                context.descriptorSets[descriptorSetIndex] = descriptorSetHandle;
            }

            return SLANG_OK;
        }

            /// Bind this object as a `ParameterBlock<X>`.
        Result bindAsParameterBlock(
            PipelineCommandEncoder* encoder,
            RootBindingContext&     context,
            BindingOffset const&    inOffset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // Because we are binding into a nested parameter block,
            // any texture/buffer/sampler bindings will now want to
            // write into the sets we allocate for this object and
            // not the sets for any parent object(s).
            //
            BindingOffset offset = inOffset;
            offset.bindingSet = offset.childSet;
            offset.binding = 0;

            // TODO: We should also be writing to `offset.pending` here,
            // because any resource/sampler bindings related to "pending"
            // data should *also* be writing into the chosen set.
            //
            // The challenge here is that we need to compute the right
            // value for `offset.pending.binding`, so that it writes after
            // all the other bindings.

            // Writing the bindings for a parameter block is relatively easy:
            // we just need to allocate the descriptor set(s) needed for this
            // object and then fill it in like a `ConstantBuffer<X>`.
            //
            SLANG_RETURN_ON_FAIL(allocateDescriptorSets(encoder, context, offset, specializedLayout));
            SLANG_RETURN_ON_FAIL(bindAsConstantBuffer(encoder, context, offset, specializedLayout));

            return SLANG_OK;
        }

            /// Bind the ordinary data buffer if needed, and adjust `ioOffset` accordingly
        Result bindOrdinaryDataBufferIfNeeded(
            PipelineCommandEncoder* encoder,
            RootBindingContext&     context,
            BindingOffset&          ioOffset,
            ShaderObjectLayoutImpl* specializedLayout)
        {
            // We start by ensuring that the buffer is created, if it is needed.
            //
            SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(encoder, specializedLayout));

            // If we did indeed need/create a buffer, then we must bind it into
            // the given `descriptorSet` and update the base range index for
            // subsequent binding operations to account for it.
            //
            if (m_constantBuffer)
            {
                auto bufferImpl = static_cast<BufferResourceImpl*>(m_constantBuffer);
                writeBufferDescriptor(
                    context,
                    ioOffset,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    bufferImpl,
                    m_constantBufferOffset,
                    m_constantBufferSize);
                ioOffset.binding++;
            }

            return SLANG_OK;
        }

            /// Bind this object as a `ConstantBuffer<X>`.
        Result bindAsConstantBuffer(
            PipelineCommandEncoder*     encoder,
            RootBindingContext&         context,
            BindingOffset const&        inOffset,
            ShaderObjectLayoutImpl*     specializedLayout)
        {
            // To bind an object as a constant buffer, we first
            // need to bind its ordinary data (if any) into an
            // ordinary data buffer, and then bind it as a "value"
            // which handles any of its recursively-contained bindings.
            //
            // The one detail is taht when binding the ordinary data
            // buffer we need to adjust the `binding` index used for
            // subsequent operations based on whether or not an ordinary
            // data buffer was used (and thus consumed a `binding`).
            //
            BindingOffset offset = inOffset;
            SLANG_RETURN_ON_FAIL(bindOrdinaryDataBufferIfNeeded(encoder, context, /*inout*/ offset, specializedLayout));
            SLANG_RETURN_ON_FAIL(bindAsValue(encoder, context, offset, specializedLayout));
            return SLANG_OK;
        }

        List<RefPtr<ResourceViewInternalBase>> m_resourceViews;

        List<RefPtr<SamplerStateImpl>> m_samplers;

        List<CombinedTextureSamplerSlot> m_combinedTextureSamplers;

        // The version number of the transient resource heap that contains up-to-date
        // constant buffer content for this shader object.
        uint64_t m_upToDateConstantBufferHeapVersion;
        // The transient constant buffer that holds the GPU copy of the constant data,
        // weak referenced.
        IBufferResource* m_constantBuffer = nullptr;
        // The offset into the transient constant buffer where the constant data starts.
        size_t m_constantBufferOffset = 0;
        size_t m_constantBufferSize = 0;

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

            auto device = getDevice();
            RefPtr<ShaderObjectLayoutImpl> layout;
            SLANG_RETURN_ON_FAIL(device->getShaderObjectLayout(
                extendedType.slangType,
                m_layout->getContainerType(),
                (ShaderObjectLayoutBase**)layout.writeRef()));

            returnRefPtrMove(outLayout, layout);
            return SLANG_OK;
        }

        RefPtr<ShaderObjectLayoutImpl> m_specializedLayout;
    };

    class EntryPointShaderObject : public ShaderObjectImpl
    {
        typedef ShaderObjectImpl Super;

    public:
        static Result create(
            IDevice* device,
            EntryPointLayout* layout,
            EntryPointShaderObject** outShaderObject)
        {
            RefPtr<EntryPointShaderObject> object = new EntryPointShaderObject();
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            returnRefPtrMove(outShaderObject, object);
            return SLANG_OK;
        }

        EntryPointLayout* getLayout() { return static_cast<EntryPointLayout*>(m_layout.Ptr()); }

            /// Bind this shader object as an entry point
        Result bindAsEntryPoint(
            PipelineCommandEncoder* encoder,
            RootBindingContext&     context,
            BindingOffset const&    inOffset,
            EntryPointLayout*       layout)
        {
            BindingOffset offset = inOffset;

            // Any ordinary data in an entry point is assumed to be allocated
            // as a push-constant range.
            //
            // TODO: Can we make this operation not bake in that assumption?
            //
            // TODO: Can/should this function be renamed as just `bindAsPushConstantBuffer`?
            //
            if (m_data.getCount())
            {
                // The index of the push constant range to bind should be
                // passed down as part of the `offset`, and we will increment
                // it here so that any further recursively-contained push-constant
                // ranges use the next index.
                //
                auto pushConstantRangeIndex = offset.pushConstantRange++;

                // Information about the push constant ranges (including offsets
                // and stage flags) was pre-computed for the entire program and
                // stored on the binding context.
                //
                auto const& pushConstantRange = context.pushConstantRanges[pushConstantRangeIndex];

                // We expect that the size of the range as reflected matches the
                // amount of ordinary data stored on this object.
                //
                // TODO: This would not be the case if specialization for interface-type
                // parameters led to the entry point having "pending" ordinary data.
                //
                SLANG_ASSERT(pushConstantRange.size == (uint32_t)m_data.getCount());

                auto pushConstantData = m_data.getBuffer();

                encoder->m_api->vkCmdPushConstants(
                    encoder->m_commandBuffer->m_commandBuffer,
                    context.pipelineLayout,
                    pushConstantRange.stageFlags,
                    pushConstantRange.offset,
                    pushConstantRange.size,
                    pushConstantData);
            }

            // Any remaining bindings in the object can be handled through the
            // "value" case.
            //
            SLANG_RETURN_ON_FAIL(bindAsValue(encoder, context, offset, layout));
            return SLANG_OK;
        }

    protected:
        Result init(IDevice* device, EntryPointLayout* layout)
        {
            SLANG_RETURN_ON_FAIL(Super::init(device, layout));
            return SLANG_OK;
        }
    };

    class RootShaderObjectImpl : public ShaderObjectImpl
    {
        typedef ShaderObjectImpl Super;
    public:
        // Override default reference counting behavior to disable lifetime management.
        // Root objects are managed by command buffer and does not need to be freed by the user.
        SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 1; }
        SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 1; }
    public:
        RootShaderObjectLayout* getLayout()
        {
            return static_cast<RootShaderObjectLayout*>(m_layout.Ptr());
        }

        RootShaderObjectLayout* getSpecializedLayout()
        {
            RefPtr<ShaderObjectLayoutImpl> specializedLayout;
            _getSpecializedLayout(specializedLayout.writeRef());
            return static_cast<RootShaderObjectLayout*>(m_specializedLayout.Ptr());
        }

        List<RefPtr<EntryPointShaderObject>> const& getEntryPoints() const { return m_entryPoints; }

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

            /// Bind this object as a root shader object
        Result bindAsRoot(
            PipelineCommandEncoder*     encoder,
            RootBindingContext&         context,
            RootShaderObjectLayout*     layout)
        {
            BindingOffset offset = {};
            offset.pending = layout->getPendingDataOffset();

            // Note: the operations here are quite similar to what `bindAsParameterBlock` does.
            // The key difference in practice is that we do *not* make use of the adjustment
            // that `bindOrdinaryDataBufferIfNeeded` applied to the offset passed into it.
            //
            // The reason for this difference in behavior is that the layout information
            // for root shader parameters is in practice *already* offset appropriately
            // (so that it ends up using absolute offsets).
            //
            // TODO: One more wrinkle here is that the `ordinaryDataBufferOffset` below
            // might not be correct if `binding=0,set=0` was already claimed via explicit
            // binding information. We should really be getting the offset information for
            // the ordinary data buffer directly from the reflection information for
            // the global scope.

            SLANG_RETURN_ON_FAIL(allocateDescriptorSets(encoder, context, offset, layout));

            BindingOffset ordinaryDataBufferOffset = offset;
            SLANG_RETURN_ON_FAIL(bindOrdinaryDataBufferIfNeeded(encoder, context, /*inout*/ ordinaryDataBufferOffset, layout));

            SLANG_RETURN_ON_FAIL(bindAsValue(encoder, context, offset, layout));

            auto entryPointCount = layout->getEntryPoints().getCount();
            for( Index i = 0; i < entryPointCount; ++i )
            {
                auto entryPoint = m_entryPoints[i];
                auto const& entryPointInfo = layout->getEntryPoint(i);

                // Note: we do *not* need to add the entry point offset
                // information to the global `offset` because the
                // `RootShaderObjectLayout` has already baked any offsets
                // from the global layout into the `entryPointInfo`.

                entryPoint->bindAsEntryPoint(encoder, context, entryPointInfo.offset, entryPointInfo.layout);
            }

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
        Result init(IDevice* device, RootShaderObjectLayout* layout)
        {
            SLANG_RETURN_ON_FAIL(Super::init(device, layout));
            m_specializedLayout = nullptr;
            m_entryPoints.clear();
            for (auto entryPointInfo : layout->getEntryPoints())
            {
                RefPtr<EntryPointShaderObject> entryPoint;
                SLANG_RETURN_ON_FAIL(EntryPointShaderObject::create(
                    device, entryPointInfo.layout, entryPoint.writeRef()));
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
            RefPtr<RootShaderObjectLayout> specializedLayout;
            RootShaderObjectLayout::create(
                static_cast<VKDevice*>(getRenderer()),
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

        List<RefPtr<EntryPointShaderObject>> m_entryPoints;
    };

    class TransientResourceHeapImpl;

    class CommandBufferImpl
        : public ICommandBuffer
        , public ComObject
    {
    public:
        // There are a pair of cyclic references between a `TransientResourceHeap` and
        // a `CommandBuffer` created from the heap. We need to break the cycle when
        // the public reference count of a command buffer drops to 0.
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        ICommandBuffer* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
                return static_cast<ICommandBuffer*>(this);
            return nullptr;
        }
        virtual void comFree() override { m_transientHeap.breakStrongReference(); }
    public:
        VkCommandBuffer m_commandBuffer;
        VkCommandBuffer m_preCommandBuffer = VK_NULL_HANDLE;
        VkCommandPool m_pool;
        VKDevice* m_renderer;
        BreakableReference<TransientResourceHeapImpl> m_transientHeap;
        bool m_isPreCommandBufferEmpty = true;
        RootShaderObjectImpl m_rootObject;
        // Command buffers are deallocated by its command pool,
        // so no need to free individually.
        ~CommandBufferImpl() = default;

        Result init(
            VKDevice* renderer,
            VkCommandPool pool,
            TransientResourceHeapImpl* transientHeap)
        {
            m_renderer = renderer;
            m_transientHeap = transientHeap;
            m_pool = pool;

            auto& api = renderer->m_api;
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            SLANG_VK_RETURN_ON_FAIL(
                api.vkAllocateCommandBuffers(api.m_device, &allocInfo, &m_commandBuffer));

            beginCommandBuffer();
            return SLANG_OK;
        }

        void beginCommandBuffer()
        {
            auto& api = m_renderer->m_api;
            VkCommandBufferBeginInfo beginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                nullptr,
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
            api.vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
            if (m_preCommandBuffer)
            {
                api.vkBeginCommandBuffer(m_preCommandBuffer, &beginInfo);
            }
            m_isPreCommandBufferEmpty = true;
        }

        Result createPreCommandBuffer()
        {
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            auto& api = m_renderer->m_api;
            SLANG_VK_RETURN_ON_FAIL(
                api.vkAllocateCommandBuffers(api.m_device, &allocInfo, &m_preCommandBuffer));
            VkCommandBufferBeginInfo beginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                nullptr,
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
            api.vkBeginCommandBuffer(m_preCommandBuffer, &beginInfo);
            return SLANG_OK;
        }

        VkCommandBuffer getPreCommandBuffer()
        {
            m_isPreCommandBufferEmpty = false;
            if (m_preCommandBuffer)
                return m_preCommandBuffer;
            createPreCommandBuffer();
            return m_preCommandBuffer;
        }

        static void _writeTimestamp(
            VulkanApi* api,
            VkCommandBuffer vkCmdBuffer,
            IQueryPool* queryPool,
            SlangInt index)
        {
            auto queryPoolImpl = static_cast<QueryPoolImpl*>(queryPool);
            api->vkCmdResetQueryPool(vkCmdBuffer, queryPoolImpl->m_pool, (uint32_t)index, 1);
            api->vkCmdWriteTimestamp(vkCmdBuffer,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                queryPoolImpl->m_pool,
                (uint32_t)index);
        }

    public:
        class RenderCommandEncoder
            : public IRenderCommandEncoder
            , public PipelineCommandEncoder

        {
        public:
            List<VkViewport> m_viewports;
            List<VkRect2D> m_scissorRects;
            List<BoundVertexBuffer> m_boundVertexBuffers;
            BoundVertexBuffer m_boundIndexBuffer;
            VkIndexType m_boundIndexFormat;

        public:
            void beginPass(IRenderPassLayout* renderPass, IFramebuffer* framebuffer)
            {
                FramebufferImpl* framebufferImpl = static_cast<FramebufferImpl*>(framebuffer);
                RenderPassLayoutImpl* renderPassImpl =
                    static_cast<RenderPassLayoutImpl*>(renderPass);
                VkClearValue clearValues[kMaxAttachments] = {};
                VkRenderPassBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                beginInfo.framebuffer = framebufferImpl->m_handle;
                beginInfo.renderPass = renderPassImpl->m_renderPass;
                uint32_t attachmentCount = (uint32_t)framebufferImpl->renderTargetViews.getCount();
                if (framebufferImpl->depthStencilView)
                    attachmentCount++;
                beginInfo.clearValueCount = attachmentCount;
                beginInfo.renderArea.extent.width = framebufferImpl->m_width;
                beginInfo.renderArea.extent.height = framebufferImpl->m_height;
                beginInfo.pClearValues = framebufferImpl->m_clearValues;
                auto& api = *m_api;
                api.vkCmdBeginRenderPass(m_vkCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                auto& api = *m_api;
                api.vkCmdEndRenderPass(m_vkCommandBuffer);
                endEncodingImpl();
            }

            virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* queryPool, SlangInt index) override
            {
                _writeTimestamp(m_api, m_vkCommandBuffer, queryPool, index);
            }

            virtual SLANG_NO_THROW Result SLANG_MCALL
                bindPipeline(IPipelineState* pipelineState, IShaderObject** outRootObject) override
            {
                return setPipelineStateImpl(pipelineState, outRootObject);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setViewports(uint32_t count, const Viewport* viewports) override
            {
                static const int kMaxViewports = 8; // TODO: base on device caps
                assert(count <= kMaxViewports);

                m_viewports.setCount(count);
                for (UInt ii = 0; ii < count; ++ii)
                {
                    auto& inViewport = viewports[ii];
                    auto& vkViewport = m_viewports[ii];

                    vkViewport.x = inViewport.originX;
                    vkViewport.y = inViewport.originY;
                    vkViewport.width = inViewport.extentX;
                    vkViewport.height = inViewport.extentY;
                    vkViewport.minDepth = inViewport.minZ;
                    vkViewport.maxDepth = inViewport.maxZ;
                }

                auto& api = *m_api;
                api.vkCmdSetViewport(m_vkCommandBuffer, 0, uint32_t(count), m_viewports.getBuffer());
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setScissorRects(uint32_t count, const ScissorRect* rects) override
            {
                static const int kMaxScissorRects = 8; // TODO: base on device caps
                assert(count <= kMaxScissorRects);

                m_scissorRects.setCount(count);
                for (UInt ii = 0; ii < count; ++ii)
                {
                    auto& inRect = rects[ii];
                    auto& vkRect = m_scissorRects[ii];

                    vkRect.offset.x = int32_t(inRect.minX);
                    vkRect.offset.y = int32_t(inRect.minY);
                    vkRect.extent.width = uint32_t(inRect.maxX - inRect.minX);
                    vkRect.extent.height = uint32_t(inRect.maxY - inRect.minY);
                }

                auto& api = *m_api;
                api.vkCmdSetScissor(
                    m_vkCommandBuffer,
                    0,
                    uint32_t(count),
                    m_scissorRects.getBuffer());
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setPrimitiveTopology(PrimitiveTopology topology) override
            {
                auto& api = *m_api;
                if (api.vkCmdSetPrimitiveTopologyEXT)
                {
                    api.vkCmdSetPrimitiveTopologyEXT(
                        m_vkCommandBuffer,
                        VulkanUtil::getVkPrimitiveTopology(topology));
                }
                else
                {
                    switch (topology)
                    {
                    case PrimitiveTopology::TriangleList:
                        break;
                    default:
                        // We are using a non-list topology, but we don't have dynmaic state
                        // extension, error out.
                        assert(!"Non-list topology requires VK_EXT_extended_dynamic_states, which is not present.");
                        break;
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
                    const Index num = Index(startSlot + slotCount);
                    if (num > m_boundVertexBuffers.getCount())
                    {
                        m_boundVertexBuffers.setCount(num);
                    }
                }

                for (Index i = 0; i < Index(slotCount); i++)
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
                switch (indexFormat)
                {
                case Format::R_UInt16:
                    m_boundIndexFormat = VK_INDEX_TYPE_UINT16;
                    break;
                case Format::R_UInt32:
                    m_boundIndexFormat = VK_INDEX_TYPE_UINT32;
                    break;
                default:
                    assert(!"unsupported index format");
                }
                m_boundIndexBuffer.m_buffer = static_cast<BufferResourceImpl*>(buffer);
                m_boundIndexBuffer.m_stride = 0;
                m_boundIndexBuffer.m_offset = int(offset);
            }

            void prepareDraw()
            {
                auto pipeline = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());
                if (!pipeline || static_cast<ShaderProgramImpl*>(pipeline->m_program.Ptr())
                                         ->m_pipelineType != PipelineType::Graphics)
                {
                    assert(!"Invalid render pipeline");
                    return;
                }
                flushBindingState(VK_PIPELINE_BIND_POINT_GRAPHICS);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                draw(UInt vertexCount, UInt startVertex = 0) override
            {
                prepareDraw();
                auto& api = *m_api;
                // Bind the vertex buffer
                if (m_boundVertexBuffers.getCount() > 0 && m_boundVertexBuffers[0].m_buffer)
                {
                    const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[0];

                    VkBuffer vertexBuffers[] = {boundVertexBuffer.m_buffer->m_buffer.m_buffer};
                    VkDeviceSize offsets[] = {VkDeviceSize(boundVertexBuffer.m_offset)};

                    api.vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
                }
                api.vkCmdDraw(m_vkCommandBuffer, static_cast<uint32_t>(vertexCount), 1, 0, 0);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL
                drawIndexed(UInt indexCount, UInt startIndex = 0, UInt baseVertex = 0) override
            {
                prepareDraw();
                auto& api = *m_api;
                api.vkCmdBindIndexBuffer(
                    m_vkCommandBuffer,
                    m_boundIndexBuffer.m_buffer->m_buffer.m_buffer,
                    m_boundIndexBuffer.m_offset,
                    m_boundIndexFormat);
                // Bind the vertex buffer
                if (m_boundVertexBuffers.getCount() > 0 && m_boundVertexBuffers[0].m_buffer)
                {
                    const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[0];

                    VkBuffer vertexBuffers[] = {boundVertexBuffer.m_buffer->m_buffer.m_buffer};
                    VkDeviceSize offsets[] = {VkDeviceSize(boundVertexBuffer.m_offset)};

                    api.vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
                }
                api.vkCmdDraw(m_vkCommandBuffer, static_cast<uint32_t>(indexCount), 1, 0, 0);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setStencilReference(uint32_t referenceValue) override
            {
                auto& api = *m_api;
                api.vkCmdSetStencilReference(
                    m_vkCommandBuffer, VK_STENCIL_FRONT_AND_BACK, referenceValue);
            }
        };

        RefPtr<RenderCommandEncoder> m_renderCommandEncoder;

        virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
            IRenderPassLayout* renderPass,
            IFramebuffer* framebuffer,
            IRenderCommandEncoder** outEncoder) override
        {
            if (!m_renderCommandEncoder)
            {
                m_renderCommandEncoder = new RenderCommandEncoder();
                m_renderCommandEncoder->init(this);
            }
            m_renderCommandEncoder->beginPass(renderPass, framebuffer);
            *outEncoder = m_renderCommandEncoder.Ptr();
        }

        class ComputeCommandEncoder
            : public IComputeCommandEncoder
            , public PipelineCommandEncoder
        {
        public:
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                endEncodingImpl();
            }

            virtual SLANG_NO_THROW Result SLANG_MCALL
                bindPipeline(IPipelineState* pipelineState, IShaderObject** outRootObject) override
            {
                return setPipelineStateImpl(pipelineState, outRootObject);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
            {
                auto pipeline = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());
                if (!pipeline ||
                    static_cast<ShaderProgramImpl*>(pipeline->m_program.Ptr())->m_pipelineType !=
                        PipelineType::Compute)
                {
                    assert(!"Invalid compute pipeline");
                    return;
                }

                // Also create descriptor sets based on the given pipeline layout
                flushBindingState(VK_PIPELINE_BIND_POINT_COMPUTE);
                m_api->vkCmdDispatch(m_vkCommandBuffer, x, y, z);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* queryPool, SlangInt index) override
            {
                _writeTimestamp(m_api, m_vkCommandBuffer, queryPool, index);
            }
        };

        RefPtr<ComputeCommandEncoder> m_computeCommandEncoder;

        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeComputeCommands(IComputeCommandEncoder** outEncoder) override
        {
            if (!m_computeCommandEncoder)
            {
                m_computeCommandEncoder = new ComputeCommandEncoder();
                m_computeCommandEncoder->init(this);
            }
            *outEncoder = m_computeCommandEncoder.Ptr();
        }

        class ResourceCommandEncoder
            : public IResourceCommandEncoder
            , public RefObject
        {
        public:
            CommandBufferImpl* m_commandBuffer;
        public:
            virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
                IBufferResource* dst,
                size_t dstOffset,
                IBufferResource* src,
                size_t srcOffset,
                size_t size) override
            {
                auto& vkAPI = m_commandBuffer->m_renderer->m_api;

                auto dstBuffer = static_cast<BufferResourceImpl*>(dst);
                auto srcBuffer = static_cast<BufferResourceImpl*>(src);

                VkBufferCopy copyRegion;
                copyRegion.dstOffset = dstOffset;
                copyRegion.srcOffset = srcOffset;
                copyRegion.size = size;

                // Note: Vulkan puts the source buffer first in the copy
                // command, going against the dominant tradition for copy
                // operations in C/C++.
                //
                vkAPI.vkCmdCopyBuffer(
                    m_commandBuffer->m_commandBuffer,
                    srcBuffer->m_buffer.m_buffer,
                    dstBuffer->m_buffer.m_buffer,
                    /* regionCount: */ 1,
                    &copyRegion);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL
                uploadBufferData(IBufferResource* buffer, size_t offset, size_t size, void* data) override
            {
                PipelineCommandEncoder::_uploadBufferData(
                    m_commandBuffer->m_commandBuffer,
                    static_cast<BufferResourceImpl*>(buffer),
                    offset,
                    size,
                    data);
            }
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                // Insert memory barrier to ensure transfers are visible to the GPU.
                auto& vkAPI = m_commandBuffer->m_renderer->m_api;

                VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                vkAPI.vkCmdPipelineBarrier(
                    m_commandBuffer->m_commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    0,
                    1,
                    &memBarrier,
                    0,
                    nullptr,
                    0,
                    nullptr);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* queryPool, SlangInt index) override
            {
                _writeTimestamp(
                    &m_commandBuffer->m_renderer->m_api,
                    m_commandBuffer->m_commandBuffer,
                    queryPool,
                    index);
            }

            void init(CommandBufferImpl* commandBuffer)
            {
                m_commandBuffer = commandBuffer;
            }
        };

        RefPtr<ResourceCommandEncoder> m_resourceCommandEncoder;

        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeResourceCommands(IResourceCommandEncoder** outEncoder) override
        {
            if (!m_resourceCommandEncoder)
            {
                m_resourceCommandEncoder = new ResourceCommandEncoder();
                m_resourceCommandEncoder->init(this);
            }
            *outEncoder = m_resourceCommandEncoder.Ptr();
        }

        class RayTracingCommandEncoder
            : public IRayTracingCommandEncoder
            , public RefObject
        {
        public:
            CommandBufferImpl* m_commandBuffer;

        public:
            void init(CommandBufferImpl* commandBuffer) { m_commandBuffer = commandBuffer; }

            inline VkAccessFlags translateAccelerationStructureAccessFlag(AccessFlag::Enum access)
            {
                VkAccessFlags result = 0;
                if (access & AccessFlag::Read)
                    result |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
                              VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
                if (access & AccessFlag::Write)
                    result |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
                return result;
            }

            inline void _memoryBarrier(
                int count,
                IAccelerationStructure* const* structures,
                AccessFlag::Enum srcAccess,
                AccessFlag::Enum destAccess)
            {
                ShortList<VkBufferMemoryBarrier> memBarriers;
                memBarriers.setCount(count);
                for (int i = 0; i < count; i++)
                {
                    memBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                    memBarriers[i].pNext = nullptr;
                    memBarriers[i].dstAccessMask =
                        translateAccelerationStructureAccessFlag(destAccess);
                    memBarriers[i].srcAccessMask =
                        translateAccelerationStructureAccessFlag(srcAccess);
                    memBarriers[i].srcQueueFamilyIndex =
                        m_commandBuffer->m_renderer->m_queueFamilyIndex;
                    memBarriers[i].dstQueueFamilyIndex =
                        m_commandBuffer->m_renderer->m_queueFamilyIndex;

                    auto asImpl = static_cast<AccelerationStructureImpl*>(structures[i]);
                    memBarriers[i].buffer = asImpl->m_buffer->m_buffer.m_buffer;
                    memBarriers[i].offset = asImpl->m_offset;
                    memBarriers[i].size = asImpl->m_size;
                }
                m_commandBuffer->m_renderer->m_api.vkCmdPipelineBarrier(
                    m_commandBuffer->m_commandBuffer,
                    VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR |
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR |
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0,
                    nullptr,
                    (uint32_t)memBarriers.getCount(),
                    memBarriers.getArrayView().getBuffer(),
                    0,
                    nullptr);
            }

            inline void _queryAccelerationStructureProperties(
                int accelerationStructureCount,
                IAccelerationStructure* const* accelerationStructures,
                int queryCount,
                AccelerationStructureQueryDesc* queryDescs)
            {
                ShortList<VkAccelerationStructureKHR> vkHandles;
                vkHandles.setCount(accelerationStructureCount);
                for (int i = 0; i < accelerationStructureCount; i++)
                {
                    vkHandles[i] =
                        static_cast<AccelerationStructureImpl*>(accelerationStructures[i])
                            ->m_vkHandle;
                }
                auto vkHandlesView = vkHandles.getArrayView();
                for (int i = 0; i < queryCount; i++)
                {
                    VkQueryType queryType;
                    switch (queryDescs[i].queryType)
                    {
                    case QueryType::AccelerationStructureCompactedSize:
                        queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
                        break;
                    case QueryType::AccelerationStructureSerializedSize:
                        queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
                        break;
                    default:
                        getDebugCallback()->handleMessage(DebugMessageType::Error, DebugMessageSource::Layer,
                            "Invalid query type for use in queryAccelerationStructureProperties.");
                        return;
                    }
                    auto queryPool = static_cast<QueryPoolImpl*>(queryDescs[i].queryPool)->m_pool;
                    m_commandBuffer->m_renderer->m_api.vkCmdResetQueryPool(
                        m_commandBuffer->m_commandBuffer,
                        queryPool,
                        (uint32_t)queryDescs[i].firstQueryIndex,
                        1);
                    m_commandBuffer->m_renderer->m_api
                        .vkCmdWriteAccelerationStructuresPropertiesKHR(
                            m_commandBuffer->m_commandBuffer,
                            accelerationStructureCount,
                            vkHandlesView.getBuffer(),
                            queryType,
                            queryPool,
                            queryDescs[i].firstQueryIndex);
                }
            }

            virtual SLANG_NO_THROW void SLANG_MCALL buildAccelerationStructure(
                const IAccelerationStructure::BuildDesc& desc,
                int propertyQueryCount,
                AccelerationStructureQueryDesc* queryDescs) override
            {
                AccelerationStructureBuildGeometryInfoBuilder geomInfoBuilder;
                if (geomInfoBuilder.build(desc.inputs, getDebugCallback()) != SLANG_OK)
                    return;

                if (desc.dest)
                {
                    geomInfoBuilder.buildInfo.dstAccelerationStructure =
                        static_cast<AccelerationStructureImpl*>(desc.dest)->m_vkHandle;
                }
                if (desc.source)
                {
                    geomInfoBuilder.buildInfo.srcAccelerationStructure =
                        static_cast<AccelerationStructureImpl*>(desc.source)->m_vkHandle;
                }
                geomInfoBuilder.buildInfo.scratchData.deviceAddress = desc.scratchData;

                List<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos;
                rangeInfos.setCount(geomInfoBuilder.primitiveCounts.getCount());
                for (Index i = 0; i < geomInfoBuilder.primitiveCounts.getCount(); i++)
                {
                    auto& rangeInfo = rangeInfos[i];
                    rangeInfo.primitiveCount = geomInfoBuilder.primitiveCounts[i];
                    rangeInfo.firstVertex = 0;
                    rangeInfo.primitiveOffset = 0;
                    rangeInfo.transformOffset = 0;
                }

                auto rangeInfoPtr = rangeInfos.getBuffer();
                m_commandBuffer->m_renderer->m_api.vkCmdBuildAccelerationStructuresKHR(
                    m_commandBuffer->m_commandBuffer, 1, &geomInfoBuilder.buildInfo, &rangeInfoPtr);

                if (propertyQueryCount)
                {
                    _memoryBarrier(1, &desc.dest, AccessFlag::Write, AccessFlag::Read);
                    _queryAccelerationStructureProperties(
                        1, &desc.dest, propertyQueryCount, queryDescs);
                }
            }

            virtual SLANG_NO_THROW void SLANG_MCALL copyAccelerationStructure(
                IAccelerationStructure* dest,
                IAccelerationStructure* src,
                AccelerationStructureCopyMode mode) override
            {
                VkCopyAccelerationStructureInfoKHR copyInfo = {
                    VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR};
                copyInfo.src = static_cast<AccelerationStructureImpl*>(src)->m_vkHandle;
                copyInfo.dst = static_cast<AccelerationStructureImpl*>(dest)->m_vkHandle;
                switch (mode)
                {
                case AccelerationStructureCopyMode::Clone:
                    copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR;
                    break;
                case AccelerationStructureCopyMode::Compact:
                    copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;
                    break;
                default:
                    getDebugCallback()->handleMessage(
                        DebugMessageType::Error,
                        DebugMessageSource::Layer,
                        "Unsupported AccelerationStructureCopyMode.");
                    return;
                }
                m_commandBuffer->m_renderer->m_api.vkCmdCopyAccelerationStructureKHR(
                    m_commandBuffer->m_commandBuffer, &copyInfo);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL queryAccelerationStructureProperties(
                int accelerationStructureCount,
                IAccelerationStructure* const* accelerationStructures,
                int queryCount,
                AccelerationStructureQueryDesc* queryDescs) override
            {
                _queryAccelerationStructureProperties(
                    accelerationStructureCount, accelerationStructures, queryCount, queryDescs);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL serializeAccelerationStructure(
                DeviceAddress dest,
                IAccelerationStructure* source) override
            {
                VkCopyAccelerationStructureToMemoryInfoKHR copyInfo = {
                    VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_TO_MEMORY_INFO_KHR};
                copyInfo.src = static_cast<AccelerationStructureImpl*>(source)->m_vkHandle;
                copyInfo.dst.deviceAddress = dest;
                copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_SERIALIZE_KHR;
                m_commandBuffer->m_renderer->m_api.vkCmdCopyAccelerationStructureToMemoryKHR(
                    m_commandBuffer->m_commandBuffer, &copyInfo);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL deserializeAccelerationStructure(
                IAccelerationStructure* dest,
                DeviceAddress source) override
            {
                VkCopyMemoryToAccelerationStructureInfoKHR copyInfo = {
                    VK_STRUCTURE_TYPE_COPY_MEMORY_TO_ACCELERATION_STRUCTURE_INFO_KHR};
                copyInfo.src.deviceAddress = source;
                copyInfo.dst = static_cast<AccelerationStructureImpl*>(dest)->m_vkHandle;
                copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_DESERIALIZE_KHR;
                m_commandBuffer->m_renderer->m_api.vkCmdCopyMemoryToAccelerationStructureKHR(
                    m_commandBuffer->m_commandBuffer, &copyInfo);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL memoryBarrier(
                int count,
                IAccelerationStructure* const* structures,
                AccessFlag::Enum srcAccess,
                AccessFlag::Enum destAccess) override
            {
                _memoryBarrier(count, structures, srcAccess, destAccess);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                writeTimestamp(IQueryPool* queryPool, SlangInt index) override
            {
                _writeTimestamp(
                    &m_commandBuffer->m_renderer->m_api,
                    m_commandBuffer->m_commandBuffer,
                    queryPool,
                    index);
            }
        };

        RefPtr<RayTracingCommandEncoder> m_rayTracingCommandEncoder;

        virtual SLANG_NO_THROW void SLANG_MCALL
            encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) override
        {
            if (!m_rayTracingCommandEncoder)
            {
                if (m_renderer->m_api.vkCmdBuildAccelerationStructuresKHR)
                {
                    m_rayTracingCommandEncoder = new RayTracingCommandEncoder();
                    m_rayTracingCommandEncoder->init(this);
                }
            }
            *outEncoder = m_rayTracingCommandEncoder.Ptr();
        }

        virtual SLANG_NO_THROW void SLANG_MCALL close() override
        {
            auto& vkAPI = m_renderer->m_api;
            if (!m_isPreCommandBufferEmpty)
            {
                // `preCmdBuffer` contains buffer transfer commands for shader object
                // uniform buffers, and we need a memory barrier here to ensure the
                // transfers are visible to shaders.
                VkMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
                memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                memBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                vkAPI.vkCmdPipelineBarrier(
                    m_preCommandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    0,
                    1,
                    &memBarrier,
                    0,
                    nullptr,
                    0,
                    nullptr);
                vkAPI.vkEndCommandBuffer(m_preCommandBuffer);
            }
            vkAPI.vkEndCommandBuffer(m_commandBuffer);
        }
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

    public:
        Desc m_desc;
        RefPtr<VKDevice> m_renderer;
        VkQueue m_queue;
        uint32_t m_queueFamilyIndex;
        VkSemaphore m_pendingWaitSemaphores[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        List<VkCommandBuffer> m_submitCommandBuffers;
        static const int kSemaphoreCount = 32;
        uint32_t m_currentSemaphoreIndex;
        VkSemaphore m_semaphores[kSemaphoreCount];
        ~CommandQueueImpl()
        {
            m_renderer->m_api.vkQueueWaitIdle(m_queue);

            m_renderer->m_queueAllocCount--;
            for (int i = 0; i < kSemaphoreCount; i++)
            {
                m_renderer->m_api.vkDestroySemaphore(
                    m_renderer->m_api.m_device, m_semaphores[i], nullptr);
            }
        }

        void init(VKDevice* renderer, VkQueue queue, uint32_t queueFamilyIndex)
        {
            m_renderer = renderer;
            m_currentSemaphoreIndex = 0;
            m_queue = queue;
            m_queueFamilyIndex = queueFamilyIndex;
            for (int i = 0; i < kSemaphoreCount; i++)
            {
                VkSemaphoreCreateInfo semaphoreCreateInfo = {};
                semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                semaphoreCreateInfo.flags = 0;
                m_renderer->m_api.vkCreateSemaphore(
                    m_renderer->m_api.m_device, &semaphoreCreateInfo, nullptr, &m_semaphores[i]);
            }
        }

        virtual SLANG_NO_THROW void SLANG_MCALL wait() override
        {
            auto& vkAPI = m_renderer->m_api;
            vkAPI.vkQueueWaitIdle(m_queue);
        }

        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override
        {
            return m_desc;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            executeCommandBuffers(
            uint32_t count,
            ICommandBuffer* const* commandBuffers) override
        {
            if (count == 0)
                return;

            auto& vkAPI = m_renderer->m_api;
            m_submitCommandBuffers.clear();
            for (uint32_t i = 0; i < count; i++)
            {
                auto cmdBufImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
                if (!cmdBufImpl->m_isPreCommandBufferEmpty)
                    m_submitCommandBuffers.add(cmdBufImpl->m_preCommandBuffer);
                auto vkCmdBuf = cmdBufImpl->m_commandBuffer;
                m_submitCommandBuffers.add(vkCmdBuf);
            }
            VkSemaphore signalSemaphore = m_semaphores[m_currentSemaphoreIndex];
            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkPipelineStageFlags stageFlag[] = {
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
            submitInfo.pWaitDstStageMask = stageFlag;
            submitInfo.commandBufferCount = (uint32_t)m_submitCommandBuffers.getCount();
            submitInfo.pCommandBuffers = m_submitCommandBuffers.getBuffer();
            Array<VkSemaphore, 2> waitSemaphores;
            for (auto s : m_pendingWaitSemaphores)
            {
                if (s != VK_NULL_HANDLE)
                {
                    waitSemaphores.add(s);
                }
            }
            submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.getCount();
            if (submitInfo.waitSemaphoreCount)
            {
                submitInfo.pWaitSemaphores = waitSemaphores.getBuffer();
            }
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &signalSemaphore;

            auto commandBufferImpl = static_cast<CommandBufferImpl*>(commandBuffers[0]);
            auto fence = commandBufferImpl->m_transientHeap->getCurrentFence();
            vkAPI.vkResetFences(vkAPI.m_device, 1, &fence);
            vkAPI.vkQueueSubmit(m_queue, 1, &submitInfo, fence);
            m_pendingWaitSemaphores[0] = signalSemaphore;
            m_pendingWaitSemaphores[1] = VK_NULL_HANDLE;
            commandBufferImpl->m_transientHeap->advanceFence();

            m_currentSemaphoreIndex++;
            m_currentSemaphoreIndex = m_currentSemaphoreIndex % kSemaphoreCount;
        }
    };

    class TransientResourceHeapImpl
        : public TransientResourceHeapBase<VKDevice, BufferResourceImpl>
    {
    private:
        typedef TransientResourceHeapBase<VKDevice, BufferResourceImpl> Super;

    public:
        VkCommandPool m_commandPool;
        DescriptorSetAllocator m_descSetAllocator;
        List<VkFence> m_fences;
        Index m_fenceIndex = -1;
        List<RefPtr<CommandBufferImpl>> m_commandBufferPool;
        uint32_t m_commandBufferAllocId = 0;
        VkFence getCurrentFence()
        {
            return m_fences[m_fenceIndex];
        }
        void advanceFence()
        {
            m_fenceIndex++;
            if (m_fenceIndex >= m_fences.getCount())
            {
                m_fences.setCount(m_fenceIndex + 1);
                VkFenceCreateInfo fenceCreateInfo = {};
                fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                m_device->m_api.vkCreateFence(
                    m_device->m_api.m_device, &fenceCreateInfo, nullptr, &m_fences[m_fenceIndex]);
            }
        }

        Result init(const ITransientResourceHeap::Desc& desc, VKDevice* device);
        ~TransientResourceHeapImpl()
        {
            m_commandBufferPool = decltype(m_commandBufferPool)();
            m_device->m_api.vkDestroyCommandPool(m_device->m_api.m_device, m_commandPool, nullptr);
            for (auto fence : m_fences)
            {
                m_device->m_api.vkDestroyFence(m_device->m_api.m_device, fence, nullptr);
            }
            m_descSetAllocator.close();
        }
    public:
        virtual SLANG_NO_THROW Result SLANG_MCALL
            createCommandBuffer(ICommandBuffer** outCommandBuffer) override;
        virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override;
    };

    class QueryPoolImpl
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
        Result init(const IQueryPool::Desc& desc, VKDevice* device)
        {
            m_device = device;
            VkQueryPoolCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            createInfo.queryCount = (uint32_t)desc.count;
            switch (desc.type)
            {
            case QueryType::Timestamp:
                createInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
                break;
            case QueryType::AccelerationStructureCompactedSize:
                createInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
                break;
            case QueryType::AccelerationStructureSerializedSize:
                createInfo.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR;
                break;
            default:
                return SLANG_E_INVALID_ARG;
            }
            SLANG_VK_RETURN_ON_FAIL(m_device->m_api.vkCreateQueryPool(
                m_device->m_api.m_device, &createInfo, nullptr, &m_pool));
            return SLANG_OK;
        }
        ~QueryPoolImpl()
        {
            m_device->m_api.vkDestroyQueryPool(m_device->m_api.m_device, m_pool, nullptr);
        }
    public:
        virtual SLANG_NO_THROW Result SLANG_MCALL getResult(SlangInt index, SlangInt count, uint64_t* data) override
        {
            SLANG_VK_RETURN_ON_FAIL(m_device->m_api.vkGetQueryPoolResults(
                m_device->m_api.m_device,
                m_pool,
                (uint32_t)index,
                (uint32_t)count,
                sizeof(uint64_t) * count,
                data,
                sizeof(uint64_t), 0));
            return SLANG_OK;
        }
    public:
        VkQueryPool m_pool;
        RefPtr<VKDevice> m_device;
    };

    class SwapchainImpl
        : public ISwapchain
        , public ComObject
    {
    public:
        SLANG_COM_OBJECT_IUNKNOWN_ALL
        ISwapchain* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISwapchain)
                return static_cast<ISwapchain*>(this);
            return nullptr;
        }

    public:
        struct PlatformDesc
        {};

#if SLANG_WINDOWS_FAMILY
        struct WinPlatformDesc : public PlatformDesc
        {
            HINSTANCE m_hinstance;
            HWND m_hwnd;
        };
#else
        struct XPlatformDesc : public PlatformDesc
        {
            Display* m_display;
            Window m_window;
        };
#endif
    public:
        VkSwapchainKHR m_swapChain;
        VkSurfaceKHR m_surface;
        VkSemaphore m_nextImageSemaphore; // Semaphore to signal after `acquireNextImage`.
        ISwapchain::Desc m_desc;
        VkFormat m_vkformat;
        RefPtr<CommandQueueImpl> m_queue;
        ShortList<RefPtr<TextureResourceImpl>> m_images;
        RefPtr<VKDevice> m_renderer;
        VulkanApi* m_api;
        uint32_t m_currentImageIndex = 0;
        WindowHandle m_windowHandle;
        void destroySwapchainAndImages()
        {
            m_api->vkQueueWaitIdle(m_queue->m_queue);
            if (m_swapChain != VK_NULL_HANDLE)
            {
                m_api->vkDestroySwapchainKHR(m_api->m_device, m_swapChain, nullptr);
                m_swapChain = VK_NULL_HANDLE;
            }

            // Mark that it is no longer used
            m_images.clear();
        }

        void getWindowSize(int* widthOut, int* heightOut) const
        {
#if SLANG_WINDOWS_FAMILY
            RECT rc;
            ::GetClientRect((HWND)m_windowHandle.handleValues[0], &rc);
            *widthOut = rc.right - rc.left;
            *heightOut = rc.bottom - rc.top;
#elif defined(SLANG_ENABLE_XLIB)
            XWindowAttributes winAttr = {};
            XGetWindowAttributes(
                (Display*)m_windowHandle.handleValues[0],
                (Window)m_windowHandle.handleValues[1],
                &winAttr);

            *widthOut = winAttr.width;
            *heightOut = winAttr.height;
#else
            *widthOut = 0;
            *heightOut = 0;
#endif
        }

        Result createSwapchainAndImages()
        {
            int width, height;
            getWindowSize(&width, &height);

            VkExtent2D imageExtent = {};
            imageExtent.width = width;
            imageExtent.height = height;

            m_desc.width = width;
            m_desc.height = height;

            // catch this before throwing error
            if (width == 0 || height == 0)
            {
                return SLANG_FAIL;
            }

            // It is necessary to query the caps -> otherwise the LunarG verification layer will
            // issue an error
            {
                VkSurfaceCapabilitiesKHR surfaceCaps;

                SLANG_VK_RETURN_ON_FAIL(m_api->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                    m_api->m_physicalDevice, m_surface, &surfaceCaps));
            }

            VkPresentModeKHR presentMode;
            List<VkPresentModeKHR> presentModes;
            uint32_t numPresentModes = 0;
            m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(
                m_api->m_physicalDevice, m_surface, &numPresentModes, nullptr);
            presentModes.setCount(numPresentModes);
            m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(
                m_api->m_physicalDevice, m_surface, &numPresentModes, presentModes.getBuffer());

            {
                int numCheckPresentOptions = 3;
                VkPresentModeKHR presentOptions[] = {
                    VK_PRESENT_MODE_IMMEDIATE_KHR,
                    VK_PRESENT_MODE_MAILBOX_KHR,
                    VK_PRESENT_MODE_FIFO_KHR};
                if (m_desc.enableVSync)
                {
                    presentOptions[0] = VK_PRESENT_MODE_FIFO_KHR;
                    presentOptions[1] = VK_PRESENT_MODE_IMMEDIATE_KHR;
                    presentOptions[2] = VK_PRESENT_MODE_MAILBOX_KHR;
                }

                presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR; // Invalid

                // Find the first option that's available on the device
                for (int j = 0; j < numCheckPresentOptions; j++)
                {
                    if (presentModes.indexOf(presentOptions[j]) != Index(-1))
                    {
                        presentMode = presentOptions[j];
                        break;
                    }
                }

                if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
                {
                    return SLANG_FAIL;
                }
            }

            VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;

            VkSwapchainCreateInfoKHR swapchainDesc = {};
            swapchainDesc.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swapchainDesc.surface = m_surface;
            swapchainDesc.minImageCount = m_desc.imageCount;
            swapchainDesc.imageFormat = m_vkformat;
            swapchainDesc.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            swapchainDesc.imageExtent = imageExtent;
            swapchainDesc.imageArrayLayers = 1;
            swapchainDesc.imageUsage =
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            swapchainDesc.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainDesc.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            swapchainDesc.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            swapchainDesc.presentMode = presentMode;
            swapchainDesc.clipped = VK_TRUE;
            swapchainDesc.oldSwapchain = oldSwapchain;

            SLANG_VK_RETURN_ON_FAIL(m_api->vkCreateSwapchainKHR(
                m_api->m_device, &swapchainDesc, nullptr, &m_swapChain));

            uint32_t numSwapChainImages = 0;
            m_api->vkGetSwapchainImagesKHR(
                m_api->m_device, m_swapChain, &numSwapChainImages, nullptr);
            List<VkImage> vkImages;
            {
                vkImages.setCount(numSwapChainImages);
                m_api->vkGetSwapchainImagesKHR(
                    m_api->m_device, m_swapChain, &numSwapChainImages, vkImages.getBuffer());
            }

            for (uint32_t i = 0; i < m_desc.imageCount; i++)
            {
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
                RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc, m_renderer);
                image->m_image = vkImages[i];
                image->m_imageMemory = 0;
                image->m_vkformat = m_vkformat;
                image->m_isWeakImageReference = true;
                m_images.add(image);
            }
            return SLANG_OK;
        }
    public:
        ~SwapchainImpl()
        {
            destroySwapchainAndImages();
            if (m_surface)
            {
                m_api->vkDestroySurfaceKHR(m_api->m_instance, m_surface, nullptr);
                m_surface = VK_NULL_HANDLE;
            }
            m_renderer->m_api.vkDestroySemaphore(
                m_renderer->m_api.m_device, m_nextImageSemaphore, nullptr);
        }

        static Index _indexOfFormat(List<VkSurfaceFormatKHR>& formatsIn, VkFormat format)
        {
            const Index numFormats = formatsIn.getCount();
            const VkSurfaceFormatKHR* formats = formatsIn.getBuffer();

            for (Index i = 0; i < numFormats; ++i)
            {
                if (formats[i].format == format)
                {
                    return i;
                }
            }
            return -1;
        }

        Result init(VKDevice* renderer, const ISwapchain::Desc& desc, WindowHandle window)
        {
            m_desc = desc;
            m_renderer = renderer;
            m_api = &renderer->m_api;
            m_queue = static_cast<CommandQueueImpl*>(desc.queue);
            m_windowHandle = window;

            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            SLANG_VK_RETURN_ON_FAIL(renderer->m_api.vkCreateSemaphore(
                renderer->m_api.m_device, &semaphoreCreateInfo, nullptr, &m_nextImageSemaphore));

            m_queue = static_cast<CommandQueueImpl*>(desc.queue);

             // Make sure it's not set initially
            m_vkformat = VK_FORMAT_UNDEFINED;

#if SLANG_WINDOWS_FAMILY
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hinstance = ::GetModuleHandle(nullptr);
            surfaceCreateInfo.hwnd = (HWND)window.handleValues[0];
            SLANG_VK_RETURN_ON_FAIL(m_api->vkCreateWin32SurfaceKHR(
                m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#else
            VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.dpy = (Display*)window.handleValues[0];
            surfaceCreateInfo.window = (Window)window.handleValues[1];
            SLANG_VK_RETURN_ON_FAIL(m_api->vkCreateXlibSurfaceKHR(
                m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#endif

            VkBool32 supported = false;
            m_api->vkGetPhysicalDeviceSurfaceSupportKHR(
                m_api->m_physicalDevice, renderer->m_queueFamilyIndex, m_surface, &supported);

            uint32_t numSurfaceFormats = 0;
            List<VkSurfaceFormatKHR> surfaceFormats;
            m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(
                m_api->m_physicalDevice, m_surface, &numSurfaceFormats, nullptr);
            surfaceFormats.setCount(int(numSurfaceFormats));
            m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(
                m_api->m_physicalDevice, m_surface, &numSurfaceFormats, surfaceFormats.getBuffer());

            // Look for a suitable format
            List<VkFormat> formats;
            formats.add(VulkanUtil::getVkFormat(desc.format));
            // HACK! To check for a different format if couldn't be found
            if (desc.format == Format::RGBA_Unorm_UInt8)
            {
                formats.add(VK_FORMAT_B8G8R8A8_UNORM);
            }

            for (Index i = 0; i < formats.getCount(); ++i)
            {
                VkFormat format = formats[i];
                if (_indexOfFormat(surfaceFormats, format) >= 0)
                {
                    m_vkformat = format;
                }
            }

            if (m_vkformat == VK_FORMAT_UNDEFINED)
            {
                return SLANG_FAIL;
            }

            // Save the desc
            m_desc = desc;
            if (m_desc.format == Format::RGBA_Unorm_UInt8 &&
                m_vkformat == VK_FORMAT_B8G8R8A8_UNORM)
            {
                m_desc.format = Format::BGRA_Unorm_UInt8;
            }

            SLANG_RETURN_ON_FAIL(createSwapchainAndImages());
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }
        virtual SLANG_NO_THROW Result
            SLANG_MCALL getImage(uint32_t index, ITextureResource** outResource) override
        {
            if (m_images.getCount() <= (Index)index)
                return SLANG_FAIL;
            returnComPtr(outResource, m_images[index]);
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) override
        {
            SLANG_UNUSED(width);
            SLANG_UNUSED(height);
            destroySwapchainAndImages();
            return createSwapchainAndImages();
        }
        virtual SLANG_NO_THROW Result SLANG_MCALL present() override
        {
            uint32_t swapChainIndices[] = {uint32_t(m_currentImageIndex)};

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &m_swapChain;
            presentInfo.pImageIndices = swapChainIndices;
            Array<VkSemaphore, 2> waitSemaphores;
            for (auto s : m_queue->m_pendingWaitSemaphores)
            {
                if (s != VK_NULL_HANDLE)
                {
                    waitSemaphores.add(s);
                }
            }
            presentInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.getCount();
            if (presentInfo.waitSemaphoreCount)
            {
                presentInfo.pWaitSemaphores = waitSemaphores.getBuffer();
            }
            m_api->vkQueuePresentKHR(m_queue->m_queue, &presentInfo);
            m_queue->m_pendingWaitSemaphores[0] = VK_NULL_HANDLE;
            m_queue->m_pendingWaitSemaphores[1] = VK_NULL_HANDLE;
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override
        {
            if (!m_images.getCount())
                return -1;

            m_currentImageIndex = -1;
            VkResult result = m_api->vkAcquireNextImageKHR(
                m_api->m_device,
                m_swapChain,
                UINT64_MAX,
                m_nextImageSemaphore,
                VK_NULL_HANDLE,
                (uint32_t*)&m_currentImageIndex);

            if (result != VK_SUCCESS)
            {
                m_currentImageIndex = -1;
                destroySwapchainAndImages();
                return m_currentImageIndex;
            }
            // Make the queue's next submit wait on `m_nextImageSemaphore`.
            m_queue->m_pendingWaitSemaphores[1] = m_nextImageSemaphore;
            return m_currentImageIndex;
        }
    };

    VkBool32 handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg);

        /// Note that the outShaderModule value should be cleaned up when no longer needed by caller
        /// via vkShaderModuleDestroy()
    VkPipelineShaderStageCreateInfo compileEntryPoint(
        ISlangBlob* code,
        VkShaderStageFlagBits stage,
        VkShaderModule& outShaderModule);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

    void _transitionImageLayout(VkImage image, VkFormat format, const TextureResource::Desc& desc, VkImageLayout oldLayout, VkImageLayout newLayout);

    uint32_t getQueueFamilyIndex(ICommandQueue::QueueType queueType)
    {
        switch (queueType)
        {
        case ICommandQueue::QueueType::Graphics:
        default:
            return m_queueFamilyIndex;
        }
    }
public:
    // VKDevice members.

    DeviceInfo m_info;
    String m_adapterName;

    VkDebugReportCallbackEXT m_debugReportCallback = VK_NULL_HANDLE;

    VkDevice m_device = VK_NULL_HANDLE;

    VulkanModule m_module;
    VulkanApi m_api;

    VulkanDeviceQueue m_deviceQueue;
    uint32_t m_queueFamilyIndex;

    Desc m_desc;

    DescriptorSetAllocator descriptorSetAllocator;

    uint32_t m_queueAllocCount;

    // A list to hold objects that may have a strong back reference to the device
    // instance. Because of the pipeline cache in `RendererBase`, there could be a reference
    // cycle among `VKDevice`->`PipelineStateImpl`->`ShaderProgramImpl`->`VkDevice`.
    // Depending on whether a `PipelineState` objects gets stored in pipeline cache, there
    // may or may not be such a reference cycle.
    // We need to hold strong references to any objects that may become part of the reference
    // cycle here, so that when objects like `ShaderProgramImpl` lost all public refernces, we
    // can always safely break the strong reference in `ShaderProgramImpl::m_device` without
    // worrying the `ShaderProgramImpl` object getting destroyed after the completion of
    // `VKDevice::~VKDevice()'.
    ChunkedList<RefPtr<RefObject>, 1024> m_deviceObjectsWithPotentialBackReferences;
};

void VKDevice::PipelineCommandEncoder::init(CommandBufferImpl* commandBuffer)
{
    m_commandBuffer = commandBuffer;
    m_device = commandBuffer->m_renderer;
    m_vkCommandBuffer = m_commandBuffer->m_commandBuffer;
    m_api = &m_commandBuffer->m_renderer->m_api;
}

Result VKDevice::PipelineCommandEncoder::bindRootShaderObjectImpl(
    VkPipelineBindPoint bindPoint)
{
    // Obtain specialized root layout.
    auto rootObjectImpl = &m_commandBuffer->m_rootObject;

    auto specializedLayout = rootObjectImpl->getSpecializedLayout();
    if (!specializedLayout)
        return SLANG_FAIL;

    // We will set up the context required when binding shader objects
    // to the pipeline. Note that this is mostly just being packaged
    // together to minimize the number of parameters that have to
    // be dealt with in the complex recursive call chains.
    //
    RootBindingContext context;
    context.pipelineLayout = specializedLayout->m_pipelineLayout;
    context.device = m_device;
    context.descriptorSetAllocator = &m_commandBuffer->m_transientHeap->m_descSetAllocator;
    context.pushConstantRanges = specializedLayout->getAllPushConstantRanges().getArrayView();

    // The context includes storage for the descriptor sets we will bind,
    // and the number of sets we need to make space for is determined
    // by the specialized program layout.
    //
    List<VkDescriptorSet> descriptorSetsStorage;
    auto descriptorSetCount = specializedLayout->getTotalDescriptorSetCount();

    descriptorSetsStorage.setCount(descriptorSetCount);
    auto descriptorSets = descriptorSetsStorage.getBuffer();

    context.descriptorSets = descriptorSets;

    // We kick off recursive binding of shader objects to the pipeline (plus
    // the state in `context`).
    //
    // Note: this logic will directly write any push-constant ranges needed,
    // and will also fill in any descriptor sets. Currently it does not
    // *bind* the descriptor sets it fills in.
    //
    // TODO: It could probably bind the descriptor sets as well.
    //
    rootObjectImpl->bindAsRoot(this, context, specializedLayout);

    // Once we've filled in all the descriptor sets, we bind them
    // to the pipeline at once.
    //
    m_device->m_api.vkCmdBindDescriptorSets(
        m_commandBuffer->m_commandBuffer,
        bindPoint,
        specializedLayout->m_pipelineLayout,
        0,
        (uint32_t) descriptorSetCount,
        descriptorSets,
        0,
        nullptr);

    return SLANG_OK;
}


/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VKDevice::Buffer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Result VKDevice::Buffer::init(const VulkanApi& api, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqMemoryProperties)
{
    assert(!isInitialized());

    m_api = &api;
    m_memory = VK_NULL_HANDLE;
    m_buffer = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferCreateInfo.size = bufferSize;
    bufferCreateInfo.usage = usage;
    SLANG_VK_CHECK(api.vkCreateBuffer(api.m_device, &bufferCreateInfo, nullptr, &m_buffer));

    VkMemoryRequirements memoryReqs = {};
    api.vkGetBufferMemoryRequirements(api.m_device, m_buffer, &memoryReqs);

    int memoryTypeIndex = api.findMemoryTypeIndex(memoryReqs.memoryTypeBits, reqMemoryProperties);
    assert(memoryTypeIndex >= 0);

    VkMemoryPropertyFlags actualMemoryProperites = api.m_deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

    VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocateInfo.allocationSize = memoryReqs.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;
    VkMemoryAllocateFlagsInfo flagInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        flagInfo.deviceMask = 1;
        flagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        allocateInfo.pNext = &flagInfo;
    }
 
    SLANG_VK_CHECK(api.vkAllocateMemory(api.m_device, &allocateInfo, nullptr, &m_memory));
    SLANG_VK_CHECK(api.vkBindBufferMemory(api.m_device, m_buffer, m_memory, 0));

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VKDevice !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Result SLANG_MCALL createVKDevice(const IDevice::Desc* desc, IDevice** outRenderer)
{
    RefPtr<VKDevice> result = new VKDevice();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outRenderer, result);
    return SLANG_OK;
}

VKDevice::~VKDevice()
{
    // Check the device queue is valid else, we can't wait on it..
    if (m_deviceQueue.isValid())
    {
        waitForGpu();
    }

    m_shaderObjectLayoutCache = decltype(m_shaderObjectLayoutCache)();
    shaderCache.free();
    m_deviceObjectsWithPotentialBackReferences.clearAndDeallocate();

    // Same as clear but, also dtors all elements, which clear does not
    m_deviceQueue.destroy();

    descriptorSetAllocator.close();

    if (m_device != VK_NULL_HANDLE)
    {
        m_api.vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
        if (m_debugReportCallback != VK_NULL_HANDLE)
            m_api.vkDestroyDebugReportCallbackEXT(m_api.m_instance, m_debugReportCallback, nullptr);
        if (m_api.m_instance != VK_NULL_HANDLE)
            m_api.vkDestroyInstance(m_api.m_instance, nullptr);
    }
}


VkBool32 VKDevice::handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg)
{
    DebugMessageType msgType = DebugMessageType::Info;

    char const* severity = "message";
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        severity = "warning";
        msgType = DebugMessageType::Warning;
    }
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        severity = "error";
        msgType = DebugMessageType::Error;
    }

    // pMsg can be really big (it can be assembler dump for example)
    // Use a dynamic buffer to store
    size_t bufferSize = strlen(pMsg) + 1 + 1024;
    List<char> bufferArray;
    bufferArray.setCount(bufferSize);
    char* buffer = bufferArray.getBuffer();

    sprintf_s(buffer,
        bufferSize,
        "%s: %s %d: %s\n",
        pLayerPrefix,
        severity,
        msgCode,
        pMsg);

    getDebugCallback()->handleMessage(msgType, DebugMessageSource::Driver, buffer);
    return VK_FALSE;
}

/* static */VKAPI_ATTR VkBool32 VKAPI_CALL VKDevice::debugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
{
    return ((VKDevice*)pUserData)->handleDebugMessage(flags, objType, srcObject, location, msgCode, pLayerPrefix, pMsg);
}

VkPipelineShaderStageCreateInfo VKDevice::compileEntryPoint(
    ISlangBlob* code,
    VkShaderStageFlagBits stage,
    VkShaderModule& outShaderModule)
{
    char const* dataBegin = (char const*) code->getBufferPointer();
    char const* dataEnd = (char const*)code->getBufferPointer() + code->getBufferSize();

    // We need to make a copy of the code, since the Slang compiler
    // will free the memory after a compile request is closed.

    VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    moduleCreateInfo.pCode = (uint32_t*)code->getBufferPointer();
    moduleCreateInfo.codeSize = code->getBufferSize();

    VkShaderModule module;
    SLANG_VK_CHECK(m_api.vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &module));
    outShaderModule = module;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    shaderStageCreateInfo.stage = stage;

    shaderStageCreateInfo.module = module;
    shaderStageCreateInfo.pName = "main";

    return shaderStageCreateInfo;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

Result VKDevice::initVulkanInstanceAndDevice(bool useValidationLayer)
{
    m_queueAllocCount = 0;

    VkInstance instance = VK_NULL_HANDLE;
    VkApplicationInfo applicationInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    applicationInfo.pApplicationName = "slang-gfx";
    applicationInfo.pEngineName = "slang-gfx";
    applicationInfo.apiVersion = VK_API_VERSION_1_1;
    applicationInfo.engineVersion = 1;
    applicationInfo.applicationVersion = 1;

    for (int tryUseSurfaceExtensions = 1; tryUseSurfaceExtensions >= 0; tryUseSurfaceExtensions--)
    {
        Array<const char*, 4> instanceExtensions;

        instanceExtensions.add(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        if (tryUseSurfaceExtensions)
        {
            instanceExtensions.add(VK_KHR_SURFACE_EXTENSION_NAME);
#if SLANG_WINDOWS_FAMILY
            instanceExtensions.add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(SLANG_ENABLE_XLIB)
            instanceExtensions.add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#if ENABLE_VALIDATION_LAYER
            instanceExtensions.add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
        }

        VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.getCount();
        instanceCreateInfo.ppEnabledExtensionNames = &instanceExtensions[0];

        if (useValidationLayer)
        {
            // Depending on driver version, validation layer may or may not exist.
            // Newer drivers comes with "VK_LAYER_KHRONOS_validation", while older
            // drivers provide only the deprecated
            // "VK_LAYER_LUNARG_standard_validation" layer.
            // We will check what layers are available, and use the newer
            // "VK_LAYER_KHRONOS_validation" layer when possible.
            uint32_t layerCount;
            m_api.vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            List<VkLayerProperties> availableLayers;
            availableLayers.setCount(layerCount);
            m_api.vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.getBuffer());

            const char* layerNames[] = {nullptr};
            for (auto& layer : availableLayers)
            {
                if (strncmp(
                        layer.layerName,
                        "VK_LAYER_KHRONOS_validation",
                        sizeof("VK_LAYER_KHRONOS_validation")) == 0)
                {
                    layerNames[0] = "VK_LAYER_KHRONOS_validation";
                    break;
                }
            }
            // On older drivers, only "VK_LAYER_LUNARG_standard_validation" exists,
            // so we try to use it if we can't find "VK_LAYER_KHRONOS_validation".
            if (!layerNames[0])
            {
                for (auto& layer : availableLayers)
                {
                    if (strncmp(
                            layer.layerName,
                            "VK_LAYER_LUNARG_standard_validation",
                            sizeof("VK_LAYER_LUNARG_standard_validation")) == 0)
                    {
                        layerNames[0] = "VK_LAYER_LUNARG_standard_validation";
                        break;
                    }
                }
            }
            if (layerNames[0])
            {
                instanceCreateInfo.enabledLayerCount = SLANG_COUNT_OF(layerNames);
                instanceCreateInfo.ppEnabledLayerNames = layerNames;
            }
        }
        uint32_t apiVersionsToTry[] = {VK_API_VERSION_1_2, VK_API_VERSION_1_1, VK_API_VERSION_1_0};
        for (auto apiVersion : apiVersionsToTry)
        {
            applicationInfo.apiVersion = apiVersion;
            if (m_api.vkCreateInstance(&instanceCreateInfo, nullptr, &instance) == VK_SUCCESS)
            {
                break;
            }
        }
        if (instance)
            break;
    }
    if (!instance)
        return SLANG_FAIL;

    SLANG_RETURN_ON_FAIL(m_api.initInstanceProcs(instance));

    if (useValidationLayer && m_api.vkCreateDebugReportCallbackEXT)
    {
        VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

        VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
        debugCreateInfo.pfnCallback = &debugMessageCallback;
        debugCreateInfo.pUserData = this;
        debugCreateInfo.flags = debugFlags;

        SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, nullptr, &m_debugReportCallback));
    }

    uint32_t numPhysicalDevices = 0;
    SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr));

    List<VkPhysicalDevice> physicalDevices;
    physicalDevices.setCount(numPhysicalDevices);
    SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.getBuffer()));

    Index selectedDeviceIndex = 0;

    if (m_desc.adapter)
    {
        selectedDeviceIndex = -1;

        String lowerAdapter = String(m_desc.adapter).toLower();

        for (Index i = 0; i < physicalDevices.getCount(); ++i)
        {
            auto physicalDevice = physicalDevices[i];

            VkPhysicalDeviceProperties basicProps = {};
            m_api.vkGetPhysicalDeviceProperties(physicalDevice, &basicProps);

            String lowerName = String(basicProps.deviceName).toLower();

            if (lowerName.indexOf(lowerAdapter) != Index(-1))
            {
                selectedDeviceIndex = i;
                break;
            }
        }
        if (selectedDeviceIndex < 0)
        {
            // Device not found
            return SLANG_FAIL;
        }
    }

    SLANG_RETURN_ON_FAIL(m_api.initPhysicalDevice(physicalDevices[selectedDeviceIndex]));

    // Obtain the name of the selected adapter.
    {
        VkPhysicalDeviceProperties basicProps = {};
        m_api.vkGetPhysicalDeviceProperties(physicalDevices[selectedDeviceIndex], &basicProps);
        m_adapterName = basicProps.deviceName;
        m_info.adapterName = m_adapterName.begin();
    }

    List<const char*> deviceExtensions;
    deviceExtensions.add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &m_api.m_deviceFeatures;

    // Get the device features (doesn't use, but useful when debugging)
    if (m_api.vkGetPhysicalDeviceFeatures2)
    {
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        m_api.vkGetPhysicalDeviceFeatures2(m_api.m_physicalDevice, &deviceFeatures2);
    }

    VkPhysicalDeviceProperties basicProps = {};
    m_api.vkGetPhysicalDeviceProperties(m_api.m_physicalDevice, &basicProps);

    // Compute timestamp frequency.
    m_info.timestampFrequency = uint64_t(1e9 / basicProps.limits.timestampPeriod);

    // Get the API version
    const uint32_t majorVersion = VK_VERSION_MAJOR(basicProps.apiVersion);
    const uint32_t minorVersion = VK_VERSION_MINOR(basicProps.apiVersion);

    auto& extendedFeatures = m_api.m_extendedFeatures;

    // API version check, can't use vkGetPhysicalDeviceProperties2 yet since this device might not support it
    if (VK_MAKE_VERSION(majorVersion, minorVersion, 0) >= VK_API_VERSION_1_1 &&
        m_api.vkGetPhysicalDeviceProperties2 &&
        m_api.vkGetPhysicalDeviceFeatures2)
    {
        // Get device features
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // Inline uniform block
        extendedFeatures.inlineUniformBlockFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.inlineUniformBlockFeatures;

        // Buffer device address features
        extendedFeatures.bufferDeviceAddressFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.bufferDeviceAddressFeatures;

        // Ray query features
        extendedFeatures.rayQueryFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.rayQueryFeatures;

        // Acceleration structure features
        extendedFeatures.accelerationStructureFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.accelerationStructureFeatures;

        // Subgroup features
        extendedFeatures.shaderSubgroupExtendedTypeFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.shaderSubgroupExtendedTypeFeatures;

        // Extended dynamic states
        extendedFeatures.extendedDynamicStateFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.extendedDynamicStateFeatures;

        // Timeline Semaphore
        extendedFeatures.timelineFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.timelineFeatures;

        // Float16
        extendedFeatures.float16Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.float16Features;

        // 16-bit storage
        extendedFeatures.storage16BitFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.storage16BitFeatures;

        // Atomic64
        extendedFeatures.atomicInt64Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.atomicInt64Features;

        // Atomic Float
        // To detect atomic float we need
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPhysicalDeviceShaderAtomicFloatFeaturesEXT.html

        extendedFeatures.atomicFloatFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedFeatures.atomicFloatFeatures;

        m_api.vkGetPhysicalDeviceFeatures2(m_api.m_physicalDevice, &deviceFeatures2);

        if (deviceFeatures2.features.shaderResourceMinLod)
        {
            m_features.add("shader-resource-min-lod");
        }
        if (deviceFeatures2.features.shaderFloat64)
        {
            m_features.add("double");
        }
        if (deviceFeatures2.features.shaderInt64)
        {
            m_features.add("int64");
        }
        if (deviceFeatures2.features.shaderInt16)
        {
            m_features.add("int16");
        }
        // If we have float16 features then enable
        if (extendedFeatures.float16Features.shaderFloat16)
        {
            // Link into the creation features
            extendedFeatures.float16Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.float16Features;

            // Add the Float16 extension
            deviceExtensions.add(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);

            // We have half support
            m_features.add("half");
        }

        if (extendedFeatures.storage16BitFeatures.storageBuffer16BitAccess)
        {
            // Link into the creation features
            extendedFeatures.storage16BitFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.storage16BitFeatures;

            // Add the 16-bit storage extension
            deviceExtensions.add(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);

            // We have half support
            m_features.add("16-bit-storage");
        }

        if (extendedFeatures.atomicInt64Features.shaderBufferInt64Atomics)
        {
            // Link into the creation features
            extendedFeatures.atomicInt64Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.atomicInt64Features;

            deviceExtensions.add(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME);
            m_features.add("atomic-int64");
        }

        if (extendedFeatures.atomicFloatFeatures.shaderBufferFloat32AtomicAdd)
        {
            // Link into the creation features
            extendedFeatures.atomicFloatFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.atomicFloatFeatures;

            deviceExtensions.add(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
            m_features.add("atomic-float");
        }

        if (extendedFeatures.timelineFeatures.timelineSemaphore)
        {
            // Link into the creation features
            extendedFeatures.timelineFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.timelineFeatures;
            deviceExtensions.add(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
            m_features.add("timeline-semaphore");
        }

        if (extendedFeatures.extendedDynamicStateFeatures.extendedDynamicState)
        {
            // Link into the creation features
            extendedFeatures.extendedDynamicStateFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.extendedDynamicStateFeatures;
            deviceExtensions.add(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
            m_features.add("extended-dynamic-states");
        }

        if (extendedFeatures.shaderSubgroupExtendedTypeFeatures.shaderSubgroupExtendedTypes)
        {
            extendedFeatures.shaderSubgroupExtendedTypeFeatures.pNext =
                (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.shaderSubgroupExtendedTypeFeatures;
            deviceExtensions.add(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
            m_features.add("shader-subgroup-extended-types");
        }

        if (extendedFeatures.accelerationStructureFeatures.accelerationStructure)
        {
            extendedFeatures.accelerationStructureFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.accelerationStructureFeatures;
            deviceExtensions.add(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensions.add(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            m_features.add("acceleration-structure");
        }

        if (extendedFeatures.rayQueryFeatures.rayQuery)
        {
            extendedFeatures.rayQueryFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.rayQueryFeatures;
            deviceExtensions.add(VK_KHR_RAY_QUERY_EXTENSION_NAME);
            m_features.add("ray-query");
            m_features.add("ray-tracing");
        }

        if (extendedFeatures.bufferDeviceAddressFeatures.bufferDeviceAddress)
        {
            extendedFeatures.bufferDeviceAddressFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.bufferDeviceAddressFeatures;
            deviceExtensions.add(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            m_features.add("buffer-device-address");
        }

        if (extendedFeatures.inlineUniformBlockFeatures.inlineUniformBlock)
        {
            extendedFeatures.inlineUniformBlockFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedFeatures.inlineUniformBlockFeatures;
            deviceExtensions.add(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            m_features.add("inline-uniform-block");
        }
    }
    if (m_api.m_module->isSoftware())
    {
        m_features.add("software-device");
    }
    else
    {
        m_features.add("hardware-device");
    }

    m_queueFamilyIndex = m_api.findQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    assert(m_queueFamilyIndex >= 0);

    float queuePriority = 0.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueFamilyIndex = m_queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    deviceCreateInfo.enabledExtensionCount = uint32_t(deviceExtensions.getCount());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.getBuffer();
 
    if (m_api.vkCreateDevice(m_api.m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
        return SLANG_FAIL;
    SLANG_RETURN_ON_FAIL(m_api.initDeviceProcs(m_device));

    return SLANG_OK;
}

SlangResult VKDevice::initialize(const Desc& desc)
{
    // Initialize device info.
    {
        m_info.apiName = "Vulkan";
        m_info.bindingStyle = BindingStyle::Vulkan;
        m_info.projectionStyle = ProjectionStyle::Vulkan;
        m_info.deviceType = DeviceType::Vulkan;
        static const float kIdentity[] = {1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    m_desc = desc;

    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

    SLANG_RETURN_ON_FAIL(m_module.init());
    SLANG_RETURN_ON_FAIL(m_api.initGlobalProcs(m_module));
    descriptorSetAllocator.m_api = &m_api;

    SLANG_RETURN_ON_FAIL(initVulkanInstanceAndDevice(ENABLE_VALIDATION_LAYER != 0));
    {
        VkQueue queue;
        m_api.vkGetDeviceQueue(m_device, m_queueFamilyIndex, 0, &queue);
        SLANG_RETURN_ON_FAIL(m_deviceQueue.init(m_api, queue, m_queueFamilyIndex));
    }

    SLANG_RETURN_ON_FAIL(slangContext.initialize(
        desc.slang,
        SLANG_SPIRV,
        "sm_5_1",
        makeArray(slang::PreprocessorMacroDesc{ "__VK__", "1" }).getView()));
    return SLANG_OK;
}

void VKDevice::waitForGpu()
{
    m_deviceQueue.flushAndWait();
}

Result VKDevice::TransientResourceHeapImpl::init(
    const ITransientResourceHeap::Desc& desc,
    VKDevice* device)
{
    Super::init(
        desc,
        (uint32_t)device->m_api.m_deviceProperties.limits.minUniformBufferOffsetAlignment,
        device);

    m_descSetAllocator.m_api = &device->m_api;

    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex =
        device->getQueueFamilyIndex(ICommandQueue::QueueType::Graphics);
    device->m_api.vkCreateCommandPool(
        device->m_api.m_device, &poolCreateInfo, nullptr, &m_commandPool);

    advanceFence();
    return SLANG_OK;
}

Result VKDevice::TransientResourceHeapImpl::createCommandBuffer(ICommandBuffer** outCmdBuffer)
{
    if (m_commandBufferAllocId < (uint32_t)m_commandBufferPool.getCount())
    {
        auto result = m_commandBufferPool[m_commandBufferAllocId];
        result->beginCommandBuffer();
        m_commandBufferAllocId++;
        returnComPtr(outCmdBuffer, result);
        return SLANG_OK;
    }

    RefPtr<CommandBufferImpl> commandBuffer = new CommandBufferImpl();
    SLANG_RETURN_ON_FAIL(commandBuffer->init(
        m_device, m_commandPool, this));
    m_commandBufferPool.add(commandBuffer);
    m_commandBufferAllocId++;
    returnComPtr(outCmdBuffer, commandBuffer);
    return SLANG_OK;
}

Result VKDevice::TransientResourceHeapImpl::synchronizeAndReset()
{
    m_commandBufferAllocId = 0;
    auto& api = m_device->m_api;
    if (api.vkWaitForFences(
            api.m_device, (uint32_t)m_fences.getCount(), m_fences.getBuffer(), 1, UINT64_MAX) !=
        VK_SUCCESS)
    {
        return SLANG_FAIL;
    }
    api.vkResetCommandPool(api.m_device, m_commandPool, 0);
    m_descSetAllocator.reset();
    m_fenceIndex = 0;
    Super::reset();
    return SLANG_OK;
}

Result VKDevice::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc,
    ITransientResourceHeap** outHeap)
{
    RefPtr<TransientResourceHeapImpl> result = new TransientResourceHeapImpl();
    SLANG_RETURN_ON_FAIL(result->init(desc, this));
    returnComPtr(outHeap, result);
    return SLANG_OK;
}

Result VKDevice::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    // Only support one queue for now.
    if (m_queueAllocCount != 0)
        return SLANG_FAIL;
    auto queueFamilyIndex = m_api.findQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    VkQueue vkQueue;
    m_api.vkGetDeviceQueue(m_api.m_device, queueFamilyIndex, 0, &vkQueue);
    RefPtr<CommandQueueImpl> result = new CommandQueueImpl();
    result->init(this, vkQueue, queueFamilyIndex);
    returnComPtr(outQueue, result);
    m_queueAllocCount++;
    return SLANG_OK;
}

Result VKDevice::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
#if !defined(SLANG_ENABLE_XLIB)
    if (window.type == WindowHandle::Type::XLibHandle)
    {
        return SLANG_FAIL;
    }
#endif

    RefPtr<SwapchainImpl> sc = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(sc->init(this, desc, window));
    returnComPtr(outSwapchain, sc);
    return SLANG_OK;
}

Result VKDevice::createFramebufferLayout(const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    SLANG_RETURN_ON_FAIL(layout->init(this, desc));
    m_deviceObjectsWithPotentialBackReferences.add(layout);
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result VKDevice::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc,
    IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl();
    SLANG_RETURN_ON_FAIL(result->init(this, desc));
    returnComPtr(outRenderPassLayout, result);
    return SLANG_OK;
}

Result VKDevice::createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer)
{
    RefPtr<FramebufferImpl> fb = new FramebufferImpl();
    SLANG_RETURN_ON_FAIL(fb->init(this, desc));
    returnComPtr(outFramebuffer, fb);
    return SLANG_OK;
}

SlangResult VKDevice::readTextureResource(
    ITextureResource* texture,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    SLANG_UNUSED(texture);
    SLANG_UNUSED(outBlob);
    SLANG_UNUSED(outRowPitch);
    SLANG_UNUSED(outPixelSize);
    return SLANG_FAIL;
}

SlangResult VKDevice::readBufferResource(
    IBufferResource* inBuffer,
    size_t offset,
    size_t size,
    ISlangBlob** outBlob)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(inBuffer);
    
    RefPtr<ListBlob> blob = new ListBlob();
    blob->m_data.setCount(size);

    // create staging buffer
    Buffer staging;

    SLANG_RETURN_ON_FAIL(staging.init(
        m_api,
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    // Copy from real buffer to staging buffer
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    VkBufferCopy copyInfo = {};
    copyInfo.size = size;
    copyInfo.srcOffset = offset;
    m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_buffer.m_buffer, staging.m_buffer, 1, &copyInfo);

    m_deviceQueue.flushAndWait();

    // Write out the data from the buffer
    void* mappedData = nullptr;
    SLANG_RETURN_ON_FAIL(
        m_api.vkMapMemory(m_device, staging.m_memory, 0, size, 0, &mappedData));

    ::memcpy(blob->m_data.getBuffer(), mappedData, size);
    m_api.vkUnmapMemory(m_device, staging.m_memory);

    returnComPtr(outBlob, blob);
    return SLANG_OK;
}

Result VKDevice::getAccelerationStructurePrebuildInfo(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IAccelerationStructure::PrebuildInfo* outPrebuildInfo)
{
    if (!m_api.vkGetAccelerationStructureBuildSizesKHR)
    {
        return SLANG_E_NOT_AVAILABLE;
    }
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    AccelerationStructureBuildGeometryInfoBuilder geomInfoBuilder;
    SLANG_RETURN_ON_FAIL(geomInfoBuilder.build(buildInputs, getDebugCallback()));
    m_api.vkGetAccelerationStructureBuildSizesKHR(
        m_api.m_device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &geomInfoBuilder.buildInfo,
        geomInfoBuilder.primitiveCounts.getBuffer(),
        &sizeInfo);
    outPrebuildInfo->resultDataMaxSize = sizeInfo.accelerationStructureSize;
    outPrebuildInfo->scratchDataSize = sizeInfo.buildScratchSize;
    outPrebuildInfo->updateScratchDataSize = sizeInfo.updateScratchSize;
    return SLANG_OK;
}

Result VKDevice::createAccelerationStructure(
    const IAccelerationStructure::CreateDesc& desc,
    IAccelerationStructure** outAS)
{
    if (!m_api.vkCreateAccelerationStructureKHR)
    {
        return SLANG_E_NOT_AVAILABLE;
    }
    RefPtr<AccelerationStructureImpl> resultAS = new AccelerationStructureImpl();
    resultAS->m_offset = desc.offset;
    resultAS->m_size = desc.size;
    resultAS->m_buffer = static_cast<BufferResourceImpl*>(desc.buffer);
    resultAS->m_device = this;
    resultAS->m_desc.type = IResourceView::Type::AccelerationStructure;
    VkAccelerationStructureCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    createInfo.buffer = resultAS->m_buffer->m_buffer.m_buffer;
    createInfo.offset = desc.offset;
    createInfo.size = desc.size;
    switch (desc.kind)
    {
    case IAccelerationStructure::Kind::BottomLevel:
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        break;
    case IAccelerationStructure::Kind::TopLevel:
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        break;
    default:
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "invalid value of IAccelerationStructure::Kind encountered in desc.kind");
        return SLANG_E_INVALID_ARG;
    }

    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateAccelerationStructureKHR(
        m_api.m_device, &createInfo, nullptr, &resultAS->m_vkHandle));
    returnComPtr(outAS, resultAS);
    return SLANG_OK;
}

static VkBufferUsageFlagBits _calcBufferUsageFlags(ResourceState state)
{
    switch (state)
    {
    case ResourceState::VertexBuffer:
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    case ResourceState::IndexBuffer:
        return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    case ResourceState::ConstantBuffer:
        return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    case ResourceState::StreamOutput:
        return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
    case ResourceState::RenderTarget:
    case ResourceState::DepthRead:
    case ResourceState::DepthWrite:
        {
            assert(!"Invalid resource state for buffer resource.");
            return VkBufferUsageFlagBits(0);
        }
    case ResourceState::UnorderedAccess:
        return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    case ResourceState::ShaderResource:
        return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    case ResourceState::CopySource:
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    case ResourceState::CopyDestination:
        return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    case ResourceState::AccelerationStructure:
        return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    default:
        return VkBufferUsageFlagBits(0);
    }
}

static VkBufferUsageFlagBits _calcBufferUsageFlags(ResourceStateSet states)
{
    int dstFlags = 0;
    for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; i++)
    {
        auto state = (ResourceState)i;
        if (states.contains(state))
            dstFlags |= _calcBufferUsageFlags(state);
    }
    return VkBufferUsageFlagBits(dstFlags);
}

static VkImageUsageFlagBits _calcImageUsageFlags(ResourceState state)
{
    switch (state)
    {
    case ResourceState::RenderTarget:
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    case ResourceState::DepthWrite:
        return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    case ResourceState::DepthRead:
        return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    case ResourceState::ShaderResource:
        return VK_IMAGE_USAGE_SAMPLED_BIT;
    case ResourceState::UnorderedAccess:
        return VK_IMAGE_USAGE_STORAGE_BIT;
    case ResourceState::CopySource:
        return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    case ResourceState::CopyDestination:
        return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    default:
        {
            assert(!"Unsupported");
            return VkImageUsageFlagBits(0);
        }
    }
}

static VkImageUsageFlagBits _calcImageUsageFlags(ResourceStateSet states)
{
    int dstFlags = 0;
    for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; i++)
    {
        auto state = (ResourceState)i;
        if (states.contains(state))
            dstFlags |= _calcImageUsageFlags(state);
    }
    return VkImageUsageFlagBits(dstFlags);
}

static VkImageUsageFlags _calcImageUsageFlags(
    ResourceStateSet states,
    int cpuAccessFlags,
    const void* initData)
{
    VkImageUsageFlags usage = _calcImageUsageFlags(states);

    if ((cpuAccessFlags & AccessFlag::Write) || initData)
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    return usage;
}

bool isDepthFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    }
    return false;
}

bool isStencilFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;
    }
    return false;
}

void VKDevice::_transitionImageLayout(VkImage image, VkFormat format, const TextureResource::Desc& desc, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (isDepthFormat(format))
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (isStencilFormat(format))
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    if (barrier.subresourceRange.aspectMask == 0)
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = desc.numMipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (
        oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (
        oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ||
         newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if (
        oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if (
        oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else
    {
        assert(!"unsupported layout transition!");
        return;
    }

    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    m_api.vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

size_t calcRowSize(Format format, int width)
{
    size_t pixelSize = gfxGetFormatSize(format);
    if (pixelSize == 0)
    {
        return 0;
    }
    return size_t(pixelSize * width);
}

size_t calcNumRows(Format format, int height)
{
    return (size_t)height;
}

Result VKDevice::createTextureResource(const ITextureResource::Desc& descIn, const ITextureResource::SubresourceData* initData, ITextureResource** outResource)
{
    TextureResource::Desc desc = fixupTextureDesc(descIn);

    const VkFormat format = VulkanUtil::getVkFormat(desc.format);
    if (format == VK_FORMAT_UNDEFINED)
    {
        assert(!"Unhandled image format");
        return SLANG_FAIL;
    }

    const int arraySize = calcEffectiveArraySize(desc);

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(desc, this));
    texture->m_vkformat = format;
    // Create the image
    {
        VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

        switch (desc.type)
        {
            case IResource::Type::Texture1D:
            {
                imageInfo.imageType = VK_IMAGE_TYPE_1D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), 1, 1 };
                break;
            }
            case IResource::Type::Texture2D:
            {
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1 };
                break;
            }
            case IResource::Type::TextureCube:
            {
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), uint32_t(descIn.size.height), 1 };
                break;
            }
            case IResource::Type::Texture3D:
            {
                // Can't have an array and 3d texture
                assert(desc.arraySize <= 1);

                imageInfo.imageType = VK_IMAGE_TYPE_3D;
                imageInfo.extent = VkExtent3D{ uint32_t(descIn.size.width), uint32_t(descIn.size.height), uint32_t(descIn.size.depth) };
                break;
            }
            default:
            {
                assert(!"Unhandled type");
                return SLANG_FAIL;
            }
        }

        imageInfo.mipLevels = desc.numMipLevels;
        imageInfo.arrayLayers = arraySize;

        imageInfo.format = format;

        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = _calcImageUsageFlags(desc.allowedStates, desc.cpuAccessFlags, initData);
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateImage(m_device, &imageInfo, nullptr, &texture->m_image));
    }

    VkMemoryRequirements memRequirements;
    m_api.vkGetImageMemoryRequirements(m_device, texture->m_image, &memRequirements);

    // Allocate the memory
    {
        VkMemoryPropertyFlags reqMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};

        int memoryTypeIndex = m_api.findMemoryTypeIndex(memRequirements.memoryTypeBits, reqMemoryProperties);
        assert(memoryTypeIndex >= 0);

        VkMemoryPropertyFlags actualMemoryProperites = m_api.m_deviceMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;

        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        SLANG_VK_RETURN_ON_FAIL(m_api.vkAllocateMemory(m_device, &allocInfo, nullptr, &texture->m_imageMemory));
    }

    // Bind the memory to the image
    m_api.vkBindImageMemory(m_device, texture->m_image, texture->m_imageMemory, 0);

    Buffer uploadBuffer;
    if (initData)
    {
        List<TextureResource::Size> mipSizes;

        VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

        const int numMipMaps = desc.numMipLevels;

        // Calculate how large the buffer has to be
        size_t bufferSize = 0;
        // Calculate how large an array entry is
        for (int j = 0; j < numMipMaps; ++j)
        {
            const TextureResource::Size mipSize = calcMipSize(desc.size, j);

            auto rowSizeInBytes = calcRowSize(desc.format, mipSize.width);
            auto numRows = calcNumRows(desc.format, mipSize.height);

            mipSizes.add(mipSize);

            bufferSize += (rowSizeInBytes * numRows) * mipSize.depth;
        }


        // Calculate the total size taking into account the array
        bufferSize *= arraySize;

        SLANG_RETURN_ON_FAIL(uploadBuffer.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        assert(mipSizes.getCount() == numMipMaps);

        // Copy into upload buffer
        {
            int subResourceCounter = 0;

            uint8_t* dstData;
            m_api.vkMapMemory(m_device, uploadBuffer.m_memory, 0, bufferSize, 0, (void**)&dstData);

            size_t dstSubresourceOffset = 0;
            for (int i = 0; i < arraySize; ++i)
            {
                for (Index j = 0; j < mipSizes.getCount(); ++j)
                {
                    const auto& mipSize = mipSizes[j];

                    int subResourceIndex = subResourceCounter++;
                    auto initSubresource = initData[subResourceIndex];

                    const ptrdiff_t srcRowStride = (ptrdiff_t)initSubresource.strideY;
                    const ptrdiff_t srcLayerStride = (ptrdiff_t)initSubresource.strideZ;

                    auto dstRowSizeInBytes = calcRowSize(desc.format, mipSize.width);
                    auto numRows = calcNumRows(desc.format, mipSize.height);
                    auto dstLayerSizeInBytes = dstRowSizeInBytes * numRows;

                    const uint8_t* srcLayer = (const uint8_t*) initSubresource.data;
                    uint8_t* dstLayer = dstData + dstSubresourceOffset;

                    for (int k = 0; k < mipSize.depth; k++)
                    {
                        const uint8_t* srcRow = srcLayer;
                        uint8_t* dstRow = dstLayer;

                        for (uint32_t l = 0; l < numRows; l++)
                        {
                            ::memcpy(dstRow, srcRow, dstRowSizeInBytes);

                            dstRow += dstRowSizeInBytes;
                            srcRow += srcRowStride;
                        }

                        dstLayer += dstLayerSizeInBytes;
                        srcLayer += srcLayerStride;
                    }

                    dstSubresourceOffset += dstLayerSizeInBytes * mipSize.depth;
                }
            }

            m_api.vkUnmapMemory(m_device, uploadBuffer.m_memory);
        }

        _transitionImageLayout(texture->m_image, format, *texture->getDesc(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        {
            size_t srcOffset = 0;
            for (int i = 0; i < arraySize; ++i)
            {
                for (Index j = 0; j < mipSizes.getCount(); ++j)
                {
                    const auto& mipSize = mipSizes[j];

                    auto rowSizeInBytes = calcRowSize(desc.format, mipSize.width);
                    auto numRows = calcNumRows(desc.format, mipSize.height);

                    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkBufferImageCopy.html
                    // bufferRowLength and bufferImageHeight specify the data in buffer memory as a subregion of a larger two- or three-dimensional image,
                    // and control the addressing calculations of data in buffer memory. If either of these values is zero, that aspect of the buffer memory
                    // is considered to be tightly packed according to the imageExtent.

                    VkBufferImageCopy region = {};

                    region.bufferOffset = srcOffset;
                    region.bufferRowLength = 0; //rowSizeInBytes;
                    region.bufferImageHeight = 0;

                    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.imageSubresource.mipLevel = uint32_t(j);
                    region.imageSubresource.baseArrayLayer = i;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = { 0, 0, 0 };
                    region.imageExtent = { uint32_t(mipSize.width), uint32_t(mipSize.height), uint32_t(mipSize.depth) };

                    // Do the copy (do all depths in a single go)
                    m_api.vkCmdCopyBufferToImage(commandBuffer, uploadBuffer.m_buffer, texture->m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

                    // Next
                    srcOffset += rowSizeInBytes * numRows * mipSize.depth;
                }
            }
        }
        auto defaultLayout = VulkanUtil::getImageLayoutFromState(desc.defaultState);
        _transitionImageLayout(
            texture->m_image,
            format,
            *texture->getDesc(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            defaultLayout);
    }
    else
    {
        auto defaultLayout = VulkanUtil::getImageLayoutFromState(desc.defaultState);
        if (defaultLayout != VK_IMAGE_LAYOUT_UNDEFINED)
        {
            _transitionImageLayout(
                texture->m_image,
                format,
                *texture->getDesc(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                defaultLayout);
        }
    }
    m_deviceQueue.flushAndWait();
    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result VKDevice::createBufferResource(const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc desc = fixupBufferDesc(descIn);

    const size_t bufferSize = desc.sizeInBytes;

    VkMemoryPropertyFlags reqMemoryProperties = 0;

    VkBufferUsageFlags usage = _calcBufferUsageFlags(desc.allowedStates);
    if (m_api.m_extendedFeatures.bufferDeviceAddressFeatures.bufferDeviceAddress)
    {
        usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    if (initData)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    if (desc.allowedStates.contains(ResourceState::ConstantBuffer))
    {
        reqMemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(desc, this));
    SLANG_RETURN_ON_FAIL(buffer->m_buffer.init(m_api, desc.sizeInBytes, usage, reqMemoryProperties));

    if ((desc.cpuAccessFlags & AccessFlag::Write) || initData)
    {
        SLANG_RETURN_ON_FAIL(buffer->m_uploadBuffer.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    }

    if (initData)
    {
        // TODO: only create staging buffer if the memory type
        // used for the buffer doesn't let us fill things in
        // directly.
        // Copy into staging buffer
        void* mappedData = nullptr;
        SLANG_VK_CHECK(m_api.vkMapMemory(m_device, buffer->m_uploadBuffer.m_memory, 0, bufferSize, 0, &mappedData));
        ::memcpy(mappedData, initData, bufferSize);
        m_api.vkUnmapMemory(m_device, buffer->m_uploadBuffer.m_memory);

        // Copy from staging buffer to real buffer
        VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

        VkBufferCopy copyInfo = {};
        copyInfo.size = bufferSize;
        m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_uploadBuffer.m_buffer, buffer->m_buffer.m_buffer, 1, &copyInfo);
        m_deviceQueue.flush();
    }

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

VkFilter translateFilterMode(TextureFilteringMode mode)
{
    switch (mode)
    {
    default:
        return VkFilter(0);

#define CASE(SRC, DST) \
    case TextureFilteringMode::SRC: return VK_FILTER_##DST

        CASE(Point, NEAREST);
        CASE(Linear, LINEAR);

#undef CASE
    }
}

VkSamplerMipmapMode translateMipFilterMode(TextureFilteringMode mode)
{
    switch (mode)
    {
    default:
        return VkSamplerMipmapMode(0);

#define CASE(SRC, DST) \
    case TextureFilteringMode::SRC: return VK_SAMPLER_MIPMAP_MODE_##DST

        CASE(Point, NEAREST);
        CASE(Linear, LINEAR);

#undef CASE
    }
}

VkSamplerAddressMode translateAddressingMode(TextureAddressingMode mode)
{
    switch (mode)
    {
    default:
        return VkSamplerAddressMode(0);

#define CASE(SRC, DST) \
    case TextureAddressingMode::SRC: return VK_SAMPLER_ADDRESS_MODE_##DST

    CASE(Wrap,          REPEAT);
    CASE(ClampToEdge,   CLAMP_TO_EDGE);
    CASE(ClampToBorder, CLAMP_TO_BORDER);
    CASE(MirrorRepeat,  MIRRORED_REPEAT);
    CASE(MirrorOnce,    MIRROR_CLAMP_TO_EDGE);

#undef CASE
    }
}

static VkCompareOp translateComparisonFunc(ComparisonFunc func)
{
    switch (func)
    {
    default:
        // TODO: need to report failures
        return VK_COMPARE_OP_ALWAYS;

#define CASE(FROM, TO) \
    case ComparisonFunc::FROM: return VK_COMPARE_OP_##TO

        CASE(Never, NEVER);
        CASE(Less, LESS);
        CASE(Equal, EQUAL);
        CASE(LessEqual, LESS_OR_EQUAL);
        CASE(Greater, GREATER);
        CASE(NotEqual, NOT_EQUAL);
        CASE(GreaterEqual, GREATER_OR_EQUAL);
        CASE(Always, ALWAYS);
#undef CASE
    }
}

static VkStencilOp translateStencilOp(StencilOp op)
{
    switch (op)
    {
    case StencilOp::DecrementSaturate:
        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOp::DecrementWrap:
        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    case StencilOp::IncrementSaturate:
        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOp::IncrementWrap:
        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOp::Invert:
        return VK_STENCIL_OP_INVERT;
    case StencilOp::Keep:
        return VK_STENCIL_OP_KEEP;
    case StencilOp::Replace:
        return VK_STENCIL_OP_REPLACE;
    case StencilOp::Zero:
        return VK_STENCIL_OP_ZERO;
    default:
        return VK_STENCIL_OP_KEEP;
    }
}

static VkStencilOpState translateStencilState(DepthStencilOpDesc desc)
{
    VkStencilOpState rs;
    rs.compareMask = 0xFF;
    rs.compareOp = translateComparisonFunc(desc.stencilFunc);
    rs.depthFailOp = translateStencilOp(desc.stencilDepthFailOp);
    rs.failOp = translateStencilOp(desc.stencilFailOp);
    rs.passOp = translateStencilOp(desc.stencilPassOp);
    rs.reference = 0;
    rs.writeMask = 0xFF;
    return rs;
}

Result VKDevice::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

    samplerInfo.magFilter = translateFilterMode(desc.minFilter);
    samplerInfo.minFilter = translateFilterMode(desc.magFilter);

    samplerInfo.addressModeU = translateAddressingMode(desc.addressU);
    samplerInfo.addressModeV = translateAddressingMode(desc.addressV);
    samplerInfo.addressModeW = translateAddressingMode(desc.addressW);

    samplerInfo.anisotropyEnable = desc.maxAnisotropy > 1;
    samplerInfo.maxAnisotropy = (float) desc.maxAnisotropy;

    // TODO: support translation of border color...
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = desc.reductionOp == TextureReductionOp::Comparison;
    samplerInfo.compareOp = translateComparisonFunc(desc.comparisonFunc);
    samplerInfo.mipmapMode = translateMipFilterMode(desc.mipFilter);

    VkSampler sampler;
    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler));

    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl(this);
    samplerImpl->m_sampler = sampler;
    returnComPtr(outSampler, samplerImpl);
    return SLANG_OK;
}

Result VKDevice::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = static_cast<TextureResourceImpl*>(texture);
    RefPtr<TextureResourceViewImpl> view = new TextureResourceViewImpl(this);
    view->m_texture = resourceImpl;
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.format = resourceImpl->m_vkformat;
    createInfo.image = resourceImpl->m_image;
    createInfo.components = VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,VK_COMPONENT_SWIZZLE_B,VK_COMPONENT_SWIZZLE_A };
    switch (resourceImpl->getType())
    {
    case IResource::Type::Texture1D:
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
        break;
    case IResource::Type::Texture2D:
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case IResource::Type::Texture3D:
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case IResource::Type::TextureCube:
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        break;
    default:
        SLANG_UNIMPLEMENTED_X("Unknown Texture type.");
        break;
    }
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    createInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    switch (desc.type)
    {
    case IResourceView::Type::DepthStencil:
        view->m_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        switch (resourceImpl->m_vkformat)
        {
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
        case VK_FORMAT_S8_UINT:
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
        default:
            break;
        }
        createInfo.subresourceRange.levelCount = 1;
        break;
    case IResourceView::Type::RenderTarget:
        view->m_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        createInfo.subresourceRange.levelCount = 1;
        break;
    case IResourceView::Type::ShaderResource:
        view->m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case IResourceView::Type::UnorderedAccess:
        view->m_layout = VK_IMAGE_LAYOUT_GENERAL;
        break;
    default:
        SLANG_UNIMPLEMENTED_X("Unknown TextureViewDesc type.");
        break;
    }
    view->m_desc = desc;
    m_api.vkCreateImageView(m_device, &createInfo, nullptr, &view->m_view);
    returnComPtr(outView, view);
    return SLANG_OK;
}

Result VKDevice::createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*) buffer;

    // TODO: These should come from the `ResourceView::Desc`
    VkDeviceSize offset = 0;
    VkDeviceSize size = resourceImpl->getDesc()->sizeInBytes;

    // There are two different cases we need to think about for buffers.
    //
    // One is when we have a "uniform texel buffer" or "storage texel buffer,"
    // in which case we need to construct a `VkBufferView` to represent the
    // formatting that is applied to the buffer. This case would correspond
    // to a `textureBuffer` or `imageBuffer` in GLSL, and more or less to
    // `Buffer<..>` or `RWBuffer<...>` in HLSL.
    //
    // The other case is a `storage buffer` which is the catch-all for any
    // non-formatted R/W access to a buffer. In GLSL this is a `buffer { ... }`
    // declaration, while in HLSL it covers a bunch of different `RW*Buffer`
    // cases. In these cases we do *not* need a `VkBufferView`, but in
    // order to be compatible with other APIs that require views for any
    // potentially writable access, we will have to create one anyway.
    //
    // We will distinguish the two cases by looking at whether the view
    // is being requested with a format or not.
    //

    switch(desc.type)
    {
    default:
        assert(!"unhandled");
        return SLANG_FAIL;

    case IResourceView::Type::UnorderedAccess:
    case IResourceView::Type::ShaderResource:
        // Is this a formatted view?
        //
        if (desc.format == Format::Unknown)
        {
            // Buffer usage that doesn't involve formatting doesn't
            // require a view in Vulkan.
            RefPtr<PlainBufferResourceViewImpl> viewImpl = new PlainBufferResourceViewImpl(this);
            viewImpl->m_buffer = resourceImpl;
            viewImpl->offset = 0;
            viewImpl->size = size;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        //
        // If the view is formatted, then we need to handle
        // it just like we would for a "sampled" buffer:
        //
        // FALLTHROUGH
        {
            VkBufferViewCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };

            info.format = VulkanUtil::getVkFormat(desc.format);
            info.buffer = resourceImpl->m_buffer.m_buffer;
            info.offset = offset;
            info.range = size;

            VkBufferView view;
            SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateBufferView(m_device, &info, nullptr, &view));

            RefPtr<TexelBufferResourceViewImpl> viewImpl = new TexelBufferResourceViewImpl(this);
            viewImpl->m_buffer = resourceImpl;
            viewImpl->m_view = view;
            viewImpl->m_desc = desc;

            returnComPtr(outView, viewImpl);
            return SLANG_OK;
        }
        break;
    }
}

Result VKDevice::createInputLayout(const InputElementDesc* elements, UInt numElements, IInputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    List<VkVertexInputAttributeDescription>& dstVertexDescs = layout->m_vertexDescs;

    size_t vertexSize = 0;
    dstVertexDescs.setCount(numElements);

    for (UInt i = 0; i <  numElements; ++i)
    {
        const InputElementDesc& srcDesc = elements[i];
        VkVertexInputAttributeDescription& dstDesc = dstVertexDescs[i];

        dstDesc.location = uint32_t(i);
        dstDesc.binding = 0;
        dstDesc.format = VulkanUtil::getVkFormat(srcDesc.format);
        if (dstDesc.format == VK_FORMAT_UNDEFINED)
        {
            return SLANG_FAIL;
        }

        dstDesc.offset = uint32_t(srcDesc.offset);

        const size_t elementSize = gfxGetFormatSize(srcDesc.format);
        assert(elementSize > 0);
        const size_t endElement = srcDesc.offset + elementSize;

        vertexSize = (vertexSize < endElement) ? endElement : vertexSize;
    }

    // Work out the overall size
    layout->m_vertexSize = int(vertexSize);
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

static VkImageViewType _calcImageViewType(ITextureResource::Type type, const ITextureResource::Desc& desc)
{
    switch (type)
    {
        case IResource::Type::Texture1D:        return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
        case IResource::Type::Texture2D:        return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        case IResource::Type::TextureCube:      return desc.arraySize > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        case IResource::Type::Texture3D:
        {
            // Can't have an array and 3d texture
            assert(desc.arraySize <= 1);
            if (desc.arraySize <= 1)
            {
                return VK_IMAGE_VIEW_TYPE_3D;
            }
            break;
        }
        default: break;
    }

    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}

Result VKDevice::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl(this, desc.pipelineType);
    shaderProgram->m_pipelineType = desc.pipelineType;
    shaderProgram->slangProgram = desc.slangProgram;
    m_deviceObjectsWithPotentialBackReferences.add(shaderProgram);

    RootShaderObjectLayout::create(
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

    // For a fully specialized program, create `VkShaderModule`s for each shader stage.
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
        shaderProgram->m_codeBlobs.add(kernelCode);
        VkShaderModule shaderModule;
        shaderProgram->m_stageCreateInfos.add(compileEntryPoint(
            kernelCode,
            (VkShaderStageFlagBits)VulkanUtil::getShaderStage(stage),
            shaderModule));
        shaderProgram->m_modules.add(shaderModule);
    }
    returnComPtr(outProgram, shaderProgram);
    return SLANG_OK;
}

Result VKDevice::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout,
    ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(
        ShaderObjectLayoutImpl::createForElementType(this, typeLayout, layout.writeRef()));
    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result VKDevice::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(
        this, static_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    returnComPtr(outObject, shaderObject);
    return SLANG_OK;
}

Result VKDevice::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    auto programImpl = static_cast<ShaderProgramImpl*>(desc.program);

    if (!programImpl->m_rootObjectLayout->m_pipelineLayout)
    {
        RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
        pipelineStateImpl->init(desc);
        pipelineStateImpl->establishStrongDeviceReference();
        m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
        returnComPtr(outState, pipelineStateImpl);
        return SLANG_OK;
    }

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    auto inputLayoutImpl = (InputLayoutImpl*) desc.inputLayout;

    // VertexBuffer/s
    // Currently only handles one

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkVertexInputBindingDescription vertexInputBindingDescription;

    if (inputLayoutImpl)
    {
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = inputLayoutImpl->m_vertexSize;
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        const auto& srcAttributeDescs = inputLayoutImpl->m_vertexDescs;

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(srcAttributeDescs.getCount());
        vertexInputInfo.pVertexAttributeDescriptions = srcAttributeDescs.getBuffer();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    // Use PRITIMVE_LIST topology for each primitive type here.
    // All other forms of primitive toplogies are specified via dynamic state.
    switch (inDesc.primitiveType)
    {
    case PrimitiveType::Point:
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        break;
    case PrimitiveType::Line:
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        break;
    case PrimitiveType::Triangle:
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        break;
    case PrimitiveType::Patch:
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        break;
    default:
        assert(!"unknown topology type.");
        break;
    }
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    // We are using dynamic viewport and scissor state.
    // Here we specify an arbitrary size, actual viewport will be set at `beginRenderPass` time.
    viewport.width = 16.0f;
    viewport.height = 16.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { uint32_t(16), uint32_t(16) };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 3;
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_STENCIL_REFERENCE};
    dynamicStateInfo.pDynamicStates = dynamicStates;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.depthTestEnable = inDesc.depthStencil.depthTestEnable ? 1 : 0;
    depthStencilStateInfo.back = translateStencilState(inDesc.depthStencil.backFace);
    depthStencilStateInfo.front = translateStencilState(inDesc.depthStencil.frontFace);
    depthStencilStateInfo.back.compareMask = inDesc.depthStencil.stencilReadMask;
    depthStencilStateInfo.back.writeMask = inDesc.depthStencil.stencilWriteMask;
    depthStencilStateInfo.front.compareMask = inDesc.depthStencil.stencilReadMask;
    depthStencilStateInfo.front.writeMask = inDesc.depthStencil.stencilWriteMask;
    depthStencilStateInfo.depthBoundsTestEnable = 0;
    depthStencilStateInfo.depthCompareOp = translateComparisonFunc(inDesc.depthStencil.depthFunc);
    depthStencilStateInfo.depthWriteEnable = inDesc.depthStencil.depthWriteEnable ? 1 : 0;
    depthStencilStateInfo.stencilTestEnable = inDesc.depthStencil.stencilEnable ? 1 : 0;

    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = (uint32_t)programImpl->m_stageCreateInfos.getCount();
    pipelineInfo.pStages = programImpl->m_stageCreateInfos.getBuffer();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
    pipelineInfo.layout = programImpl->m_rootObjectLayout->m_pipelineLayout;
    pipelineInfo.renderPass = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout)->m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    VkPipeline pipeline = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateGraphicsPipelines(m_device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->init(desc);
    pipelineStateImpl->establishStrongDeviceReference();
    m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result VKDevice::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    auto programImpl = static_cast<ShaderProgramImpl*>(desc.program);
    if (!programImpl->m_rootObjectLayout->m_pipelineLayout)
    {
        RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
        pipelineStateImpl->init(desc);
        m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
        pipelineStateImpl->establishStrongDeviceReference();
        returnComPtr(outState, pipelineStateImpl);
        return SLANG_OK;
    }

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    VkPipeline pipeline = VK_NULL_HANDLE;

    VkComputePipelineCreateInfo computePipelineInfo = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    computePipelineInfo.stage = programImpl->m_stageCreateInfos[0];
    computePipelineInfo.layout = programImpl->m_rootObjectLayout->m_pipelineLayout;
    SLANG_VK_CHECK(m_api.vkCreateComputePipelines(
        m_device, pipelineCache, 1, &computePipelineInfo, nullptr, &pipeline));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->init(desc);
    m_deviceObjectsWithPotentialBackReferences.add(pipelineStateImpl);
    pipelineStateImpl->establishStrongDeviceReference();
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result VKDevice::createQueryPool(
    const IQueryPool::Desc& desc,
    IQueryPool** outPool)
{
    RefPtr<QueryPoolImpl> result = new QueryPoolImpl();
    SLANG_RETURN_ON_FAIL(result->init(desc, this));
    returnComPtr(outPool, result);
    return SLANG_OK;
}

} //  renderer_test
