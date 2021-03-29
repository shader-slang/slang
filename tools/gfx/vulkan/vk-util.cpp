// vk-util.cpp
#include "vk-util.h"

#include <stdlib.h>
#include <stdio.h>

namespace gfx {

/* static */VkFormat VulkanUtil::getVkFormat(Format format)
{
    switch (format)
    {
        case Format::RGBA_Float32:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::RGB_Float32:       return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::RG_Float32:        return VK_FORMAT_R32G32_SFLOAT;
        case Format::R_Float32:         return VK_FORMAT_R32_SFLOAT;
        case Format::RGBA_Unorm_UInt8:  return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::BGRA_Unorm_UInt8:  return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::R_UInt32:          return VK_FORMAT_R32_UINT;

        case Format::D_Float32:         return VK_FORMAT_D32_SFLOAT;
        case Format::D_Unorm24_S8:      return VK_FORMAT_D24_UNORM_S8_UINT;

        default:                        return VK_FORMAT_UNDEFINED;
    }
}

/* static */SlangResult VulkanUtil::toSlangResult(VkResult res)
{
    return (res == VK_SUCCESS) ? SLANG_OK : SLANG_FAIL;
}

VkShaderStageFlags VulkanUtil::getShaderStage(SlangStage stage)
{
    switch (stage)
    {
    case SLANG_STAGE_ANY_HIT:
        return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    case SLANG_STAGE_CALLABLE:
        return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    case SLANG_STAGE_CLOSEST_HIT:
        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    case SLANG_STAGE_COMPUTE:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    case SLANG_STAGE_DOMAIN:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case SLANG_STAGE_FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case SLANG_STAGE_GEOMETRY:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    case SLANG_STAGE_HULL:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case SLANG_STAGE_INTERSECTION:
        return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    case SLANG_STAGE_RAY_GENERATION:
        return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    case SLANG_STAGE_VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    default:
        assert(!"unsupported stage.");
        return VkShaderStageFlags(-1);
    }
}

VkPipelineBindPoint VulkanUtil::getPipelineBindPoint(PipelineType pipelineType)
{
    switch (pipelineType)
    {
    case gfx::PipelineType::Graphics:
        return VK_PIPELINE_BIND_POINT_GRAPHICS;
    case gfx::PipelineType::Compute:
        return VK_PIPELINE_BIND_POINT_COMPUTE;
    case gfx::PipelineType::RayTracing:
        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    default:
        return VkPipelineBindPoint(-1);
    }
}

/* static */Slang::Result VulkanUtil::handleFail(VkResult res)
{
    if (res != VK_SUCCESS)
    {
        assert(!"Vulkan returned a failure");
    }
    return toSlangResult(res);
}

/* static */void VulkanUtil::checkFail(VkResult res)
{
    assert(res != VK_SUCCESS);
    assert(!"Vulkan check failed");

}

/* static */VkPrimitiveTopology VulkanUtil::getVkPrimitiveTopology(PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::TriangleList:       return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default: break;
    }
    assert(!"Unknown topology");
    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkImageLayout VulkanUtil::mapResourceStateToLayout(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case ResourceState::ShaderResource:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::UnorderedAccess:
        return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::RenderTarget:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthRead:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ResourceState::DepthWrite:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ResourceState::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case ResourceState::CopySource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::CopyDestination:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ResourceState::ResolveSource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::ResolveDestination:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    default:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

} // renderer_test
