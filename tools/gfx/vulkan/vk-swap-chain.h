// vk-swap-chain.h
#pragma once

#include "vk-api.h"
#include "vk-device-queue.h"

#include "../render.h"

#include "core/slang-list.h"

namespace gfx {

struct VulkanSwapChain
{
    /* enum
    {
        kMaxImages = 8,
    }; */

        /// Base class for platform specific information
    struct PlatformDesc
    {
    };

#if SLANG_WINDOWS_FAMILY
    struct WinPlatformDesc: public PlatformDesc
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

    struct Desc
    {
        void init()
        {
            m_format = Format::Unknown;
            m_depthFormatTypeless = Format::Unknown;
            m_depthFormat = Format::Unknown;
            m_textureDepthFormat = Format::Unknown;
        }

        Format m_format;
        //bool m_enableFormat;
        Format m_depthFormatTypeless;
        Format m_depthFormat;
        Format m_textureDepthFormat;
    };

    struct Image
    {
        VkImage m_image = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkFramebuffer m_frameBuffer = VK_NULL_HANDLE;
    };


        /// Must be called before the swap chain can be used
    SlangResult init(VulkanDeviceQueue* deviceQueue, const Desc& desc, const PlatformDesc* platformDesc);

        /// Create the frame buffers (they must be compatible with the supplied renderPass)
    SlangResult createFrameBuffers(VkRenderPass renderPass);

        /// Returned the desc used to construct the swap chain.
        /// Is invalid if init hasn't returned with successful result.
    const Desc& getDesc() const { return m_desc; }

        /// True if the swap chain is available
    bool hasValidSwapChain() const { return m_images.getCount() > 0; }

        /// Present to the display
    void present(bool vsync);

        /// Get the current size of the window (in pixels written to widthOut, heightOut)
    void getWindowSize(int* widthOut, int* heightOut) const;

        /// Get the VkFormat for the back buffer
    VkFormat getVkFormat() const { return m_format; }

        /// Get width of the back buffers
    int getWidth() const { return m_width; }
        /// Get the height of the back buffer
    int getHeight() const { return m_height; }

        /// Get the detail about the images
    const Slang::List<Image>& getImages() const { return m_images; }

        /// Get the next front render image index. Returns -1, if image couldn't be found
    int nextFrontImageIndex();

    void destroy();

        /// Dtor
    ~VulkanSwapChain();

    protected:


    template <typename T>
    void _setPlatformDesc(const T& desc)
    {
        const PlatformDesc* check = &desc;
        int size = (sizeof(T) + sizeof(void*) - 1) / sizeof(void*);
        m_platformDescBuffer.setCount(size);
        *(T*)m_platformDescBuffer.getBuffer() = desc;
    }
    template <typename T>
    const T* _getPlatformDesc() const { return static_cast<const T*>((const PlatformDesc*)m_platformDescBuffer.getBuffer()); }
    SlangResult _createSwapChain();
    void _destroySwapChain();
    SlangResult _createFrameBuffers(VkRenderPass renderPass);
    void _destroyFrameBuffers();

    bool m_vsync = true;
    int m_width = 0;
    int m_height = 0;

    VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    VkFormat m_format = VK_FORMAT_UNDEFINED;                ///< The format used for backbuffer. Valid after successful init.

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;             //< Not owned

    int m_currentSwapChainIndex = 0;

    Slang::List<Image> m_images;

    VulkanDeviceQueue* m_deviceQueue = nullptr;
    const VulkanApi* m_api = nullptr;

    Desc m_desc;                                            ///< The desc used to init this swap chain
    Slang::List<void*> m_platformDescBuffer;                ///< Buffer to hold the platform specific description parameters (as passed in platformDesc)
};

} // renderer_test
