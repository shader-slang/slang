// vk-util.cpp
#include "vk-util.h"
#include "core/slang-math.h"

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

        case Format::RGBA_Float16:      return VK_FORMAT_R16G16B16A16_SFLOAT;
        case Format::RG_Float16:        return VK_FORMAT_R16G16_SFLOAT;
        case Format::R_Float16:         return VK_FORMAT_R16_SFLOAT;

        case Format::RGBA_Unorm_UInt8:  return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::BGRA_Unorm_UInt8:  return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::RGBA_Snorm_UInt16: return VK_FORMAT_R16G16B16A16_SNORM;
        case Format::RG_Snorm_UInt16:   return VK_FORMAT_R16G16_SNORM;
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

VkImageLayout VulkanUtil::getImageLayoutFromState(ResourceState state)
{
    switch (state)
    {
    case ResourceState::ShaderResource:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::UnorderedAccess:
        return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::Present:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case ResourceState::CopySource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::CopyDestination:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ResourceState::RenderTarget:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthWrite:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthRead:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ResourceState::ResolveSource:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::ResolveDestination:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    default:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
    return VkImageLayout();
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

Result AccelerationStructureBuildGeometryInfoBuilder::build(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IDebugCallback* debugCallback)
{
    buildInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    switch (buildInputs.kind)
    {
    case IAccelerationStructure::Kind::BottomLevel:
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        break;
    case IAccelerationStructure::Kind::TopLevel:
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        break;
    default:
        debugCallback->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "invalid value of IAccelerationStructure::Kind encountered in buildInputs.kind");
        return SLANG_E_INVALID_ARG;
    }
    if (buildInputs.flags & IAccelerationStructure::BuildFlags::Enum::PerformUpdate)
    {
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
    }
    else
    {
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    }
    if (buildInputs.flags & IAccelerationStructure::BuildFlags::Enum::AllowCompaction)
    {
        buildInfo.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    }
    if (buildInputs.flags & IAccelerationStructure::BuildFlags::Enum::AllowUpdate)
    {
        buildInfo.flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    }
    if (buildInputs.flags & IAccelerationStructure::BuildFlags::Enum::MinimizeMemory)
    {
        buildInfo.flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;
    }
    if (buildInputs.flags & IAccelerationStructure::BuildFlags::Enum::PreferFastBuild)
    {
        buildInfo.flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    }
    if (buildInputs.flags & IAccelerationStructure::BuildFlags::Enum::PreferFastTrace)
    {
        buildInfo.flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    }
    if (buildInputs.kind == IAccelerationStructure::Kind::BottomLevel)
    {
        m_geometryInfos.setCount(buildInputs.descCount);
        primitiveCounts.setCount(buildInputs.descCount);
        memset(
            m_geometryInfos.getBuffer(),
            0,
            sizeof(VkAccelerationStructureGeometryKHR) * buildInputs.descCount);
        for (int i = 0; i < buildInputs.descCount; i++)
        {
            auto& geomDesc = buildInputs.geometryDescs[i];
            m_geometryInfos[i].sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            if (geomDesc.flags & IAccelerationStructure::GeometryFlags::NoDuplicateAnyHitInvocation)
            {
                m_geometryInfos[i].flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
            }
            else if (geomDesc.flags & IAccelerationStructure::GeometryFlags::Opaque)
            {
                m_geometryInfos[i].flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;
            }
            auto& vkGeomData = m_geometryInfos[i].geometry;
            switch (geomDesc.type)
            {
            case IAccelerationStructure::GeometryType::Triangles:
                m_geometryInfos[i].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                vkGeomData.triangles.sType =
                    VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                vkGeomData.triangles.vertexFormat =
                    VulkanUtil::getVkFormat(geomDesc.content.triangles.vertexFormat);
                vkGeomData.triangles.vertexData.deviceAddress =
                    geomDesc.content.triangles.vertexData;
                vkGeomData.triangles.vertexStride = geomDesc.content.triangles.vertexStride;
                vkGeomData.triangles.maxVertex = geomDesc.content.triangles.vertexCount - 1;
                switch (geomDesc.content.triangles.indexFormat)
                {
                case Format::R_UInt32:
                    vkGeomData.triangles.indexType = VK_INDEX_TYPE_UINT32;
                    break;
                case Format::R_UInt16:
                    vkGeomData.triangles.indexType = VK_INDEX_TYPE_UINT16;
                    break;
                case Format::Unknown:
                    vkGeomData.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
                    break;
                default:
                    debugCallback->handleMessage(
                        DebugMessageType::Error,
                        DebugMessageSource::Layer,
                        "unsupported value of Format encountered in "
                        "GeometryDesc::content.triangles.indexFormat");
                    return SLANG_E_INVALID_ARG;
                }
                vkGeomData.triangles.indexData.deviceAddress = geomDesc.content.triangles.indexData;
                vkGeomData.triangles.transformData.deviceAddress =
                    geomDesc.content.triangles.transform3x4;
                primitiveCounts[i] = Slang::Math::Max(
                                         geomDesc.content.triangles.vertexCount,
                                         geomDesc.content.triangles.indexCount) /
                                     3;
                break;
            case IAccelerationStructure::GeometryType::ProcedurePrimitives:
                m_geometryInfos[i].geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
                vkGeomData.aabbs.sType =
                    VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
                vkGeomData.aabbs.data.deviceAddress = geomDesc.content.proceduralAABBs.data;
                vkGeomData.aabbs.stride = geomDesc.content.proceduralAABBs.stride;
                primitiveCounts[i] =
                    (uint32_t)buildInputs.geometryDescs[i].content.proceduralAABBs.count;
                break;
            default:
                debugCallback->handleMessage(
                    DebugMessageType::Error,
                    DebugMessageSource::Layer,
                    "invalid value of IAccelerationStructure::GeometryType encountered in "
                    "buildInputs.geometryDescs");
                return SLANG_E_INVALID_ARG;
            }
        }
        buildInfo.geometryCount = buildInputs.descCount;
        buildInfo.pGeometries = m_geometryInfos.getBuffer();
    }
    else
    {
        m_vkInstanceInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        m_vkInstanceInfo.geometry.instances.sType =
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        m_vkInstanceInfo.geometry.instances.arrayOfPointers = 0;
        m_vkInstanceInfo.geometry.instances.data.deviceAddress = buildInputs.instanceDescs;
        buildInfo.pGeometries = &m_vkInstanceInfo;
        buildInfo.geometryCount = 1;
        primitiveCounts.setCount(1);
        primitiveCounts[0] = buildInputs.descCount;
    }
    return SLANG_OK;
}

} // namespace gfx
