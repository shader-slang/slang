// slang-hgvm-resource.h
#pragma once

#include "core/slang-string-util.h"
#include "slang-hgbc-format.h"

#include <slang-rhi.h>

namespace Slang::HGVM
{

/// Formats public RHI usage flags for both trace output and debugger resource summaries.
/// Unknown bits remain visible when RHI adds a flag that has not yet been named here.
struct ResourceUsageFlagName
{
    uint32_t flag;
    const char* name;
};

inline String getResourceUsageName(uint32_t remaining, ConstArrayView<ResourceUsageFlagName> flags)
{
    StringBuilder result;
    for (const auto& entry : flags)
    {
        if (!(remaining & entry.flag))
            continue;
        if (result.getLength())
            result << '|';
        result << entry.name;
        remaining &= ~entry.flag;
    }
    if (remaining)
        StringUtil::appendFormat(result, result.getLength() ? "|0x%x" : "0x%x", remaining);
    else if (!result.getLength())
        result << "None";
    return result.produceString();
}

inline String getBufferUsageName(rhi::BufferUsage usage)
{
    static const ResourceUsageFlagName flags[] = {
        {uint32_t(rhi::BufferUsage::VertexBuffer), "VertexBuffer"},
        {uint32_t(rhi::BufferUsage::IndexBuffer), "IndexBuffer"},
        {uint32_t(rhi::BufferUsage::ConstantBuffer), "ConstantBuffer"},
        {uint32_t(rhi::BufferUsage::ShaderResource), "ShaderResource"},
        {uint32_t(rhi::BufferUsage::UnorderedAccess), "UnorderedAccess"},
        {uint32_t(rhi::BufferUsage::IndirectArgument), "IndirectArgument"},
        {uint32_t(rhi::BufferUsage::CopySource), "CopySource"},
        {uint32_t(rhi::BufferUsage::CopyDestination), "CopyDestination"},
        {uint32_t(rhi::BufferUsage::AccelerationStructure), "AccelerationStructure"},
        {uint32_t(rhi::BufferUsage::AccelerationStructureBuildInput),
         "AccelerationStructureBuildInput"},
        {uint32_t(rhi::BufferUsage::ShaderTable), "ShaderTable"},
        {uint32_t(rhi::BufferUsage::Shared), "Shared"},
    };
    return getResourceUsageName(uint32_t(usage), makeConstArrayView(flags));
}

inline String getTextureUsageName(rhi::TextureUsage usage)
{
    static const ResourceUsageFlagName flags[] = {
        {uint32_t(rhi::TextureUsage::ShaderResource), "ShaderResource"},
        {uint32_t(rhi::TextureUsage::UnorderedAccess), "UnorderedAccess"},
        {uint32_t(rhi::TextureUsage::RenderTarget), "RenderTarget"},
        {uint32_t(rhi::TextureUsage::DepthStencil), "DepthStencil"},
        {uint32_t(rhi::TextureUsage::Present), "Present"},
        {uint32_t(rhi::TextureUsage::CopySource), "CopySource"},
        {uint32_t(rhi::TextureUsage::CopyDestination), "CopyDestination"},
        {uint32_t(rhi::TextureUsage::ResolveSource), "ResolveSource"},
        {uint32_t(rhi::TextureUsage::ResolveDestination), "ResolveDestination"},
        {uint32_t(rhi::TextureUsage::Typeless), "Typeless"},
        {uint32_t(rhi::TextureUsage::Shared), "Shared"},
    };
    return getResourceUsageName(uint32_t(usage), makeConstArrayView(flags));
}

/// Converts one Slang/HGBC image-format value to its RHI representation.
///
/// Consider `OpAllocateResource Texture Texture2D RGBA32Float ...`: HGVM uses this mapping instead
/// of inferring a format from the result's `RWTexture2D<float4>` type. Slang and HGBC deliberately
/// share one numeric format space; this conversion remains necessary because RHI's broader native
/// `Format` enum has different values.
inline rhi::Format getRHIResourceFormat(HGBCResourceFormat format)
{
    switch (format)
    {
    case HGBCResourceFormat::Undefined:
        return rhi::Format::Undefined;
    case HGBCResourceFormat::RGBA32Float:
        return rhi::Format::RGBA32Float;
    case HGBCResourceFormat::RGBA16Float:
        return rhi::Format::RGBA16Float;
    case HGBCResourceFormat::RG32Float:
        return rhi::Format::RG32Float;
    case HGBCResourceFormat::RG16Float:
        return rhi::Format::RG16Float;
    case HGBCResourceFormat::R11G11B10Float:
        return rhi::Format::R11G11B10Float;
    case HGBCResourceFormat::R32Float:
        return rhi::Format::R32Float;
    case HGBCResourceFormat::R16Float:
        return rhi::Format::R16Float;
    case HGBCResourceFormat::RGBA16Unorm:
        return rhi::Format::RGBA16Unorm;
    case HGBCResourceFormat::RGB10A2Unorm:
        return rhi::Format::RGB10A2Unorm;
    case HGBCResourceFormat::RGBA8Unorm:
        return rhi::Format::RGBA8Unorm;
    case HGBCResourceFormat::RG16Unorm:
        return rhi::Format::RG16Unorm;
    case HGBCResourceFormat::RG8Unorm:
        return rhi::Format::RG8Unorm;
    case HGBCResourceFormat::R16Unorm:
        return rhi::Format::R16Unorm;
    case HGBCResourceFormat::R8Unorm:
        return rhi::Format::R8Unorm;
    case HGBCResourceFormat::RGBA16Snorm:
        return rhi::Format::RGBA16Snorm;
    case HGBCResourceFormat::RGBA8Snorm:
        return rhi::Format::RGBA8Snorm;
    case HGBCResourceFormat::RG16Snorm:
        return rhi::Format::RG16Snorm;
    case HGBCResourceFormat::RG8Snorm:
        return rhi::Format::RG8Snorm;
    case HGBCResourceFormat::R16Snorm:
        return rhi::Format::R16Snorm;
    case HGBCResourceFormat::R8Snorm:
        return rhi::Format::R8Snorm;
    case HGBCResourceFormat::RGBA32Sint:
        return rhi::Format::RGBA32Sint;
    case HGBCResourceFormat::RGBA16Sint:
        return rhi::Format::RGBA16Sint;
    case HGBCResourceFormat::RGBA8Sint:
        return rhi::Format::RGBA8Sint;
    case HGBCResourceFormat::RG32Sint:
        return rhi::Format::RG32Sint;
    case HGBCResourceFormat::RG16Sint:
        return rhi::Format::RG16Sint;
    case HGBCResourceFormat::RG8Sint:
        return rhi::Format::RG8Sint;
    case HGBCResourceFormat::R32Sint:
        return rhi::Format::R32Sint;
    case HGBCResourceFormat::R16Sint:
        return rhi::Format::R16Sint;
    case HGBCResourceFormat::R8Sint:
        return rhi::Format::R8Sint;
    case HGBCResourceFormat::RGBA32Uint:
        return rhi::Format::RGBA32Uint;
    case HGBCResourceFormat::RGBA16Uint:
        return rhi::Format::RGBA16Uint;
    case HGBCResourceFormat::RGB10A2Uint:
        return rhi::Format::RGB10A2Uint;
    case HGBCResourceFormat::RGBA8Uint:
        return rhi::Format::RGBA8Uint;
    case HGBCResourceFormat::RG32Uint:
        return rhi::Format::RG32Uint;
    case HGBCResourceFormat::RG16Uint:
        return rhi::Format::RG16Uint;
    case HGBCResourceFormat::RG8Uint:
        return rhi::Format::RG8Uint;
    case HGBCResourceFormat::R32Uint:
        return rhi::Format::R32Uint;
    case HGBCResourceFormat::R16Uint:
        return rhi::Format::R16Uint;
    case HGBCResourceFormat::R8Uint:
        return rhi::Format::R8Uint;
    case HGBCResourceFormat::R64Uint:
        return rhi::Format::R64Uint;
    case HGBCResourceFormat::R64Sint:
        return rhi::Format::R64Sint;
    case HGBCResourceFormat::BGRA8Unorm:
        return rhi::Format::BGRA8Unorm;
    case HGBCResourceFormat::RGBA8UnormSrgb:
        return rhi::Format::RGBA8UnormSrgb;
    case HGBCResourceFormat::D32Float:
        return rhi::Format::D32Float;
    case HGBCResourceFormat::D16Unorm:
        return rhi::Format::D16Unorm;
    case HGBCResourceFormat::D32FloatS8Uint:
        return rhi::Format::D32FloatS8Uint;
    }
    SLANG_UNREACHABLE("validated HGBC resource format");
}

/// Reports whether a native presentation format implements one logical HGBC surface format.
///
/// Consider logical RGBA8 sRGB on Metal: the native surface exposes BGRA8 sRGB, but typed shader
/// access preserves `(r, g, b, a)`. BGRX is also valid because presentation does not preserve a
/// recoverable alpha channel.
inline bool isEquivalentSurfaceFormat(rhi::Format logicalFormat, rhi::Format nativeFormat)
{
    if (logicalFormat == nativeFormat)
        return true;
    switch (logicalFormat)
    {
    case rhi::Format::RGBA8Unorm:
        return nativeFormat == rhi::Format::BGRA8Unorm || nativeFormat == rhi::Format::BGRX8Unorm;
    case rhi::Format::RGBA8UnormSrgb:
        return nativeFormat == rhi::Format::BGRA8UnormSrgb ||
               nativeFormat == rhi::Format::BGRX8UnormSrgb;
    default:
        return false;
    }
}

/// Selects the first native surface format equivalent to one logical HGBC format.
inline rhi::Format selectSurfaceFormat(
    const rhi::SurfaceInfo& surfaceInfo,
    rhi::Format logicalFormat)
{
    for (uint32_t formatIndex = 0; formatIndex < surfaceInfo.formatCount; ++formatIndex)
    {
        if (isEquivalentSurfaceFormat(logicalFormat, surfaceInfo.formats[formatIndex]))
            return surfaceInfo.formats[formatIndex];
    }
    return rhi::Format::Undefined;
}

/// Converts an explicit HGBC memory class without examining the allocated result type.
inline rhi::MemoryType getRHIResourceMemoryType(HGBCResourceMemoryType memoryType)
{
    switch (memoryType)
    {
    case HGBCResourceMemoryType::DeviceLocal:
        return rhi::MemoryType::DeviceLocal;
    case HGBCResourceMemoryType::Upload:
        return rhi::MemoryType::Upload;
    case HGBCResourceMemoryType::ReadBack:
        return rhi::MemoryType::ReadBack;
    }
    SLANG_UNREACHABLE("validated HGBC resource memory type");
}

/// Converts the initial state carried by `OpAllocateResource` directly to RHI.
inline rhi::ResourceState getRHIResourceState(HGBCResourceState state)
{
    switch (state)
    {
    case HGBCResourceState::Undefined:
        return rhi::ResourceState::Undefined;
    case HGBCResourceState::ShaderResource:
        return rhi::ResourceState::ShaderResource;
    case HGBCResourceState::UnorderedAccess:
        return rhi::ResourceState::UnorderedAccess;
    case HGBCResourceState::CopySource:
        return rhi::ResourceState::CopySource;
    case HGBCResourceState::CopyDestination:
        return rhi::ResourceState::CopyDestination;
    case HGBCResourceState::RenderTarget:
        return rhi::ResourceState::RenderTarget;
    case HGBCResourceState::DepthWrite:
        return rhi::ResourceState::DepthWrite;
    }
    SLANG_UNREACHABLE("validated HGBC resource state");
}

/// Maps one validated buffer usage enumerant to the corresponding RHI flag.
///
/// The caller combines repeated `OpAllocateResource` usage operands. Texture-only usage values
/// never reach this function because the HGBC validator rejects that combination first.
inline rhi::BufferUsage getRHIBufferUsage(HGBCResourceUsage usage)
{
    switch (usage)
    {
    case HGBCResourceUsage::ShaderResource:
        return rhi::BufferUsage::ShaderResource;
    case HGBCResourceUsage::UnorderedAccess:
        return rhi::BufferUsage::UnorderedAccess;
    case HGBCResourceUsage::CopySource:
        return rhi::BufferUsage::CopySource;
    case HGBCResourceUsage::CopyDestination:
        return rhi::BufferUsage::CopyDestination;
    case HGBCResourceUsage::VertexBuffer:
        return rhi::BufferUsage::VertexBuffer;
    case HGBCResourceUsage::IndexBuffer:
        return rhi::BufferUsage::IndexBuffer;
    case HGBCResourceUsage::AccelerationStructureBuildInput:
        return rhi::BufferUsage::AccelerationStructureBuildInput;
    case HGBCResourceUsage::RenderTarget:
    case HGBCResourceUsage::DepthStencil:
        break;
    }
    SLANG_UNREACHABLE("validated HGBC buffer usage");
}

/// Maps one validated texture usage enumerant to the corresponding RHI flag.
inline rhi::TextureUsage getRHITextureUsage(HGBCResourceUsage usage)
{
    switch (usage)
    {
    case HGBCResourceUsage::ShaderResource:
        return rhi::TextureUsage::ShaderResource;
    case HGBCResourceUsage::UnorderedAccess:
        return rhi::TextureUsage::UnorderedAccess;
    case HGBCResourceUsage::CopySource:
        return rhi::TextureUsage::CopySource;
    case HGBCResourceUsage::CopyDestination:
        return rhi::TextureUsage::CopyDestination;
    case HGBCResourceUsage::RenderTarget:
        return rhi::TextureUsage::RenderTarget;
    case HGBCResourceUsage::DepthStencil:
        return rhi::TextureUsage::DepthStencil;
    case HGBCResourceUsage::VertexBuffer:
    case HGBCResourceUsage::IndexBuffer:
    case HGBCResourceUsage::AccelerationStructureBuildInput:
        break;
    }
    SLANG_UNREACHABLE("validated HGBC texture usage");
}

} // namespace Slang::HGVM
