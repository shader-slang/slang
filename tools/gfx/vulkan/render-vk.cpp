// render-vk.cpp
#include "render-vk.h"

//WORKING:#include "options.h"
#include "../renderer-shared.h"

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
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override;
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
    virtual Result createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
        override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createRootShaderObject(IShaderProgram* program, IShaderObject** outObject) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc,
        IPipelineState** outState) override;

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
    void waitForGpu();
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const override
    {
        return m_info;
    }
        /// Dtor
    ~VKDevice();

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

    class InputLayoutImpl : public IInputLayout, public RefObject
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
        List<VkVertexInputAttributeDescription> m_vertexDescs;
        int m_vertexSize;
    };

    class BufferResourceImpl: public BufferResource
    {
        public:
        typedef BufferResource Parent;

        BufferResourceImpl(IResource::Usage initialUsage, const IBufferResource::Desc& desc, VKDevice* renderer):
            Parent(desc),
            m_renderer(renderer),
            m_initialUsage(initialUsage)
        {
            assert(renderer);
        }

        IResource::Usage m_initialUsage;
        VKDevice* m_renderer;
        Buffer m_buffer;
        Buffer m_uploadBuffer;
    };

    class TextureResourceImpl : public TextureResource
    {
    public:
        typedef TextureResource Parent;

        TextureResourceImpl(const Desc& desc, Usage initialUsage, const VulkanApi* api) :
            Parent(desc),
            m_initialUsage(initialUsage),
            m_api(api)
        {
        }
        ~TextureResourceImpl()
        {
            if (m_api)
            {
                if (m_imageMemory != VK_NULL_HANDLE)
                {
                    m_api->vkFreeMemory(m_api->m_device, m_imageMemory, nullptr);
                }
                if (m_image != VK_NULL_HANDLE && !m_isWeakImageReference)
                {
                    m_api->vkDestroyImage(m_api->m_device, m_image, nullptr);
                }
            }
        }

        Usage m_initialUsage;

        VkImage m_image = VK_NULL_HANDLE;
        VkFormat m_vkformat = VK_FORMAT_R8G8B8A8_UNORM;
        VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
        bool m_isWeakImageReference = false;
        const VulkanApi* m_api;
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
        VkSampler m_sampler;
        const VulkanApi* m_api;
        SamplerStateImpl(const VulkanApi* api)
            : m_api(api)
        {}
        ~SamplerStateImpl()
        {
            m_api->vkDestroySampler(m_api->m_device, m_sampler, nullptr);
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
        enum class ViewType
        {
            Texture,
            TexelBuffer,
            PlainBuffer,
        };
    public:
        ResourceViewImpl(ViewType viewType, const VulkanApi* api)
            : m_type(viewType), m_api(api)
        {
        }
        ViewType            m_type;
        const VulkanApi* m_api;
    };

    class TextureResourceViewImpl : public ResourceViewImpl
    {
    public:
        TextureResourceViewImpl(const VulkanApi* api)
            : ResourceViewImpl(ViewType::Texture, api)
        {
        }
        ~TextureResourceViewImpl()
        {
            m_api->vkDestroyImageView(m_api->m_device, m_view, nullptr);
        }
        RefPtr<TextureResourceImpl> m_texture;
        VkImageView                 m_view;
        VkImageLayout               m_layout;
    };

    class TexelBufferResourceViewImpl : public ResourceViewImpl
    {
    public:
        TexelBufferResourceViewImpl(const VulkanApi* api)
            : ResourceViewImpl(ViewType::TexelBuffer, api)
        {
        }
        ~TexelBufferResourceViewImpl()
        {
            m_api->vkDestroyBufferView(m_api->m_device, m_view, nullptr);
        }
        RefPtr<BufferResourceImpl>  m_buffer;
        VkBufferView m_view;
    };

    class PlainBufferResourceViewImpl : public ResourceViewImpl
    {
    public:
        PlainBufferResourceViewImpl(const VulkanApi* api)
            : ResourceViewImpl(ViewType::PlainBuffer, api)
        {
        }
        RefPtr<BufferResourceImpl>  m_buffer;
        VkDeviceSize                offset;
        VkDeviceSize                size;
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
        VkRenderPass m_renderPass;
        VKDevice* m_renderer;
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
        , public RefObject
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        IRenderPassLayout* getInterface(const Guid& guid)
        {
            if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IRenderPassLayout)
                return static_cast<IRenderPassLayout*>(this);
            return nullptr;
        }

    public:
        VkRenderPass m_renderPass;
        VKDevice* m_renderer;

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
        VkFramebuffer m_handle;
        ShortList<ComPtr<IResourceView>> renderTargetViews;
        ComPtr<IResourceView> depthStencilView;
        uint32_t m_width;
        uint32_t m_height;
        VKDevice* m_renderer;
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
        PipelineStateImpl(const VulkanApi& api):
            m_api(&api)
        {
        }
        ~PipelineStateImpl()
        {
            if (m_pipeline != VK_NULL_HANDLE)
            {
                m_api->vkDestroyPipeline(m_api->m_device, m_pipeline, nullptr);
            }
        }

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

        const VulkanApi* m_api;

        RefPtr<FramebufferLayoutImpl> m_framebufferLayout;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
    };

    class ShaderObjectLayoutImpl : public ShaderObjectLayoutBase
    {
    public:
        struct BindingRangeInfo
        {
            slang::BindingType bindingType;
            Index count;
            Index baseIndex;
            Index descriptorSetIndex;
            Index rangeIndexInDescriptorSet;

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

            List<BindingRangeInfo> m_bindingRanges;
            List<SubObjectRangeInfo> m_subObjectRanges;

            Index m_resourceViewCount = 0;
            Index m_samplerCount = 0;
            Index m_combinedTextureSamplerCount = 0;
            Index m_subObjectCount = 0;
            Index m_varyingInputCount = 0;
            Index m_varyingOutputCount = 0;
            uint32_t m_pushConstantSize = 0;
            List<DescriptorSetInfo> m_descriptorSetBuildInfos;
            Dictionary<Index, Index> m_mapSpaceToDescriptorSetIndex;

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

            Result _addDescriptorSets(
                slang::TypeLayoutReflection* typeLayout,
                slang::VariableLayoutReflection* varLayout = nullptr)
            {
                SlangInt descriptorSetCount = typeLayout->getDescriptorSetCount();
                for (SlangInt s = 0; s < descriptorSetCount; ++s)
                {
                    SlangInt descriptorRangeCount =
                        typeLayout->getDescriptorSetDescriptorRangeCount(s);
                    if (descriptorRangeCount == 0)
                        continue;
                    auto descriptorSetIndex =
                        findOrAddDescriptorSet(typeLayout->getDescriptorSetSpaceOffset(s));
                    auto& descriptorSetInfo = m_descriptorSetBuildInfos[descriptorSetIndex];

                    for (SlangInt r = 0; r < descriptorRangeCount; ++r)
                    {
                        auto slangBindingType =
                            typeLayout->getDescriptorSetDescriptorRangeType(s, r);

                        switch (slangBindingType)
                        {
                        case slang::BindingType::ExistentialValue:
                        case slang::BindingType::InlineUniformData:
                        case slang::BindingType::PushConstant:
                            continue;
                        default:
                            break;
                        }

                        auto vkDescriptorType = _mapDescriptorType(slangBindingType);

                        VkDescriptorSetLayoutBinding vkBindingRangeDesc = {};
                        vkBindingRangeDesc.binding =
                            (uint32_t)typeLayout->getDescriptorSetDescriptorRangeIndexOffset(s, r);
                        vkBindingRangeDesc.descriptorCount =
                            (uint32_t)typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(
                                s, r);
                        vkBindingRangeDesc.descriptorType = vkDescriptorType;
                        vkBindingRangeDesc.stageFlags = VK_SHADER_STAGE_ALL;
                        if (varLayout)
                        {
                            auto category =
                                typeLayout->getDescriptorSetDescriptorRangeCategory(s, r);
                            vkBindingRangeDesc.binding += (uint32_t)varLayout->getOffset(category);
                        }
                        descriptorSetInfo.vkBindings.add(vkBindingRangeDesc);
                    }
                    VkDescriptorSetLayoutCreateInfo createInfo = {};
                    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    createInfo.pBindings = descriptorSetInfo.vkBindings.getBuffer();
                    createInfo.bindingCount = (uint32_t)descriptorSetInfo.vkBindings.getCount();
                    VkDescriptorSetLayout vkDescSetLayout;
                    SLANG_RETURN_ON_FAIL(m_renderer->m_api.vkCreateDescriptorSetLayout(
                        m_renderer->m_api.m_device, &createInfo, nullptr, &vkDescSetLayout));
                    descriptorSetInfo.descriptorSetLayout = vkDescSetLayout;
                }
                return SLANG_OK;
            }

            Result setElementTypeLayout(slang::TypeLayoutReflection* typeLayout)
            {
                typeLayout = _unwrapParameterGroups(typeLayout);

                m_elementTypeLayout = typeLayout;

                // First we will use the Slang layout information to allocate
                // the descriptor set layout(s) required to store values
                // of the given type.
                //
                SLANG_RETURN_ON_FAIL(_addDescriptorSets(typeLayout));

                // Next we will compute the binding ranges that are used to store
                // the logical contents of the object in memory. These will relate
                // to the descriptor ranges in the various sets, but not always
                // in a one-to-one fashion.

                SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
                for (SlangInt r = 0; r < bindingRangeCount; ++r)
                {
                    slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
                    uint32_t count = (uint32_t)typeLayout->getBindingRangeBindingCount(r);
                    slang::TypeLayoutReflection* slangLeafTypeLayout =
                        typeLayout->getBindingRangeLeafTypeLayout(r);

                    SlangInt descriptorSetIndex = typeLayout->getBindingRangeDescriptorSetIndex(r);
                    SlangInt rangeIndexInDescriptorSet =
                        typeLayout->getBindingRangeFirstDescriptorRangeIndex(r);

                    Index baseIndex = 0;
                    switch (slangBindingType)
                    {
                    case slang::BindingType::ConstantBuffer:
                    case slang::BindingType::ParameterBlock:
                    case slang::BindingType::ExistentialValue:
                        baseIndex = m_subObjectCount;
                        m_subObjectCount += count;
                        break;

                    case slang::BindingType::Sampler:
                        baseIndex = m_samplerCount;
                        m_samplerCount += count;
                        break;

                    case slang::BindingType::CombinedTextureSampler:
                        baseIndex = m_combinedTextureSamplerCount;
                        m_combinedTextureSamplerCount += count;
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
                        break;
                    }

                    BindingRangeInfo bindingRangeInfo;
                    bindingRangeInfo.bindingType = slangBindingType;
                    bindingRangeInfo.count = count;
                    bindingRangeInfo.baseIndex = baseIndex;
                    bindingRangeInfo.descriptorSetIndex = descriptorSetIndex;
                    bindingRangeInfo.rangeIndexInDescriptorSet = rangeIndexInDescriptorSet;

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
                        ShaderObjectLayoutImpl::createForElementType(
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
                auto layout = RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
                SLANG_RETURN_ON_FAIL(layout->_init(this));

                *outLayout = layout.detach();
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

        List<DescriptorSetInfo> const& getDescriptorSets() { return m_descriptorSetInfos; }

        uint32_t getPushConstantSize() { return m_pushConstantSize; }

        List<BindingRangeInfo> const& getBindingRanges() { return m_bindingRanges; }

        Index getBindingRangeCount() { return m_bindingRanges.getCount(); }

        BindingRangeInfo const& getBindingRange(Index index) { return m_bindingRanges[index]; }

        slang::TypeLayoutReflection* getElementTypeLayout() { return m_elementTypeLayout; }

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
            m_pushConstantSize = builder->m_pushConstantSize;
            m_resourceViewCount = builder->m_resourceViewCount;
            m_samplerCount = builder->m_samplerCount;
            m_combinedTextureSamplerCount = builder->m_combinedTextureSamplerCount;
            m_subObjectCount = builder->m_subObjectCount;
            m_subObjectRanges = builder->m_subObjectRanges;
            return SLANG_OK;
        }

        List<DescriptorSetInfo> m_descriptorSetInfos;
        List<BindingRangeInfo> m_bindingRanges;
        Index m_resourceViewCount = 0;
        Index m_samplerCount = 0;
        Index m_combinedTextureSamplerCount = 0;
        Index m_subObjectCount = 0;
        uint32_t m_pushConstantSize = 0;
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

                *outLayout = layout.detach();
                return SLANG_OK;
            }

            void addEntryPointParams(slang::EntryPointLayout* entryPointLayout)
            {
                m_slangEntryPointLayout = entryPointLayout;
                setElementTypeLayout(entryPointLayout->getTypeLayout());
                m_pushConstantSize = (uint32_t)_unwrapParameterGroups(entryPointLayout->getTypeLayout())
                                         ->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
                m_stage = VulkanUtil::getShaderStage(entryPointLayout->getStage());
            }

            slang::EntryPointLayout* m_slangEntryPointLayout = nullptr;

            VkShaderStageFlags m_stage;
        };

        Result _init(Builder const* builder)
        {
            auto renderer = builder->m_renderer;

            SLANG_RETURN_ON_FAIL(Super::_init(builder));

            m_slangEntryPointLayout = builder->m_slangEntryPointLayout;
            m_stage = builder->m_stage;
            return SLANG_OK;
        }

        VkShaderStageFlags getStage() const { return m_stage; }

        slang::EntryPointLayout* getSlangLayout() const { return m_slangEntryPointLayout; };

        slang::EntryPointLayout* m_slangEntryPointLayout;
        VkShaderStageFlags m_stage;
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

        struct EntryPointInfo
        {
            RefPtr<EntryPointLayout> layout;
            Index rangeOffset;
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
                *outLayout = layout.detach();
                return SLANG_OK;
            }

            void addGlobalParams(slang::VariableLayoutReflection* globalsLayout)
            {
                setElementTypeLayout(globalsLayout->getTypeLayout());
            }

            void addEntryPoint(EntryPointLayout* entryPointLayout)
            {
                EntryPointInfo info;
                info.layout = entryPointLayout;

                if (m_descriptorSetBuildInfos.getCount())
                {
                    info.rangeOffset = m_descriptorSetBuildInfos[0].vkBindings.getCount();
                }
                else
                {
                    info.rangeOffset = 0;
                }

                auto slangEntryPointLayout = entryPointLayout->getSlangLayout();
                _addDescriptorSets(
                    slangEntryPointLayout->getTypeLayout(), slangEntryPointLayout->getVarLayout());
                m_entryPoints.add(info);
            }

            slang::IComponentType* m_program;
            slang::ProgramLayout* m_programLayout;
            List<EntryPointInfo> m_entryPoints;
        };

        Index findEntryPointIndex(VkShaderStageFlags stage)
        {
            auto entryPointCount = m_entryPoints.getCount();
            for (Index i = 0; i < entryPointCount; ++i)
            {
                auto entryPoint = m_entryPoints[i];
                if (entryPoint.layout->getStage() == stage)
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
            m_renderer = renderer;

            if (m_program->getSpecializationParamCount() != 0)
                return SLANG_OK;

            // For fully specialized shader programs, we create a Vulkan pipeline layout now.

            // First, collect `VkDescriptorSetLayout`s for the global scope and all sub-objects
            // referenced via a `ParameterBlock` from shader object layouts.
            SLANG_RETURN_ON_FAIL(addDescriptorSetLayoutRec(this));

            // Next, collect push constant ranges. We will use one descriptor range for each
            // entry point that has uniform parameters.
            uint32_t pushConstantOffset = 0;
            for (auto& entryPoint : m_entryPoints)
            {
                auto size = entryPoint.layout->getPushConstantSize();
                if (size)
                {
                    VkPushConstantRange pushConstantRange = {};
                    pushConstantRange.offset = pushConstantOffset;
                    pushConstantRange.size = size;
                    pushConstantRange.stageFlags = entryPoint.layout->getStage();
                    m_pushConstantRanges.add(pushConstantRange);
                    pushConstantOffset += size;
                }
            }

            // Now call Vulkan API to create a pipeline layout.
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)m_vkDescriptorSetLayouts.getCount();
            pipelineLayoutCreateInfo.pSetLayouts = m_vkDescriptorSetLayouts.getBuffer();
            if (m_pushConstantRanges.getCount())
            {
                pipelineLayoutCreateInfo.pushConstantRangeCount =
                    (uint32_t)m_pushConstantRanges.getCount();
                pipelineLayoutCreateInfo.pPushConstantRanges =
                    m_pushConstantRanges.getBuffer();
            }
            SLANG_RETURN_ON_FAIL(m_renderer->m_api.vkCreatePipelineLayout(
                m_renderer->m_api.m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
            return SLANG_OK;
        }

        // Recusively add `VkDescriptorSetLayout` for all descriptor sets used by this and children
        // shader objects and add them to `m_vkDescriptorSetLayouts`.
        Result addDescriptorSetLayoutRec(ShaderObjectLayoutImpl* layout)
        {
            for (auto& descSetInfo : layout->getDescriptorSets())
            {
                m_vkDescriptorSetLayouts.add(descSetInfo.descriptorSetLayout);
            }

            // Note: entry point parameters in a `RootShaderObject` has already been included
            // in `layout->getDescriptorSets()` during `RootShaderObjectLayout` construction,
            // so we do not need to enumerate entry point array here.

            // However, for sub-objects referenced through `ParameterBlock`s, we do need to
            // add their descriptor sets to our pipeline layout.
            // Binding ranges for sub-objects referenced through `ConstantBuffer`s are also
            // included in this object's layout already, so no need to skip those.

            for (auto& subObject : layout->getSubObjectRanges())
            {
                auto bindingRange = layout->getBindingRange(subObject.bindingRangeIndex);
                if (bindingRange.bindingType == slang::BindingType::ParameterBlock)
                {
                    SLANG_RETURN_ON_FAIL(addDescriptorSetLayoutRec(subObject.layout));
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
        Array<VkPushConstantRange, 8> m_pushConstantRanges;
        RefPtr<VKDevice> m_renderer;
    };
    
    class ShaderProgramImpl : public ShaderProgramBase
    {
    public:
        ShaderProgramImpl(const VulkanApi& api, PipelineType pipelineType)
            : m_api(&api)
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
                    m_api->vkDestroyShaderModule(m_api->m_device, shaderModule, nullptr);
                }
            }
        }

        const VulkanApi* m_api;

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
        bool m_isOpen = false;
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

        RefPtr<ShaderObjectBase> m_rootShaderObject;

        void init(CommandBufferImpl* commandBuffer);

        void endEncodingImpl()
        {
            m_isOpen = false;
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

        Result bindRootShaderObjectImpl(PipelineType pipelineType, IShaderObject* object);

        void setPipelineStateImpl(IPipelineState* state)
        {
            m_currentPipeline = static_cast<PipelineStateImpl*>(state);
        }

        void flushBindingState(VkPipelineBindPoint pipelineBindPoint)
        {
            auto& api = *m_api;
            // Get specialized pipeline state and bind it.
            //
            RefPtr<PipelineStateBase> newPipeline;
            m_device->maybeSpecializePipeline(m_currentPipeline, m_rootShaderObject, newPipeline);
            PipelineStateImpl* newPipelineImpl = static_cast<PipelineStateImpl*>(newPipeline.Ptr());
            auto pipelineBindPointId = getBindPointIndex(pipelineBindPoint);
            if (m_boundPipelines[pipelineBindPointId] != newPipelineImpl->m_pipeline)
            {
                api.vkCmdBindPipeline(
                    m_vkCommandBuffer, pipelineBindPoint, newPipelineImpl->m_pipeline);
                m_boundPipelines[pipelineBindPointId] = newPipelineImpl->m_pipeline;
            }
        }
    };

    union VulkanDescriptorInfo
    {
        VkDescriptorBufferInfo bufferInfo;
        VkDescriptorImageInfo imageInfo;
    };
    struct RootBindingState
    {
        ShortList<VkWriteDescriptorSet, 32> descriptorSetWrites;
        ChunkedList<VulkanDescriptorInfo, 32> descriptorInfos;
        ChunkedList<VkBufferView, 8> bufferViews;
        Array<VkDescriptorSet, kMaxDescriptorSets> descriptorSets;
        ArrayView<VkPushConstantRange> pushConstantRanges;
        VkPipelineLayout pipelineLayout;
        DescriptorSetAllocator* descriptorSetAllocator;
        VKDevice* device;
    };
    struct BindingOffset
    {
        uint32_t uniformOffset;
        uint32_t pushConstantRangeOffset;
        uint32_t descriptorSetIndexOffset;
        uint32_t descriptorRangeOffset;
    };

    class ShaderObjectImpl : public ShaderObjectBase
    {
    public:
        static Result create(
            IDevice* device,
            ShaderObjectLayoutImpl* layout,
            ShaderObjectImpl** outShaderObject)
        {
            auto object = RefPtr<ShaderObjectImpl>(new ShaderObjectImpl());
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
                }
            }

            return SLANG_E_NOT_IMPLEMENTED;
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

            auto object = m_objects[bindingRange.baseIndex + offset.bindingArrayIndex].Ptr();
            object->addRef();
            *outObject = object;

            //        auto& subObjectRange =
            //        m_layout->getSubObjectRange(bindingRange.subObjectRangeIndex); *outObject =
            //        m_objects[subObjectRange.baseIndex + offset.bindingArrayIndex];

            return SLANG_OK;

#if 0
        SLANG_ASSERT(bindingRange.descriptorSetIndex >= 0);
        SLANG_ASSERT(bindingRange.descriptorSetIndex < m_descriptorSets.getCount());
        auto& descriptorSet = m_descriptorSets[bindingRange.descriptorSetIndex];

        descriptorSet->setConstantBuffer(bindingRange.rangeIndexInDescriptorSet, offset.bindingArrayIndex, buffer);
        return SLANG_OK;
#endif
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

            m_resourceViews[bindingRange.baseIndex + offset.bindingArrayIndex] =
                static_cast<ResourceViewImpl*>(resourceView);
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
                auto subObject = m_objects[bindingRange.baseIndex + subObjectIndexInRange];

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
        friend class RootShaderObjectLayout;

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
                    m_objects[bindingRangeInfo.baseIndex + i] = subObject;
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
            auto src = m_ordinaryData.getBuffer();
            auto srcSize = size_t(m_ordinaryData.getCount());

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

                for (Slang::Index i = 0; i < count; ++i)
                {
                    auto subObject = m_objects[bindingRangeInfo.baseIndex + i];

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

    public:
        struct CombinedTextureSamplerSlot
        {
            RefPtr<TextureResourceViewImpl> textureView;
            RefPtr<SamplerStateImpl> sampler;
            operator bool() { return textureView && sampler; }
        };

        // A shared template function for composing a VkWriteDescriptorSet structure.
        // The signature for `WriteDescriptorInfoFunc` is
        // `void(VkWriteDescriptorSet&, int startElement, int elementCount)`, which sets up
        // `VkWriteDescriptorSet::pBufferInfo`, `pImageInfo` or `pTexelBufferView` fields.
        template<typename WriteDescriptorInfoFunc, typename TResourceArrayView>
        static void _writeDescriptorRange(
            RootBindingState* bindingState,
            BindingOffset offset,
            VkDescriptorType descriptorType,
            TResourceArrayView resourceViews,
            const WriteDescriptorInfoFunc& writeDescriptorInfo)
        {
            auto descriptorSet = bindingState->descriptorSets[offset.descriptorSetIndexOffset];
            bool hasNullBinding = false;
            for (auto& ptr : resourceViews)
            {
                if (!ptr)
                {
                    hasNullBinding = true;
                    break;
                }
            }
            if (hasNullBinding)
            {
                for (Index i = 0; i < resourceViews.getCount(); i++)
                {
                    if (!resourceViews[i])
                        continue;
                    VkWriteDescriptorSet write = {};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.descriptorCount = 1;
                    write.descriptorType = descriptorType;
                    write.dstArrayElement = (uint32_t)i;
                    write.dstBinding = offset.descriptorRangeOffset;
                    write.dstSet = descriptorSet;
                    auto infos = bindingState->descriptorInfos.reserveRange(1);
                    writeDescriptorInfo(write, (uint32_t)i, 1);
                    bindingState->descriptorSetWrites.add(write);
                }
                return;
            }

            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorCount = (uint32_t)resourceViews.getCount();
            write.descriptorType = descriptorType;
            write.dstArrayElement = 0;
            write.dstBinding = offset.descriptorRangeOffset;
            write.dstSet = descriptorSet;
            writeDescriptorInfo(write, 0, write.descriptorCount);
            bindingState->descriptorSetWrites.add(write);
        }

        static void writeBufferDescriptor(
            RootBindingState* bindingState,
            BindingOffset offset,
            VkDescriptorType descriptorType,
            BufferResourceImpl* buffer)
        {
            auto descriptorSet = bindingState->descriptorSets[offset.descriptorSetIndexOffset];
            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorCount = 1;
            write.descriptorType = descriptorType;
            write.dstArrayElement = 0;
            write.dstBinding = offset.descriptorRangeOffset;
            write.dstSet = descriptorSet;
            auto& bufferInfo = bindingState->descriptorInfos.reserveRange(1)->bufferInfo;
            write.pBufferInfo = &bufferInfo;
            bufferInfo.buffer = buffer->m_buffer.m_buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = buffer->getDesc()->sizeInBytes;
            bindingState->descriptorSetWrites.add(write);
        }

        static void writePlainBufferDescriptor(
            RootBindingState* bindingState,
            BindingOffset offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<ResourceViewImpl>> resourceViews)
        {
            auto writeDescriptorInfo = [=](VkWriteDescriptorSet& write,
                                           uint32_t startElement,
                                           uint32_t count)
            {
                auto infos = bindingState->descriptorInfos.reserveRange(count);
                write.pBufferInfo = (VkDescriptorBufferInfo*)infos;
                for (uint32_t i = startElement; i < count; i++)
                {
                    auto bufferView =
                        static_cast<PlainBufferResourceViewImpl*>(resourceViews[i].Ptr());
                    if (bufferView)
                    {
                        infos[i].bufferInfo.buffer = bufferView->m_buffer->m_buffer.m_buffer;
                        infos[i].bufferInfo.offset = 0;
                        infos[i].bufferInfo.range = bufferView->m_buffer->getDesc()->sizeInBytes;
                    }
                }
            };
            _writeDescriptorRange(
                bindingState, offset, descriptorType, resourceViews, writeDescriptorInfo);
        }

        static void writeTexelBufferDescriptor(
            RootBindingState* bindingState,
            BindingOffset offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<ResourceViewImpl>> resourceViews)
        {
            auto writeDescriptorInfo = [=](VkWriteDescriptorSet& write,
                                           uint32_t startElement,
                                           uint32_t count)
            {
                auto views = bindingState->bufferViews.reserveRange(write.descriptorCount);
                write.pTexelBufferView = views;
                for (uint32_t i = startElement; i < count; i++)
                {
                    views[i] =
                        static_cast<TexelBufferResourceViewImpl*>(resourceViews[i].Ptr())->m_view;
                }
            };
            _writeDescriptorRange(
                bindingState, offset, descriptorType, resourceViews, writeDescriptorInfo);
        }

        static void writeTextureSamplerDescriptor(
            RootBindingState* bindingState,
            BindingOffset offset,
            VkDescriptorType descriptorType,
            ArrayView<CombinedTextureSamplerSlot> slots)
        {
            auto writeDescriptorInfo = [=](VkWriteDescriptorSet& write,
                                           uint32_t startElement,
                                           uint32_t count)
            {
                auto infos = bindingState->descriptorInfos.reserveRange(write.descriptorCount);
                write.pImageInfo = (VkDescriptorImageInfo*)infos;
                for (uint32_t i = startElement; i < count; i++)
                {
                    auto texture = slots[i].textureView;
                    auto sampler = slots[i].sampler;
                    auto& imageInfo = ((VkDescriptorImageInfo*)infos)[i];
                    imageInfo.imageView = texture->m_view;
                    imageInfo.imageLayout = texture->m_layout;
                    imageInfo.sampler = sampler->m_sampler;
                }
            };
            _writeDescriptorRange(bindingState, offset, descriptorType, slots, writeDescriptorInfo);
        }

        static void writeTextureDescriptor(
            RootBindingState* bindingState,
            BindingOffset offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<ResourceViewImpl>> resourceViews)
        {
            auto writeDescriptorInfo =
                [=](VkWriteDescriptorSet& write, uint32_t startElement, uint32_t count)
            {
                auto infos = bindingState->descriptorInfos.reserveRange(write.descriptorCount);
                write.pImageInfo = (VkDescriptorImageInfo*)infos;
                for (uint32_t i = startElement; i < count; i++)
                {
                    auto texture = static_cast<TextureResourceViewImpl*>(resourceViews[i].Ptr());
                    auto& imageInfo = ((VkDescriptorImageInfo*)infos)[i];
                    imageInfo.imageView = texture->m_view;
                    imageInfo.imageLayout = texture->m_layout;
                    imageInfo.sampler = 0;
                }
            };
            _writeDescriptorRange(
                bindingState, offset, descriptorType, resourceViews, writeDescriptorInfo);
        }

        static void writeSamplerDescriptor(
            RootBindingState* bindingState,
            BindingOffset offset,
            VkDescriptorType descriptorType,
            ArrayView<RefPtr<SamplerStateImpl>> samplers)
        {
            auto writeDescriptorInfo =
                [=](VkWriteDescriptorSet& write, uint32_t startElement, uint32_t count)
            {
                auto infos = bindingState->descriptorInfos.reserveRange(write.descriptorCount);
                write.pImageInfo = (VkDescriptorImageInfo*)infos;
                for (uint32_t i = startElement; i < count; i++)
                {
                    auto texture = samplers[i]->m_sampler;
                    auto& imageInfo = ((VkDescriptorImageInfo*)infos)[i];
                    imageInfo.imageView = 0;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    imageInfo.sampler = samplers[i]->m_sampler;
                }
            };
            _writeDescriptorRange(
                bindingState, offset, descriptorType, samplers, writeDescriptorInfo);
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
            SLANG_RETURN_ON_FAIL(_getSpecializedLayout(specializedLayout.writeRef()));

            auto specializedOrdinaryDataSize = specializedLayout->getElementTypeLayout()->getSize();
            if (specializedOrdinaryDataSize == 0)
                return SLANG_OK;

            // Once we have computed how large the buffer should be, we can allocate
            // it using the existing public `IDevice` API.
            //
            IDevice* device = getRenderer();
            IBufferResource::Desc bufferDesc;
            bufferDesc.init(specializedOrdinaryDataSize);
            bufferDesc.cpuAccessFlags |= IResource::AccessFlag::Write;
            SLANG_RETURN_ON_FAIL(device->createBufferResource(
                IResource::Usage::ConstantBuffer,
                bufferDesc,
                nullptr,
                m_ordinaryDataBuffer.writeRef()));

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
        Result _bindOrdinaryDataBufferIfNeeded(
            PipelineCommandEncoder* encoder,
            RootBindingState* bindingState,
            BindingOffset& offset)
        {
            // We are going to need to tweak the base binding range index
            // used for descriptor-set writes if and only if we actually
            // bind a buffer for ordinary data.
            //
            auto& baseRangeIndex = offset.descriptorRangeOffset;

            // We start by ensuring that the buffer is created, if it is needed.
            //
            SLANG_RETURN_ON_FAIL(_ensureOrdinaryDataBufferCreatedIfNeeded(encoder));

            // If we did indeed need/create a buffer, then we must bind it into
            // the given `descriptorSet` and update the base range index for
            // subsequent binding operations to account for it.
            //
            if (m_ordinaryDataBuffer)
            {
                auto bufferImpl = static_cast<BufferResourceImpl*>(m_ordinaryDataBuffer.get());
                writeBufferDescriptor(
                    bindingState, offset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferImpl);
                offset.descriptorRangeOffset++;
            }

            return SLANG_OK;
        }

    public:
        Result bindDescriptorRanges(
            PipelineCommandEncoder* encoder,
            RootBindingState* bindingState,
            BindingOffset& offset)
        {
            auto layout = getLayout();

            // Fill in the descriptor sets based on binding ranges
            //
            for (auto bindingRangeInfo : layout->getBindingRanges())
            {
                auto rangeIndex =
                    bindingRangeInfo.rangeIndexInDescriptorSet + offset.descriptorRangeOffset;
                auto baseIndex = bindingRangeInfo.baseIndex;
                auto count = (uint32_t)bindingRangeInfo.count;
                switch (bindingRangeInfo.bindingType)
                {
                case slang::BindingType::ConstantBuffer:
                    for (uint32_t i = 0; i < count; ++i)
                    {
                        ShaderObjectImpl* subObject = m_objects[baseIndex + i];
                        subObject->bindObjectIntoConstantBuffer(encoder, bindingState, offset);
                    }
                    break;
                case slang::BindingType::ParameterBlock:
                    for (uint32_t i = 0; i < count; ++i)
                    {
                        ShaderObjectImpl* subObject = m_objects[baseIndex + i];
                        auto newOffset = offset;
                        subObject->bindObjectIntoParameterBlock(encoder, bindingState, newOffset);
                        offset.pushConstantRangeOffset = newOffset.pushConstantRangeOffset;
                    }
                    break;
                case slang::BindingType::Texture:
                    writeTextureDescriptor(
                        bindingState,
                        offset,
                        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                        m_resourceViews.getArrayView(baseIndex, count));
                    offset.descriptorRangeOffset++;
                    break;
                case slang::BindingType::MutableTexture:
                    writeTextureDescriptor(
                        bindingState,
                        offset,
                        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                        m_resourceViews.getArrayView(baseIndex, count));
                    offset.descriptorRangeOffset++;
                    break;
                case slang::BindingType::CombinedTextureSampler:
                    writeTextureSamplerDescriptor(
                        bindingState,
                        offset,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        m_combinedTextureSamplers.getArrayView(baseIndex, count));
                    offset.descriptorRangeOffset++;
                    break;

                case slang::BindingType::Sampler:
                    writeSamplerDescriptor(
                        bindingState,
                        offset,
                        VK_DESCRIPTOR_TYPE_SAMPLER,
                        m_samplers.getArrayView(baseIndex, count));
                    offset.descriptorRangeOffset++;
                    break;

                case slang::BindingType::RawBuffer:
                case slang::BindingType::MutableRawBuffer:
                    writePlainBufferDescriptor(
                        bindingState,
                        offset,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        m_resourceViews.getArrayView(baseIndex, count));
                    offset.descriptorRangeOffset++;
                    break;

                case slang::BindingType::TypedBuffer:
                    writeTexelBufferDescriptor(
                        bindingState,
                        offset,
                        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
                        m_resourceViews.getArrayView(baseIndex, count));
                    offset.descriptorRangeOffset++;
                    break;
                case slang::BindingType::MutableTypedBuffer:
                    writeTexelBufferDescriptor(
                        bindingState,
                        offset,
                        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                        m_resourceViews.getArrayView(baseIndex, count));
                    offset.descriptorRangeOffset++;
                    break;

                case slang::BindingType::VaryingInput:
                case slang::BindingType::VaryingOutput:
                    break;

                case slang::BindingType::ExistentialValue:
                    //
                    // TODO: If the existential value is one that "fits" into the storage available,
                    // then we should write its data directly into that area. Otherwise, we need
                    // to bind its content as "pending" data which will come after any other data
                    // beloning to the same set (that is, it's starting descriptorRangeIndex will
                    // need to be one after the number of ranges accounted for in the original type)
                    //
                    break;

                default:
                    SLANG_ASSERT(!"unsupported binding type");
                    return SLANG_FAIL;
                    break;
                }
            }
            return SLANG_OK;
        }

        virtual Result bindObjectIntoConstantBuffer(
            PipelineCommandEncoder* encoder,
            RootBindingState* bindingState,
            BindingOffset& offset)
        {
            SLANG_RETURN_ON_FAIL(_bindOrdinaryDataBufferIfNeeded(encoder, bindingState, offset));

            SLANG_RETURN_ON_FAIL(bindDescriptorRanges(encoder, bindingState, offset));
            return SLANG_OK;
        }

        virtual Result bindObjectIntoParameterBlock(
            PipelineCommandEncoder* encoder,
            RootBindingState* bindingState,
            BindingOffset& offset)
        {
            auto& descriptorSetInfos = getLayout()->getDescriptorSets();
            offset.descriptorSetIndexOffset = (uint32_t)bindingState->descriptorSets.getCount();
            offset.uniformOffset = 0;
            offset.descriptorRangeOffset = 0;
            for (auto info : descriptorSetInfos)
            {
                bindingState->descriptorSets.add(
                    bindingState->descriptorSetAllocator->allocate(info.descriptorSetLayout)
                        .handle);
            }
            SLANG_RETURN_ON_FAIL(bindObjectIntoConstantBuffer(encoder, bindingState, offset));
            return SLANG_OK;
        }

        /// Any "ordinary" / uniform data for this object
        List<char> m_ordinaryData;

        List<RefPtr<ResourceViewImpl>> m_resourceViews;

        List<RefPtr<SamplerStateImpl>> m_samplers;

        List<CombinedTextureSamplerSlot> m_combinedTextureSamplers;

        List<RefPtr<ShaderObjectImpl>> m_objects;

        /// A constant buffer used to stored ordinary data for this object
        /// and existential-type sub-objects.
        ///
        /// Created on demand with `_createOrdinaryDataBufferIfNeeded()`
        ComPtr<IBufferResource> m_ordinaryDataBuffer;

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

            auto device = getDevice();
            RefPtr<ShaderObjectLayoutBase> layout;
            SLANG_RETURN_ON_FAIL(
                device->getShaderObjectLayout(extendedType.slangType, layout.writeRef()));

            *outLayout = static_cast<ShaderObjectLayoutImpl*>(layout.detach());
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

            *outShaderObject = object.detach();
            return SLANG_OK;
        }

        EntryPointLayout* getLayout() { return static_cast<EntryPointLayout*>(m_layout.Ptr()); }

        virtual Result bindObjectIntoConstantBuffer(
            PipelineCommandEncoder* encoder,
            RootBindingState* bindingState,
            BindingOffset& offset) override
        {
            // Set data in `m_ordinaryData` into the push constant range.
            if (m_ordinaryData.getCount())
            {
                auto pushConstantRange =
                    bindingState->pushConstantRanges[offset.pushConstantRangeOffset];
                encoder->m_api->vkCmdPushConstants(
                    encoder->m_commandBuffer->m_commandBuffer,
                    bindingState->pipelineLayout,
                    pushConstantRange.stageFlags,
                    pushConstantRange.offset,
                    pushConstantRange.size,
                    m_ordinaryData.getBuffer());
                offset.pushConstantRangeOffset++;
            }

            // Process the rest of binding ranges.
            SLANG_RETURN_ON_FAIL(bindDescriptorRanges(encoder, bindingState, offset));
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
        static Result create(
            IDevice* device,
            RootShaderObjectLayout* layout,
            RootShaderObjectImpl** outShaderObject)
        {
            RefPtr<RootShaderObjectImpl> object = new RootShaderObjectImpl();
            SLANG_RETURN_ON_FAIL(object->init(device, layout));

            *outShaderObject = object.detach();
            return SLANG_OK;
        }

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
            *outEntryPoint = m_entryPoints[index];
            m_entryPoints[index]->addRef();
            return SLANG_OK;
        }

        virtual Result bindObjectIntoParameterBlock(
            PipelineCommandEncoder* encoder,
            RootBindingState* bindingState,
            BindingOffset& offset) override
        {
            SLANG_RETURN_ON_FAIL(Super::bindObjectIntoParameterBlock(encoder, bindingState, offset));

            // Bind all entry points.
            for (auto& entryPoint : m_entryPoints)
            {
                entryPoint->bindObjectIntoConstantBuffer(encoder, bindingState, offset);
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

    protected:
        Result init(IDevice* device, RootShaderObjectLayout* layout)
        {
            SLANG_RETURN_ON_FAIL(Super::init(device, layout));

            for (auto entryPointInfo : layout->getEntryPoints())
            {
                RefPtr<EntryPointShaderObject> entryPoint;
                SLANG_RETURN_ON_FAIL(EntryPointShaderObject::create(
                    device, entryPointInfo.layout, entryPoint.writeRef()));
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

            *outLayout = specializedLayout.detach();
            return SLANG_OK;
        }

        List<RefPtr<EntryPointShaderObject>> m_entryPoints;
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
        VkCommandBuffer m_commandBuffer;
        VkCommandBuffer m_preCommandBuffer = VK_NULL_HANDLE;
        VkCommandPool m_pool;
        VKDevice* m_renderer;
        DescriptorSetAllocator* m_transientDescSetAllocator;
        // Command buffers are deallocated by its command pool,
        // so no need to free individually.
        ~CommandBufferImpl() = default;

        Result init(
            VKDevice* renderer,
            VkCommandPool pool,
            DescriptorSetAllocator* transientDescSetAllocator)
        {
            m_renderer = renderer;
            m_transientDescSetAllocator = transientDescSetAllocator;
            m_pool = pool;

            auto& api = renderer->m_api;
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = pool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            SLANG_VK_RETURN_ON_FAIL(
                api.vkAllocateCommandBuffers(api.m_device, &allocInfo, &m_commandBuffer));

            VkCommandBufferBeginInfo beginInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                nullptr,
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
            api.vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
            return SLANG_OK;
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
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
            api.vkBeginCommandBuffer(m_preCommandBuffer, &beginInfo);
            return SLANG_OK;
        }

        VkCommandBuffer getPreCommandBuffer()
        {
            if (m_preCommandBuffer)
                return m_preCommandBuffer;
            createPreCommandBuffer();
            return m_preCommandBuffer;
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
            SLANG_REF_OBJECT_IUNKNOWN_ALL
            IRenderCommandEncoder* getInterface(const Guid& guid)
            {
                if (guid == GfxGUID::IID_ISlangUnknown ||
                    guid == GfxGUID::IID_IRenderCommandEncoder ||
                    guid == GfxGUID::IID_ICommandEncoder)
                    return static_cast<IRenderCommandEncoder*>(this);
                return nullptr;
            }

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
                m_isOpen = true;
            }

            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                auto& api = *m_api;
                api.vkCmdEndRenderPass(m_vkCommandBuffer);
                endEncodingImpl();
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setPipelineState(IPipelineState* pipelineState) override
            {
                setPipelineStateImpl(pipelineState);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                bindRootShaderObject(IShaderObject* object) override
            {
                bindRootShaderObjectImpl(PipelineType::Graphics, object);
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
                if (!pipeline || static_cast<ShaderProgramImpl*>(pipeline->m_program.get())
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
            assert(!m_renderCommandEncoder->m_isOpen);
            m_renderCommandEncoder->beginPass(renderPass, framebuffer);
            *outEncoder = m_renderCommandEncoder.Ptr();
            m_renderCommandEncoder->addRef();
        }

        class ComputeCommandEncoder
            : public IComputeCommandEncoder
            , public PipelineCommandEncoder
        {
        public:
            SLANG_REF_OBJECT_IUNKNOWN_ALL
            IComputeCommandEncoder* getInterface(const Guid& guid)
            {
                if (guid == GfxGUID::IID_ISlangUnknown ||
                    guid == GfxGUID::IID_IComputeCommandEncoder ||
                    guid == GfxGUID::IID_ICommandEncoder)
                    return static_cast<IComputeCommandEncoder*>(this);
                return nullptr;
            }

        public:
            virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() override
            {
                endEncodingImpl();
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                setPipelineState(IPipelineState* pipelineState) override
            {
                setPipelineStateImpl(pipelineState);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL
                bindRootShaderObject(IShaderObject* object) override
            {
                bindRootShaderObjectImpl(PipelineType::Compute, object);
            }

            virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) override
            {
                auto pipeline = static_cast<PipelineStateImpl*>(m_currentPipeline.Ptr());
                if (!pipeline ||
                    static_cast<ShaderProgramImpl*>(pipeline->m_program.get())->m_pipelineType !=
                        PipelineType::Compute)
                {
                    assert(!"Invalid compute pipeline");
                    return;
                }

                // Also create descriptor sets based on the given pipeline layout
                flushBindingState(VK_PIPELINE_BIND_POINT_COMPUTE);
                m_api->vkCmdDispatch(m_vkCommandBuffer, x, y, z);
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
            assert(!m_computeCommandEncoder->m_isOpen);
            *outEncoder = m_computeCommandEncoder.Ptr();
            m_computeCommandEncoder->addRef();
        }

        class ResourceCommandEncoder
            : public IResourceCommandEncoder
            , public RefObject
        {
        public:
            CommandBufferImpl* m_commandBuffer;
        public:
            SLANG_REF_OBJECT_IUNKNOWN_ALL
            IResourceCommandEncoder* getInterface(const Guid& guid)
            {
                if (guid == GfxGUID::IID_ISlangUnknown ||
                    guid == GfxGUID::IID_IResourceCommandEncoder ||
                    guid == GfxGUID::IID_ICommandEncoder)
                    return static_cast<IResourceCommandEncoder*>(this);
                return nullptr;
            }

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
            m_resourceCommandEncoder->addRef();
        }

        virtual SLANG_NO_THROW void SLANG_MCALL close() override
        {
            auto& vkAPI = m_renderer->m_api;
            if (m_preCommandBuffer != VK_NULL_HANDLE)
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
        Desc m_desc;
        uint32_t m_poolIndex;
        RefPtr<VKDevice> m_renderer;
        VkQueue m_queue;
        uint32_t m_queueFamilyIndex;
        VkSemaphore m_pendingWaitSemaphore = VK_NULL_HANDLE;
        List<VkCommandBuffer> m_submitCommandBuffers;
        static const int kCommandPoolCount = 8;
        VkCommandPool m_commandPools[kCommandPoolCount];
        DescriptorSetAllocator m_descSetAllocators[kCommandPoolCount];
        VkFence m_fences[kCommandPoolCount];
        VkSemaphore m_semaphores[kCommandPoolCount];
        ~CommandQueueImpl()
        {
            m_renderer->m_api.vkQueueWaitIdle(m_queue);

            m_renderer->m_queueAllocCount--;
            for (int i = 0; i < kCommandPoolCount; i++)
            {
                m_renderer->m_api.vkDestroyCommandPool(
                    m_renderer->m_api.m_device, m_commandPools[i], nullptr);
                m_renderer->m_api.vkDestroyFence(m_renderer->m_api.m_device, m_fences[i], nullptr);
                m_renderer->m_api.vkDestroySemaphore(
                    m_renderer->m_api.m_device, m_semaphores[i], nullptr);
                m_descSetAllocators[i].close();
            }
        }

        void init(VKDevice* renderer, VkQueue queue, uint32_t queueFamilyIndex)
        {
            m_renderer = renderer;
            m_poolIndex = 0;
            m_queue = queue;
            m_queueFamilyIndex = queueFamilyIndex;
            for (int i = 0; i < kCommandPoolCount; i++)
            {
                m_descSetAllocators[i].m_api = &m_renderer->m_api;

                VkCommandPoolCreateInfo poolCreateInfo = {};
                poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                poolCreateInfo.queueFamilyIndex = queueFamilyIndex;
                m_renderer->m_api.vkCreateCommandPool(
                    m_renderer->m_api.m_device, &poolCreateInfo, nullptr, &m_commandPools[i]);

                VkFenceCreateInfo fenceCreateInfo = {};
                fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                m_renderer->m_api.vkCreateFence(
                    m_renderer->m_api.m_device, &fenceCreateInfo, nullptr, &m_fences[i]);

                VkSemaphoreCreateInfo semaphoreCreateInfo = {};
                semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                semaphoreCreateInfo.flags = 0;
                m_renderer->m_api.vkCreateSemaphore(
                    m_renderer->m_api.m_device, &semaphoreCreateInfo, nullptr, &m_semaphores[i]);
            }
        }

        // Swaps to and resets the next command pool.
        // Wait if command lists in the next pool are still in flight.
        Result swapPools()
        {
            auto& vkAPI = m_renderer->m_api;
            m_poolIndex++;
            m_poolIndex = m_poolIndex % kCommandPoolCount;

            if (vkAPI.vkWaitForFences(vkAPI.m_device, 1, &m_fences[m_poolIndex], 1, UINT64_MAX) !=
                VK_SUCCESS)
            {
                return SLANG_FAIL;
            }
            vkAPI.vkResetCommandPool(vkAPI.m_device, m_commandPools[m_poolIndex], 0);
            m_descSetAllocators[m_poolIndex].reset();
            return SLANG_OK;
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

        virtual SLANG_NO_THROW Result SLANG_MCALL
            createCommandBuffer(ICommandBuffer** result) override
        {
            RefPtr<CommandBufferImpl> commandBuffer = new CommandBufferImpl();
            SLANG_RETURN_ON_FAIL(commandBuffer->init(
                m_renderer, m_commandPools[m_poolIndex], &m_descSetAllocators[m_poolIndex]));
            *result = commandBuffer.detach();
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW void SLANG_MCALL
            executeCommandBuffers(
            uint32_t count,
            ICommandBuffer* const* commandBuffers) override
        {
            auto& vkAPI = m_renderer->m_api;
            m_submitCommandBuffers.clear();
            for (uint32_t i = 0; i < count; i++)
            {
                auto cmdBufImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
                if (cmdBufImpl->m_preCommandBuffer != VK_NULL_HANDLE)
                    m_submitCommandBuffers.add(cmdBufImpl->m_preCommandBuffer);
                auto vkCmdBuf = cmdBufImpl->m_commandBuffer;
                m_submitCommandBuffers.add(vkCmdBuf);
            }
            VkSemaphore waitSemaphore = m_pendingWaitSemaphore;
            VkSemaphore signalSemaphore = m_semaphores[m_poolIndex];
            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkPipelineStageFlags stageFlag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            submitInfo.pWaitDstStageMask = &stageFlag;
            submitInfo.commandBufferCount = (uint32_t)m_submitCommandBuffers.getCount();
            submitInfo.pCommandBuffers = m_submitCommandBuffers.getBuffer();
            if (m_pendingWaitSemaphore != VK_NULL_HANDLE)
            {
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitSemaphores = &waitSemaphore;
            }
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &signalSemaphore;
            vkAPI.vkResetFences(vkAPI.m_device, 1, &m_fences[m_poolIndex]);
            vkAPI.vkQueueSubmit(m_queue, 1, &submitInfo, m_fences[m_poolIndex]);
            m_pendingWaitSemaphore = signalSemaphore;
            swapPools();
        }
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

                imageDesc.init2D(
                    IResource::Type::Texture2D, m_desc.format, m_desc.width, m_desc.height, 1);
                RefPtr<TextureResourceImpl> image = new TextureResourceImpl(
                    imageDesc, gfx::IResource::Usage::RenderTarget, m_api);
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
            *outResource = m_images[index];
            m_images[index]->addRef();
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
            if (m_queue->m_pendingWaitSemaphore != VK_NULL_HANDLE)
            {
                presentInfo.waitSemaphoreCount = 1;
                presentInfo.pWaitSemaphores = &m_queue->m_pendingWaitSemaphore;
            }
            m_api->vkQueuePresentKHR(m_queue->m_queue, &presentInfo);
            m_queue->m_pendingWaitSemaphore = VK_NULL_HANDLE;
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
            m_queue->m_pendingWaitSemaphore = m_nextImageSemaphore;
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
};

void VKDevice::PipelineCommandEncoder::init(CommandBufferImpl* commandBuffer)
{
    m_commandBuffer = commandBuffer;
    m_device = commandBuffer->m_renderer;
    m_vkCommandBuffer = m_commandBuffer->m_commandBuffer;
    m_api = &m_commandBuffer->m_renderer->m_api;
}

Result VKDevice::PipelineCommandEncoder::bindRootShaderObjectImpl(
    PipelineType pipelineType,
    IShaderObject* object)
{
    // Obtain specialized root layout.
    auto rootObjectImpl = static_cast<RootShaderObjectImpl*>(object);
    m_rootShaderObject = rootObjectImpl;

    auto specializedLayout = rootObjectImpl->getSpecializedLayout();
    if (!specializedLayout)
        return SLANG_FAIL;

    RootBindingState bindState = {};
    bindState.pushConstantRanges = specializedLayout->m_pushConstantRanges.getView();
    bindState.pipelineLayout = specializedLayout->m_pipelineLayout;
    bindState.device = m_device;
    bindState.descriptorSetAllocator = m_commandBuffer->m_transientDescSetAllocator;

    // Write bindings into descriptor sets. This step allocate descriptor sets and collects
    // all `VkWriteDescriptorSet` operations in `bindState.descriptorSetWrites`.
    BindingOffset offset = {};
    rootObjectImpl->bindObjectIntoParameterBlock(this, &bindState, offset);

    // Execute descriptor writes collected in `bindState.descriptorSetWrites`.
    m_device->m_api.vkUpdateDescriptorSets(
        m_device->m_device,
        (uint32_t)bindState.descriptorSetWrites.getCount(),
        bindState.descriptorSetWrites.getArrayView().arrayView.getBuffer(),
        0,
        nullptr);

    // Bind descriptor sets.
    m_device->m_api.vkCmdBindDescriptorSets(
        m_commandBuffer->m_commandBuffer,
        VulkanUtil::getPipelineBindPoint(pipelineType),
        specializedLayout->m_pipelineLayout,
        0,
        (uint32_t)bindState.descriptorSets.getCount(),
        bindState.descriptorSets.getBuffer(),
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

    SLANG_VK_CHECK(api.vkAllocateMemory(api.m_device, &allocateInfo, nullptr, &m_memory));
    SLANG_VK_CHECK(api.vkBindBufferMemory(api.m_device, m_buffer, m_memory, 0));

    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VKDevice !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Result SLANG_MCALL createVKDevice(const IDevice::Desc* desc, IDevice** outRenderer)
{
    RefPtr<VKDevice> result = new VKDevice();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    *outRenderer = result.detach();
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
    char const* severity = "message";
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        severity = "warning";
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        severity = "error";

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

    fprintf(stderr, "%s", buffer);
    fflush(stderr);
#ifdef _WIN32
    OutputDebugStringA(buffer);
#endif
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

    VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    applicationInfo.pApplicationName = "slang-render-test";
    applicationInfo.pEngineName = "slang-render-test";
    applicationInfo.apiVersion = VK_API_VERSION_1_0;
    applicationInfo.engineVersion = 1;
    applicationInfo.applicationVersion = 1;
    const char* instanceExtensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,

        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,

#if SLANG_WINDOWS_FAMILY
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(SLANG_ENABLE_XLIB)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif

#if ENABLE_VALIDATION_LAYER
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    };

    VkInstance instance = VK_NULL_HANDLE;

    VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = SLANG_COUNT_OF(instanceExtensions);
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

        const char* layerNames[] = { nullptr };
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
    SLANG_RETURN_ON_FAIL(m_api.vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
    SLANG_RETURN_ON_FAIL(m_api.initInstanceProcs(instance));

    if (useValidationLayer)
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

    // Get the API version
    const uint32_t majorVersion = VK_VERSION_MAJOR(basicProps.apiVersion);
    const uint32_t minorVersion = VK_VERSION_MINOR(basicProps.apiVersion);

    // Need in this scope because it will be linked into the device creation (if it is available)

    // Float16 features
    VkPhysicalDeviceFloat16Int8FeaturesKHR float16Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };
    // AtomicInt64 features
    VkPhysicalDeviceShaderAtomicInt64FeaturesKHR atomicInt64Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR };
    // Atomic Float features
    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomicFloatFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT };
    // Timeline Semaphore features
    VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES };
    // Extended dynamic state features
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
    // API version check, can't use vkGetPhysicalDeviceProperties2 yet since this device might not support it
    if (VK_MAKE_VERSION(majorVersion, minorVersion, 0) >= VK_API_VERSION_1_1 &&
        m_api.vkGetPhysicalDeviceProperties2 &&
        m_api.vkGetPhysicalDeviceFeatures2)
    {
        // Get device features
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // Extended dynamic states
        extendedDynamicStateFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &extendedDynamicStateFeatures;

        // Timeline Semaphore
        timelineFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &timelineFeatures;

        // Float16
        float16Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &float16Features;

        // Atomic64
        atomicInt64Features.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &atomicInt64Features;

        // Atomic Float
        // To detect atomic float we need
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPhysicalDeviceShaderAtomicFloatFeaturesEXT.html

        atomicFloatFeatures.pNext = deviceFeatures2.pNext;
        deviceFeatures2.pNext = &atomicFloatFeatures;

        m_api.vkGetPhysicalDeviceFeatures2(m_api.m_physicalDevice, &deviceFeatures2);

        // If we have float16 features then enable
        if (float16Features.shaderFloat16)
        {
            // Link into the creation features
            float16Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &float16Features;

            // Add the Float16 extension
            deviceExtensions.add(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);

            // We have half support
            m_features.add("half");
        }

        if (atomicInt64Features.shaderBufferInt64Atomics)
        {
            // Link into the creation features
            atomicInt64Features.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &atomicInt64Features;

            deviceExtensions.add(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME);
            m_features.add("atomic-int64");
        }

        if (atomicFloatFeatures.shaderBufferFloat32AtomicAdd)
        {
            // Link into the creation features
            atomicFloatFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &atomicFloatFeatures;

            deviceExtensions.add(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
            m_features.add("atomic-float");
        }

        if (timelineFeatures.timelineSemaphore)
        {
            // Link into the creation features
            timelineFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &timelineFeatures;
            deviceExtensions.add(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
            m_features.add("timeline-semaphore");
        }

        if (extendedDynamicStateFeatures.extendedDynamicState)
        {
            // Link into the creation features
            extendedDynamicStateFeatures.pNext = (void*)deviceCreateInfo.pNext;
            deviceCreateInfo.pNext = &extendedDynamicStateFeatures;
            deviceExtensions.add(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
            m_features.add("extended-dynamic-states");
        }
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

    SLANG_RETURN_ON_FAIL(slangContext.initialize(desc.slang, SLANG_SPIRV, "sm_5_1"));
    return SLANG_OK;
}

void VKDevice::waitForGpu()
{
    m_deviceQueue.flushAndWait();
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
    *outQueue = result.detach();
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
    *outSwapchain = sc.detach();
    return SLANG_OK;
}

Result VKDevice::createFramebufferLayout(const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    SLANG_RETURN_ON_FAIL(layout->init(this, desc));
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result VKDevice::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc,
    IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl();
    SLANG_RETURN_ON_FAIL(result->init(this, desc));
    *outRenderPassLayout = result.detach();
    return SLANG_OK;
}

Result VKDevice::createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer)
{
    RefPtr<FramebufferImpl> fb = new FramebufferImpl();
    SLANG_RETURN_ON_FAIL(fb->init(this, desc));
    *outFramebuffer = fb.detach();
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

    *outBlob = blob.detach();
    return SLANG_OK;
}

static VkBufferUsageFlagBits _calcBufferUsageFlags(IResource::BindFlag::Enum bind)
{
    typedef IResource::BindFlag BindFlag;

    switch (bind)
    {
        case BindFlag::VertexBuffer:            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BindFlag::IndexBuffer:             return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BindFlag::ConstantBuffer:          return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BindFlag::StreamOutput:
        case BindFlag::RenderTarget:
        case BindFlag::DepthStencil:
        {
            assert(!"Not supported yet");
            return VkBufferUsageFlagBits(0);
        }
        case BindFlag::UnorderedAccess:         return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        case BindFlag::PixelShaderResource:     return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BindFlag::NonPixelShaderResource:  return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        default:                                return VkBufferUsageFlagBits(0);
    }
}

static VkBufferUsageFlagBits _calcBufferUsageFlags(int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;
        dstFlags |= _calcBufferUsageFlags(IResource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return VkBufferUsageFlagBits(dstFlags);
}

static VkBufferUsageFlags _calcBufferUsageFlags(int bindFlags, int cpuAccessFlags, const void* initData)
{
    VkBufferUsageFlags usage = _calcBufferUsageFlags(bindFlags);

    if (cpuAccessFlags & IResource::AccessFlag::Read)
    {
        // If it can be read from, set this
        usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if ((cpuAccessFlags & IResource::AccessFlag::Write) || initData)
    {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return usage;
}

static VkImageUsageFlagBits _calcImageUsageFlags(IResource::BindFlag::Enum bind)
{
    typedef IResource::BindFlag BindFlag;

    switch (bind)
    {
        case BindFlag::RenderTarget:            return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        case BindFlag::DepthStencil:            return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        case BindFlag::NonPixelShaderResource:
        case BindFlag::PixelShaderResource:
        {
            // Ignore
            return VkImageUsageFlagBits(0);
        }
        default:
        {
            assert(!"Unsupported");
            return VkImageUsageFlagBits(0);
        }
    }
}

static VkImageUsageFlagBits _calcImageUsageFlags(int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;
        dstFlags |= _calcImageUsageFlags(IResource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return VkImageUsageFlagBits(dstFlags);
}

static VkImageUsageFlags _calcImageUsageFlags(int bindFlags, int cpuAccessFlags, const void* initData)
{
    VkImageUsageFlags usage = _calcImageUsageFlags(bindFlags);

    usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if (cpuAccessFlags & IResource::AccessFlag::Read)
    {
        // If it can be read from, set this
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ((cpuAccessFlags & IResource::AccessFlag::Write) || initData)
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

Result VKDevice::createTextureResource(IResource::Usage initialUsage, const ITextureResource::Desc& descIn, const ITextureResource::SubresourceData* initData, ITextureResource** outResource)
{
    TextureResource::Desc desc(descIn);
    desc.setDefaults(initialUsage);

    const VkFormat format = VulkanUtil::getVkFormat(desc.format);
    if (format == VK_FORMAT_UNDEFINED)
    {
        assert(!"Unhandled image format");
        return SLANG_FAIL;
    }

    const int arraySize = desc.calcEffectiveArraySize();

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(desc, initialUsage, &m_api));
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
        imageInfo.usage = _calcImageUsageFlags(desc.bindFlags, desc.cpuAccessFlags, initData);
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
            const TextureResource::Size mipSize = desc.size.calcMipSize(j);

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
        _transitionImageLayout(texture->m_image, format, *texture->getDesc(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    else
    {
        switch (initialUsage)
        {
        case IResource::Usage::RenderTarget:
            _transitionImageLayout(
                texture->m_image,
                format,
                *texture->getDesc(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            break;
        case IResource::Usage::DepthWrite:
            _transitionImageLayout(
                texture->m_image,
                format,
                *texture->getDesc(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            break;
        default:
            break;
        }
    }
    m_deviceQueue.flushAndWait();
    *outResource = texture.detach();
    return SLANG_OK;
}

Result VKDevice::createBufferResource(IResource::Usage initialUsage, const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc desc(descIn);
    desc.setDefaults(initialUsage);

    const size_t bufferSize = desc.sizeInBytes;

    VkMemoryPropertyFlags reqMemoryProperties = 0;

    VkBufferUsageFlags usage = _calcBufferUsageFlags(desc.bindFlags, desc.cpuAccessFlags, initData);

    switch (initialUsage)
    {
        case IResource::Usage::ConstantBuffer:
        {
            reqMemoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        }
        default: break;
    }

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(initialUsage, desc, this));
    SLANG_RETURN_ON_FAIL(buffer->m_buffer.init(m_api, desc.sizeInBytes, usage, reqMemoryProperties));

    if ((desc.cpuAccessFlags & IResource::AccessFlag::Write) || initData)
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

    *outResource = buffer.detach();
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

    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl(&m_api);
    samplerImpl->m_sampler = sampler;
    *outSampler = samplerImpl.detach();
    return SLANG_OK;
}

Result VKDevice::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = static_cast<TextureResourceImpl*>(texture);
    RefPtr<TextureResourceViewImpl> view = new TextureResourceViewImpl(&m_api);
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
    m_api.vkCreateImageView(m_device, &createInfo, nullptr, &view->m_view);
    *outView = view.detach();
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
        // Is this a formatted view?
        //
        if(desc.format == Format::Unknown)
        {
            // Buffer usage that doesn't involve formatting doesn't
            // require a view in Vulkan.
            RefPtr<PlainBufferResourceViewImpl> viewImpl = new PlainBufferResourceViewImpl(&m_api);
            viewImpl->m_buffer = resourceImpl;
            viewImpl->offset = 0;
            viewImpl->size = size;
            *outView = viewImpl.detach();
            return SLANG_OK;
        }
        //
        // If the view is formatted, then we need to handle
        // it just like we would for a "sampled" buffer:
        //
        // FALLTHROUGH
    case IResourceView::Type::ShaderResource:
        {
            VkBufferViewCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };

            info.format = VulkanUtil::getVkFormat(desc.format);
            info.buffer = resourceImpl->m_buffer.m_buffer;
            info.offset = offset;
            info.range = size;

            VkBufferView view;
            SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateBufferView(m_device, &info, nullptr, &view));

            RefPtr<TexelBufferResourceViewImpl> viewImpl = new TexelBufferResourceViewImpl(&m_api);
            viewImpl->m_buffer = resourceImpl;
            viewImpl->m_view = view;
            *outView = viewImpl.detach();
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
    *outLayout = layout.detach();
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
    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl(m_api, desc.pipelineType);
    shaderProgram->m_pipelineType = desc.pipelineType;
    shaderProgram->slangProgram = desc.slangProgram;
    RootShaderObjectLayout::create(
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
            // TODO: report compile error.
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
    *outProgram = shaderProgram.detach();
    return SLANG_OK;
}

Result VKDevice::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout,
    ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(
        ShaderObjectLayoutImpl::createForElementType(this, typeLayout, layout.writeRef()));
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result VKDevice::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(
        this, static_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result SLANG_MCALL
    VKDevice::createRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    auto programImpl = dynamic_cast<ShaderProgramImpl*>(program);
    RefPtr<RootShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(RootShaderObjectImpl::create(
        this, programImpl->m_rootObjectLayout, shaderObject.writeRef()));
    *outObject = shaderObject.detach();
    return SLANG_OK;
}

Result VKDevice::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    auto programImpl = static_cast<ShaderProgramImpl*>(desc.program);

    if (!programImpl->m_rootObjectLayout->m_pipelineLayout)
    {
        RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
        pipelineStateImpl->init(desc);
        *outState = pipelineStateImpl.detach();
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

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->m_framebufferLayout =
        static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
    pipelineStateImpl->init(desc);
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result VKDevice::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    auto programImpl = static_cast<ShaderProgramImpl*>(desc.program);
    if (!programImpl->m_rootObjectLayout->m_pipelineLayout)
    {
        RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
        pipelineStateImpl->init(desc);
        *outState = pipelineStateImpl.detach();
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

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->init(desc);
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

} // renderer_test
