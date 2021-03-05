// vk-swap-chain.cpp
#include "vk-swap-chain.h"

#include "vk-util.h"

#include "core/slang-list.h"

#include <stdlib.h>
#include <stdio.h>

namespace gfx {
using namespace Slang;

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

SlangResult VulkanSwapChain::init(
    VulkanApi* vkapi,
    VkQueue queue,
    uint32_t queueFamilyIndex,
    const Desc& descIn,
    const PlatformDesc* platformDescIn)
{
    assert(platformDescIn);

    m_queue = queue;
    m_api = vkapi;

    // Make sure it's not set initially
    m_format = VK_FORMAT_UNDEFINED;

    Desc desc(descIn);

#if SLANG_WINDOWS_FAMILY
    const WinPlatformDesc* platformDesc = static_cast<const WinPlatformDesc*>(platformDescIn);
    _setPlatformDesc(*platformDesc);

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = platformDesc->m_hinstance;
    surfaceCreateInfo.hwnd = platformDesc->m_hwnd;

    SLANG_VK_RETURN_ON_FAIL(m_api->vkCreateWin32SurfaceKHR(m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#else
    const XPlatformDesc* platformDesc = static_cast<const XPlatformDesc*>(platformDescIn);
    _setPlatformDesc(*platformDesc);

    VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.dpy = platformDesc->m_display;
    surfaceCreateInfo.window = platformDesc->m_window;

    SLANG_VK_RETURN_ON_FAIL(m_api->vkCreateXlibSurfaceKHR(m_api->m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#endif

    VkBool32 supported = false;
    m_api->vkGetPhysicalDeviceSurfaceSupportKHR(m_api->m_physicalDevice, queueFamilyIndex, m_surface, &supported);

    uint32_t numSurfaceFormats = 0;
    List<VkSurfaceFormatKHR> surfaceFormats;
    m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(m_api->m_physicalDevice, m_surface, &numSurfaceFormats, nullptr);
    surfaceFormats.setCount(int(numSurfaceFormats));
    m_api->vkGetPhysicalDeviceSurfaceFormatsKHR(m_api->m_physicalDevice, m_surface, &numSurfaceFormats, surfaceFormats.getBuffer());

    // Look for a suitable format
    List<VkFormat> formats;
    formats.add(VulkanUtil::getVkFormat(desc.m_format));
    // HACK! To check for a different format if couldn't be found
    if (descIn.m_format == Format::RGBA_Unorm_UInt8)
    {
        formats.add(VK_FORMAT_B8G8R8A8_UNORM);
    }

    for(Index i = 0; i < formats.getCount(); ++i)
    {
        VkFormat format = formats[i];
        if (_indexOfFormat(surfaceFormats, format) >= 0)
        {
            m_format = format;
        }
    }

    if (m_format == VK_FORMAT_UNDEFINED)
    {
        return SLANG_FAIL;
    }

    // Save the desc
    m_desc = desc;

    if (descIn.m_format == Format::RGBA_Unorm_UInt8 && m_format == VK_FORMAT_B8G8R8A8_UNORM)
    {
        m_desc.m_format = Format::BGRA_Unorm_UInt8;
    }

    SLANG_RETURN_ON_FAIL(_createSwapChain());
    return SLANG_OK;
}

void VulkanSwapChain::getWindowSize(int* widthOut, int* heightOut) const
{
#if SLANG_WINDOWS_FAMILY
    auto platformDesc = _getPlatformDesc<WinPlatformDesc>();

    RECT rc;
    ::GetClientRect(platformDesc->m_hwnd, &rc);
    *widthOut = rc.right - rc.left;
    *heightOut = rc.bottom - rc.top;
#else
    auto platformDesc = _getPlatformDesc<XPlatformDesc>();

    XWindowAttributes winAttr = {};
    XGetWindowAttributes(platformDesc->m_display, platformDesc->m_window, &winAttr);

    *widthOut = winAttr.width;
    *heightOut = winAttr.height;
#endif
}

SlangResult VulkanSwapChain::_createSwapChain()
{
    int width, height;
    getWindowSize(&width, &height);

    VkExtent2D imageExtent = {};
    imageExtent.width = width;
    imageExtent.height = height;

    m_width = width;
    m_height = height;

    // catch this before throwing error
    if (m_width == 0 || m_height == 0)
    {
        return SLANG_FAIL;
    }

    // It is necessary to query the caps -> otherwise the LunarG verification layer will issue an error
    {
        VkSurfaceCapabilitiesKHR surfaceCaps;

        SLANG_VK_RETURN_ON_FAIL(m_api->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_api->m_physicalDevice, m_surface, &surfaceCaps));
    }

    List<VkPresentModeKHR> presentModes;
    uint32_t numPresentModes = 0;
    m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(m_api->m_physicalDevice, m_surface, &numPresentModes, nullptr);
    presentModes.setCount(numPresentModes);
    m_api->vkGetPhysicalDeviceSurfacePresentModesKHR(m_api->m_physicalDevice, m_surface, &numPresentModes, presentModes.getBuffer());

    {
        int numCheckPresentOptions = 3;
        VkPresentModeKHR presentOptions[] = { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
        if (m_desc.m_vsync)
        {
            presentOptions[0] = VK_PRESENT_MODE_FIFO_KHR;
            presentOptions[1] = VK_PRESENT_MODE_IMMEDIATE_KHR;
            presentOptions[2] = VK_PRESENT_MODE_MAILBOX_KHR;
        }

        m_presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;       // Invalid

        // Find the first option that's available on the device
        for (int j = 0; j < numCheckPresentOptions; j++)
        {
            if (presentModes.indexOf(presentOptions[j]) != Index(-1))
            {
                m_presentMode = presentOptions[j];
                break;
            }
        }

        if (m_presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
        {
            return SLANG_FAIL;
        }
    }

    VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainCreateInfoKHR swapchainDesc = {};
    swapchainDesc.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainDesc.surface = m_surface;
    swapchainDesc.minImageCount = m_desc.m_imageCount;
    swapchainDesc.imageFormat = m_format;
    swapchainDesc.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainDesc.imageExtent = imageExtent;
    swapchainDesc.imageArrayLayers = 1;
    swapchainDesc.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainDesc.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainDesc.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainDesc.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainDesc.presentMode = m_presentMode;
    swapchainDesc.clipped = VK_TRUE;
    swapchainDesc.oldSwapchain = oldSwapchain;

    SLANG_VK_RETURN_ON_FAIL(m_api->vkCreateSwapchainKHR(m_api->m_device, &swapchainDesc, nullptr, &m_swapChain));

    uint32_t numSwapChainImages = 0;
    m_api->vkGetSwapchainImagesKHR(m_api->m_device, m_swapChain, &numSwapChainImages, nullptr);
    m_desc.m_imageCount = numSwapChainImages;
    {
        m_images.setCount(numSwapChainImages);
        m_api->vkGetSwapchainImagesKHR(
            m_api->m_device, m_swapChain, &numSwapChainImages, m_images.getBuffer());
    }
    return SLANG_OK;
}

void VulkanSwapChain::_destroySwapChain()
{
    if (m_swapChain != VK_NULL_HANDLE)
    {
        m_api->vkDestroySwapchainKHR(m_api->m_device, m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }

    // Mark that it is no longer used
    m_images.clear();
}

void VulkanSwapChain::destroy()
{
    _destroySwapChain();

    if (m_surface)
    {
        m_api->vkDestroySurfaceKHR(m_api->m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}


VulkanSwapChain::~VulkanSwapChain()
{
    destroy();
}

int VulkanSwapChain::nextFrontImageIndex(VkSemaphore signalSemaphore)
{
    uint32_t swapChainIndex = 0;
    VkResult result = m_api->vkAcquireNextImageKHR(
        m_api->m_device, m_swapChain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &swapChainIndex);

    if (result != VK_SUCCESS)
    {
        _destroySwapChain();
        return -1;
    }
    m_currentSwapChainIndex = int(swapChainIndex);
    return swapChainIndex;
}

void VulkanSwapChain::present(VkSemaphore waitSemaphore)
{
    uint32_t swapChainIndices[] = { uint32_t(m_currentSwapChainIndex) };

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain;
    presentInfo.pImageIndices = swapChainIndices;
    if (waitSemaphore != VK_NULL_HANDLE)
    {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
    }
    m_api->vkQueuePresentKHR(m_queue, &presentInfo);
}

} // renderer_test
