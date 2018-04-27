// vk-util.cpp
#include "vk-util.h"

#include <stdlib.h>
#include <stdio.h>

namespace renderer_test {

/* static */VkFormat VulkanUtil::calcVkFormat(Format format)
{
    switch (format)
    {
        case Format::RGBA_Float32:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::RGB_Float32:       return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::RG_Float32:        return VK_FORMAT_R32G32_SFLOAT;
        case Format::R_Float32:         return VK_FORMAT_R32_SFLOAT;
        case Format::RGBA_Unorm_UInt8:  return VK_FORMAT_R8G8B8A8_UNORM;
        default:                        return VK_FORMAT_UNDEFINED;
    }
}

} // renderer_test
