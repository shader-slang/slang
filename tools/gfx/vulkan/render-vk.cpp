// render-vk.cpp
#include "render-vk.h"

//WORKING:#include "options.h"
#include "../renderer-shared.h"
#include "../render-graphics-common.h"

#include "core/slang-basic.h"

#include "vk-api.h"
#include "vk-util.h"
#include "vk-device-queue.h"
#include "vk-swap-chain.h"

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

namespace gfx {
using namespace Slang;

class VKRenderer : public GraphicsAPIRenderer
{
public:
    enum
    {
        kMaxRenderTargets = 8,
        kMaxAttachments = kMaxRenderTargets + 1,

        kMaxDescriptorSets = 8,
    };
    // Renderer    implementation
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL initialize(const Desc& desc) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setClearColor(const float color[4]) override;
    virtual SLANG_NO_THROW void SLANG_MCALL clearFrame() override;
    virtual SLANG_NO_THROW void SLANG_MCALL beginFrame() override;
    virtual SLANG_NO_THROW void SLANG_MCALL endFrame() override;
    virtual SLANG_NO_THROW void SLANG_MCALL
        makeSwapchainImagePresentable(ISwapchain* swapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebufferLayout(const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer) override;
    virtual SLANG_NO_THROW void SLANG_MCALL setFramebuffer(IFramebuffer* frameBuffer) override;
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
        const IDescriptorSetLayout::Desc& desc,
        IDescriptorSetLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createDescriptorSet(IDescriptorSetLayout* layout, IDescriptorSet::Flag::Enum flag, IDescriptorSet** outDescriptorSet) override;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc,
        IPipelineState** outState) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc& desc,
        IPipelineState** outState) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* texture, ISlangBlob** outBlob, size_t* outRowPitch, size_t* outPixelSize) override;

    virtual SLANG_NO_THROW void* SLANG_MCALL map(IBufferResource* buffer, MapFlavor flavor) override;
    virtual SLANG_NO_THROW void SLANG_MCALL unmap(IBufferResource* buffer) override;
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
    virtual SLANG_NO_THROW void SLANG_MCALL submitGpuWork() override;
    virtual SLANG_NO_THROW void SLANG_MCALL waitForGpu() override;
    virtual SLANG_NO_THROW RendererType SLANG_MCALL getRendererType() const override
    {
        return RendererType::Vulkan;
    }
    virtual PipelineStateBase* getCurrentPipeline() override
    {
        return m_currentPipeline.Ptr();
    }
        /// Dtor
    ~VKRenderer();

    protected:

        /// Flush state from descriptor set bindings into `commandBuffer`
    void _flushBindingState(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint);

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

        BufferResourceImpl(IResource::Usage initialUsage, const IBufferResource::Desc& desc, VKRenderer* renderer):
            Parent(desc),
            m_renderer(renderer),
            m_initialUsage(initialUsage)
        {
            assert(renderer);
        }

        IResource::Usage m_initialUsage;
        VKRenderer* m_renderer;
        Buffer m_buffer;
        Buffer m_uploadBuffer;
        List<uint8_t> m_readBuffer;                         ///< Stores the contents when a map read is performed

        MapFlavor m_mapFlavor = MapFlavor::Unknown;         ///< If resource is mapped, records what kind of mapping else Unknown (if not mapped)
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
        VulkanSwapChain m_swapChain;
        ISwapchain::Desc m_desc;
        ShortList<RefPtr<TextureResourceImpl>> m_images;
        VKRenderer* m_renderer;
        uint32_t m_currentImageIndex = 0;
    public:
        Result init(VKRenderer* renderer, const ISwapchain::Desc& desc, WindowHandle window)
        {
            m_desc = desc;
            m_renderer = renderer;

            VulkanSwapChain::Desc swapchainDesc;
            VulkanSwapChain::PlatformDesc* platformDesc = nullptr;
            swapchainDesc.m_imageCount = desc.imageCount;
            swapchainDesc.init();
            swapchainDesc.m_format = desc.format;
            swapchainDesc.m_vsync = desc.enableVSync;
#if SLANG_WINDOWS_FAMILY
            VulkanSwapChain::WinPlatformDesc winPlatformDesc;
            winPlatformDesc.m_hinstance = ::GetModuleHandle(nullptr);
            winPlatformDesc.m_hwnd = (HWND)window.handleValues[0];
            platformDesc = &winPlatformDesc;
#endif

            SLANG_RETURN_ON_FAIL(m_swapChain.init(&renderer->m_deviceQueue, swapchainDesc, platformDesc));
            m_desc.format = m_swapChain.getDesc().m_format;
            m_desc.width = m_swapChain.getWidth();
            m_desc.height = m_swapChain.getHeight();
            m_desc.imageCount = m_swapChain.getDesc().m_imageCount;
            auto& images = m_swapChain.getImages();
            for (uint32_t i = 0; i < desc.imageCount; i++)
            {
                ITextureResource::Desc imageDesc = {};
                
                imageDesc.init2D(
                    IResource::Type::Texture2D,
                    m_swapChain.getDesc().m_format,
                    m_swapChain.getWidth(),
                    m_swapChain.getHeight(),
                    1);
                RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc, gfx::IResource::Usage::RenderTarget, &renderer->m_api);
                image->m_image = images[i];
                image->m_imageMemory = 0;
                image->m_vkformat = m_swapChain.getVkFormat();
                image->m_isWeakImageReference = true;
                m_images.add(image);
            }
            return SLANG_OK;
        }

        virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc()
        {
            return m_desc;
        }
        virtual SLANG_NO_THROW Result getImage(uint32_t index, ITextureResource** outResource)
        {
            *outResource = m_images[index];
            m_images[index]->addRef();
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW Result present()
        {
            m_swapChain.present(m_desc.enableVSync);
            return SLANG_OK;
        }
        virtual SLANG_NO_THROW uint32_t acquireNextImage()
        {
            m_currentImageIndex = (uint32_t)m_swapChain.nextFrontImageIndex();
            auto image = m_images[m_currentImageIndex];
            m_renderer->_transitionImageLayout(
                image->m_image,
                image->m_vkformat,
                *image->getDesc(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            return m_currentImageIndex;
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
        VkRenderPass m_renderPass;
        VKRenderer* m_renderer;

    public:
        ~FramebufferLayoutImpl()
        {
            m_renderer->m_api.vkDestroyRenderPass(m_renderer->m_api.m_device, m_renderPass, nullptr);
        }
        Result init(VKRenderer* renderer, const IFramebufferLayout::Desc& desc)
        {
            m_renderer = renderer;
            // Create render pass.
            int numAttachments = desc.renderTargetCount;
            if (desc.depthStencil)
            {
                numAttachments++;
            }
            bool shouldClear = false;
            bool shouldClearDepth = false;
            bool shouldClearStencil = false;

            // We need extra space if we have depth buffer
            Array<VkAttachmentDescription, kMaxAttachments> attachmentDesc;
            attachmentDesc.setCount(numAttachments);
            for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
            {
                auto& renderTarget = desc.renderTargets[i];
                VkAttachmentDescription& dst = attachmentDesc[i];

                dst.flags = 0;
                dst.format = VulkanUtil::getVkFormat(renderTarget.format);
                dst.samples = (VkSampleCountFlagBits)renderTarget.sampleCount;
                dst.loadOp =
                    shouldClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                dst.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                dst.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            if (desc.depthStencil)
            {
                VkAttachmentDescription& dst = attachmentDesc[desc.renderTargetCount];
                dst.flags = 0;
                dst.format = VulkanUtil::getVkFormat(desc.depthStencil->format);
                dst.samples = (VkSampleCountFlagBits)desc.depthStencil->sampleCount;
                dst.loadOp =
                    shouldClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                dst.stencilLoadOp = shouldClearStencil ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                                        : VK_ATTACHMENT_LOAD_OP_LOAD;
                dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                dst.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                dst.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }

            Array<VkAttachmentReference, kMaxRenderTargets> colorAttachments;
            colorAttachments.setCount(desc.renderTargetCount);
            for (uint32_t i = 0; i < desc.renderTargetCount; ++i)
            {
                VkAttachmentReference& dst = colorAttachments[i];
                dst.attachment = i;
                dst.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }

            VkAttachmentReference depthAttachment = {};
            depthAttachment.attachment = desc.renderTargetCount;
            depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpassDesc = {};
            subpassDesc.flags = 0;
            subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDesc.inputAttachmentCount = 0u;
            subpassDesc.pInputAttachments = nullptr;
            subpassDesc.colorAttachmentCount = desc.renderTargetCount;
            subpassDesc.pColorAttachments = colorAttachments.getBuffer();
            subpassDesc.pResolveAttachments = nullptr;
            subpassDesc.pDepthStencilAttachment = desc.depthStencil ? &depthAttachment : nullptr;
            subpassDesc.preserveAttachmentCount = 0u;
            subpassDesc.pPreserveAttachments = nullptr;

            VkRenderPassCreateInfo renderPassCreateInfo = {};
            renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassCreateInfo.attachmentCount = numAttachments;
            renderPassCreateInfo.pAttachments = attachmentDesc.getBuffer();
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
        VKRenderer* m_renderer;
        RefPtr<FramebufferLayoutImpl> m_layout;
    public:
        ~FramebufferImpl()
        {
            m_renderer->m_api.vkDestroyFramebuffer(m_renderer->m_api.m_device, m_handle, nullptr);
        }
        Result init(VKRenderer* renderer, const IFramebuffer::Desc& desc)
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
            }

            if (dsv)
            {
                imageViews[desc.renderTargetCount] = dsv->m_view;
                depthStencilView = dsv;
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

    class ShaderProgramImpl: public GraphicsCommonShaderProgram
    {
    public:

        ShaderProgramImpl(const VulkanApi& api, PipelineType pipelineType):
            m_api(&api),
            m_pipelineType(pipelineType)
        {
            for (auto& shaderModule : m_modules) shaderModule = VK_NULL_HANDLE;
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

        VkPipelineShaderStageCreateInfo m_compute;
        VkPipelineShaderStageCreateInfo m_vertex;
        VkPipelineShaderStageCreateInfo m_fragment;

        List<char> m_buffers[2];                                //< To keep storage of code in scope
        VkShaderModule m_modules[2];
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
        DescriptorSetLayoutImpl(const VulkanApi& api)
            : m_api(&api)
        {
        }

        ~DescriptorSetLayoutImpl()
        {
            if(m_descriptorSetLayout != VK_NULL_HANDLE)
            {
                m_api->vkDestroyDescriptorSetLayout(m_api->m_device, m_descriptorSetLayout, nullptr);
            }
        }

        VulkanApi const* m_api;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

        // Vulkan descriptor sets are the closest in design to what
        // the `Renderer` abstraction exposes as a `DescriptorSet`.
        // The main difference is that a `DescriptorSet` can include
        // root constant ranges, while under Vulkan push constant
        // ranges are part of the `VkPipelineLayout`, but not part
        // of any `VkDescriptorSetLayout`.
        //
        // Information about each descriptor slot range in the
        // original `Desc` will be stored as `RangeInfo` values,
        // which store the relevant information from the `Desc`
        // as well as additional information specific to the
        // Vulkan implementation path.
        //
        struct RangeInfo
        {
                /// The type of descriptor slot range from the original `Desc`
            DescriptorSlotType  type;

                /// The start index of the range in the appropriate type-specific array
            Index               arrayIndex;

                /// The equivalent Vulkan descriptor type, where applicable
            VkDescriptorType    vkDescriptorType;

                /// The Vulkan `binding` index for this range
            uint32_t            vkBindingIndex;
        };
        List<RangeInfo> m_ranges;

        // Because root constant ranges aren't part of a `VkDescriptorSetLayout`,
        // we store additional data to represent the ranges so that
        // we can store their data on a `DescriptorSetImpl` and then
        // bind it to the API later.
        //
        struct RootConstantRangeInfo
        {
                /// The offset of the range's data in the backing storage.
            Index offset;

                /// The size of the range's data.
            Index size;
        };
        Slang::List<RootConstantRangeInfo> m_rootConstantRanges;

            /// The total size, in bytes, or root constant data for this descriptor set.
        uint32_t m_rootConstantDataSize = 0;

            /// The total number of reference counted objects that can be bound
            /// to descriptor sets described by this layout.
            ///
        Index m_totalBoundObjectCount = 0;

            /// Vulkan Descriptor set bindings
        Slang::List<VkDescriptorSetLayoutBinding> m_vkBindings;

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
        PipelineLayoutImpl(const VulkanApi& api)
            : m_api(&api)
        {
        }

        ~PipelineLayoutImpl()
        {
            if (m_pipelineLayout != VK_NULL_HANDLE)
            {
                m_api->vkDestroyPipelineLayout(m_api->m_device, m_pipelineLayout, nullptr);
            }
        }

        VulkanApi const*    m_api;
        VkPipelineLayout    m_pipelineLayout        = VK_NULL_HANDLE;
        UInt                m_descriptorSetCount    = 0;

            /// For each descriptor set, stores the start offset of that set's root constant data in the pipeline layout
        List<uint32_t>      m_descriptorSetRootConstantOffsets;
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
        DescriptorSetImpl(VKRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        ~DescriptorSetImpl()
        {
            m_renderer->descriptorSetAllocator.free(m_descriptorSet);
        }

        virtual SLANG_NO_THROW void SLANG_MCALL setConstantBuffer(UInt range, UInt index, IBufferResource* buffer) override;
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

        VKRenderer*                         m_renderer = nullptr;   ///< Weak pointer, can't be strong, because if set will become circular reference
        RefPtr<DescriptorSetLayoutImpl>     m_layout;
        VulkanDescriptorSet m_descriptorSet = {};

            /// Records entities that are bound to this descriptor set, and keeps the associated resources/views/state in scope
        List<RefPtr<RefObject>>             m_boundObjects;

            /// Backing storage for root constant ranges belonging to this descriptor set
        List<char>                          m_rootConstantData;

        bool m_isTransient = false;
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

        RefPtr<PipelineLayoutImpl>  m_pipelineLayout;

        RefPtr<FramebufferLayoutImpl> m_framebufferLayout;

        RefPtr<ShaderProgramImpl> m_shaderProgram;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
    };

    VkBool32 handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg);

        /// Note that the outShaderModule value should be cleaned up when no longer needed by caller
        /// via vkShaderModuleDestroy()
    VkPipelineShaderStageCreateInfo compileEntryPoint(
        IShaderProgram::KernelDesc const&    kernelDesc,
        VkShaderStageFlagBits stage,
        List<char>& outBuffer,
        VkShaderModule& outShaderModule);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData);

    void _endRender();

    Slang::Result _beginPass();
    void _endPass();
    void _transitionImageLayout(VkImage image, VkFormat format, const TextureResource::Desc& desc, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkDebugReportCallbackEXT m_debugReportCallback;

    RefPtr<PipelineLayoutImpl>  m_currentPipelineLayout;

    RefPtr<DescriptorSetImpl>   m_currentDescriptorSetImpls [kMaxDescriptorSets];
    VkDescriptorSet             m_currentDescriptorSets     [kMaxDescriptorSets];

    RefPtr<PipelineStateImpl>   m_currentPipeline;

    RefPtr<FramebufferImpl> m_currentFramebuffer;

    List<BoundVertexBuffer> m_boundVertexBuffers;

    VkPrimitiveTopology m_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkDevice m_device = VK_NULL_HANDLE;

    VulkanModule m_module;
    VulkanApi m_api;

    VulkanDeviceQueue m_deviceQueue;

    float m_clearColor[4] = { 0, 0, 0, 0 };
    List<VkViewport> m_viewports;
    List<VkRect2D> m_scissorRects;

    Desc m_desc;

    DescriptorSetAllocator descriptorSetAllocator;

    // Temporary list used by flushBindingState to avoid per-frame allocation.
    List<VkCopyDescriptorSet> m_descSetCopies;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VkRenderer::Buffer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Result VKRenderer::Buffer::init(const VulkanApi& api, size_t bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags reqMemoryProperties)
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! VkRenderer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

Result VKRenderer::_beginPass()
{
    const int numRenderTargets = 1;

    int numAttachments = 0;

    // Start render pass
    VkClearValue clearValues[kMaxAttachments];
    clearValues[numAttachments++] = VkClearValue{ m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3] };

    bool hasDepthBuffer = false;
    if (hasDepthBuffer)
    {
        VkClearValue& clearValue = clearValues[numAttachments++];

        clearValue.depthStencil.depth = 1.0f;
        clearValue.depthStencil.stencil = 0;
    }

    const int width = m_currentFramebuffer->m_width;
    const int height = m_currentFramebuffer->m_height;

    VkCommandBuffer cmdBuffer = m_deviceQueue.getCommandBuffer();

    VkRenderPassBeginInfo renderPassBegin = {};
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.renderPass = m_currentFramebuffer->m_layout->m_renderPass;
    renderPassBegin.framebuffer = m_currentFramebuffer->m_handle;
    renderPassBegin.renderArea.offset.x = 0;
    renderPassBegin.renderArea.offset.y = 0;
    renderPassBegin.renderArea.extent.width = width;
    renderPassBegin.renderArea.extent.height = height;
    renderPassBegin.clearValueCount = numAttachments;
    renderPassBegin.pClearValues = clearValues;

    m_api.vkCmdBeginRenderPass(cmdBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

    // Set up scissor and viewport
    if (m_scissorRects.getCount())
    {
        m_api.vkCmdSetScissor(
            cmdBuffer, 0, (uint32_t)m_scissorRects.getCount(), m_scissorRects.getBuffer());
    }
    if (m_viewports.getCount())
    {
        m_api.vkCmdSetViewport(
            cmdBuffer, 0, (uint32_t)m_viewports.getCount(), m_viewports.getBuffer());
    }

    return SLANG_OK;
}

void VKRenderer::_endPass()
{
    VkCommandBuffer cmdBuffer = m_deviceQueue.getCommandBuffer();
    m_api.vkCmdEndRenderPass(cmdBuffer);
}

void VKRenderer::_endRender()
{
    m_deviceQueue.flush();

    // Make m_currentDescriptorSets consistent with m_currentDescriptorSetImpls
    // so that we don't mistakenly treat any transient descriptor sets as "copied" in the next frame.
    for (uint32_t i = 0; i < kMaxDescriptorSets; i++)
    {
        if (m_currentDescriptorSetImpls[i])
            m_currentDescriptorSets[i] = m_currentDescriptorSetImpls[i]->m_descriptorSet.handle;
    }
}

Result SLANG_MCALL createVKRenderer(const IRenderer::Desc* desc, IRenderer** outRenderer)
{
    RefPtr<VKRenderer> result = new VKRenderer();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    *outRenderer = result.detach();
    return SLANG_OK;
}

VKRenderer::~VKRenderer()
{
    // Check the device queue is valid else, we can't wait on it..
    if (m_deviceQueue.isValid())
    {
        waitForGpu();
    }

    m_currentFramebuffer.setNull();

    m_currentPipeline.setNull();

    // Same as clear but, also dtors all elements, which clear does not
    m_boundVertexBuffers = List<BoundVertexBuffer>();

    m_currentPipelineLayout.setNull();
    for (auto& impl : m_currentDescriptorSetImpls)
    {
        impl.setNull();
    }

    m_deviceQueue.destroy();

    descriptorSetAllocator.close();

    if (m_device != VK_NULL_HANDLE)
    {
        m_api.vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
}


VkBool32 VKRenderer::handleDebugMessage(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
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

    OutputDebugStringA(buffer);

    return VK_FALSE;
}

/* static */VKAPI_ATTR VkBool32 VKAPI_CALL VKRenderer::debugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
    size_t location, int32_t msgCode, const char* pLayerPrefix, const char* pMsg, void* pUserData)
{
    return ((VKRenderer*)pUserData)->handleDebugMessage(flags, objType, srcObject, location, msgCode, pLayerPrefix, pMsg);
}

VkPipelineShaderStageCreateInfo VKRenderer::compileEntryPoint(
    IShaderProgram::KernelDesc const&    kernelDesc,
    VkShaderStageFlagBits stage,
    List<char>& outBuffer,
    VkShaderModule& outShaderModule)
{
    char const* dataBegin = (char const*) kernelDesc.codeBegin;
    char const* dataEnd = (char const*) kernelDesc.codeEnd;

    // We need to make a copy of the code, since the Slang compiler
    // will free the memory after a compile request is closed.
    size_t codeSize = dataEnd - dataBegin;

    outBuffer.insertRange(0, dataBegin, codeSize);

    char* codeBegin = outBuffer.getBuffer();

    VkShaderModuleCreateInfo moduleCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    moduleCreateInfo.pCode = (uint32_t*)codeBegin;
    moduleCreateInfo.codeSize = codeSize;

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

SlangResult VKRenderer::initialize(const Desc& desc)
{
    SLANG_RETURN_ON_FAIL(slangContext.initialize(desc.slang, SLANG_SPIRV, "sm_5_1"));

    SLANG_RETURN_ON_FAIL(GraphicsAPIRenderer::initialize(desc));

    SLANG_RETURN_ON_FAIL(m_module.init());
    SLANG_RETURN_ON_FAIL(m_api.initGlobalProcs(m_module));
    descriptorSetAllocator.m_api = &m_api;
    m_desc = desc;

    VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    applicationInfo.pApplicationName = "slang-render-test";
    applicationInfo.pEngineName = "slang-render-test";
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    char const* instanceExtensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,

        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,

#if SLANG_WINDOWS_FAMILY
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
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

#if ENABLE_VALIDATION_LAYER
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
#endif

    if (m_api.vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
        return SLANG_FAIL;
    SLANG_RETURN_ON_FAIL(m_api.initInstanceProcs(instance));

#if ENABLE_VALIDATION_LAYER
    VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

    VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
    debugCreateInfo.pfnCallback = &debugMessageCallback;
    debugCreateInfo.pUserData = this;
    debugCreateInfo.flags = debugFlags;

    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateDebugReportCallbackEXT(instance, &debugCreateInfo, nullptr, &m_debugReportCallback));
#endif

    uint32_t numPhysicalDevices = 0;
    SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr));

    List<VkPhysicalDevice> physicalDevices;
    physicalDevices.setCount(numPhysicalDevices);
    SLANG_VK_RETURN_ON_FAIL(m_api.vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.getBuffer()));

    Index selectedDeviceIndex = 0;

    if (desc.adapter)
    {
        selectedDeviceIndex = -1;

        String lowerAdapter = String(desc.adapter).toLower();

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

    // API version check, can't use vkGetPhysicalDeviceProperties2 yet since this device might not support it
    if (VK_MAKE_VERSION(majorVersion, minorVersion, 0) >= VK_API_VERSION_1_1 &&
        m_api.vkGetPhysicalDeviceProperties2 &&
        m_api.vkGetPhysicalDeviceFeatures2)
    {

        // Get device features
        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

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
    }

    int queueFamilyIndex = m_api.findQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    assert(queueFamilyIndex >= 0);

    float queuePriority = 0.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    deviceCreateInfo.enabledExtensionCount = uint32_t(deviceExtensions.getCount());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.getBuffer();

    SLANG_VK_RETURN_ON_FAIL(m_api.vkCreateDevice(m_api.m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));
    SLANG_RETURN_ON_FAIL(m_api.initDeviceProcs(m_device));

    {
        VkQueue queue;
        m_api.vkGetDeviceQueue(m_device, queueFamilyIndex, 0, &queue);
        SLANG_RETURN_ON_FAIL(m_deviceQueue.init(m_api, queue, queueFamilyIndex));
    }
    return SLANG_OK;
}

void VKRenderer::submitGpuWork()
{
    m_deviceQueue.flush();
}

void VKRenderer::waitForGpu()
{
    m_deviceQueue.flushAndWait();
}

void VKRenderer::setClearColor(const float color[4])
{
    for (int ii = 0; ii < 4; ++ii)
        m_clearColor[ii] = color[ii];
}

void VKRenderer::clearFrame()
{
    _beginPass();
    ShortList<VkClearAttachment> clears;
    for (Index i = 0; i < m_currentFramebuffer->renderTargetViews.getCount(); i++)
    {
        VkClearAttachment attachment;
        attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        memcpy(attachment.clearValue.color.float32, m_clearColor, sizeof(float) * 4);
        attachment.colorAttachment = (int)i;
        clears.add(attachment);
    }
    if (m_currentFramebuffer->depthStencilView)
    {
        VkClearAttachment attachment;
        attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        attachment.clearValue.depthStencil.depth = 1.0f;
        attachment.clearValue.depthStencil.stencil = 0;
        clears.add(attachment);
    }
    VkClearRect rect = {};
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;
    rect.rect.extent.width = m_currentFramebuffer->m_width;
    rect.rect.extent.height = m_currentFramebuffer->m_height;
    m_api.vkCmdClearAttachments(
        m_deviceQueue.getCommandBuffer(),
        (uint32_t)clears.getCount(),
        clears.getArrayView().getBuffer(),
        1,
        &rect);
    _endPass();
}

void VKRenderer::beginFrame()
{
    if (m_deviceQueue.isCurrent(VulkanDeviceQueue::EventType::EndFrame))
        m_deviceQueue.makeCompleted(VulkanDeviceQueue::EventType::EndFrame);
}

void VKRenderer::endFrame()
{
    _endRender();
}

void VKRenderer::makeSwapchainImagePresentable(ISwapchain* swapchain)
{
    auto swapchainImpl = static_cast<SwapchainImpl*>(swapchain);
    auto image = swapchainImpl->m_images[swapchainImpl->m_currentImageIndex];
    _transitionImageLayout(
        image->m_image,
        image->m_vkformat,
        *image->getDesc(),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    if (!m_deviceQueue.isCurrent(VulkanDeviceQueue::EventType::EndFrame))
        m_deviceQueue.makeCurrent(VulkanDeviceQueue::EventType::EndFrame);
}

Result VKRenderer::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> sc = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(sc->init(this, desc, window));
    *outSwapchain = sc.detach();
    return SLANG_OK;
}

Result VKRenderer::createFramebufferLayout(const IFramebufferLayout::Desc& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    SLANG_RETURN_ON_FAIL(layout->init(this, desc));
    *outLayout = layout.detach();
    return SLANG_OK;
}

Result VKRenderer::createFramebuffer(const IFramebuffer::Desc& desc, IFramebuffer** outFramebuffer)
{
    RefPtr<FramebufferImpl> fb = new FramebufferImpl();
    SLANG_RETURN_ON_FAIL(fb->init(this, desc));
    *outFramebuffer = fb.detach();
    return SLANG_OK;
}

void VKRenderer::setFramebuffer(IFramebuffer* framebuffer)
{
    m_currentFramebuffer = static_cast<FramebufferImpl*>(framebuffer);
}

SlangResult VKRenderer::readTextureResource(
    ITextureResource* texture, ISlangBlob** outBlob, size_t* outRowPitch, size_t* outPixelSize)
{
    SLANG_UNUSED(texture);
    SLANG_UNUSED(outBlob);
    SLANG_UNUSED(outRowPitch);
    SLANG_UNUSED(outPixelSize);
    return SLANG_FAIL;
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

void VKRenderer::_transitionImageLayout(VkImage image, VkFormat format, const TextureResource::Desc& desc, VkImageLayout oldLayout, VkImageLayout newLayout)
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

Result VKRenderer::createTextureResource(IResource::Usage initialUsage, const ITextureResource::Desc& descIn, const ITextureResource::Data* initData, ITextureResource** outResource)
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

    if (initData)
    {
        List<TextureResource::Size> mipSizes;

        VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

        const int numMipMaps = desc.numMipLevels;
        assert(initData->numMips == numMipMaps);

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

        Buffer uploadBuffer;
        SLANG_RETURN_ON_FAIL(uploadBuffer.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        assert(mipSizes.getCount() == numMipMaps);

        // Copy into upload buffer
        {
            int subResourceIndex = 0;

            uint8_t* dstData;
            m_api.vkMapMemory(m_device, uploadBuffer.m_memory, 0, bufferSize, 0, (void**)&dstData);

            for (int i = 0; i < arraySize; ++i)
            {
                for (Index j = 0; j < mipSizes.getCount(); ++j)
                {
                    const auto& mipSize = mipSizes[j];

                    const ptrdiff_t srcRowStride = initData->mipRowStrides[j];
                    auto dstRowSizeInBytes = calcRowSize(desc.format, mipSize.width);
                    auto numRows = calcNumRows(desc.format, mipSize.height);

                    for (int k = 0; k < mipSize.depth; k++)
                    {
                        const uint8_t* srcData = (const uint8_t*)(initData->subResources[subResourceIndex]);

                        for (uint32_t l = 0; l < numRows; l++)
                        {
                            ::memcpy(dstData, srcData, dstRowSizeInBytes);

                            dstData += dstRowSizeInBytes;
                            srcData += srcRowStride;
                        }

                        subResourceIndex++;
                    }
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

        m_deviceQueue.flushAndWait();
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
    *outResource = texture.detach();
    return SLANG_OK;
}

Result VKRenderer::createBufferResource(IResource::Usage initialUsage, const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
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

        //flushCommandBuffer(commandBuffer);
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
    rs.compareMask = desc.stencilCompareMask;
    rs.compareOp = translateComparisonFunc(desc.stencilFunc);
    rs.depthFailOp = translateStencilOp(desc.stencilDepthFailOp);
    rs.failOp = translateStencilOp(desc.stencilFailOp);
    rs.passOp = translateStencilOp(desc.stencilPassOp);
    rs.reference = desc.stencilReference;
    rs.writeMask = desc.stencilWriteMask;
    return rs;
}

Result VKRenderer::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
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

Result VKRenderer::createTextureView(ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
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

Result VKRenderer::createBufferView(IBufferResource* buffer, IResourceView::Desc const& desc, IResourceView** outView)
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

Result VKRenderer::createInputLayout(const InputElementDesc* elements, UInt numElements, IInputLayout** outLayout)
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

void* VKRenderer::map(IBufferResource* bufferIn, MapFlavor flavor)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    assert(buffer->m_mapFlavor == MapFlavor::Unknown);

    // Make sure everything has completed before reading...
    m_deviceQueue.flushAndWait();

    const size_t bufferSize = buffer->getDesc()->sizeInBytes;

    switch (flavor)
    {
        case MapFlavor::WriteDiscard:
        case MapFlavor::HostWrite:
        {
            if (!buffer->m_uploadBuffer.isInitialized())
            {
                return nullptr;
            }

            void* mappedData = nullptr;
            SLANG_VK_CHECK(m_api.vkMapMemory(m_device, buffer->m_uploadBuffer.m_memory, 0, bufferSize, 0, &mappedData));
            buffer->m_mapFlavor = flavor;
            return mappedData;
        }
        case MapFlavor::HostRead:
        {
            // Make sure there is space in the read buffer
            buffer->m_readBuffer.setCount(bufferSize);

            // create staging buffer
            Buffer staging;

            SLANG_RETURN_NULL_ON_FAIL(staging.init(m_api, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

            // Copy from real buffer to staging buffer
            VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

            VkBufferCopy copyInfo = {};
            copyInfo.size = bufferSize;
            m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_buffer.m_buffer, staging.m_buffer, 1, &copyInfo);

            m_deviceQueue.flushAndWait();

            // Write out the data from the buffer
            void* mappedData = nullptr;
            SLANG_VK_CHECK(m_api.vkMapMemory(m_device, staging.m_memory, 0, bufferSize, 0, &mappedData));

            ::memcpy(buffer->m_readBuffer.getBuffer(), mappedData, bufferSize);
            m_api.vkUnmapMemory(m_device, staging.m_memory);

            buffer->m_mapFlavor = flavor;

            return buffer->m_readBuffer.getBuffer();
        }
        default:
            return nullptr;
    }
}

void VKRenderer::unmap(IBufferResource* bufferIn)
{
    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);
    assert(buffer->m_mapFlavor != MapFlavor::Unknown);

    const size_t bufferSize = buffer->getDesc()->sizeInBytes;

    switch (buffer->m_mapFlavor)
    {
        case MapFlavor::WriteDiscard:
        case MapFlavor::HostWrite:
        {
            m_api.vkUnmapMemory(m_device, buffer->m_uploadBuffer.m_memory);

            // Copy from staging buffer to real buffer
            VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

            VkBufferCopy copyInfo = {};
            copyInfo.size = bufferSize;
            m_api.vkCmdCopyBuffer(commandBuffer, buffer->m_uploadBuffer.m_buffer, buffer->m_buffer.m_buffer, 1, &copyInfo);

            // TODO: is this necessary?
            //m_deviceQueue.flushAndWait();
            break;
        }
        default: break;
    }

    // Mark as no longer mapped
    buffer->m_mapFlavor = MapFlavor::Unknown;
}

void VKRenderer::setPrimitiveTopology(PrimitiveTopology topology)
{
    m_primitiveTopology = VulkanUtil::getVkPrimitiveTopology(topology);
}

void VKRenderer::setVertexBuffers(UInt startSlot, UInt slotCount, IBufferResource*const* buffers, const UInt* strides, const UInt* offsets)
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

void VKRenderer::setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
{
}

void VKRenderer::setViewports(UInt count, Viewport const* viewports)
{
    static const int kMaxViewports = 8; // TODO: base on device caps
    assert(count <= kMaxViewports);

    m_viewports.setCount(count);
    for(UInt ii = 0; ii < count; ++ii)
    {
        auto& inViewport = viewports[ii];
        auto& vkViewport = m_viewports[ii];

        vkViewport.x        = inViewport.originX;
        vkViewport.y        = inViewport.originY;
        vkViewport.width    = inViewport.extentX;
        vkViewport.height   = inViewport.extentY;
        vkViewport.minDepth = inViewport.minZ;
        vkViewport.maxDepth = inViewport.maxZ;
    }

    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();
    m_api.vkCmdSetViewport(commandBuffer, 0, uint32_t(count), m_viewports.getBuffer());
}

void VKRenderer::setScissorRects(UInt count, ScissorRect const* rects)
{
    static const int kMaxScissorRects = 8; // TODO: base on device caps
    assert(count <= kMaxScissorRects);

    m_scissorRects.setCount(count);
    for(UInt ii = 0; ii < count; ++ii)
    {
        auto& inRect = rects[ii];
        auto& vkRect = m_scissorRects[ii];

        vkRect.offset.x         = int32_t(inRect.minX);
        vkRect.offset.y         = int32_t(inRect.minY);
        vkRect.extent.width     = uint32_t(inRect.maxX - inRect.minX);
        vkRect.extent.height    = uint32_t(inRect.maxY - inRect.minY);

    }

    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();
    m_api.vkCmdSetScissor(commandBuffer, 0, uint32_t(count), m_scissorRects.getBuffer());
}

void VKRenderer::setPipelineState(IPipelineState* state)
{
    m_currentPipeline = static_cast<PipelineStateImpl*>(state);
}

void VKRenderer::_flushBindingState(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint)
{
    auto pipeline = m_currentPipeline;
    auto& descSetCopies = m_descSetCopies;
    descSetCopies.clear();
    // We start by binding the pipeline state.
    //
    m_api.vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline->m_pipeline);

    // Next we bind all the descriptor sets that were set in the `VKRenderer`.
    //
    auto pipelineLayoutImpl = pipeline->m_pipelineLayout.Ptr();
    auto vkPipelineLayout = pipelineLayoutImpl->m_pipelineLayout;
    auto descriptorSetCount = pipelineLayoutImpl->m_descriptorSetCount;
    for (uint32_t i = 0; i < (uint32_t)descriptorSetCount; i++)
    {
        if (m_currentDescriptorSetImpls[i]->m_isTransient)
        {
            // A transient descriptor set may go out of life cycle after command list recording,
            // therefore we must make a copy of it in the per-frame descriptor pool.

            // If we have already created a transient copy for this descriptor set, skip the copy.
            if (m_currentDescriptorSetImpls[i]->m_descriptorSet.handle !=
                m_currentDescriptorSets[i])
                continue;

            auto descSet = m_deviceQueue.allocTransientDescriptorSet(
                m_currentDescriptorSetImpls[i]->m_layout->m_descriptorSetLayout);
            uint32_t bindingIndex = 0;
            for (auto binding : m_currentDescriptorSetImpls[i]->m_layout->m_vkBindings)
            {
                VkCopyDescriptorSet copy = {};
                copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                copy.srcSet = m_currentDescriptorSetImpls[i]->m_descriptorSet.handle;
                copy.dstSet = descSet.handle;
                copy.srcBinding = copy.dstBinding = bindingIndex;
                copy.srcArrayElement = copy.dstArrayElement = 0;
                copy.descriptorCount = binding.descriptorCount;
                descSetCopies.add(copy);
                bindingIndex++;
            }
            m_currentDescriptorSets[i] = descSet.handle;
        }
    }
    if (descSetCopies.getCount())
    {
        m_api.vkUpdateDescriptorSets(
            m_api.m_device, 0, nullptr, (uint32_t)descSetCopies.getCount(), descSetCopies.getBuffer());
    }
    m_api.vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, vkPipelineLayout,
        0, uint32_t(descriptorSetCount),
        &m_currentDescriptorSets[0],
        0, nullptr);

    // For any descriptor sets with root-constant ranges, we need to
    // bind the relevant data to the context.
    //
    for(gfx::UInt ii = 0; ii < descriptorSetCount; ++ii)
    {
        auto descriptorSet = m_currentDescriptorSetImpls[ii];
        auto descriptorSetLayout = descriptorSet->m_layout;
        auto size = descriptorSetLayout->m_rootConstantDataSize;
        if(size == 0)
            continue;
        auto data = descriptorSet->m_rootConstantData.getBuffer();

        // The absolute offset of the descriptor set's data in
        // the push-constant data for the entire pipeline was
        // computed and cached in the pipeline layout.
        //
        uint32_t offset = pipelineLayoutImpl->m_descriptorSetRootConstantOffsets[ii];

        m_api.vkCmdPushConstants(commandBuffer, vkPipelineLayout, VK_SHADER_STAGE_ALL, offset, size, data);
    }
}

void VKRenderer::draw(UInt vertexCount, UInt startVertex = 0)
{
    auto pipeline = m_currentPipeline;
    if (!pipeline || pipeline->m_shaderProgram->m_pipelineType != PipelineType::Graphics)
    {
        assert(!"Invalid render pipeline");
        return;
    }

    SLANG_RETURN_VOID_ON_FAIL(_beginPass());

    // Also create descriptor sets based on the given pipeline layout
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    _flushBindingState(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

    // Bind the vertex buffer
    if (m_boundVertexBuffers.getCount() > 0 && m_boundVertexBuffers[0].m_buffer)
    {
        const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[0];

        VkBuffer vertexBuffers[] = { boundVertexBuffer.m_buffer->m_buffer.m_buffer };
        VkDeviceSize offsets[] = { VkDeviceSize(boundVertexBuffer.m_offset) };

        m_api.vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    }

    m_api.vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertexCount), 1, 0, 0);

    _endPass();
}

void VKRenderer::drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
{
}

void VKRenderer::dispatchCompute(int x, int y, int z)
{
    auto pipeline = m_currentPipeline;
    if (!pipeline || pipeline->m_shaderProgram->m_pipelineType != PipelineType::Compute)
    {
        assert(!"Invalid compute pipeline");
        return;
    }

    // Also create descriptor sets based on the given pipeline layout
    VkCommandBuffer commandBuffer = m_deviceQueue.getCommandBuffer();

    _flushBindingState(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);

    m_api.vkCmdDispatch(commandBuffer, x, y, z);
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

static VkDescriptorType translateDescriptorType(DescriptorSlotType type)
{
    switch(type)
    {
    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;

#define CASE(SRC, DST) \
    case DescriptorSlotType::SRC: return VK_DESCRIPTOR_TYPE_##DST

        CASE(Sampler,               SAMPLER);
        CASE(CombinedImageSampler,  COMBINED_IMAGE_SAMPLER);
        CASE(SampledImage,          SAMPLED_IMAGE);
        CASE(StorageImage,          STORAGE_IMAGE);
        CASE(UniformTexelBuffer,    UNIFORM_TEXEL_BUFFER);
        CASE(StorageTexelBuffer,    STORAGE_TEXEL_BUFFER);
        CASE(UniformBuffer,         UNIFORM_BUFFER);
        CASE(ReadOnlyStorageBuffer, STORAGE_BUFFER);
        CASE(StorageBuffer,         STORAGE_BUFFER);
        CASE(DynamicUniformBuffer,  UNIFORM_BUFFER_DYNAMIC);
        CASE(DynamicStorageBuffer,  STORAGE_BUFFER_DYNAMIC);
        CASE(InputAttachment,       INPUT_ATTACHMENT);

#undef CASE
    }
}

Result VKRenderer::createDescriptorSetLayout(const IDescriptorSetLayout::Desc& desc, IDescriptorSetLayout** outLayout)
{
    RefPtr<DescriptorSetLayoutImpl> descriptorSetLayoutImpl = new DescriptorSetLayoutImpl(m_api);

    auto& dstBindings = descriptorSetLayoutImpl->m_vkBindings;

    Slang::List<uint32_t> descriptorCountForTypes; 

    UInt rangeCount = desc.slotRangeCount;
    for(UInt rr = 0; rr < rangeCount; ++rr)
    {
        auto& srcRange = desc.slotRanges[rr];

        if(srcRange.type == DescriptorSlotType::RootConstant)
        {
            // Root constant ranges are a special case, since they
            // don't actually map to `VkDescriptorSetLayoutBinding`s
            // like the other cases.

            // We start by computing the offset of the range within
            // the backing storage for the descriptor set, while
            // also updating the computed total size of root constant
            // data needed by the set.
            //
            auto size = uint32_t(srcRange.count);
            auto offset = descriptorSetLayoutImpl->m_rootConstantDataSize;
            descriptorSetLayoutImpl->m_rootConstantDataSize += size;

            // We will keep track of the information for this
            // range as part of the descriptor set layout.
            //
            DescriptorSetLayoutImpl::RootConstantRangeInfo rootConstantRangeInfo;
            rootConstantRangeInfo.offset = offset;
            rootConstantRangeInfo.size = size;

            auto rootConstantRangeIndex = descriptorSetLayoutImpl->m_rootConstantRanges.getCount();
            descriptorSetLayoutImpl->m_rootConstantRanges.add(rootConstantRangeInfo);

            // We will also add a `RangeInfo` to represent this
            // range, even though it doesn't map to a VK-level
            // descriptor range.
            //
            DescriptorSetLayoutImpl::RangeInfo rangeInfo;
            rangeInfo.type = srcRange.type;
            rangeInfo.vkDescriptorType = VkDescriptorType(-1);
            rangeInfo.arrayIndex = rootConstantRangeIndex;
            descriptorSetLayoutImpl->m_ranges.add(rangeInfo);

            // Finally, we bail out instead of performing
            // the logic that applies to the other descriptor
            // range types.
            //
            continue;
        }

        // Note: Because of the existence of root constant ranges,
        // we cannot assume that the `binding` for a range is
        // the same as its index in the input array of ranges.
        //
        // Instead, the `binding` for a range is its index in
        // the output array of `VkDescriptorSetLayoutBinding`s.
        //
        uint32_t bindingIndex = uint32_t(dstBindings.getCount());

        VkDescriptorType dstDescriptorType = translateDescriptorType(srcRange.type);

        VkDescriptorSetLayoutBinding dstBinding;
        dstBinding.binding = uint32_t(bindingIndex);
        dstBinding.descriptorType = dstDescriptorType;
        dstBinding.descriptorCount = uint32_t(srcRange.count);
        dstBinding.stageFlags = VK_SHADER_STAGE_ALL;
        dstBinding.pImmutableSamplers = nullptr;

        if (descriptorCountForTypes.getCount() <= dstDescriptorType)
        {
            descriptorCountForTypes.setCount(dstDescriptorType + 1);
        }

        descriptorCountForTypes[dstDescriptorType] += uint32_t(srcRange.count);

        dstBindings.add(dstBinding);

        UInt boundObjectCount = srcRange.count;
        if( srcRange.type == DescriptorSlotType::CombinedImageSampler )
        {
            boundObjectCount = 2 * srcRange.count;
        }

        auto boundObjectArrayIndex = descriptorSetLayoutImpl->m_totalBoundObjectCount;
        descriptorSetLayoutImpl->m_totalBoundObjectCount += boundObjectCount;

        DescriptorSetLayoutImpl::RangeInfo rangeInfo;
        rangeInfo.type = srcRange.type;
        rangeInfo.vkDescriptorType = dstDescriptorType;
        rangeInfo.vkBindingIndex = bindingIndex;
        rangeInfo.arrayIndex = boundObjectArrayIndex;
        descriptorSetLayoutImpl->m_ranges.add(rangeInfo);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    descriptorSetLayoutInfo.bindingCount = uint32_t(dstBindings.getCount());
    descriptorSetLayoutInfo.pBindings = dstBindings.getBuffer();

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout));

    descriptorSetLayoutImpl->m_descriptorSetLayout = descriptorSetLayout;

    *outLayout = descriptorSetLayoutImpl.detach();
    return SLANG_OK;
}

Result VKRenderer::createPipelineLayout(const IPipelineLayout::Desc& desc, IPipelineLayout** outLayout)
{
    UInt descriptorSetCount = desc.descriptorSetCount;

    VkDescriptorSetLayout descriptorSetLayouts[kMaxDescriptorSets];
    uint32_t descriptorSetRootConstantOffsets[kMaxDescriptorSets];
    uint32_t totalRootConstantSize = 0;
    for(UInt ii = 0; ii < descriptorSetCount; ++ii)
    {
        auto descriptorSetLayoutImpl = (DescriptorSetLayoutImpl*) desc.descriptorSets[ii].layout;
        descriptorSetLayouts[ii] = descriptorSetLayoutImpl->m_descriptorSetLayout;

        descriptorSetRootConstantOffsets[ii] = totalRootConstantSize;
        totalRootConstantSize += descriptorSetLayoutImpl->m_rootConstantDataSize;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = uint32_t(desc.descriptorSetCount);
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts[0];

    // Our abstraction allows the user to specify any number of root-constant
    // ranges across all of their descriptor sets, but Vulkan has a restriction
    // that a pipeline layout may only include a single push constant range
    // accessible from a given stage. (In other words, the only situation where
    // multiple push-constant ranges are allowed is if you want to have, say,
    // distinct ranges for the vertex and fragment stages to access).
    //
    // We handle this by declaring at most one push constant range, which
    // represents the concatenation of the data from all ranges that the
    // user might have asked for.
    //
    // Note: The Slang compiler doesn't yet have logic to concatenate multiple
    // push-constant ranges in this way, but if/when it does, it should hopefully
    // Just Work with this logic.
    //
    VkPushConstantRange pushConstantRange;
    if( totalRootConstantSize )
    {
        pushConstantRange.offset = 0;
        pushConstantRange.size = totalRootConstantSize;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;

        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    }

    VkPipelineLayout pipelineLayout;
    SLANG_VK_CHECK(m_api.vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    RefPtr<PipelineLayoutImpl> pipelineLayoutImpl = new PipelineLayoutImpl(m_api);
    pipelineLayoutImpl->m_pipelineLayout = pipelineLayout;
    pipelineLayoutImpl->m_descriptorSetCount = descriptorSetCount;

    for(UInt ii = 0; ii < descriptorSetCount; ++ii)
    {
        pipelineLayoutImpl->m_descriptorSetRootConstantOffsets.add(
            descriptorSetRootConstantOffsets[ii]);
    }

    *outLayout = pipelineLayoutImpl.detach();
    return SLANG_OK;
}

Result VKRenderer::createDescriptorSet(
    IDescriptorSetLayout* layout,
    IDescriptorSet::Flag::Enum flag,
    IDescriptorSet** outDescriptorSet)
{
    auto layoutImpl = (DescriptorSetLayoutImpl*)layout;

    RefPtr<DescriptorSetImpl> descriptorSetImpl = new DescriptorSetImpl(this);
    descriptorSetImpl->m_layout = layoutImpl;
    descriptorSetImpl->m_descriptorSet =
        descriptorSetAllocator.allocate(layoutImpl->m_descriptorSetLayout);
    descriptorSetImpl->m_isTransient = (flag == IDescriptorSet::Flag::Enum::Transient);
    descriptorSetImpl->m_rootConstantData.setCount(layoutImpl->m_rootConstantDataSize);
    descriptorSetImpl->m_boundObjects.setCount(layoutImpl->m_totalBoundObjectCount);

    *outDescriptorSet = descriptorSetImpl.detach();
    return SLANG_OK;
}

void VKRenderer::DescriptorSetImpl::setConstantBuffer(UInt range, UInt index, IBufferResource* buffer)
{
    auto bufferImpl = (BufferResourceImpl*)buffer;

    SLANG_ASSERT(range < UInt(m_layout->m_ranges.getCount()));
    auto& rangeInfo = m_layout->m_ranges[range];
    auto bindingIndex = rangeInfo.vkBindingIndex;
    auto boundObjectIndex = rangeInfo.arrayIndex + index;

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = bufferImpl->m_buffer.m_buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = bufferImpl->getDesc()->sizeInBytes;

    VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writeInfo.dstSet = m_descriptorSet.handle;
    writeInfo.dstBinding = uint32_t(bindingIndex);
    writeInfo.dstArrayElement = uint32_t(index);
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = rangeInfo.vkDescriptorType;
    writeInfo.pBufferInfo = &bufferInfo;

    m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);
    m_boundObjects[boundObjectIndex] = dynamic_cast<RefObject*>(buffer);
}

void VKRenderer::DescriptorSetImpl::setResource(UInt range, UInt index, IResourceView* view)
{
    SLANG_ASSERT(range < UInt(m_layout->m_ranges.getCount()));
    auto& rangeInfo = m_layout->m_ranges[range];
    auto bindingIndex = rangeInfo.vkBindingIndex;
    auto boundObjectIndex = rangeInfo.arrayIndex + index;
    auto descriptorType = rangeInfo.vkDescriptorType;

    auto viewImpl = (ResourceViewImpl*)view;
    switch (viewImpl->m_type)
    {
    case ResourceViewImpl::ViewType::Texture:
        {
            auto textureViewImpl = (TextureResourceViewImpl*)viewImpl;
            VkDescriptorImageInfo imageInfo = {};
            imageInfo.imageView = textureViewImpl->m_view;
            imageInfo.imageLayout = textureViewImpl->m_layout;

            VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeInfo.dstSet = m_descriptorSet.handle;
            writeInfo.dstBinding = uint32_t(bindingIndex);
            writeInfo.dstArrayElement = uint32_t(index);
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = descriptorType;
            writeInfo.pImageInfo = &imageInfo;

            m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);
        }
        break;

    case ResourceViewImpl::ViewType::TexelBuffer:
        {
            auto bufferViewImpl = (TexelBufferResourceViewImpl*)viewImpl;

            VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeInfo.dstSet = m_descriptorSet.handle;
            writeInfo.dstBinding = uint32_t(bindingIndex);
            writeInfo.dstArrayElement = uint32_t(index);
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = descriptorType;
            writeInfo.pTexelBufferView = &bufferViewImpl->m_view;

            m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);
        }
        break;

    case ResourceViewImpl::ViewType::PlainBuffer:
        {
            auto bufferViewImpl = (PlainBufferResourceViewImpl*) viewImpl;

            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = bufferViewImpl->m_buffer->m_buffer.m_buffer;
            bufferInfo.offset = bufferViewImpl->offset;
            bufferInfo.range = bufferViewImpl->size;

            VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            writeInfo.dstSet = m_descriptorSet.handle;
            writeInfo.dstBinding = uint32_t(bindingIndex);
            writeInfo.dstArrayElement = uint32_t(index);
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = descriptorType;
            writeInfo.pBufferInfo = &bufferInfo;

            m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);
        }
        break;
    }

    m_boundObjects[boundObjectIndex] = dynamic_cast<RefObject*>(view);
}

void VKRenderer::DescriptorSetImpl::setSampler(UInt range, UInt index, ISamplerState* sampler)
{
    SLANG_ASSERT(range < UInt(m_layout->m_ranges.getCount()));
    auto& rangeInfo = m_layout->m_ranges[range];
    SLANG_ASSERT(rangeInfo.type == DescriptorSlotType::Sampler);
    auto bindingIndex = rangeInfo.vkBindingIndex;
    auto boundObjectIndex = rangeInfo.arrayIndex + index;
    auto descriptorType = rangeInfo.vkDescriptorType;

    VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writeInfo.dstSet = m_descriptorSet.handle;
    writeInfo.dstBinding = uint32_t(bindingIndex);
    writeInfo.dstArrayElement = uint32_t(index);
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = descriptorType;
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = static_cast<SamplerStateImpl*>(sampler)->m_sampler;
    writeInfo.pImageInfo = &imageInfo;
    m_renderer->m_api.vkUpdateDescriptorSets(m_renderer->m_device, 1, &writeInfo, 0, nullptr);

    m_boundObjects[boundObjectIndex] = dynamic_cast<RefObject*>(sampler);
}

void VKRenderer::DescriptorSetImpl::setCombinedTextureSampler(
    UInt range,
    UInt index,
    IResourceView*   textureView,
    ISamplerState*   sampler)
{
    SLANG_ASSERT(range < UInt(m_layout->m_ranges.getCount()));
    auto& rangeInfo = m_layout->m_ranges[range];
    SLANG_ASSERT(rangeInfo.type == DescriptorSlotType::CombinedImageSampler);
    auto bindingIndex = rangeInfo.vkBindingIndex;
    auto descriptorType = rangeInfo.vkDescriptorType;

    // TODO: Actually bind it!

    // Note: Each entry in a combined texture/sampler range consumes
    // two entries in the `m_boundObjects` array, since we have
    // to keep both the texture view and the sampler object live.
    //
    auto boundObjectIndex = rangeInfo.arrayIndex + 2 * index;
    m_boundObjects[boundObjectIndex + 0] = dynamic_cast<RefObject*>(textureView);
    m_boundObjects[boundObjectIndex + 1] = dynamic_cast<RefObject*>(sampler);
}

void VKRenderer::DescriptorSetImpl::setRootConstants(
    UInt range,
    UInt offset,
    UInt size,
    void const* data)
{
    // The `range` variabel is the index of one of the descriptor
    // slot ranges, which had better be a `RootConstant` range.
    //
    SLANG_ASSERT(range < UInt(m_layout->m_ranges.getCount()));
    auto& rangeInfo = m_layout->m_ranges[range];
    SLANG_ASSERT(rangeInfo.type == DescriptorSlotType::RootConstant);

    // The `arrayIndex` for the descriptor slot range will refer
    // to a root constant range, which is the range to be set.
    //
    auto rootConstantIndex = rangeInfo.arrayIndex;
    SLANG_ASSERT(rootConstantIndex >= 0);
    SLANG_ASSERT(rootConstantIndex < m_layout->m_rootConstantRanges.getCount());
    auto& rootConstantRangeInfo = m_layout->m_rootConstantRanges[rootConstantIndex];
    SLANG_ASSERT(offset + size <= UInt(rootConstantRangeInfo.size));

    memcpy(m_rootConstantData.getBuffer() + rootConstantRangeInfo.offset + offset, data, size);
}

void VKRenderer::setDescriptorSet(PipelineType pipelineType, IPipelineLayout* layout, UInt index, IDescriptorSet* descriptorSet)
{
    // Ideally this should eventually be as simple as:
    //
    //      m_api.vkCmdBindDescriptorSets(
    //          commandBuffer,
    //          translatePipelineBindPoint(pipelineType),
    //          layout->m_pipelineLayout,
    //          index,
    //          1,
    //          ((DescriptorSetImpl*) descriptorSet)->m_descriptorSet,
    //          0,
    //          nullptr);
    //
    // For now we are lazily flushing state right before drawing, so
    // we will hang onto the parameters that were passed in and then
    // use them later.
    //

    auto descriptorSetImpl = (DescriptorSetImpl*)descriptorSet;
    m_currentDescriptorSetImpls[index] = descriptorSetImpl;
    m_currentDescriptorSets[index] = descriptorSetImpl->m_descriptorSet.handle;
}

Result VKRenderer::createProgram(const IShaderProgram::Desc& desc, IShaderProgram** outProgram)
{
    if (desc.slangProgram && desc.slangProgram->getSpecializationParamCount() != 0)
    {
        // For a specializable program, we don't invoke any actual slang compilation yet.
        RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl(m_api, desc.pipelineType);
        initProgramCommon(shaderProgram, desc);
        *outProgram = shaderProgram.detach();
        return SLANG_OK;
    }

    if( desc.kernelCount == 0 )
    {
        return createProgramFromSlang(this, desc, outProgram);
    }

    RefPtr<ShaderProgramImpl> impl = new ShaderProgramImpl(m_api, desc.pipelineType);
    if( desc.pipelineType == PipelineType::Compute)
    {
        auto computeKernel = desc.findKernel(StageType::Compute);
        impl->m_compute = compileEntryPoint(*computeKernel, VK_SHADER_STAGE_COMPUTE_BIT, impl->m_buffers[0], impl->m_modules[0]);
    }
    else
    {
        auto vertexKernel = desc.findKernel(StageType::Vertex);
        auto fragmentKernel = desc.findKernel(StageType::Fragment);

        impl->m_vertex = compileEntryPoint(*vertexKernel, VK_SHADER_STAGE_VERTEX_BIT, impl->m_buffers[0], impl->m_modules[0]);
        impl->m_fragment = compileEntryPoint(*fragmentKernel, VK_SHADER_STAGE_FRAGMENT_BIT, impl->m_buffers[1], impl->m_modules[1]);
    }
    initProgramCommon(impl, desc);
    *outProgram = impl.detach();
    return SLANG_OK;
}

Result VKRenderer::createGraphicsPipelineState(const GraphicsPipelineStateDesc& inDesc, IPipelineState** outState)
{
    GraphicsPipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    auto programImpl = (ShaderProgramImpl*) desc.program;
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;
    auto inputLayoutImpl = (InputLayoutImpl*) desc.inputLayout;

    // Shader Stages
    //
    // Currently only handles vertex/fragment.

    static const uint32_t kMaxShaderStages = 2;
    VkPipelineShaderStageCreateInfo shaderStages[kMaxShaderStages];

    uint32_t shaderStageCount = 0;
    shaderStages[shaderStageCount++] = programImpl->m_vertex;
    shaderStages[shaderStageCount++] = programImpl->m_fragment;

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
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
    dynamicStateInfo.dynamicStateCount = 2;
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT , VK_DYNAMIC_STATE_SCISSOR};
    dynamicStateInfo.pDynamicStates = dynamicStates;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
    depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo.depthTestEnable = inDesc.depthStencil.depthTestEnable ? 1 : 0;
    depthStencilStateInfo.back = translateStencilState(inDesc.depthStencil.backFace);
    depthStencilStateInfo.front = translateStencilState(inDesc.depthStencil.frontFace);
    depthStencilStateInfo.depthBoundsTestEnable = 0;
    depthStencilStateInfo.depthCompareOp = translateComparisonFunc(inDesc.depthStencil.depthFunc);
    depthStencilStateInfo.depthWriteEnable = inDesc.depthStencil.depthWriteEnable ? 1 : 0;
    depthStencilStateInfo.stencilTestEnable = inDesc.depthStencil.stencilEnable ? 1 : 0;

    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
    pipelineInfo.layout = pipelineLayoutImpl->m_pipelineLayout;
    pipelineInfo.renderPass = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout)->m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    VkPipeline pipeline = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateGraphicsPipelines(m_device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_framebufferLayout =
        static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
    pipelineStateImpl->m_shaderProgram = programImpl;
    pipelineStateImpl->init(desc);
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}

Result VKRenderer::createComputePipelineState(const ComputePipelineStateDesc& inDesc, IPipelineState** outState)
{
    ComputePipelineStateDesc desc = inDesc;
    preparePipelineDesc(desc);

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;

    auto programImpl = (ShaderProgramImpl*) desc.program;
    auto pipelineLayoutImpl = (PipelineLayoutImpl*) desc.pipelineLayout;

    VkComputePipelineCreateInfo computePipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    computePipelineInfo.stage = programImpl->m_compute;
    computePipelineInfo.layout = pipelineLayoutImpl->m_pipelineLayout;

    VkPipeline pipeline = VK_NULL_HANDLE;
    SLANG_VK_CHECK(m_api.vkCreateComputePipelines(m_device, pipelineCache, 1, &computePipelineInfo, nullptr, &pipeline));

    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(m_api);
    pipelineStateImpl->m_pipeline = pipeline;
    pipelineStateImpl->m_pipelineLayout = pipelineLayoutImpl;
    pipelineStateImpl->m_shaderProgram = programImpl;
    pipelineStateImpl->init(desc);
    *outState = pipelineStateImpl.detach();
    return SLANG_OK;
}


#if 0
    else if (m_currentProgram->m_pipelineType == PipelineType::Graphics)
    {
        // Create the graphics pipeline

        const int width = m_swapChain.getWidth();
        const int height = m_swapChain.getHeight();





        //


    }
    else
    {
        assert(!"Unhandled program type");
        return SLANG_FAIL;
    }

    pipelineOut = pipeline;
    return SLANG_OK;


#endif

} // renderer_test
