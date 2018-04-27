// vk-util.h
#pragma once

#include "vk-api.h"
#include "render.h"

namespace renderer_test {

// Utility functions for Vulkan
struct VulkanUtil
{
        /// Calculate the VkFormat from the renderer format
    static VkFormat calcVkFormat(Format format);
};

} // renderer_test
