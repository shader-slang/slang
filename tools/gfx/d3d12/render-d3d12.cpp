// render-d3d12.cpp
#include "render-d3d12.h"

#ifdef GFX_NVAPI
#    include "../nvapi/nvapi-include.h"
#endif

#include "../d3d/d3d-util.h"
#include "../flag-combiner.h"
#include "../nvapi/nvapi-util.h"
#include "slang-com-ptr.h"
#include <stdio.h>

#ifdef _DEBUG
#    define ENABLE_DEBUG_LAYER 1
#else
#    define ENABLE_DEBUG_LAYER 0
#endif

namespace gfx
{
using namespace Slang;

namespace d3d12
{

namespace
{
bool isSupportedNVAPIOp(ID3D12Device* dev, uint32_t op)
{
#ifdef GFX_NVAPI
    {
        bool isSupported;
        NvAPI_Status status =
            NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(dev, NvU32(op), &isSupported);
        return status == NVAPI_OK && isSupported;
    }
#else
    return false;
#endif
}

D3D12_RESOURCE_FLAGS calcResourceFlag(ResourceState state)
{
    switch (state)
    {
    case ResourceState::RenderTarget:
        return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    case ResourceState::DepthRead:
    case ResourceState::DepthWrite:
        return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    case ResourceState::UnorderedAccess:
    case ResourceState::AccelerationStructure:
        return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    default:
        return D3D12_RESOURCE_FLAG_NONE;
    }
}

D3D12_RESOURCE_FLAGS calcResourceFlags(ResourceStateSet states)
{
    int dstFlags = 0;
    for (uint32_t i = 0; i < (uint32_t)ResourceState::_Count; i++)
    {
        auto state = (ResourceState)i;
        if (states.contains(state))
            dstFlags |= calcResourceFlag(state);
    }
    return (D3D12_RESOURCE_FLAGS)dstFlags;
}

D3D12_RESOURCE_DIMENSION calcResourceDimension(IResource::Type type)
{
    switch (type)
    {
    case IResource::Type::Buffer:
        return D3D12_RESOURCE_DIMENSION_BUFFER;
    case IResource::Type::Texture1D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    case IResource::Type::TextureCube:
    case IResource::Type::Texture2D:
        {
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        }
    case IResource::Type::Texture3D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    default:
        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}

DXGI_FORMAT getTypelessFormatFromDepthFormat(Format format)
{
    switch (format)
    {
    case Format::D16_UNORM:
        return DXGI_FORMAT_R16_TYPELESS;
    case Format::D32_FLOAT:
        return DXGI_FORMAT_R32_TYPELESS;
    default:
        return D3DUtil::getMapFormat(format);
    }
}

bool isTypelessDepthFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R32_TYPELESS:
        return true;
    default:
        return false;
    }
}

D3D12_FILTER_TYPE translateFilterMode(TextureFilteringMode mode)
{
    switch (mode)
    {
    default:
        return D3D12_FILTER_TYPE(0);

#define CASE(SRC, DST)              \
    case TextureFilteringMode::SRC: \
        return D3D12_FILTER_TYPE_##DST

        CASE(Point, POINT);
        CASE(Linear, LINEAR);

#undef CASE
    }
}

D3D12_FILTER_REDUCTION_TYPE translateFilterReduction(TextureReductionOp op)
{
    switch (op)
    {
    default:
        return D3D12_FILTER_REDUCTION_TYPE(0);

#define CASE(SRC, DST)            \
    case TextureReductionOp::SRC: \
        return D3D12_FILTER_REDUCTION_TYPE_##DST

        CASE(Average, STANDARD);
        CASE(Comparison, COMPARISON);
        CASE(Minimum, MINIMUM);
        CASE(Maximum, MAXIMUM);

#undef CASE
    }
}

D3D12_TEXTURE_ADDRESS_MODE translateAddressingMode(TextureAddressingMode mode)
{
    switch (mode)
    {
    default:
        return D3D12_TEXTURE_ADDRESS_MODE(0);

#define CASE(SRC, DST)               \
    case TextureAddressingMode::SRC: \
        return D3D12_TEXTURE_ADDRESS_MODE_##DST

        CASE(Wrap, WRAP);
        CASE(ClampToEdge, CLAMP);
        CASE(ClampToBorder, BORDER);
        CASE(MirrorRepeat, MIRROR);
        CASE(MirrorOnce, MIRROR_ONCE);

#undef CASE
    }
}

D3D12_COMPARISON_FUNC translateComparisonFunc(ComparisonFunc func)
{
    switch (func)
    {
    default:
        // TODO: need to report failures
        return D3D12_COMPARISON_FUNC_ALWAYS;

#define CASE(FROM, TO)         \
    case ComparisonFunc::FROM: \
        return D3D12_COMPARISON_FUNC_##TO

        CASE(Never, NEVER);
        CASE(Less, LESS);
        CASE(Equal, EQUAL);
        CASE(LessEqual, LESS_EQUAL);
        CASE(Greater, GREATER);
        CASE(NotEqual, NOT_EQUAL);
        CASE(GreaterEqual, GREATER_EQUAL);
        CASE(Always, ALWAYS);
#undef CASE
    }
}

uint32_t getViewDescriptorCount(const ITransientResourceHeap::Desc& desc)
{
    return Math::Max(
        Math::Max(
            desc.srvDescriptorCount,
            desc.uavDescriptorCount,
            desc.accelerationStructureDescriptorCount),
        desc.constantBufferDescriptorCount,
        2048u);
}

void initSrvDesc(
    IResource::Type resourceType,
    const ITextureResource::Desc& textureDesc,
    const D3D12_RESOURCE_DESC& desc,
    DXGI_FORMAT pixelFormat,
    SubresourceRange subresourceRange,
    D3D12_SHADER_RESOURCE_VIEW_DESC& descOut)
{
    // create SRV
    descOut = D3D12_SHADER_RESOURCE_VIEW_DESC();

    descOut.Format = (pixelFormat == DXGI_FORMAT_UNKNOWN)
                         ? D3DUtil::calcFormat(D3DUtil::USAGE_SRV, desc.Format)
                         : pixelFormat;
    descOut.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    if (desc.DepthOrArraySize == 1)
    {
        switch (desc.Dimension)
        {
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            descOut.Texture1D.MipLevels = subresourceRange.mipLevelCount == 0
                                              ? desc.MipLevels - subresourceRange.mipLevel
                                              : subresourceRange.mipLevelCount;
            descOut.Texture1D.MostDetailedMip = subresourceRange.mipLevel;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            descOut.Texture2D.PlaneSlice =
                D3DUtil::getPlaneSlice(descOut.Format, subresourceRange.aspectMask);
            descOut.Texture2D.ResourceMinLODClamp = 0.0f;
            descOut.Texture2D.MipLevels = subresourceRange.mipLevelCount == 0
                                              ? desc.MipLevels - subresourceRange.mipLevel
                                              : subresourceRange.mipLevelCount;
            descOut.Texture2D.MostDetailedMip = subresourceRange.mipLevel;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            descOut.Texture3D.MipLevels = subresourceRange.mipLevelCount == 0
                                              ? desc.MipLevels - subresourceRange.mipLevel
                                              : subresourceRange.mipLevelCount;
            descOut.Texture3D.MostDetailedMip = subresourceRange.mipLevel;
            break;
        default:
            assert(!"Unknown dimension");
        }
    }
    else if (resourceType == IResource::Type::TextureCube)
    {
        if (textureDesc.arraySize > 1)
        {
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;

            descOut.TextureCubeArray.NumCubes = subresourceRange.layerCount == 0
                                                    ? textureDesc.arraySize
                                                    : subresourceRange.layerCount / 6;
            descOut.TextureCubeArray.First2DArrayFace = subresourceRange.baseArrayLayer;
            descOut.TextureCubeArray.MipLevels = subresourceRange.mipLevelCount == 0
                                                     ? desc.MipLevels - subresourceRange.mipLevel
                                                     : subresourceRange.mipLevelCount;
            descOut.TextureCubeArray.MostDetailedMip = subresourceRange.mipLevel;
            descOut.TextureCubeArray.ResourceMinLODClamp = 0;
        }
        else
        {
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

            descOut.TextureCube.MipLevels = subresourceRange.mipLevelCount == 0
                                                ? desc.MipLevels - subresourceRange.mipLevel
                                                : subresourceRange.mipLevelCount;
            descOut.TextureCube.MostDetailedMip = subresourceRange.mipLevel;
            descOut.TextureCube.ResourceMinLODClamp = 0;
        }
    }
    else
    {
        assert(desc.DepthOrArraySize > 1);

        switch (desc.Dimension)
        {
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            descOut.Texture1D.MostDetailedMip = subresourceRange.mipLevel;
            descOut.Texture1D.MipLevels = subresourceRange.mipLevelCount == 0
                                              ? desc.MipLevels
                                              : subresourceRange.mipLevelCount;
            descOut.Texture1DArray.ArraySize = subresourceRange.layerCount == 0
                                                   ? desc.DepthOrArraySize
                                                   : subresourceRange.layerCount;
            descOut.Texture1DArray.FirstArraySlice = subresourceRange.baseArrayLayer;
            descOut.Texture1DArray.ResourceMinLODClamp = 0;
            descOut.Texture1DArray.MostDetailedMip = subresourceRange.mipLevel;
            descOut.Texture1DArray.MipLevels = subresourceRange.mipLevelCount == 0
                                                   ? desc.MipLevels - subresourceRange.mipLevel
                                                   : subresourceRange.mipLevelCount;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            descOut.Texture2DArray.ArraySize = subresourceRange.layerCount == 0
                                                   ? desc.DepthOrArraySize
                                                   : subresourceRange.layerCount;
            descOut.Texture2DArray.FirstArraySlice = subresourceRange.baseArrayLayer;
            descOut.Texture2DArray.PlaneSlice =
                D3DUtil::getPlaneSlice(descOut.Format, subresourceRange.aspectMask);
            descOut.Texture2DArray.ResourceMinLODClamp = 0;
            descOut.Texture2DArray.MostDetailedMip = subresourceRange.mipLevel;
            descOut.Texture2DArray.MipLevels = subresourceRange.mipLevelCount == 0
                                                   ? desc.MipLevels - subresourceRange.mipLevel
                                                   : subresourceRange.mipLevelCount;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            descOut.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            descOut.Texture3D.MostDetailedMip = subresourceRange.mipLevel;
            descOut.Texture3D.MipLevels = subresourceRange.mipLevelCount == 0
                                              ? desc.MipLevels
                                              : subresourceRange.mipLevelCount;
            break;

        default:
            assert(!"Unknown dimension");
        }
    }
}

Result initTextureResourceDesc(
    D3D12_RESOURCE_DESC& resourceDesc, const ITextureResource::Desc& srcDesc)
{
    const DXGI_FORMAT pixelFormat = D3DUtil::getMapFormat(srcDesc.format);
    if (pixelFormat == DXGI_FORMAT_UNKNOWN)
    {
        return SLANG_FAIL;
    }

    const int arraySize = calcEffectiveArraySize(srcDesc);

    const D3D12_RESOURCE_DIMENSION dimension = calcResourceDimension(srcDesc.type);
    if (dimension == D3D12_RESOURCE_DIMENSION_UNKNOWN)
    {
        return SLANG_FAIL;
    }

    const int numMipMaps = srcDesc.numMipLevels;
    resourceDesc.Dimension = dimension;
    resourceDesc.Format = pixelFormat;
    resourceDesc.Width = srcDesc.size.width;
    resourceDesc.Height = srcDesc.size.height;
    resourceDesc.DepthOrArraySize = (srcDesc.size.depth > 1) ? srcDesc.size.depth : arraySize;

    resourceDesc.MipLevels = numMipMaps;
    resourceDesc.SampleDesc.Count = srcDesc.sampleDesc.numSamples;
    resourceDesc.SampleDesc.Quality = srcDesc.sampleDesc.quality;

    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    resourceDesc.Flags |= calcResourceFlags(srcDesc.allowedStates);

    resourceDesc.Alignment = 0;

    if (isDepthFormat(srcDesc.format) &&
        (srcDesc.allowedStates.contains(ResourceState::ShaderResource) ||
         srcDesc.allowedStates.contains(ResourceState::UnorderedAccess)))
    {
        resourceDesc.Format = getTypelessFormatFromDepthFormat(srcDesc.format);
    }

    return SLANG_OK;
}

void initBufferResourceDesc(size_t bufferSize, D3D12_RESOURCE_DESC& out)
{
    out = {};

    out.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    out.Alignment = 0;
    out.Width = bufferSize;
    out.Height = 1;
    out.DepthOrArraySize = 1;
    out.MipLevels = 1;
    out.Format = DXGI_FORMAT_UNKNOWN;
    out.SampleDesc.Count = 1;
    out.SampleDesc.Quality = 0;
    out.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    out.Flags = D3D12_RESOURCE_FLAG_NONE;
}

Result uploadBufferDataImpl(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    TransientResourceHeapImpl* transientHeap,
    BufferResourceImpl* buffer,
    size_t offset,
    size_t size,
    void* data)
{
    IBufferResource* uploadResource;
    size_t uploadResourceOffset = 0;
    if (buffer->getDesc()->memoryType != MemoryType::Upload)
    {
        SLANG_RETURN_ON_FAIL(transientHeap->allocateStagingBuffer(
            size, uploadResource, uploadResourceOffset, MemoryType::Upload));
    }

    D3D12Resource& uploadResourceRef =
        (buffer->getDesc()->memoryType == MemoryType::Upload)
            ? buffer->m_resource
            : static_cast<BufferResourceImpl*>(uploadResource)->m_resource;

    D3D12_RANGE readRange = {};
    readRange.Begin = 0;
    readRange.End = 0;
    void* uploadData;
    SLANG_RETURN_ON_FAIL(
        uploadResourceRef.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&uploadData)));
    memcpy((uint8_t*)uploadData + uploadResourceOffset + offset, data, size);
    D3D12_RANGE writtenRange = {};
    writtenRange.Begin = uploadResourceOffset + offset;
    writtenRange.End = uploadResourceOffset + offset + size;
    uploadResourceRef.getResource()->Unmap(0, &writtenRange);

    if (buffer->getDesc()->memoryType != MemoryType::Upload)
    {
        cmdList->CopyBufferRegion(
            buffer->m_resource.getResource(),
            offset,
            uploadResourceRef.getResource(),
            uploadResourceOffset + offset,
            size);
    }

    return SLANG_OK;
}

Result createNullDescriptor(
    ID3D12Device* d3dDevice,
    D3D12_CPU_DESCRIPTOR_HANDLE destDescriptor,
    const ShaderObjectLayoutImpl::BindingRangeInfo& bindingRange)
{
    switch (bindingRange.bindingType)
    {
    case slang::BindingType::ConstantBuffer:
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = 0;
            cbvDesc.SizeInBytes = 0;
            d3dDevice->CreateConstantBufferView(&cbvDesc, destDescriptor);
        }
        break;
    case slang::BindingType::MutableRawBuffer:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
            uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            d3dDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, destDescriptor);
        }
        break;
    case slang::BindingType::MutableTypedBuffer:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            d3dDevice->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, destDescriptor);
        }
        break;
    case slang::BindingType::RawBuffer:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            d3dDevice->CreateShaderResourceView(nullptr, &srvDesc, destDescriptor);
        }
        break;
    case slang::BindingType::TypedBuffer:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            d3dDevice->CreateShaderResourceView(nullptr, &srvDesc, destDescriptor);
        }
        break;
    case slang::BindingType::Texture:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            switch (bindingRange.resourceShape)
            {
            case SLANG_TEXTURE_1D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                break;
            case SLANG_TEXTURE_1D_ARRAY:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                break;
            case SLANG_TEXTURE_2D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                break;
            case SLANG_TEXTURE_2D_ARRAY:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                break;
            case SLANG_TEXTURE_3D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                break;
            case SLANG_TEXTURE_CUBE:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                break;
            case SLANG_TEXTURE_CUBE_ARRAY:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                break;
            case SLANG_TEXTURE_2D_MULTISAMPLE:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                break;
            case SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                break;
            default:
                return SLANG_OK;
            }
            d3dDevice->CreateShaderResourceView(nullptr, &srvDesc, destDescriptor);
        }
        break;
    default:
        break;
    }
    return SLANG_OK;
}
} // namespace

Result DeviceImpl::createBuffer(
    const D3D12_RESOURCE_DESC& resourceDesc,
    const void* srcData,
    size_t srcDataSize,
    D3D12_RESOURCE_STATES finalState,
    D3D12Resource& resourceOut,
    bool isShared,
    MemoryType memoryType)
{
    const size_t bufferSize = size_t(resourceDesc.Width);

    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE;
    if (isShared)
        flags |= D3D12_HEAP_FLAG_SHARED;

    D3D12_RESOURCE_DESC desc = resourceDesc;

    D3D12_RESOURCE_STATES initialState = finalState;

    switch (memoryType)
    {
    case MemoryType::ReadBack:
        assert(!srcData);

        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        initialState |= D3D12_RESOURCE_STATE_COPY_DEST;

        break;
    case MemoryType::Upload:

        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        initialState |= D3D12_RESOURCE_STATE_GENERIC_READ;

        break;
    case MemoryType::DeviceLocal:
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        initialState = (srcData ? D3D12_RESOURCE_STATE_COPY_DEST : finalState);
        break;
    default:
        return SLANG_FAIL;
    }

    // Create the resource.
    SLANG_RETURN_ON_FAIL(
        resourceOut.initCommitted(m_device, heapProps, flags, desc, initialState, nullptr));

    if (srcData)
    {
        D3D12Resource uploadResource;

        if (memoryType == MemoryType::DeviceLocal)
        {
            // If the buffer is on the default heap, create upload buffer.
            D3D12_RESOURCE_DESC uploadDesc(resourceDesc);
            uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

            SLANG_RETURN_ON_FAIL(uploadResource.initCommitted(
                m_device,
                heapProps,
                D3D12_HEAP_FLAG_NONE,
                uploadDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr));
        }

        // Be careful not to actually copy a resource here.
        D3D12Resource& uploadResourceRef =
            (memoryType == MemoryType::DeviceLocal) ? uploadResource : resourceOut;

        // Copy data to the intermediate upload heap and then schedule a copy
        // from the upload heap to the vertex buffer.
        UINT8* dstData;
        D3D12_RANGE readRange = {}; // We do not intend to read from this resource on the CPU.

        ID3D12Resource* dxUploadResource = uploadResourceRef.getResource();

        SLANG_RETURN_ON_FAIL(
            dxUploadResource->Map(0, &readRange, reinterpret_cast<void**>(&dstData)));
        ::memcpy(dstData, srcData, srcDataSize);
        dxUploadResource->Unmap(0, nullptr);

        if (memoryType == MemoryType::DeviceLocal)
        {
            auto encodeInfo = encodeResourceCommands();
            encodeInfo.d3dCommandList->CopyBufferRegion(
                resourceOut, 0, uploadResourceRef, 0, bufferSize);
            submitResourceCommandsAndWait(encodeInfo);
        }
    }

    return SLANG_OK;
}

Result DeviceImpl::captureTextureToSurface(
    TextureResourceImpl* resourceImpl,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    auto& resource = resourceImpl->m_resource;

    const D3D12_RESOURCE_STATES initialState = D3DUtil::getResourceState(state);

    const ITextureResource::Desc& gfxDesc = *resourceImpl->getDesc();
    const D3D12_RESOURCE_DESC desc = resource.getResource()->GetDesc();

    // Don't bother supporting MSAA for right now
    if (desc.SampleDesc.Count > 1)
    {
        fprintf(stderr, "ERROR: cannot capture multi-sample texture\n");
        return SLANG_FAIL;
    }

    FormatInfo formatInfo;
    gfxGetFormatInfo(gfxDesc.format, &formatInfo);
    size_t bytesPerPixel = formatInfo.blockSizeInBytes / formatInfo.pixelsPerBlock;
    size_t rowPitch = int(desc.Width) * bytesPerPixel;
    static const size_t align = 256; // D3D requires minimum 256 byte alignment for texture data.
    rowPitch = (rowPitch + align - 1) & ~(align - 1); // Bit trick for rounding up
    size_t bufferSize = rowPitch * int(desc.Height);
    if (outRowPitch)
        *outRowPitch = rowPitch;
    if (outPixelSize)
        *outPixelSize = bytesPerPixel;

    D3D12Resource stagingResource;
    {
        D3D12_RESOURCE_DESC stagingDesc;
        initBufferResourceDesc(bufferSize, stagingDesc);

        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        SLANG_RETURN_ON_FAIL(stagingResource.initCommitted(
            m_device,
            heapProps,
            D3D12_HEAP_FLAG_NONE,
            stagingDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr));
    }

    auto encodeInfo = encodeResourceCommands();
    auto currentState = D3DUtil::getResourceState(state);

    {
        D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
        resource.transition(currentState, D3D12_RESOURCE_STATE_COPY_SOURCE, submitter);
    }

    // Do the copy
    {
        D3D12_TEXTURE_COPY_LOCATION srcLoc;
        srcLoc.pResource = resource;
        srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLoc.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION dstLoc;
        dstLoc.pResource = stagingResource;
        dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dstLoc.PlacedFootprint.Offset = 0;
        dstLoc.PlacedFootprint.Footprint.Format = desc.Format;
        dstLoc.PlacedFootprint.Footprint.Width = UINT(desc.Width);
        dstLoc.PlacedFootprint.Footprint.Height = UINT(desc.Height);
        dstLoc.PlacedFootprint.Footprint.Depth = 1;
        dstLoc.PlacedFootprint.Footprint.RowPitch = UINT(rowPitch);

        encodeInfo.d3dCommandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
    }

    {
        D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
        resource.transition(D3D12_RESOURCE_STATE_COPY_SOURCE, currentState, submitter);
    }

    // Submit the copy, and wait for copy to complete
    submitResourceCommandsAndWait(encodeInfo);

    {
        ID3D12Resource* dxResource = stagingResource;

        UINT8* data;
        D3D12_RANGE readRange = {0, bufferSize};

        SLANG_RETURN_ON_FAIL(dxResource->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        RefPtr<Slang::ListBlob> resultBlob = new Slang::ListBlob();
        resultBlob->m_data.setCount(bufferSize);
        memcpy(resultBlob->m_data.getBuffer(), data, bufferSize);
        dxResource->Unmap(0, nullptr);
        returnComPtr(outBlob, resultBlob);
        return SLANG_OK;
    }
}

Result DeviceImpl::getNativeDeviceHandles(InteropHandles* outHandles)
{
    outHandles->handles[0].handleValue = (uint64_t)m_device;
    outHandles->handles[0].api = InteropHandleAPI::D3D12;
    return SLANG_OK;
}

Result DeviceImpl::_createDevice(
    DeviceCheckFlags deviceCheckFlags,
    const UnownedStringSlice& nameMatch,
    D3D_FEATURE_LEVEL featureLevel,
    D3D12DeviceInfo& outDeviceInfo)
{
    if (m_dxDebug && (deviceCheckFlags & DeviceCheckFlag::UseDebug))
    {
        m_dxDebug->EnableDebugLayer();
    }

    outDeviceInfo.clear();

    ComPtr<IDXGIFactory> dxgiFactory;
    SLANG_RETURN_ON_FAIL(D3DUtil::createFactory(deviceCheckFlags, dxgiFactory));

    List<ComPtr<IDXGIAdapter>> dxgiAdapters;
    SLANG_RETURN_ON_FAIL(
        D3DUtil::findAdapters(deviceCheckFlags, nameMatch, dxgiFactory, dxgiAdapters));

    ComPtr<ID3D12Device> device;
    ComPtr<IDXGIAdapter> adapter;

    for (Index i = 0; i < dxgiAdapters.getCount(); ++i)
    {
        IDXGIAdapter* dxgiAdapter = dxgiAdapters[i];
        if (SLANG_SUCCEEDED(
                m_D3D12CreateDevice(dxgiAdapter, featureLevel, IID_PPV_ARGS(device.writeRef()))))
        {
            adapter = dxgiAdapter;
            break;
        }
    }

    if (!device)
    {
        return SLANG_FAIL;
    }

    if (m_dxDebug && (deviceCheckFlags & DeviceCheckFlag::UseDebug))
    {
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SLANG_SUCCEEDED(device->QueryInterface(infoQueue.writeRef())))
        {
            // Make break
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            if (m_extendedDesc.debugBreakOnD3D12Error)
            {
                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            }
            D3D12_MESSAGE_ID hideMessages[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
            };
            D3D12_INFO_QUEUE_FILTER f = {};
            f.DenyList.NumIDs = (UINT)SLANG_COUNT_OF(hideMessages);
            f.DenyList.pIDList = hideMessages;
            infoQueue->AddStorageFilterEntries(&f);

            // Apparently there is a problem with sm 6.3 with spurious errors, with debug layer
            // enabled
            D3D12_FEATURE_DATA_SHADER_MODEL featureShaderModel;
            featureShaderModel.HighestShaderModel = D3D_SHADER_MODEL(0x63);
            SLANG_SUCCEEDED(device->CheckFeatureSupport(
                D3D12_FEATURE_SHADER_MODEL, &featureShaderModel, sizeof(featureShaderModel)));

            if (featureShaderModel.HighestShaderModel >= D3D_SHADER_MODEL(0x63))
            {
                // Filter out any messages that cause issues
                // TODO: Remove this when the debug layers work properly
                D3D12_MESSAGE_ID messageIds[] = {
                    // When the debug layer is enabled this error is triggered sometimes after a
                    // CopyDescriptorsSimple call The failed check validates that the source and
                    // destination ranges of the copy do not overlap. The check assumes descriptor
                    // handles are pointers to memory, but this is not always the case and the check
                    // fails (even though everything is okay).
                    D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES,
                };

                // We filter INFO messages because they are way too many
                D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};

                D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
                infoQueueFilter.DenyList.NumSeverities = SLANG_COUNT_OF(severities);
                infoQueueFilter.DenyList.pSeverityList = severities;
                infoQueueFilter.DenyList.NumIDs = SLANG_COUNT_OF(messageIds);
                infoQueueFilter.DenyList.pIDList = messageIds;

                infoQueue->PushStorageFilter(&infoQueueFilter);
            }
        }
    }

    // Get the descs
    {
        adapter->GetDesc(&outDeviceInfo.m_desc);

        // Look up GetDesc1 info
        ComPtr<IDXGIAdapter1> adapter1;
        if (SLANG_SUCCEEDED(adapter->QueryInterface(adapter1.writeRef())))
        {
            adapter1->GetDesc1(&outDeviceInfo.m_desc1);
        }
    }

    // Save other info
    outDeviceInfo.m_device = device;
    outDeviceInfo.m_dxgiFactory = dxgiFactory;
    outDeviceInfo.m_adapter = adapter;
    outDeviceInfo.m_isWarp = D3DUtil::isWarp(dxgiFactory, adapter);
    const UINT kMicrosoftVendorId = 5140;
    outDeviceInfo.m_isSoftware =
        outDeviceInfo.m_isWarp ||
        ((outDeviceInfo.m_desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) ||
        outDeviceInfo.m_desc.VendorId == kMicrosoftVendorId;

    return SLANG_OK;
}

Result DeviceImpl::initialize(const Desc& desc)
{
    SLANG_RETURN_ON_FAIL(RendererBase::initialize(desc));

    // Find extended desc.
    for (uint32_t i = 0; i < desc.extendedDescCount; i++)
    {
        StructType stype;
        memcpy(&stype, desc.extendedDescs[i], sizeof(stype));
        if (stype == StructType::D3D12ExtendedDesc)
        {
            memcpy(&m_extendedDesc, desc.extendedDescs[i], sizeof(m_extendedDesc));
        }
    }

    // Initialize queue index allocator.
    // Support max 32 queues.
    m_queueIndexAllocator.initPool(32);

    // Initialize DeviceInfo
    {
        m_info.deviceType = DeviceType::DirectX12;
        m_info.bindingStyle = BindingStyle::DirectX;
        m_info.projectionStyle = ProjectionStyle::DirectX;
        m_info.apiName = "Direct3D 12";
        static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        ::memcpy(m_info.identityProjectionMatrix, kIdentity, sizeof(kIdentity));
    }

    // Rather than statically link against D3D, we load it dynamically.

    HMODULE d3dModule = LoadLibraryA("d3d12.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d12.dll'\n");
        return SLANG_FAIL;
    }

    // Get all the dll entry points
    m_D3D12SerializeRootSignature =
        (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)loadProc(d3dModule, "D3D12SerializeRootSignature");
    if (!m_D3D12SerializeRootSignature)
    {
        return SLANG_FAIL;
    }

    HMODULE pixModule = LoadLibraryW(L"WinPixEventRuntime.dll");
    if (pixModule)
    {
        m_BeginEventOnCommandList =
            (PFN_BeginEventOnCommandList)GetProcAddress(pixModule, "PIXBeginEventOnCommandList");
        m_EndEventOnCommandList =
            (PFN_EndEventOnCommandList)GetProcAddress(pixModule, "PIXEndEventOnCommandList");
    }

#if ENABLE_DEBUG_LAYER
    m_D3D12GetDebugInterface =
        (PFN_D3D12_GET_DEBUG_INTERFACE)loadProc(d3dModule, "D3D12GetDebugInterface");
    if (m_D3D12GetDebugInterface)
    {
        if (SLANG_SUCCEEDED(m_D3D12GetDebugInterface(IID_PPV_ARGS(m_dxDebug.writeRef()))))
        {
#    if 0
            // Can enable for extra validation. NOTE! That d3d12 warns if you do.... 
            // D3D12 MESSAGE : Device Debug Layer Startup Options : GPU - Based Validation is enabled(disabled by default).
            // This results in new validation not possible during API calls on the CPU, by creating patched shaders that have validation
            // added directly to the shader. However, it can slow things down a lot, especially for applications with numerous
            // PSOs.Time to see the first render frame may take several minutes.
            // [INITIALIZATION MESSAGE #1016: CREATEDEVICE_DEBUG_LAYER_STARTUP_OPTIONS]

            ComPtr<ID3D12Debug1> debug1;
            if (SLANG_SUCCEEDED(m_dxDebug->QueryInterface(debug1.writeRef())))
            {
                debug1->SetEnableGPUBasedValidation(true);
            }
#    endif
        }
    }
#endif

    m_D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)loadProc(d3dModule, "D3D12CreateDevice");
    if (!m_D3D12CreateDevice)
    {
        return SLANG_FAIL;
    }

    if (desc.existingDeviceHandles.handles[0].handleValue == 0)
    {
        FlagCombiner combiner;
        // TODO: we should probably provide a command-line option
        // to override UseDebug of default rather than leave it
        // up to each back-end to specify.
#if ENABLE_DEBUG_LAYER
        combiner.add(
            DeviceCheckFlag::UseDebug, ChangeType::OnOff); ///< First try debug then non debug
#else
        combiner.add(DeviceCheckFlag::UseDebug, ChangeType::Off); ///< Don't bother with debug
#endif
        combiner.add(
            DeviceCheckFlag::UseHardwareDevice,
            ChangeType::OnOff); ///< First try hardware, then reference

        const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

        const int numCombinations = combiner.getNumCombinations();
        for (int i = 0; i < numCombinations; ++i)
        {
            if (SLANG_SUCCEEDED(_createDevice(
                    combiner.getCombination(i),
                    UnownedStringSlice(desc.adapter),
                    featureLevel,
                    m_deviceInfo)))
            {
                break;
            }
        }

        if (!m_deviceInfo.m_adapter)
        {
            // Couldn't find an adapter
            return SLANG_FAIL;
        }
    }
    else
    {
        // Store the existing device handle in desc in m_deviceInfo
        m_deviceInfo.m_device = (ID3D12Device*)desc.existingDeviceHandles.handles[0].handleValue;
    }

    // Set the device
    m_device = m_deviceInfo.m_device;

    if (m_deviceInfo.m_isSoftware)
    {
        m_features.add("software-device");
    }
    else
    {
        m_features.add("hardware-device");
    }

    // NVAPI
    if (desc.nvapiExtnSlot >= 0)
    {
        if (SLANG_FAILED(NVAPIUtil::initialize()))
        {
            return SLANG_E_NOT_AVAILABLE;
        }

#ifdef GFX_NVAPI
        // From DOCS: Applications are expected to bind null UAV to this slot.
        // NOTE! We don't currently do this, but doesn't seem to be a problem.

        const NvAPI_Status status =
            NvAPI_D3D12_SetNvShaderExtnSlotSpace(m_device, NvU32(desc.nvapiExtnSlot), NvU32(0));

        if (status != NVAPI_OK)
        {
            return SLANG_E_NOT_AVAILABLE;
        }

        if (isSupportedNVAPIOp(m_device, NV_EXTN_OP_UINT64_ATOMIC))
        {
            m_features.add("atomic-int64");
        }
        if (isSupportedNVAPIOp(m_device, NV_EXTN_OP_FP32_ATOMIC))
        {
            m_features.add("atomic-float");
        }

        m_nvapi = true;
#endif
    }

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModelData = {};
    shaderModelData.HighestShaderModel = D3D_SHADER_MODEL_6_6;

    // Find what features are supported
    {
        // Check this is how this is laid out...
        SLANG_COMPILE_TIME_ASSERT(D3D_SHADER_MODEL_6_0 == 0x60);

        {
            // TODO: Currently warp causes a crash when using half, so disable for now
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                    D3D12_FEATURE_SHADER_MODEL, &shaderModelData, sizeof(shaderModelData))) &&
                m_deviceInfo.m_isWarp == false && shaderModelData.HighestShaderModel >= 0x62)
            {
                // With sm_6_2 we have half
                m_features.add("half");
            }
        }
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                    D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))))
            {
                // Check double precision support
                if (options.DoublePrecisionFloatShaderOps)
                    m_features.add("double");

                // Check conservative-rasterization support
                auto conservativeRasterTier = options.ConservativeRasterizationTier;
                if (conservativeRasterTier == D3D12_CONSERVATIVE_RASTERIZATION_TIER_3)
                {
                    m_features.add("conservative-rasterization-3");
                    m_features.add("conservative-rasterization-2");
                    m_features.add("conservative-rasterization-1");
                }
                else if (conservativeRasterTier == D3D12_CONSERVATIVE_RASTERIZATION_TIER_2)
                {
                    m_features.add("conservative-rasterization-2");
                    m_features.add("conservative-rasterization-1");
                }
                else if (conservativeRasterTier == D3D12_CONSERVATIVE_RASTERIZATION_TIER_1)
                {
                    m_features.add("conservative-rasterization-1");
                }

                // Check rasterizer ordered views support
                if (options.ROVsSupported)
                {
                    m_features.add("rasterizer-ordered-views");
                }
            }
        }
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS2 options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                    D3D12_FEATURE_D3D12_OPTIONS2, &options, sizeof(options))))
            {
                // Check programmable sample positions support
                switch (options.ProgrammableSamplePositionsTier)
                {
                case D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_2:
                    m_features.add("programmable-sample-positions-2");
                    m_features.add("programmable-sample-positions-1");
                    break;
                case D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_1:
                    m_features.add("programmable-sample-positions-1");
                    break;
                default:
                    break;
                }
            }
        }
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS3 options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                    D3D12_FEATURE_D3D12_OPTIONS3, &options, sizeof(options))))
            {
                // Check barycentrics support
                if (options.BarycentricsSupported)
                {
                    m_features.add("barycentrics");
                }
            }
        }
        // Check ray tracing support
        {
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
            if (SLANG_SUCCEEDED(m_device->CheckFeatureSupport(
                    D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options))))
            {
                if (options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
                {
                    m_features.add("ray-tracing");
                }
                if (options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1)
                {
                    m_features.add("ray-query");
                }
            }
        }
    }

    m_desc = desc;

    // Create a command queue for internal resource transfer operations.
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(m_resourceCommandQueue.writeRef()));
    // `CommandQueueImpl` holds a back reference to `D3D12Device`, make it a weak reference here
    // since this object is already owned by `D3D12Device`.
    m_resourceCommandQueue->breakStrongReferenceToDevice();
    // Retrieve timestamp frequency.
    m_resourceCommandQueue->m_d3dQueue->GetTimestampFrequency(&m_info.timestampFrequency);

    SLANG_RETURN_ON_FAIL(createTransientResourceHeapImpl(
        ITransientResourceHeap::Flags::AllowResizing,
        0,
        8,
        4,
        m_resourceCommandTransientHeap.writeRef()));
    // `TransientResourceHeap` holds a back reference to `D3D12Device`, make it a weak reference
    // here since this object is already owned by `D3D12Device`.
    m_resourceCommandTransientHeap->breakStrongReferenceToDevice();

    m_cpuViewHeap = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_cpuViewHeap->init(
        m_device,
        1024 * 1024,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
    m_cpuSamplerHeap = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_cpuSamplerHeap->init(
        m_device, 2048, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

    m_rtvAllocator = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_rtvAllocator->init(
        m_device, 16 * 1024, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));
    m_dsvAllocator = new D3D12GeneralExpandingDescriptorHeap();
    SLANG_RETURN_ON_FAIL(m_dsvAllocator->init(
        m_device, 1024, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE));

    ComPtr<IDXGIDevice> dxgiDevice;
    if (m_deviceInfo.m_adapter)
    {
        DXGI_ADAPTER_DESC adapterDesc;
        m_deviceInfo.m_adapter->GetDesc(&adapterDesc);
        m_adapterName = String::fromWString(adapterDesc.Description);
        m_info.adapterName = m_adapterName.begin();
    }

    // Initialize DXR interface.
#if SLANG_GFX_HAS_DXR_SUPPORT
    m_device->QueryInterface<ID3D12Device5>(m_deviceInfo.m_device5.writeRef());
    m_device5 = m_deviceInfo.m_device5.get();
#endif
    // Check shader model version.
    SlangCompileTarget compileTarget = SLANG_DXBC;
    const char* profileName = "sm_5_1";
    switch (shaderModelData.HighestShaderModel)
    {
    case D3D_SHADER_MODEL_5_1:
        compileTarget = SLANG_DXBC;
        profileName = "sm_5_1";
        break;
    case D3D_SHADER_MODEL_6_0:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_0";
        break;
    case D3D_SHADER_MODEL_6_1:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_1";
        break;
    case D3D_SHADER_MODEL_6_2:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_2";
        break;
    case D3D_SHADER_MODEL_6_3:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_3";
        break;
    case D3D_SHADER_MODEL_6_4:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_4";
        break;
    case D3D_SHADER_MODEL_6_5:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_5";
        break;
    default:
        compileTarget = SLANG_DXIL;
        profileName = "sm_6_6";
        break;
    }
    m_features.add(profileName);
    // If user specified a higher shader model than what the system supports, return failure.
    int userSpecifiedShaderModel = D3DUtil::getShaderModelFromProfileName(desc.slang.targetProfile);
    if (userSpecifiedShaderModel > shaderModelData.HighestShaderModel)
    {
        getDebugCallback()->handleMessage(
            gfx::DebugMessageType::Error,
            gfx::DebugMessageSource::Layer,
            "The requested shader model is not supported by the system.");
        return SLANG_E_NOT_AVAILABLE;
    }
    SLANG_RETURN_ON_FAIL(slangContext.initialize(
        desc.slang,
        compileTarget,
        profileName,
        makeArray(slang::PreprocessorMacroDesc{"__D3D12__", "1"}).getView()));

    // Allocate a D3D12 "command signature" object that matches the behavior
    // of a D3D11-style `DrawInstancedIndirect` operation.
    {
        D3D12_INDIRECT_ARGUMENT_DESC args;
        args.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

        D3D12_COMMAND_SIGNATURE_DESC desc;
        desc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
        desc.NumArgumentDescs = 1;
        desc.pArgumentDescs = &args;
        desc.NodeMask = 0;

        SLANG_RETURN_ON_FAIL(m_device->CreateCommandSignature(
            &desc, nullptr, IID_PPV_ARGS(drawIndirectCmdSignature.writeRef())));
    }

    // Allocate a D3D12 "command signature" object that matches the behavior
    // of a D3D11-style `DrawIndexedInstancedIndirect` operation.
    {
        D3D12_INDIRECT_ARGUMENT_DESC args;
        args.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

        D3D12_COMMAND_SIGNATURE_DESC desc;
        desc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
        desc.NumArgumentDescs = 1;
        desc.pArgumentDescs = &args;
        desc.NodeMask = 0;

        SLANG_RETURN_ON_FAIL(m_device->CreateCommandSignature(
            &desc, nullptr, IID_PPV_ARGS(drawIndexedIndirectCmdSignature.writeRef())));
    }

    // Allocate a D3D12 "command signature" object that matches the behavior
    // of a D3D11-style `Dispatch` operation.
    {
        D3D12_INDIRECT_ARGUMENT_DESC args;
        args.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

        D3D12_COMMAND_SIGNATURE_DESC desc;
        desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
        desc.NumArgumentDescs = 1;
        desc.pArgumentDescs = &args;
        desc.NodeMask = 0;

        SLANG_RETURN_ON_FAIL(m_device->CreateCommandSignature(
            &desc, nullptr, IID_PPV_ARGS(dispatchIndirectCmdSignature.writeRef())));
    }
    m_isInitialized = true;
    return SLANG_OK;
}

Result DeviceImpl::createTransientResourceHeap(
    const ITransientResourceHeap::Desc& desc, ITransientResourceHeap** outHeap)
{
    RefPtr<TransientResourceHeapImpl> heap;
    SLANG_RETURN_ON_FAIL(createTransientResourceHeapImpl(
        desc.flags,
        desc.constantBufferSize,
        getViewDescriptorCount(desc),
        Math::Max(1024u, desc.samplerDescriptorCount),
        heap.writeRef()));
    returnComPtr(outHeap, heap);
    return SLANG_OK;
}

Result DeviceImpl::createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue)
{
    RefPtr<CommandQueueImpl> queue;
    SLANG_RETURN_ON_FAIL(createCommandQueueImpl(queue.writeRef()));
    returnComPtr(outQueue, queue);
    return SLANG_OK;
}

Result DeviceImpl::createSwapchain(
    const ISwapchain::Desc& desc, WindowHandle window, ISwapchain** outSwapchain)
{
    RefPtr<SwapchainImpl> swapchain = new SwapchainImpl();
    SLANG_RETURN_ON_FAIL(swapchain->init(this, desc, window));
    returnComPtr(outSwapchain, swapchain);
    return SLANG_OK;
}

SlangResult DeviceImpl::readTextureResource(
    ITextureResource* resource,
    ResourceState state,
    ISlangBlob** outBlob,
    size_t* outRowPitch,
    size_t* outPixelSize)
{
    return captureTextureToSurface(
        static_cast<TextureResourceImpl*>(resource), state, outBlob, outRowPitch, outPixelSize);
}

Result DeviceImpl::getTextureAllocationInfo(
    const ITextureResource::Desc& desc, size_t* outSize, size_t* outAlignment)
{
    TextureResource::Desc srcDesc = fixupTextureDesc(desc);
    D3D12_RESOURCE_DESC resourceDesc = {};
    initTextureResourceDesc(resourceDesc, srcDesc);
    auto allocInfo = m_device->GetResourceAllocationInfo(0, 1, &resourceDesc);
    *outSize = (size_t)allocInfo.SizeInBytes;
    *outAlignment = (size_t)allocInfo.Alignment;
    return SLANG_OK;
}

Result DeviceImpl::getTextureRowAlignment(size_t* outAlignment)
{
    *outAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
    return SLANG_OK;
}

Result DeviceImpl::createTextureResource(
    const ITextureResource::Desc& descIn,
    const ITextureResource::SubresourceData* initData,
    ITextureResource** outResource)
{
    // Description of uploading on Dx12
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn899215%28v=vs.85%29.aspx

    TextureResource::Desc srcDesc = fixupTextureDesc(descIn);

    D3D12_RESOURCE_DESC resourceDesc = {};
    initTextureResourceDesc(resourceDesc, srcDesc);
    const int arraySize = calcEffectiveArraySize(srcDesc);
    const int numMipMaps = srcDesc.numMipLevels;

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(srcDesc));

    // Create the target resource
    {
        D3D12_HEAP_PROPERTIES heapProps;

        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE;
        if (descIn.isShared)
            flags |= D3D12_HEAP_FLAG_SHARED;

        D3D12_CLEAR_VALUE clearValue;
        D3D12_CLEAR_VALUE* clearValuePtr = &clearValue;
        if ((resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                                   D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
        {
            clearValuePtr = nullptr;
        }
        if (isTypelessDepthFormat(resourceDesc.Format))
        {
            clearValuePtr = nullptr;
        }
        clearValue.Format = resourceDesc.Format;
        memcpy(clearValue.Color, &descIn.optimalClearValue.color, sizeof(clearValue.Color));
        clearValue.DepthStencil.Depth = descIn.optimalClearValue.depthStencil.depth;
        clearValue.DepthStencil.Stencil = descIn.optimalClearValue.depthStencil.stencil;
        SLANG_RETURN_ON_FAIL(texture->m_resource.initCommitted(
            m_device,
            heapProps,
            flags,
            resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            clearValuePtr));

        texture->m_resource.setDebugName(L"Texture");
    }

    // Calculate the layout
    List<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
    layouts.setCount(numMipMaps);
    List<UInt64> mipRowSizeInBytes;
    mipRowSizeInBytes.setCount(srcDesc.numMipLevels);
    List<UInt32> mipNumRows;
    mipNumRows.setCount(numMipMaps);

    // NOTE! This is just the size for one array upload -> not for the whole texture
    UInt64 requiredSize = 0;
    m_device->GetCopyableFootprints(
        &resourceDesc,
        0,
        srcDesc.numMipLevels,
        0,
        layouts.begin(),
        mipNumRows.begin(),
        mipRowSizeInBytes.begin(),
        &requiredSize);

    // Sub resource indexing
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx#subresource_indexing
    if (initData)
    {
        // Create the upload texture
        D3D12Resource uploadTexture;

        {
            D3D12_HEAP_PROPERTIES heapProps;

            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CreationNodeMask = 1;
            heapProps.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC uploadResourceDesc;

            uploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            uploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            uploadResourceDesc.Width = requiredSize;
            uploadResourceDesc.Height = 1;
            uploadResourceDesc.DepthOrArraySize = 1;
            uploadResourceDesc.MipLevels = 1;
            uploadResourceDesc.SampleDesc.Count = 1;
            uploadResourceDesc.SampleDesc.Quality = 0;
            uploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            uploadResourceDesc.Alignment = 0;

            SLANG_RETURN_ON_FAIL(uploadTexture.initCommitted(
                m_device,
                heapProps,
                D3D12_HEAP_FLAG_NONE,
                uploadResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr));

            uploadTexture.setDebugName(L"TextureUpload");
        }
        // Get the pointer to the upload resource
        ID3D12Resource* uploadResource = uploadTexture;

        int subResourceIndex = 0;
        for (int arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
        {
            uint8_t* p;
            uploadResource->Map(0, nullptr, reinterpret_cast<void**>(&p));

            for (int j = 0; j < numMipMaps; ++j)
            {
                auto srcSubresource = initData[subResourceIndex + j];

                const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout = layouts[j];
                const D3D12_SUBRESOURCE_FOOTPRINT& footprint = layout.Footprint;

                TextureResource::Size mipSize = calcMipSize(srcDesc.size, j);
                if (gfxIsCompressedFormat(descIn.format))
                {
                    mipSize.width = int(D3DUtil::calcAligned(mipSize.width, 4));
                    mipSize.height = int(D3DUtil::calcAligned(mipSize.height, 4));
                }

                assert(
                    footprint.Width == mipSize.width && footprint.Height == mipSize.height &&
                    footprint.Depth == mipSize.depth);

                auto mipRowSize = mipRowSizeInBytes[j];

                const ptrdiff_t dstMipRowPitch = ptrdiff_t(footprint.RowPitch);
                const ptrdiff_t srcMipRowPitch = ptrdiff_t(srcSubresource.strideY);

                const ptrdiff_t dstMipLayerPitch = ptrdiff_t(footprint.RowPitch * footprint.Height);
                const ptrdiff_t srcMipLayerPitch = ptrdiff_t(srcSubresource.strideZ);

                // Our outer loop will copy the depth layers one at a time.
                //
                const uint8_t* srcLayer = (const uint8_t*)srcSubresource.data;
                uint8_t* dstLayer = p + layouts[j].Offset;
                for (int l = 0; l < mipSize.depth; l++)
                {
                    // Our inner loop will copy the rows one at a time.
                    //
                    const uint8_t* srcRow = srcLayer;
                    uint8_t* dstRow = dstLayer;
                    int j = gfxIsCompressedFormat(descIn.format)
                                ? 4
                                : 1; // BC compressed formats are organized into 4x4 blocks
                    for (int k = 0; k < mipSize.height; k += j)
                    {
                        ::memcpy(dstRow, srcRow, (size_t)mipRowSize);

                        srcRow += srcMipRowPitch;
                        dstRow += dstMipRowPitch;
                    }

                    srcLayer += srcMipLayerPitch;
                    dstLayer += dstMipLayerPitch;
                }

                // assert(srcRow == (const uint8_t*)(srcMip.getBuffer() + srcMip.getCount()));
            }
            uploadResource->Unmap(0, nullptr);

            auto encodeInfo = encodeResourceCommands();
            for (int mipIndex = 0; mipIndex < numMipMaps; ++mipIndex)
            {
                // https://msdn.microsoft.com/en-us/library/windows/desktop/dn903862(v=vs.85).aspx

                D3D12_TEXTURE_COPY_LOCATION src;
                src.pResource = uploadTexture;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = layouts[mipIndex];

                D3D12_TEXTURE_COPY_LOCATION dst;
                dst.pResource = texture->m_resource;
                dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dst.SubresourceIndex = subResourceIndex;
                encodeInfo.d3dCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

                subResourceIndex++;
            }

            // Block - waiting for copy to complete (so can drop upload texture)
            submitResourceCommandsAndWait(encodeInfo);
        }
    }
    {
        auto encodeInfo = encodeResourceCommands();
        {
            D3D12BarrierSubmitter submitter(encodeInfo.d3dCommandList);
            texture->m_resource.transition(
                D3D12_RESOURCE_STATE_COPY_DEST, texture->m_defaultState, submitter);
        }
        submitResourceCommandsAndWait(encodeInfo);
    }

    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result DeviceImpl::createTextureFromNativeHandle(
    InteropHandle handle, const ITextureResource::Desc& srcDesc, ITextureResource** outResource)
{
    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(srcDesc));

    if (handle.api == InteropHandleAPI::D3D12)
    {
        texture->m_resource.setResource((ID3D12Resource*)handle.handleValue);
    }
    else
    {
        return SLANG_FAIL;
    }

    returnComPtr(outResource, texture);
    return SLANG_OK;
}

Result DeviceImpl::createBufferResource(
    const IBufferResource::Desc& descIn, const void* initData, IBufferResource** outResource)
{
    BufferResource::Desc srcDesc = fixupBufferDesc(descIn);

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc));

    D3D12_RESOURCE_DESC bufferDesc;
    initBufferResourceDesc(descIn.sizeInBytes, bufferDesc);

    bufferDesc.Flags |= calcResourceFlags(srcDesc.allowedStates);

    const D3D12_RESOURCE_STATES initialState = buffer->m_defaultState;
    SLANG_RETURN_ON_FAIL(createBuffer(
        bufferDesc,
        initData,
        srcDesc.sizeInBytes,
        initialState,
        buffer->m_resource,
        descIn.isShared,
        descIn.memoryType));

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

Result DeviceImpl::createBufferFromNativeHandle(
    InteropHandle handle, const IBufferResource::Desc& srcDesc, IBufferResource** outResource)
{
    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc));

    if (handle.api == InteropHandleAPI::D3D12)
    {
        buffer->m_resource.setResource((ID3D12Resource*)handle.handleValue);
    }
    else
    {
        return SLANG_FAIL;
    }

    returnComPtr(outResource, buffer);
    return SLANG_OK;
}

Result DeviceImpl::createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler)
{
    D3D12_FILTER_REDUCTION_TYPE dxReduction = translateFilterReduction(desc.reductionOp);
    D3D12_FILTER dxFilter;
    if (desc.maxAnisotropy > 1)
    {
        dxFilter = D3D12_ENCODE_ANISOTROPIC_FILTER(dxReduction);
    }
    else
    {
        D3D12_FILTER_TYPE dxMin = translateFilterMode(desc.minFilter);
        D3D12_FILTER_TYPE dxMag = translateFilterMode(desc.magFilter);
        D3D12_FILTER_TYPE dxMip = translateFilterMode(desc.mipFilter);

        dxFilter = D3D12_ENCODE_BASIC_FILTER(dxMin, dxMag, dxMip, dxReduction);
    }

    D3D12_SAMPLER_DESC dxDesc = {};
    dxDesc.Filter = dxFilter;
    dxDesc.AddressU = translateAddressingMode(desc.addressU);
    dxDesc.AddressV = translateAddressingMode(desc.addressV);
    dxDesc.AddressW = translateAddressingMode(desc.addressW);
    dxDesc.MipLODBias = desc.mipLODBias;
    dxDesc.MaxAnisotropy = desc.maxAnisotropy;
    dxDesc.ComparisonFunc = translateComparisonFunc(desc.comparisonFunc);
    for (int ii = 0; ii < 4; ++ii)
        dxDesc.BorderColor[ii] = desc.borderColor[ii];
    dxDesc.MinLOD = desc.minLOD;
    dxDesc.MaxLOD = desc.maxLOD;

    auto& samplerHeap = m_cpuSamplerHeap;

    D3D12Descriptor cpuDescriptor;
    samplerHeap->allocate(&cpuDescriptor);
    m_device->CreateSampler(&dxDesc, cpuDescriptor.cpuHandle);

    // TODO: We really ought to have a free-list of sampler-heap
    // entries that we check before we go to the heap, and then
    // when we are done with a sampler we simply add it to the free list.
    //
    RefPtr<SamplerStateImpl> samplerImpl = new SamplerStateImpl();
    samplerImpl->m_allocator = samplerHeap;
    samplerImpl->m_descriptor = cpuDescriptor;
    returnComPtr(outSampler, samplerImpl);
    return SLANG_OK;
}

Result DeviceImpl::createTextureView(
    ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView)
{
    auto resourceImpl = (TextureResourceImpl*)texture;

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_desc = desc;
    bool isArray = resourceImpl ? resourceImpl->getDesc()->arraySize != 0 : false;
    bool isMultiSample = resourceImpl ? resourceImpl->getDesc()->sampleDesc.numSamples > 1 : false;
    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::RenderTarget:
        {
            SLANG_RETURN_ON_FAIL(m_rtvAllocator->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_rtvAllocator;
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = D3DUtil::getMapFormat(desc.format);
            isArray = desc.subresourceRange.layerCount > 1;
            switch (desc.renderTarget.shape)
            {
            case IResource::Type::Texture1D:
                rtvDesc.ViewDimension =
                    isArray ? D3D12_RTV_DIMENSION_TEXTURE1DARRAY : D3D12_RTV_DIMENSION_TEXTURE1D;
                rtvDesc.Texture1D.MipSlice = desc.subresourceRange.mipLevel;
                break;
            case IResource::Type::Texture2D:
                if (isMultiSample)
                {
                    rtvDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY
                                                    : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    rtvDesc.Texture2DMSArray.ArraySize = desc.subresourceRange.layerCount;
                    rtvDesc.Texture2DMSArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
                }
                else
                {
                    rtvDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY
                                                    : D3D12_RTV_DIMENSION_TEXTURE2D;
                    rtvDesc.Texture2DArray.MipSlice = desc.subresourceRange.mipLevel;
                    rtvDesc.Texture2DArray.PlaneSlice =
                        resourceImpl ? D3DUtil::getPlaneSlice(
                                           D3DUtil::getMapFormat(resourceImpl->getDesc()->format),
                                           desc.subresourceRange.aspectMask)
                                     : 0;
                    rtvDesc.Texture2DArray.ArraySize = desc.subresourceRange.layerCount;
                    rtvDesc.Texture2DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
                }
                break;
            case IResource::Type::Texture3D:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                rtvDesc.Texture3D.MipSlice = desc.subresourceRange.mipLevel;
                rtvDesc.Texture3D.FirstWSlice = desc.subresourceRange.baseArrayLayer;
                rtvDesc.Texture3D.WSize = desc.subresourceRange.layerCount;
                break;
            case IResource::Type::Buffer:
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
                break;
            default:
                return SLANG_FAIL;
            }
            m_device->CreateRenderTargetView(
                resourceImpl ? resourceImpl->m_resource.getResource() : nullptr,
                &rtvDesc,
                viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::DepthStencil:
        {
            SLANG_RETURN_ON_FAIL(m_dsvAllocator->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_dsvAllocator;
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = D3DUtil::getMapFormat(desc.format);
            isArray = desc.subresourceRange.layerCount > 1;
            switch (desc.renderTarget.shape)
            {
            case IResource::Type::Texture1D:
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                dsvDesc.Texture1D.MipSlice = desc.subresourceRange.mipLevel;
                break;
            case IResource::Type::Texture2D:
                if (isMultiSample)
                {
                    dsvDesc.ViewDimension = isArray ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY
                                                    : D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    dsvDesc.Texture2DMSArray.ArraySize = desc.subresourceRange.layerCount;
                    dsvDesc.Texture2DMSArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
                }
                else
                {
                    dsvDesc.ViewDimension = isArray ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY
                                                    : D3D12_DSV_DIMENSION_TEXTURE2D;
                    dsvDesc.Texture2DArray.MipSlice = desc.subresourceRange.mipLevel;
                    dsvDesc.Texture2DArray.ArraySize = desc.subresourceRange.layerCount;
                    dsvDesc.Texture2DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
                }
                break;
            default:
                return SLANG_FAIL;
            }
            m_device->CreateDepthStencilView(
                resourceImpl ? resourceImpl->m_resource.getResource() : nullptr,
                &dsvDesc,
                viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::UnorderedAccess:
        {
            // TODO: need to support the separate "counter resource" for the case
            // of append/consume buffers with attached counters.

            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;
            D3D12_UNORDERED_ACCESS_VIEW_DESC d3d12desc = {};
            auto& resourceDesc = *resourceImpl->getDesc();
            d3d12desc.Format = gfxIsTypelessFormat(texture->getDesc()->format)
                                   ? D3DUtil::getMapFormat(desc.format)
                                   : D3DUtil::getMapFormat(texture->getDesc()->format);
            switch (resourceImpl->getDesc()->type)
            {
            case IResource::Type::Texture1D:
                d3d12desc.ViewDimension = resourceDesc.arraySize == 0
                                              ? D3D12_UAV_DIMENSION_TEXTURE1D
                                              : D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                d3d12desc.Texture1D.MipSlice = desc.subresourceRange.mipLevel;
                d3d12desc.Texture1DArray.ArraySize = desc.subresourceRange.layerCount == 0
                                                         ? resourceDesc.arraySize
                                                         : desc.subresourceRange.layerCount;
                d3d12desc.Texture1DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;

                break;
            case IResource::Type::Texture2D:
                d3d12desc.ViewDimension = resourceDesc.arraySize == 0
                                              ? D3D12_UAV_DIMENSION_TEXTURE2D
                                              : D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                d3d12desc.Texture2D.MipSlice = desc.subresourceRange.mipLevel;
                d3d12desc.Texture2D.PlaneSlice =
                    D3DUtil::getPlaneSlice(d3d12desc.Format, desc.subresourceRange.aspectMask);
                d3d12desc.Texture2DArray.ArraySize = desc.subresourceRange.layerCount == 0
                                                         ? resourceDesc.arraySize
                                                         : desc.subresourceRange.layerCount;
                d3d12desc.Texture2DArray.FirstArraySlice = desc.subresourceRange.baseArrayLayer;
                break;
            case IResource::Type::Texture3D:
                d3d12desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                d3d12desc.Texture3D.MipSlice = desc.subresourceRange.mipLevel;
                d3d12desc.Texture3D.FirstWSlice = desc.subresourceRange.baseArrayLayer;
                d3d12desc.Texture3D.WSize = resourceDesc.size.depth;
                break;
            default:
                return SLANG_FAIL;
            }
            m_device->CreateUnorderedAccessView(
                resourceImpl->m_resource, nullptr, &d3d12desc, viewImpl->m_descriptor.cpuHandle);
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
            viewImpl->m_allocator = m_cpuViewHeap;

            // Need to construct the D3D12_SHADER_RESOURCE_VIEW_DESC because otherwise TextureCube
            // is not accessed appropriately (rather than just passing nullptr to
            // CreateShaderResourceView)
            const D3D12_RESOURCE_DESC resourceDesc =
                resourceImpl->m_resource.getResource()->GetDesc();
            const DXGI_FORMAT pixelFormat = desc.format == Format::Unknown
                                                ? resourceDesc.Format
                                                : D3DUtil::getMapFormat(desc.format);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
            initSrvDesc(
                resourceImpl->getType(),
                *resourceImpl->getDesc(),
                resourceDesc,
                pixelFormat,
                desc.subresourceRange,
                srvDesc);

            m_device->CreateShaderResourceView(
                resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
        }
        break;
    }

    returnComPtr(outView, viewImpl);
    return SLANG_OK;
}

Result DeviceImpl::getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates)
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT support;
    support.Format = D3DUtil::getMapFormat(format);
    SLANG_RETURN_ON_FAIL(
        m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support)));

    ResourceStateSet allowedStates;

    auto dxgi1 = support.Support1;
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_BUFFER)
        allowedStates.add(ResourceState::ConstantBuffer);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
        allowedStates.add(ResourceState::VertexBuffer);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_IA_INDEX_BUFFER)
        allowedStates.add(ResourceState::IndexBuffer);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SO_BUFFER)
        allowedStates.add(ResourceState::StreamOutput);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURE1D)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURE3D)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TEXTURECUBE)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE_COMPARISON)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_GATHER)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_SHADER_GATHER_COMPARISON)
        allowedStates.add(ResourceState::ShaderResource);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
        allowedStates.add(ResourceState::RenderTarget);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
        allowedStates.add(ResourceState::DepthWrite);
    if (dxgi1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW)
        allowedStates.add(ResourceState::UnorderedAccess);

    *outStates = allowedStates;
    return SLANG_OK;
}

Result DeviceImpl::createBufferView(
    IBufferResource* buffer,
    IBufferResource* counterBuffer,
    IResourceView::Desc const& desc,
    IResourceView** outView)
{
    auto resourceImpl = (BufferResourceImpl*)buffer;
    auto resourceDesc = *resourceImpl->getDesc();

    RefPtr<ResourceViewImpl> viewImpl = new ResourceViewImpl();
    viewImpl->m_resource = resourceImpl;
    viewImpl->m_desc = desc;

    switch (desc.type)
    {
    default:
        return SLANG_FAIL;

    case IResourceView::Type::UnorderedAccess:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Format = D3DUtil::getMapFormat(desc.format);
            uavDesc.Buffer.FirstElement = desc.bufferRange.firstElement;
            uint64_t viewSize = 0;
            if (desc.bufferElementSize)
            {
                uavDesc.Buffer.StructureByteStride = desc.bufferElementSize;
                uavDesc.Buffer.NumElements =
                    desc.bufferRange.elementCount == 0
                        ? UINT(resourceDesc.sizeInBytes / desc.bufferElementSize)
                        : (UINT)desc.bufferRange.elementCount;
                viewSize = (uint64_t)desc.bufferElementSize * uavDesc.Buffer.NumElements;
            }
            else if (desc.format == Format::Unknown)
            {
                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.NumElements = desc.bufferRange.elementCount == 0
                                                 ? UINT(resourceDesc.sizeInBytes / 4)
                                                 : UINT(desc.bufferRange.elementCount / 4);
                uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
                viewSize = 4ull * uavDesc.Buffer.NumElements;
            }
            else
            {
                FormatInfo sizeInfo;
                gfxGetFormatInfo(desc.format, &sizeInfo);
                assert(sizeInfo.pixelsPerBlock == 1);
                uavDesc.Buffer.NumElements =
                    desc.bufferRange.elementCount == 0
                        ? UINT(resourceDesc.sizeInBytes / sizeInfo.blockSizeInBytes)
                        : (UINT)desc.bufferRange.elementCount;
                viewSize = (uint64_t)uavDesc.Buffer.NumElements * sizeInfo.blockSizeInBytes;
            }

            if (viewSize >= (1ull << 32) - 8)
            {
                // D3D12 does not support view descriptors that has size near 4GB.
                // We will not create actual SRV/UAVs for such large buffers.
                // However, a buffer this large can still be bound as root parameter.
                // So instead of failing, we quietly ignore descriptor creation.
                viewImpl->m_descriptor.cpuHandle.ptr = 0;
            }
            else
            {
                auto counterResourceImpl = static_cast<BufferResourceImpl*>(counterBuffer);
                SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
                viewImpl->m_allocator = m_cpuViewHeap;
                m_device->CreateUnorderedAccessView(
                    resourceImpl->m_resource,
                    counterResourceImpl ? counterResourceImpl->m_resource.getResource() : nullptr,
                    &uavDesc,
                    viewImpl->m_descriptor.cpuHandle);
            }
        }
        break;

    case IResourceView::Type::ShaderResource:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Format = D3DUtil::getMapFormat(desc.format);
            srvDesc.Buffer.StructureByteStride = 0;
            srvDesc.Buffer.FirstElement = desc.bufferRange.firstElement;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            uint64_t viewSize = 0;
            if (desc.bufferElementSize)
            {
                srvDesc.Buffer.StructureByteStride = desc.bufferElementSize;
                srvDesc.Buffer.NumElements =
                    desc.bufferRange.elementCount == 0
                        ? UINT(resourceDesc.sizeInBytes / desc.bufferElementSize)
                        : (UINT)desc.bufferRange.elementCount;
                viewSize = (uint64_t)desc.bufferElementSize * srvDesc.Buffer.NumElements;
            }
            else if (desc.format == Format::Unknown)
            {
                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.Buffer.NumElements = desc.bufferRange.elementCount == 0
                                                 ? UINT(resourceDesc.sizeInBytes / 4)
                                                 : UINT(desc.bufferRange.elementCount / 4);
                srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
                viewSize = 4ull * srvDesc.Buffer.NumElements;
            }
            else
            {
                FormatInfo sizeInfo;
                gfxGetFormatInfo(desc.format, &sizeInfo);
                assert(sizeInfo.pixelsPerBlock == 1);
                srvDesc.Buffer.NumElements =
                    desc.bufferRange.elementCount == 0
                        ? UINT(resourceDesc.sizeInBytes / sizeInfo.blockSizeInBytes)
                        : (UINT)desc.bufferRange.elementCount;
                viewSize = (uint64_t)srvDesc.Buffer.NumElements * sizeInfo.blockSizeInBytes;
            }
            if (viewSize >= (1ull << 32) - 8)
            {
                // D3D12 does not support view descriptors that has size near 4GB.
                // We will not create actual SRV/UAVs for such large buffers.
                // However, a buffer this large can still be bound as root parameter.
                // So instead of failing, we quietly ignore descriptor creation.
                viewImpl->m_descriptor.cpuHandle.ptr = 0;
            }
            else
            {
                SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&viewImpl->m_descriptor));
                viewImpl->m_allocator = m_cpuViewHeap;
                m_device->CreateShaderResourceView(
                    resourceImpl->m_resource, &srvDesc, viewImpl->m_descriptor.cpuHandle);
            }
        }
        break;
    }

    returnComPtr(outView, viewImpl);
    return SLANG_OK;
}

Result DeviceImpl::createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFb)
{
    RefPtr<FramebufferImpl> framebuffer = new FramebufferImpl();
    framebuffer->renderTargetViews.setCount(desc.renderTargetCount);
    framebuffer->renderTargetDescriptors.setCount(desc.renderTargetCount);
    framebuffer->renderTargetClearValues.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        framebuffer->renderTargetViews[i] =
            static_cast<ResourceViewImpl*>(desc.renderTargetViews[i]);
        framebuffer->renderTargetDescriptors[i] =
            framebuffer->renderTargetViews[i]->m_descriptor.cpuHandle;
        if (static_cast<ResourceViewImpl*>(desc.renderTargetViews[i])->m_resource.Ptr())
        {
            auto clearValue =
                static_cast<TextureResourceImpl*>(
                    static_cast<ResourceViewImpl*>(desc.renderTargetViews[i])->m_resource.Ptr())
                    ->getDesc()
                    ->optimalClearValue.color;
            memcpy(&framebuffer->renderTargetClearValues[i], &clearValue, sizeof(ColorClearValue));
        }
        else
        {
            memset(&framebuffer->renderTargetClearValues[i], 0, sizeof(ColorClearValue));
        }
    }
    framebuffer->depthStencilView = static_cast<ResourceViewImpl*>(desc.depthStencilView);
    if (desc.depthStencilView)
    {
        framebuffer->depthStencilClearValue =
            static_cast<TextureResourceImpl*>(
                static_cast<ResourceViewImpl*>(desc.depthStencilView)->m_resource.Ptr())
                ->getDesc()
                ->optimalClearValue.depthStencil;
        framebuffer->depthStencilDescriptor =
            static_cast<ResourceViewImpl*>(desc.depthStencilView)->m_descriptor.cpuHandle;
    }
    else
    {
        framebuffer->depthStencilDescriptor.ptr = 0;
    }
    returnComPtr(outFb, framebuffer);
    return SLANG_OK;
}

Result DeviceImpl::createFramebufferLayout(
    IFramebufferLayout::Desc const& desc, IFramebufferLayout** outLayout)
{
    RefPtr<FramebufferLayoutImpl> layout = new FramebufferLayoutImpl();
    layout->m_renderTargets.setCount(desc.renderTargetCount);
    for (uint32_t i = 0; i < desc.renderTargetCount; i++)
    {
        layout->m_renderTargets[i] = desc.renderTargets[i];
    }

    if (desc.depthStencil)
    {
        layout->m_hasDepthStencil = true;
        layout->m_depthStencil = *desc.depthStencil;
    }
    else
    {
        layout->m_hasDepthStencil = false;
    }
    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createRenderPassLayout(
    const IRenderPassLayout::Desc& desc, IRenderPassLayout** outRenderPassLayout)
{
    RefPtr<RenderPassLayoutImpl> result = new RenderPassLayoutImpl();
    result->init(desc);
    returnComPtr(outRenderPassLayout, result);
    return SLANG_OK;
}

Result DeviceImpl::createInputLayout(IInputLayout::Desc const& desc, IInputLayout** outLayout)
{
    RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

    // Work out a buffer size to hold all text
    size_t textSize = 0;
    auto inputElementCount = desc.inputElementCount;
    auto inputElements = desc.inputElements;
    auto vertexStreamCount = desc.vertexStreamCount;
    auto vertexStreams = desc.vertexStreams;
    for (int i = 0; i < Int(inputElementCount); ++i)
    {
        const char* text = inputElements[i].semanticName;
        textSize += text ? (::strlen(text) + 1) : 0;
    }
    layout->m_text.setCount(textSize);
    char* textPos = layout->m_text.getBuffer();

    List<D3D12_INPUT_ELEMENT_DESC>& elements = layout->m_elements;
    elements.setCount(inputElementCount);

    for (Int i = 0; i < inputElementCount; ++i)
    {
        const InputElementDesc& srcEle = inputElements[i];
        const auto& srcStream = vertexStreams[srcEle.bufferSlotIndex];
        D3D12_INPUT_ELEMENT_DESC& dstEle = elements[i];

        // Add text to the buffer
        const char* semanticName = srcEle.semanticName;
        if (semanticName)
        {
            const int len = int(::strlen(semanticName));
            ::memcpy(textPos, semanticName, len + 1);
            semanticName = textPos;
            textPos += len + 1;
        }

        dstEle.SemanticName = semanticName;
        dstEle.SemanticIndex = (UINT)srcEle.semanticIndex;
        dstEle.Format = D3DUtil::getMapFormat(srcEle.format);
        dstEle.InputSlot = (UINT)srcEle.bufferSlotIndex;
        dstEle.AlignedByteOffset = (UINT)srcEle.offset;
        dstEle.InputSlotClass = D3DUtil::getInputSlotClass(srcStream.slotClass);
        dstEle.InstanceDataStepRate = (UINT)srcStream.instanceDataStepRate;
    }

    auto& vertexStreamStrides = layout->m_vertexStreamStrides;
    vertexStreamStrides.setCount(vertexStreamCount);
    for (Int i = 0; i < vertexStreamCount; ++i)
    {
        vertexStreamStrides[i] = vertexStreams[i].stride;
    }

    returnComPtr(outLayout, layout);
    return SLANG_OK;
}

const gfx::DeviceInfo& DeviceImpl::getDeviceInfo() const { return m_info; }

Result DeviceImpl::readBufferResource(
    IBufferResource* bufferIn, size_t offset, size_t size, ISlangBlob** outBlob)
{

    BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(bufferIn);

    const size_t bufferSize = buffer->getDesc()->sizeInBytes;

    // This will be slow!!! - it blocks CPU on GPU completion
    D3D12Resource& resource = buffer->m_resource;

    D3D12Resource stageBuf;
    if (buffer->getDesc()->memoryType != MemoryType::ReadBack)
    {
        auto encodeInfo = encodeResourceCommands();

        // Readback heap
        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        // Resource to readback to
        D3D12_RESOURCE_DESC stagingDesc;
        initBufferResourceDesc(size, stagingDesc);

        SLANG_RETURN_ON_FAIL(stageBuf.initCommitted(
            m_device,
            heapProps,
            D3D12_HEAP_FLAG_NONE,
            stagingDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr));

        // Do the copy
        encodeInfo.d3dCommandList->CopyBufferRegion(stageBuf, 0, resource, offset, size);

        // Wait until complete
        submitResourceCommandsAndWait(encodeInfo);
    }

    D3D12Resource& stageBufRef =
        buffer->getDesc()->memoryType != MemoryType::ReadBack ? stageBuf : resource;

    // Map and copy
    RefPtr<ListBlob> blob = new ListBlob();
    {
        UINT8* data;
        D3D12_RANGE readRange = {0, size};

        SLANG_RETURN_ON_FAIL(
            stageBufRef.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&data)));

        // Copy to memory buffer
        blob->m_data.setCount(size);
        ::memcpy(blob->m_data.getBuffer(), data, size);

        stageBufRef.getResource()->Unmap(0, nullptr);
    }
    returnComPtr(outBlob, blob);
    return SLANG_OK;
}

Result DeviceImpl::createProgram(
    const IShaderProgram::Desc& desc, IShaderProgram** outProgram, ISlangBlob** outDiagnosticBlob)
{
    RefPtr<ShaderProgramImpl> shaderProgram = new ShaderProgramImpl();
    shaderProgram->init(desc);
    ComPtr<ID3DBlob> d3dDiagnosticBlob;
    auto rootShaderLayoutResult = RootShaderObjectLayoutImpl::create(
        this,
        shaderProgram->linkedProgram,
        shaderProgram->linkedProgram->getLayout(),
        shaderProgram->m_rootObjectLayout.writeRef(),
        d3dDiagnosticBlob.writeRef());
    if (!SLANG_SUCCEEDED(rootShaderLayoutResult))
    {
        if (outDiagnosticBlob && d3dDiagnosticBlob)
        {
            RefPtr<StringBlob> diagnosticBlob =
                new StringBlob(String((const char*)d3dDiagnosticBlob->GetBufferPointer()));
            returnComPtr(outDiagnosticBlob, diagnosticBlob);
        }
        return rootShaderLayoutResult;
    }
    returnComPtr(outProgram, shaderProgram);
    return SLANG_OK;
}

Result DeviceImpl::createShaderObjectLayout(
    slang::TypeLayoutReflection* typeLayout, ShaderObjectLayoutBase** outLayout)
{
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(
        ShaderObjectLayoutImpl::createForElementType(this, typeLayout, layout.writeRef()));
    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result DeviceImpl::createShaderObject(ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    RefPtr<ShaderObjectImpl> shaderObject;
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::create(
        this, reinterpret_cast<ShaderObjectLayoutImpl*>(layout), shaderObject.writeRef()));
    returnComPtr(outObject, shaderObject);
    return SLANG_OK;
}

Result DeviceImpl::createMutableShaderObject(
    ShaderObjectLayoutBase* layout, IShaderObject** outObject)
{
    auto result = createShaderObject(layout, outObject);
    SLANG_RETURN_ON_FAIL(result);
    static_cast<ShaderObjectImpl*>(*outObject)->m_isMutable = true;
    return result;
}

Result DeviceImpl::createMutableRootShaderObject(IShaderProgram* program, IShaderObject** outObject)
{
    RefPtr<MutableRootShaderObjectImpl> result = new MutableRootShaderObjectImpl();
    result->init(this);
    auto programImpl = static_cast<ShaderProgramImpl*>(program);
    result->resetImpl(
        this, programImpl->m_rootObjectLayout, m_cpuViewHeap.Ptr(), m_cpuSamplerHeap.Ptr(), true);
    returnComPtr(outObject, result);
    return SLANG_OK;
}

Result DeviceImpl::createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outShaderTable)
{
    RefPtr<ShaderTableImpl> result = new ShaderTableImpl();
    result->m_device = this;
    result->init(desc);
    returnComPtr(outShaderTable, result);
    return SLANG_OK;
}

Result DeviceImpl::createGraphicsPipelineState(
    const GraphicsPipelineStateDesc& desc, IPipelineState** outState)
{
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result DeviceImpl::createComputePipelineState(
    const ComputePipelineStateDesc& desc, IPipelineState** outState)
{
    RefPtr<PipelineStateImpl> pipelineStateImpl = new PipelineStateImpl(this);
    pipelineStateImpl->init(desc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

DeviceImpl::ResourceCommandRecordInfo DeviceImpl::encodeResourceCommands()
{
    ResourceCommandRecordInfo info;
    m_resourceCommandTransientHeap->createCommandBuffer(info.commandBuffer.writeRef());
    info.d3dCommandList = static_cast<CommandBufferImpl*>(info.commandBuffer.get())->m_cmdList;
    return info;
}

void DeviceImpl::submitResourceCommandsAndWait(const DeviceImpl::ResourceCommandRecordInfo& info)
{
    info.commandBuffer->close();
    m_resourceCommandQueue->executeCommandBuffer(info.commandBuffer);
    m_resourceCommandTransientHeap->finish();
    m_resourceCommandTransientHeap->synchronizeAndReset();
}

Result DeviceImpl::createQueryPool(const IQueryPool::Desc& desc, IQueryPool** outState)
{
    switch (desc.type)
    {
    case QueryType::AccelerationStructureCompactedSize:
    case QueryType::AccelerationStructureSerializedSize:
    case QueryType::AccelerationStructureCurrentSize:
        {
            RefPtr<PlainBufferProxyQueryPoolImpl> queryPoolImpl =
                new PlainBufferProxyQueryPoolImpl();
            uint32_t stride = 8;
            if (desc.type == QueryType::AccelerationStructureSerializedSize)
                stride = 16;
            SLANG_RETURN_ON_FAIL(queryPoolImpl->init(desc, this, stride));
            returnComPtr(outState, queryPoolImpl);
            return SLANG_OK;
        }
    default:
        {
            RefPtr<QueryPoolImpl> queryPoolImpl = new QueryPoolImpl();
            SLANG_RETURN_ON_FAIL(queryPoolImpl->init(desc, this));
            returnComPtr(outState, queryPoolImpl);
            return SLANG_OK;
        }
    }
}

Result DeviceImpl::createFence(const IFence::Desc& desc, IFence** outFence)
{
    RefPtr<FenceImpl> fence = new FenceImpl();
    SLANG_RETURN_ON_FAIL(fence->init(this, desc));
    returnComPtr(outFence, fence);
    return SLANG_OK;
}

Result DeviceImpl::waitForFences(
    uint32_t fenceCount, IFence** fences, uint64_t* fenceValues, bool waitForAll, uint64_t timeout)
{
    ShortList<HANDLE> waitHandles;
    for (uint32_t i = 0; i < fenceCount; ++i)
    {
        auto fenceImpl = static_cast<FenceImpl*>(fences[i]);
        waitHandles.add(fenceImpl->getWaitEvent());
        SLANG_RETURN_ON_FAIL(
            fenceImpl->m_fence->SetEventOnCompletion(fenceValues[i], fenceImpl->getWaitEvent()));
    }
    auto result = WaitForMultipleObjects(
        fenceCount,
        waitHandles.getArrayView().getBuffer(),
        waitForAll ? TRUE : FALSE,
        timeout == kTimeoutInfinite ? INFINITE : (DWORD)(timeout / 1000000));
    if (result == WAIT_TIMEOUT)
        return SLANG_E_TIME_OUT;
    return result == WAIT_FAILED ? SLANG_FAIL : SLANG_OK;
}

Result DeviceImpl::getAccelerationStructurePrebuildInfo(
    const IAccelerationStructure::BuildInputs& buildInputs,
    IAccelerationStructure::PrebuildInfo* outPrebuildInfo)
{
    if (!m_device5)
        return SLANG_E_NOT_AVAILABLE;

    D3DAccelerationStructureInputsBuilder inputsBuilder;
    SLANG_RETURN_ON_FAIL(inputsBuilder.build(buildInputs, getDebugCallback()));

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo;
    m_device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputsBuilder.desc, &prebuildInfo);

    outPrebuildInfo->resultDataMaxSize = prebuildInfo.ResultDataMaxSizeInBytes;
    outPrebuildInfo->scratchDataSize = prebuildInfo.ScratchDataSizeInBytes;
    outPrebuildInfo->updateScratchDataSize = prebuildInfo.UpdateScratchDataSizeInBytes;
    return SLANG_OK;
}

Result DeviceImpl::createAccelerationStructure(
    const IAccelerationStructure::CreateDesc& desc, IAccelerationStructure** outAS)
{
#if SLANG_GFX_HAS_DXR_SUPPORT
    RefPtr<AccelerationStructureImpl> result = new AccelerationStructureImpl();
    result->m_device5 = m_device5;
    result->m_buffer = static_cast<BufferResourceImpl*>(desc.buffer);
    result->m_size = desc.size;
    result->m_offset = desc.offset;
    result->m_allocator = m_cpuViewHeap;
    result->m_desc.type = IResourceView::Type::AccelerationStructure;
    SLANG_RETURN_ON_FAIL(m_cpuViewHeap->allocate(&result->m_descriptor));
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.RaytracingAccelerationStructure.Location =
        result->m_buffer->getDeviceAddress() + desc.offset;
    m_device->CreateShaderResourceView(nullptr, &srvDesc, result->m_descriptor.cpuHandle);
    returnComPtr(outAS, result);
    return SLANG_OK;
#else
    *outAS = nullptr;
    return SLANG_FAIL;
#endif
}

Result DeviceImpl::createRayTracingPipelineState(
    const RayTracingPipelineStateDesc& inDesc, IPipelineState** outState)
{
    if (!m_device5)
    {
        return SLANG_E_NOT_AVAILABLE;
    }

    RefPtr<RayTracingPipelineStateImpl> pipelineStateImpl = new RayTracingPipelineStateImpl(this);
    pipelineStateImpl->init(inDesc);
    returnComPtr(outState, pipelineStateImpl);
    return SLANG_OK;
}

Result DeviceImpl::createTransientResourceHeapImpl(
    ITransientResourceHeap::Flags::Enum flags,
    size_t constantBufferSize,
    uint32_t viewDescriptors,
    uint32_t samplerDescriptors,
    TransientResourceHeapImpl** outHeap)
{
    RefPtr<TransientResourceHeapImpl> result = new TransientResourceHeapImpl();
    ITransientResourceHeap::Desc desc = {};
    desc.flags = flags;
    desc.samplerDescriptorCount = samplerDescriptors;
    desc.constantBufferSize = constantBufferSize;
    desc.constantBufferDescriptorCount = viewDescriptors;
    desc.accelerationStructureDescriptorCount = viewDescriptors;
    desc.srvDescriptorCount = viewDescriptors;
    desc.uavDescriptorCount = viewDescriptors;
    SLANG_RETURN_ON_FAIL(result->init(desc, this, viewDescriptors, samplerDescriptors));
    returnRefPtrMove(outHeap, result);
    return SLANG_OK;
}

Result DeviceImpl::createCommandQueueImpl(CommandQueueImpl** outQueue)
{
    int queueIndex = m_queueIndexAllocator.alloc(1);
    // If we run out of queue index space, then the user is requesting too many queues.
    if (queueIndex == -1)
        return SLANG_FAIL;

    RefPtr<CommandQueueImpl> queue = new CommandQueueImpl();
    SLANG_RETURN_ON_FAIL(queue->init(this, (uint32_t)queueIndex));
    returnRefPtrMove(outQueue, queue);
    return SLANG_OK;
}

PROC DeviceImpl::loadProc(HMODULE module, char const* name)
{
    PROC proc = ::GetProcAddress(module, name);
    if (!proc)
    {
        fprintf(stderr, "error: failed load symbol '%s'\n", name);
        return nullptr;
    }
    return proc;
}

DeviceImpl::~DeviceImpl() { m_shaderObjectLayoutCache = decltype(m_shaderObjectLayoutCache)(); }

struct GraphicsSubmitter : public Submitter
{
    virtual void setRootConstantBufferView(
        int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
    {
        m_commandList->SetGraphicsRootConstantBufferView(index, gpuBufferLocation);
    }
    virtual void setRootUAV(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
    {
        m_commandList->SetGraphicsRootUnorderedAccessView(index, gpuBufferLocation);
    }
    virtual void setRootSRV(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
    {
        m_commandList->SetGraphicsRootShaderResourceView(index, gpuBufferLocation);
    }
    virtual void setRootDescriptorTable(
        int index, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor) override
    {
        m_commandList->SetGraphicsRootDescriptorTable(index, baseDescriptor);
    }
    void setRootSignature(ID3D12RootSignature* rootSignature)
    {
        m_commandList->SetGraphicsRootSignature(rootSignature);
    }
    void setRootConstants(
        Index rootParamIndex,
        Index dstOffsetIn32BitValues,
        Index countOf32BitValues,
        void const* srcData) override
    {
        m_commandList->SetGraphicsRoot32BitConstants(
            UINT(rootParamIndex), UINT(countOf32BitValues), srcData, UINT(dstOffsetIn32BitValues));
    }
    virtual void setPipelineState(PipelineStateBase* pipeline) override
    {
        auto pipelineImpl = static_cast<PipelineStateImpl*>(pipeline);
        m_commandList->SetPipelineState(pipelineImpl->m_pipelineState.get());
    }

    GraphicsSubmitter(ID3D12GraphicsCommandList* commandList)
        : m_commandList(commandList)
    {}

    ID3D12GraphicsCommandList* m_commandList;
};

struct ComputeSubmitter : public Submitter
{
    virtual void setRootConstantBufferView(
        int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
    {
        m_commandList->SetComputeRootConstantBufferView(index, gpuBufferLocation);
    }
    virtual void setRootUAV(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
    {
        m_commandList->SetComputeRootUnorderedAccessView(index, gpuBufferLocation);
    }
    virtual void setRootSRV(int index, D3D12_GPU_VIRTUAL_ADDRESS gpuBufferLocation) override
    {
        m_commandList->SetComputeRootShaderResourceView(index, gpuBufferLocation);
    }
    virtual void setRootDescriptorTable(
        int index, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor) override
    {
        m_commandList->SetComputeRootDescriptorTable(index, baseDescriptor);
    }
    void setRootSignature(ID3D12RootSignature* rootSignature)
    {
        m_commandList->SetComputeRootSignature(rootSignature);
    }
    void setRootConstants(
        Index rootParamIndex,
        Index dstOffsetIn32BitValues,
        Index countOf32BitValues,
        void const* srcData) override
    {
        m_commandList->SetComputeRoot32BitConstants(
            UINT(rootParamIndex), UINT(countOf32BitValues), srcData, UINT(dstOffsetIn32BitValues));
    }
    virtual void setPipelineState(PipelineStateBase* pipeline) override
    {
        auto pipelineImpl = static_cast<PipelineStateImpl*>(pipeline);
        m_commandList->SetPipelineState(pipelineImpl->m_pipelineState.get());
    }
    ComputeSubmitter(ID3D12GraphicsCommandList* commandList)
        : m_commandList(commandList)
    {}

    ID3D12GraphicsCommandList* m_commandList;
};

BufferResourceImpl::BufferResourceImpl(const Desc& desc)
    : Parent(desc)
    , m_defaultState(D3DUtil::getResourceState(desc.defaultState))
{}

BufferResourceImpl::~BufferResourceImpl()
{
    if (sharedHandle.handleValue != 0)
    {
        CloseHandle((HANDLE)sharedHandle.handleValue);
    }
}

DeviceAddress BufferResourceImpl::getDeviceAddress()
{
    return (DeviceAddress)m_resource.getResource()->GetGPUVirtualAddress();
}

Result BufferResourceImpl::getNativeResourceHandle(InteropHandle* outHandle)
{
    outHandle->handleValue = (uint64_t)m_resource.getResource();
    outHandle->api = InteropHandleAPI::D3D12;
    return SLANG_OK;
}

Result BufferResourceImpl::getSharedHandle(InteropHandle* outHandle)
{
    // Check if a shared handle already exists for this resource.
    if (sharedHandle.handleValue != 0)
    {
        *outHandle = sharedHandle;
        return SLANG_OK;
    }

    // If a shared handle doesn't exist, create one and store it.
    ComPtr<ID3D12Device> pDevice;
    auto pResource = m_resource.getResource();
    pResource->GetDevice(IID_PPV_ARGS(pDevice.writeRef()));
    SLANG_RETURN_ON_FAIL(pDevice->CreateSharedHandle(
        pResource, NULL, GENERIC_ALL, nullptr, (HANDLE*)&outHandle->handleValue));
    outHandle->api = InteropHandleAPI::D3D12;
    sharedHandle = *outHandle;
    return SLANG_OK;
}

Result BufferResourceImpl::map(MemoryRange* rangeToRead, void** outPointer)
{
    D3D12_RANGE range = {};
    if (rangeToRead)
    {
        range.Begin = (SIZE_T)rangeToRead->offset;
        range.End = (SIZE_T)(rangeToRead->offset + rangeToRead->size);
    }
    SLANG_RETURN_ON_FAIL(
        m_resource.getResource()->Map(0, rangeToRead ? &range : nullptr, outPointer));
    return SLANG_OK;
}

Result BufferResourceImpl::unmap(MemoryRange* writtenRange)
{
    D3D12_RANGE range = {};
    if (writtenRange)
    {
        range.Begin = (SIZE_T)writtenRange->offset;
        range.End = (SIZE_T)(writtenRange->offset + writtenRange->size);
    }
    m_resource.getResource()->Unmap(0, writtenRange ? &range : nullptr);
    return SLANG_OK;
}

Result BufferResourceImpl::setDebugName(const char* name)
{
    Parent::setDebugName(name);
    m_resource.setDebugName(name);
    return SLANG_OK;
}

TextureResourceImpl::TextureResourceImpl(const Desc& desc)
    : Parent(desc)
    , m_defaultState(D3DUtil::getResourceState(desc.defaultState))
{}

TextureResourceImpl::~TextureResourceImpl()
{
    if (sharedHandle.handleValue != 0)
    {
        CloseHandle((HANDLE)sharedHandle.handleValue);
    }
}

Result TextureResourceImpl::getNativeResourceHandle(InteropHandle* outHandle)
{
    outHandle->handleValue = (uint64_t)m_resource.getResource();
    outHandle->api = InteropHandleAPI::D3D12;
    return SLANG_OK;
}

Result TextureResourceImpl::getSharedHandle(InteropHandle* outHandle)
{
    // Check if a shared handle already exists for this resource.
    if (sharedHandle.handleValue != 0)
    {
        *outHandle = sharedHandle;
        return SLANG_OK;
    }

    // If a shared handle doesn't exist, create one and store it.
    ComPtr<ID3D12Device> pDevice;
    auto pResource = m_resource.getResource();
    pResource->GetDevice(IID_PPV_ARGS(pDevice.writeRef()));
    SLANG_RETURN_ON_FAIL(pDevice->CreateSharedHandle(
        pResource, NULL, GENERIC_ALL, nullptr, (HANDLE*)&outHandle->handleValue));
    outHandle->api = InteropHandleAPI::D3D12;
    return SLANG_OK;
}

Result TextureResourceImpl::setDebugName(const char* name)
{
    Parent::setDebugName(name);
    m_resource.setDebugName(name);
    return SLANG_OK;
}

SamplerStateImpl::~SamplerStateImpl() { m_allocator->free(m_descriptor); }

Result SamplerStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::D3D12CpuDescriptorHandle;
    outHandle->handleValue = m_descriptor.cpuHandle.ptr;
    return SLANG_OK;
}

#if SLANG_GFX_HAS_DXR_SUPPORT

DeviceAddress AccelerationStructureImpl::getDeviceAddress()
{
    return m_buffer->getDeviceAddress() + m_offset;
}

Result AccelerationStructureImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::DeviceAddress;
    outHandle->handleValue = getDeviceAddress();
    return SLANG_OK;
}

#endif // SLANG_GFX_HAS_DXR_SUPPORT

Result TransientResourceHeapImpl::synchronizeAndReset()
{
    WaitForMultipleObjects(
        (DWORD)m_waitHandles.getCount(), m_waitHandles.getArrayView().getBuffer(), TRUE, INFINITE);
    m_waitHandles.clear();
    m_currentViewHeapIndex = -1;
    m_currentSamplerHeapIndex = -1;
    allocateNewViewDescriptorHeap(m_device);
    allocateNewSamplerDescriptorHeap(m_device);
    m_stagingCpuSamplerHeap.freeAll();
    m_stagingCpuViewHeap.freeAll();
    m_commandListAllocId = 0;
    SLANG_RETURN_ON_FAIL(m_commandAllocator->Reset());
    Super::reset();
    return SLANG_OK;
}

Result TransientResourceHeapImpl::finish()
{
    for (auto& waitInfo : m_waitInfos)
    {
        if (waitInfo.waitValue == 0)
            continue;
        if (waitInfo.fence)
        {
            waitInfo.queue->Signal(waitInfo.fence, waitInfo.waitValue);
            waitInfo.fence->SetEventOnCompletion(waitInfo.waitValue, waitInfo.fenceEvent);
            m_waitHandles.add(waitInfo.fenceEvent);
        }
    }
    return SLANG_OK;
}

TransientResourceHeapImpl::QueueWaitInfo& TransientResourceHeapImpl::getQueueWaitInfo(
    uint32_t queueIndex)
{
    if (queueIndex < (uint32_t)m_waitInfos.getCount())
    {
        return m_waitInfos[queueIndex];
    }
    auto oldCount = m_waitInfos.getCount();
    m_waitInfos.setCount(queueIndex + 1);
    for (auto i = oldCount; i < m_waitInfos.getCount(); i++)
    {
        m_waitInfos[i].waitValue = 0;
        m_waitInfos[i].fenceEvent = CreateEventEx(nullptr, false, 0, EVENT_ALL_ACCESS);
    }
    return m_waitInfos[queueIndex];
}

D3D12DescriptorHeap& TransientResourceHeapImpl::getCurrentViewHeap()
{
    return m_viewHeaps[m_currentViewHeapIndex];
}

D3D12DescriptorHeap& TransientResourceHeapImpl::getCurrentSamplerHeap()
{
    return m_samplerHeaps[m_currentSamplerHeapIndex];
}

Result TransientResourceHeapImpl::queryInterface(SlangUUID const& uuid, void** outObject)
{
    if (uuid == GfxGUID::IID_ID3D12TransientResourceHeap)
    {
        *outObject = static_cast<ID3D12TransientResourceHeap*>(this);
        addRef();
        return SLANG_OK;
    }
    return Super::queryInterface(uuid, outObject);
}

Result TransientResourceHeapImpl::allocateTransientDescriptorTable(
    DescriptorType type,
    uint32_t count,
    uint64_t& outDescriptorOffset,
    void** outD3DDescriptorHeapHandle)
{
    auto& heap =
        (type == DescriptorType::ResourceView) ? getCurrentViewHeap() : getCurrentSamplerHeap();
    int allocResult = heap.allocate((int)count);
    if (allocResult == -1)
    {
        return SLANG_E_OUT_OF_MEMORY;
    }
    outDescriptorOffset = (uint64_t)allocResult;
    *outD3DDescriptorHeapHandle = heap.getHeap();
    return SLANG_OK;
}

TransientResourceHeapImpl::~TransientResourceHeapImpl()
{
    synchronizeAndReset();
    for (auto& waitInfo : m_waitInfos)
        CloseHandle(waitInfo.fenceEvent);
}

Result TransientResourceHeapImpl::init(
    const ITransientResourceHeap::Desc& desc,
    DeviceImpl* device,
    uint32_t viewHeapSize,
    uint32_t samplerHeapSize)
{
    Super::init(desc, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, device);
    m_canResize = (desc.flags & ITransientResourceHeap::Flags::AllowResizing) != 0;
    m_viewHeapSize = viewHeapSize;
    m_samplerHeapSize = samplerHeapSize;

    m_stagingCpuViewHeap.init(
        device->m_device,
        1000000,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    m_stagingCpuSamplerHeap.init(
        device->m_device,
        1000000,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    auto d3dDevice = device->m_device;
    SLANG_RETURN_ON_FAIL(d3dDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.writeRef())));

    allocateNewViewDescriptorHeap(device);
    allocateNewSamplerDescriptorHeap(device);

    return SLANG_OK;
}

Result TransientResourceHeapImpl::allocateNewViewDescriptorHeap(DeviceImpl* device)
{
    auto nextHeapIndex = m_currentViewHeapIndex + 1;
    if (nextHeapIndex < m_viewHeaps.getCount())
    {
        m_viewHeaps[nextHeapIndex].deallocateAll();
        m_currentViewHeapIndex = nextHeapIndex;
        return SLANG_OK;
    }
    auto d3dDevice = device->m_device;
    D3D12DescriptorHeap viewHeap;
    SLANG_RETURN_ON_FAIL(viewHeap.init(
        d3dDevice,
        m_viewHeapSize,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
    m_currentViewHeapIndex = (int32_t)m_viewHeaps.getCount();
    m_viewHeaps.add(_Move(viewHeap));
    return SLANG_OK;
}

Result TransientResourceHeapImpl::allocateNewSamplerDescriptorHeap(DeviceImpl* device)
{
    auto nextHeapIndex = m_currentSamplerHeapIndex + 1;
    if (nextHeapIndex < m_samplerHeaps.getCount())
    {
        m_samplerHeaps[nextHeapIndex].deallocateAll();
        m_currentSamplerHeapIndex = nextHeapIndex;
        return SLANG_OK;
    }
    auto d3dDevice = device->m_device;
    D3D12DescriptorHeap samplerHeap;
    SLANG_RETURN_ON_FAIL(samplerHeap.init(
        d3dDevice,
        m_samplerHeapSize,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
    m_currentSamplerHeapIndex = (int32_t)m_samplerHeaps.getCount();
    m_samplerHeaps.add(_Move(samplerHeap));
    return SLANG_OK;
}

Result TransientResourceHeapImpl::createCommandBuffer(ICommandBuffer** outCmdBuffer)
{
    if ((Index)m_commandListAllocId < m_commandBufferPool.getCount())
    {
        auto result =
            static_cast<CommandBufferImpl*>(m_commandBufferPool[m_commandListAllocId].Ptr());
        m_d3dCommandListPool[m_commandListAllocId]->Reset(m_commandAllocator, nullptr);
        result->reinit();
        ++m_commandListAllocId;
        returnComPtr(outCmdBuffer, result);
        return SLANG_OK;
    }
    ComPtr<ID3D12GraphicsCommandList> cmdList;
    m_device->m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator,
        nullptr,
        IID_PPV_ARGS(cmdList.writeRef()));

    m_d3dCommandListPool.add(cmdList);
    RefPtr<CommandBufferImpl> cmdBuffer = new CommandBufferImpl();
    cmdBuffer->init(m_device, cmdList, this);
    m_commandBufferPool.add(cmdBuffer);
    ++m_commandListAllocId;
    returnComPtr(outCmdBuffer, cmdBuffer);
    return SLANG_OK;
}

int PipelineCommandEncoder::getBindPointIndex(PipelineType type)
{
    switch (type)
    {
    case PipelineType::Graphics:
        return 0;
    case PipelineType::Compute:
        return 1;
    case PipelineType::RayTracing:
        return 2;
    default:
        assert(!"unknown pipeline type.");
        return -1;
    }
}

void PipelineCommandEncoder::init(CommandBufferImpl* commandBuffer)
{
    m_commandBuffer = commandBuffer;
    m_d3dCmdList = m_commandBuffer->m_cmdList;
    m_renderer = commandBuffer->m_renderer;
    m_transientHeap = commandBuffer->m_transientHeap;
    m_device = commandBuffer->m_renderer->m_device;
}

Result PipelineCommandEncoder::bindPipelineImpl(
    IPipelineState* pipelineState, IShaderObject** outRootObject)
{
    m_currentPipeline = static_cast<PipelineStateBase*>(pipelineState);
    auto rootObject = &m_commandBuffer->m_rootShaderObject;
    m_commandBuffer->m_mutableRootShaderObject = nullptr;
    SLANG_RETURN_ON_FAIL(rootObject->reset(
        m_renderer,
        m_currentPipeline->getProgram<ShaderProgramImpl>()->m_rootObjectLayout,
        m_commandBuffer->m_transientHeap));
    *outRootObject = rootObject;
    m_bindingDirty = true;
    return SLANG_OK;
}

Result PipelineCommandEncoder::bindPipelineWithRootObjectImpl(
    IPipelineState* pipelineState, IShaderObject* rootObject)
{
    m_currentPipeline = static_cast<PipelineStateBase*>(pipelineState);
    m_commandBuffer->m_mutableRootShaderObject =
        static_cast<MutableRootShaderObjectImpl*>(rootObject);
    m_bindingDirty = true;
    return SLANG_OK;
}

Result PipelineCommandEncoder::_bindRenderState(
    Submitter* submitter, RefPtr<PipelineStateBase>& newPipeline)
{
    RootShaderObjectImpl* rootObjectImpl = m_commandBuffer->m_mutableRootShaderObject
                                               ? m_commandBuffer->m_mutableRootShaderObject.Ptr()
                                               : &m_commandBuffer->m_rootShaderObject;
    SLANG_RETURN_ON_FAIL(
        m_renderer->maybeSpecializePipeline(m_currentPipeline, rootObjectImpl, newPipeline));
    PipelineStateBase* newPipelineImpl = static_cast<PipelineStateBase*>(newPipeline.Ptr());
    auto commandList = m_d3dCmdList;
    auto pipelineTypeIndex = (int)newPipelineImpl->desc.type;
    auto programImpl = static_cast<ShaderProgramImpl*>(newPipelineImpl->m_program.Ptr());
    newPipelineImpl->ensureAPIPipelineStateCreated();
    submitter->setRootSignature(programImpl->m_rootObjectLayout->m_rootSignature);
    submitter->setPipelineState(newPipelineImpl);
    RootShaderObjectLayoutImpl* rootLayoutImpl = programImpl->m_rootObjectLayout;

    // We need to set up a context for binding shader objects to the pipeline state.
    // This type mostly exists to bundle together a bunch of parameters that would
    // otherwise need to be tunneled down through all the shader object binding
    // logic.
    //
    BindingContext context = {};
    context.encoder = this;
    context.submitter = submitter;
    context.device = m_renderer;
    context.transientHeap = m_transientHeap;
    context.outOfMemoryHeap = (D3D12_DESCRIPTOR_HEAP_TYPE)(-1);
    // We kick off binding of shader objects at the root object, and the objects
    // themselves will be responsible for allocating, binding, and filling in
    // any descriptor tables or other root parameters needed.
    //
    m_commandBuffer->bindDescriptorHeaps();
    if (rootObjectImpl->bindAsRoot(&context, rootLayoutImpl) == SLANG_E_OUT_OF_MEMORY)
    {
        if (!m_transientHeap->canResize())
        {
            return SLANG_E_OUT_OF_MEMORY;
        }

        // If we run out of heap space while binding, allocate new descriptor heaps and try again.
        ID3D12DescriptorHeap* d3dheap = nullptr;
        m_commandBuffer->invalidateDescriptorHeapBinding();
        switch (context.outOfMemoryHeap)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            SLANG_RETURN_ON_FAIL(m_transientHeap->allocateNewViewDescriptorHeap(m_renderer));
            d3dheap = m_transientHeap->getCurrentViewHeap().getHeap();
            m_commandBuffer->bindDescriptorHeaps();
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            SLANG_RETURN_ON_FAIL(m_transientHeap->allocateNewSamplerDescriptorHeap(m_renderer));
            d3dheap = m_transientHeap->getCurrentSamplerHeap().getHeap();
            m_commandBuffer->bindDescriptorHeaps();
            break;
        default:
            assert(!"shouldn't be here");
            return SLANG_FAIL;
        }

        // Try again.
        SLANG_RETURN_ON_FAIL(rootObjectImpl->bindAsRoot(&context, rootLayoutImpl));
    }

    return SLANG_OK;
}

Result QueryPoolImpl::init(const IQueryPool::Desc& desc, DeviceImpl* device)
{
    m_desc = desc;

    // Translate query type.
    D3D12_QUERY_HEAP_DESC heapDesc = {};
    heapDesc.Count = (UINT)desc.count;
    heapDesc.NodeMask = 1;
    switch (desc.type)
    {
    case QueryType::Timestamp:
        heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        m_queryType = D3D12_QUERY_TYPE_TIMESTAMP;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }

    // Create query heap.
    auto d3dDevice = device->m_device;
    SLANG_RETURN_ON_FAIL(
        d3dDevice->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(m_queryHeap.writeRef())));

    // Create readback buffer.
    D3D12_HEAP_PROPERTIES heapProps;
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;
    D3D12_RESOURCE_DESC resourceDesc = {};
    initBufferResourceDesc(sizeof(uint64_t) * desc.count, resourceDesc);
    SLANG_RETURN_ON_FAIL(m_readBackBuffer.initCommitted(
        d3dDevice,
        heapProps,
        D3D12_HEAP_FLAG_NONE,
        resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr));

    // Create command allocator.
    SLANG_RETURN_ON_FAIL(d3dDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.writeRef())));

    // Create command list.
    SLANG_RETURN_ON_FAIL(d3dDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocator,
        nullptr,
        IID_PPV_ARGS(m_commandList.writeRef())));
    m_commandList->Close();

    // Create fence.
    SLANG_RETURN_ON_FAIL(
        d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.writeRef())));

    // Get command queue from device.
    m_commandQueue = device->m_resourceCommandQueue->m_d3dQueue;

    // Create wait event.
    m_waitEvent = CreateEventEx(nullptr, false, 0, EVENT_ALL_ACCESS);

    return SLANG_OK;
}

Result QueryPoolImpl::getResult(SlangInt queryIndex, SlangInt count, uint64_t* data)
{
    m_commandList->Reset(m_commandAllocator, nullptr);
    m_commandList->ResolveQueryData(
        m_queryHeap,
        m_queryType,
        (UINT)queryIndex,
        (UINT)count,
        m_readBackBuffer,
        sizeof(uint64_t) * queryIndex);
    m_commandList->Close();
    ID3D12CommandList* cmdList = m_commandList;
    m_commandQueue->ExecuteCommandLists(1, &cmdList);
    m_eventValue++;
    m_fence->SetEventOnCompletion(m_eventValue, m_waitEvent);
    m_commandQueue->Signal(m_fence, m_eventValue);
    WaitForSingleObject(m_waitEvent, INFINITE);
    m_commandAllocator->Reset();

    int8_t* mappedData = nullptr;
    D3D12_RANGE readRange = {
        sizeof(uint64_t) * queryIndex, sizeof(uint64_t) * (queryIndex + count)};
    m_readBackBuffer.getResource()->Map(0, &readRange, (void**)&mappedData);
    memcpy(data, mappedData + sizeof(uint64_t) * queryIndex, sizeof(uint64_t) * count);
    m_readBackBuffer.getResource()->Unmap(0, nullptr);
    return SLANG_OK;
}

void QueryPoolImpl::writeTimestamp(ID3D12GraphicsCommandList* cmdList, SlangInt index)
{
    cmdList->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, (UINT)index);
}

IQueryPool* PlainBufferProxyQueryPoolImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_IQueryPool)
        return static_cast<IQueryPool*>(this);
    return nullptr;
}

Result PlainBufferProxyQueryPoolImpl::init(
    const IQueryPool::Desc& desc, DeviceImpl* device, uint32_t stride)
{
    ComPtr<IBufferResource> bufferResource;
    IBufferResource::Desc bufferDesc = {};
    bufferDesc.defaultState = ResourceState::CopySource;
    bufferDesc.elementSize = 0;
    bufferDesc.type = IResource::Type::Buffer;
    bufferDesc.sizeInBytes = desc.count * stride;
    bufferDesc.format = Format::Unknown;
    bufferDesc.allowedStates.add(ResourceState::UnorderedAccess);
    SLANG_RETURN_ON_FAIL(
        device->createBufferResource(bufferDesc, nullptr, bufferResource.writeRef()));
    m_bufferResource = static_cast<BufferResourceImpl*>(bufferResource.get());
    m_queryType = desc.type;
    m_device = device;
    m_stride = stride;
    m_count = (uint32_t)desc.count;
    m_desc = desc;
    return SLANG_OK;
}

Result PlainBufferProxyQueryPoolImpl::reset()
{
    m_resultDirty = true;
    auto encodeInfo = m_device->encodeResourceCommands();
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    barrier.Transition.pResource = m_bufferResource->m_resource.getResource();
    encodeInfo.d3dCommandList->ResourceBarrier(1, &barrier);
    m_device->submitResourceCommandsAndWait(encodeInfo);
    return SLANG_OK;
}

Result PlainBufferProxyQueryPoolImpl::getResult(SlangInt queryIndex, SlangInt count, uint64_t* data)
{
    if (m_resultDirty)
    {
        auto encodeInfo = m_device->encodeResourceCommands();
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barrier.Transition.pResource = m_bufferResource->m_resource.getResource();
        encodeInfo.d3dCommandList->ResourceBarrier(1, &barrier);

        D3D12Resource stageBuf;

        auto size = (size_t)m_count * m_stride;
        D3D12_HEAP_PROPERTIES heapProps;
        heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC stagingDesc;
        initBufferResourceDesc(size, stagingDesc);

        SLANG_RETURN_ON_FAIL(stageBuf.initCommitted(
            m_device->m_device,
            heapProps,
            D3D12_HEAP_FLAG_NONE,
            stagingDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr));

        encodeInfo.d3dCommandList->CopyBufferRegion(
            stageBuf, 0, m_bufferResource->m_resource.getResource(), 0, size);
        m_device->submitResourceCommandsAndWait(encodeInfo);
        void* ptr = nullptr;
        stageBuf.getResource()->Map(0, nullptr, &ptr);
        m_result.setCount(m_count * m_stride);
        memcpy(m_result.getBuffer(), ptr, m_result.getCount());

        m_resultDirty = false;
    }

    memcpy(data, m_result.getBuffer() + queryIndex * m_stride, count * m_stride);

    return SLANG_OK;
}

void translatePostBuildInfoDescs(
    int propertyQueryCount,
    AccelerationStructureQueryDesc* queryDescs,
    List<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC>& postBuildInfoDescs)
{
    postBuildInfoDescs.setCount(propertyQueryCount);
    for (int i = 0; i < propertyQueryCount; i++)
    {
        switch (queryDescs[i].queryType)
        {
        case QueryType::AccelerationStructureCompactedSize:
            postBuildInfoDescs[i].InfoType =
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE;
            postBuildInfoDescs[i].DestBuffer =
                static_cast<PlainBufferProxyQueryPoolImpl*>(queryDescs[i].queryPool)
                    ->m_bufferResource->getDeviceAddress() +
                sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC) *
                    queryDescs[i].firstQueryIndex;
            break;
        case QueryType::AccelerationStructureCurrentSize:
            postBuildInfoDescs[i].InfoType =
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_CURRENT_SIZE;
            postBuildInfoDescs[i].DestBuffer =
                static_cast<PlainBufferProxyQueryPoolImpl*>(queryDescs[i].queryPool)
                    ->m_bufferResource->getDeviceAddress() +
                sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC) *
                    queryDescs[i].firstQueryIndex;
            break;
        case QueryType::AccelerationStructureSerializedSize:
            postBuildInfoDescs[i].InfoType =
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_SERIALIZATION;
            postBuildInfoDescs[i].DestBuffer =
                static_cast<PlainBufferProxyQueryPoolImpl*>(queryDescs[i].queryPool)
                    ->m_bufferResource->getDeviceAddress() +
                sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_SERIALIZATION_DESC) *
                    queryDescs[i].firstQueryIndex;
            break;
        }
    }
}

#if SLANG_GFX_HAS_DXR_SUPPORT

void RayTracingCommandEncoderImpl::buildAccelerationStructure(
    const IAccelerationStructure::BuildDesc& desc,
    int propertyQueryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    if (!m_commandBuffer->m_cmdList4)
    {
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "Ray-tracing is not supported on current system.");
        return;
    }
    AccelerationStructureImpl* destASImpl = nullptr;
    if (desc.dest)
        destASImpl = static_cast<AccelerationStructureImpl*>(desc.dest);
    AccelerationStructureImpl* srcASImpl = nullptr;
    if (desc.source)
        srcASImpl = static_cast<AccelerationStructureImpl*>(desc.source);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.DestAccelerationStructureData = destASImpl->getDeviceAddress();
    buildDesc.SourceAccelerationStructureData = srcASImpl ? srcASImpl->getDeviceAddress() : 0;
    buildDesc.ScratchAccelerationStructureData = desc.scratchData;
    D3DAccelerationStructureInputsBuilder builder;
    builder.build(desc.inputs, getDebugCallback());
    buildDesc.Inputs = builder.desc;

    List<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC> postBuildInfoDescs;
    translatePostBuildInfoDescs(propertyQueryCount, queryDescs, postBuildInfoDescs);
    m_commandBuffer->m_cmdList4->BuildRaytracingAccelerationStructure(
        &buildDesc, (UINT)propertyQueryCount, postBuildInfoDescs.getBuffer());
}

void RayTracingCommandEncoderImpl::copyAccelerationStructure(
    IAccelerationStructure* dest, IAccelerationStructure* src, AccelerationStructureCopyMode mode)
{
    auto destASImpl = static_cast<AccelerationStructureImpl*>(dest);
    auto srcASImpl = static_cast<AccelerationStructureImpl*>(src);
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE copyMode;
    switch (mode)
    {
    case AccelerationStructureCopyMode::Clone:
        copyMode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_CLONE;
        break;
    case AccelerationStructureCopyMode::Compact:
        copyMode = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT;
        break;
    default:
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "Unsupported AccelerationStructureCopyMode.");
        return;
    }
    m_commandBuffer->m_cmdList4->CopyRaytracingAccelerationStructure(
        destASImpl->getDeviceAddress(), srcASImpl->getDeviceAddress(), copyMode);
}

void RayTracingCommandEncoderImpl::queryAccelerationStructureProperties(
    int accelerationStructureCount,
    IAccelerationStructure* const* accelerationStructures,
    int queryCount,
    AccelerationStructureQueryDesc* queryDescs)
{
    List<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC> postBuildInfoDescs;
    List<DeviceAddress> asAddresses;
    asAddresses.setCount(accelerationStructureCount);
    for (int i = 0; i < accelerationStructureCount; i++)
        asAddresses[i] = accelerationStructures[i]->getDeviceAddress();
    translatePostBuildInfoDescs(queryCount, queryDescs, postBuildInfoDescs);
    m_commandBuffer->m_cmdList4->EmitRaytracingAccelerationStructurePostbuildInfo(
        postBuildInfoDescs.getBuffer(), (UINT)accelerationStructureCount, asAddresses.getBuffer());
}

void RayTracingCommandEncoderImpl::serializeAccelerationStructure(
    DeviceAddress dest, IAccelerationStructure* src)
{
    auto srcASImpl = static_cast<AccelerationStructureImpl*>(src);
    m_commandBuffer->m_cmdList4->CopyRaytracingAccelerationStructure(
        dest,
        srcASImpl->getDeviceAddress(),
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_SERIALIZE);
}

void RayTracingCommandEncoderImpl::deserializeAccelerationStructure(
    IAccelerationStructure* dest, DeviceAddress source)
{
    auto destASImpl = static_cast<AccelerationStructureImpl*>(dest);
    m_commandBuffer->m_cmdList4->CopyRaytracingAccelerationStructure(
        dest->getDeviceAddress(),
        source,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_DESERIALIZE);
}

void RayTracingCommandEncoderImpl::bindPipeline(
    IPipelineState* state, IShaderObject** outRootObject)
{
    bindPipelineImpl(state, outRootObject);
}

void RayTracingCommandEncoderImpl::dispatchRays(
    uint32_t rayGenShaderIndex,
    IShaderTable* shaderTable,
    int32_t width,
    int32_t height,
    int32_t depth)
{
    RefPtr<PipelineStateBase> newPipeline;
    PipelineStateBase* pipeline = m_currentPipeline.Ptr();
    {
        struct RayTracingSubmitter : public ComputeSubmitter
        {
            ID3D12GraphicsCommandList4* m_cmdList4;
            RayTracingSubmitter(ID3D12GraphicsCommandList4* cmdList4)
                : ComputeSubmitter(cmdList4)
                , m_cmdList4(cmdList4)
            {}
            virtual void setPipelineState(PipelineStateBase* pipeline) override
            {
                auto pipelineImpl = static_cast<RayTracingPipelineStateImpl*>(pipeline);
                m_cmdList4->SetPipelineState1(pipelineImpl->m_stateObject.get());
            }
        };
        RayTracingSubmitter submitter(m_commandBuffer->m_cmdList4);
        if (SLANG_FAILED(_bindRenderState(&submitter, newPipeline)))
        {
            assert(!"Failed to bind render state");
        }
        if (newPipeline)
            pipeline = newPipeline.Ptr();
    }
    auto pipelineImpl = static_cast<RayTracingPipelineStateImpl*>(pipeline);

    auto shaderTableImpl = static_cast<ShaderTableImpl*>(shaderTable);

    auto shaderTableBuffer =
        shaderTableImpl->getOrCreateBuffer(pipelineImpl, m_transientHeap, static_cast<ResourceCommandEncoderImpl*>(this));
    auto shaderTableAddr = shaderTableBuffer->getDeviceAddress();

    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};

    dispatchDesc.RayGenerationShaderRecord.StartAddress =
        shaderTableAddr + shaderTableImpl->m_rayGenTableOffset +
        rayGenShaderIndex * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    dispatchDesc.MissShaderTable.StartAddress =
        shaderTableAddr + shaderTableImpl->m_missTableOffset;
    dispatchDesc.MissShaderTable.SizeInBytes =
        shaderTableImpl->m_missShaderCount * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    dispatchDesc.MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    dispatchDesc.HitGroupTable.StartAddress =
        shaderTableAddr + shaderTableImpl->m_hitGroupTableOffset;
    dispatchDesc.HitGroupTable.SizeInBytes =
        shaderTableImpl->m_hitGroupCount * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    dispatchDesc.HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    dispatchDesc.Width = (UINT)width;
    dispatchDesc.Height = (UINT)height;
    dispatchDesc.Depth = (UINT)depth;
    m_commandBuffer->m_cmdList4->DispatchRays(&dispatchDesc);
}

RayTracingPipelineStateImpl::RayTracingPipelineStateImpl(DeviceImpl* device)
    : m_device(device)
{}

void RayTracingPipelineStateImpl::init(const RayTracingPipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::RayTracing;
    pipelineDesc.rayTracing.set(inDesc);
    initializeBase(pipelineDesc);
}

Result RayTracingPipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_RETURN_ON_FAIL(ensureAPIPipelineStateCreated());
    outHandle->api = InteropHandleAPI::D3D12;
    outHandle->handleValue = reinterpret_cast<uint64_t>(m_stateObject.get());
    return SLANG_OK;
}

Result RayTracingPipelineStateImpl::ensureAPIPipelineStateCreated()
{
    if (m_stateObject)
        return SLANG_OK;

    auto program = static_cast<ShaderProgramImpl*>(m_program.Ptr());
    auto slangGlobalScope = program->linkedProgram;
    auto programLayout = slangGlobalScope->getLayout();

    List<D3D12_STATE_SUBOBJECT> subObjects;
    ChunkedList<D3D12_DXIL_LIBRARY_DESC> dxilLibraries;
    ChunkedList<D3D12_HIT_GROUP_DESC> hitGroups;
    ChunkedList<ComPtr<ISlangBlob>> codeBlobs;
    ChunkedList<D3D12_EXPORT_DESC> exports;
    ChunkedList<const wchar_t*> strPtrs;

    ComPtr<ISlangBlob> diagnostics;
    ChunkedList<OSString> stringPool;
    auto getWStr = [&](const char* name)
    {
        String str = String(name);
        auto wstr = str.toWString();
        return stringPool.add(wstr)->begin();
    };
    auto compileShader = [&](slang::EntryPointLayout* entryPointInfo,
                             slang::IComponentType* component,
                             SlangInt entryPointIndex)
    {
        ComPtr<ISlangBlob> codeBlob;
        auto compileResult = component->getEntryPointCode(
            entryPointIndex, 0, codeBlob.writeRef(), diagnostics.writeRef());
        if (diagnostics.get())
        {
            getDebugCallback()->handleMessage(
                compileResult == SLANG_OK ? DebugMessageType::Warning : DebugMessageType::Error,
                DebugMessageSource::Slang,
                (char*)diagnostics->getBufferPointer());
        }
        SLANG_RETURN_ON_FAIL(compileResult);
        codeBlobs.add(codeBlob);
        D3D12_DXIL_LIBRARY_DESC library = {};
        library.DXILLibrary.BytecodeLength = codeBlob->getBufferSize();
        library.DXILLibrary.pShaderBytecode = codeBlob->getBufferPointer();
        library.NumExports = 1;
        D3D12_EXPORT_DESC exportDesc = {};
        exportDesc.Name = getWStr(entryPointInfo->getNameOverride());
        exportDesc.ExportToRename = getWStr(entryPointInfo->getNameOverride());
        exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;
        library.pExports = exports.add(exportDesc);

        D3D12_STATE_SUBOBJECT dxilSubObject = {};
        dxilSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
        dxilSubObject.pDesc = dxilLibraries.add(library);
        subObjects.add(dxilSubObject);
        return SLANG_OK;
    };
    if (program->linkedEntryPoints.getCount() == 0)
    {
        for (SlangUInt i = 0; i < programLayout->getEntryPointCount(); i++)
        {
            SLANG_RETURN_ON_FAIL(compileShader(
                programLayout->getEntryPointByIndex(i), program->linkedProgram, (SlangInt)i));
        }
    }
    else
    {
        for (auto& entryPoint : program->linkedEntryPoints)
        {
            SLANG_RETURN_ON_FAIL(
                compileShader(entryPoint->getLayout()->getEntryPointByIndex(0), entryPoint, 0));
        }
    }

    for (Index i = 0; i < desc.rayTracing.hitGroupDescs.getCount(); i++)
    {
        auto& hitGroup = desc.rayTracing.hitGroups[i];
        D3D12_HIT_GROUP_DESC hitGroupDesc = {};
        hitGroupDesc.Type = hitGroup.intersectionEntryPoint.getLength() == 0
                                ? D3D12_HIT_GROUP_TYPE_TRIANGLES
                                : D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;

        if (hitGroup.anyHitEntryPoint.getLength())
        {
            hitGroupDesc.AnyHitShaderImport = getWStr(hitGroup.anyHitEntryPoint.getBuffer());
        }
        if (hitGroup.closestHitEntryPoint.getLength())
        {
            hitGroupDesc.ClosestHitShaderImport =
                getWStr(hitGroup.closestHitEntryPoint.getBuffer());
        }
        if (hitGroup.intersectionEntryPoint.getLength())
        {
            hitGroupDesc.IntersectionShaderImport =
                getWStr(hitGroup.intersectionEntryPoint.getBuffer());
        }
        hitGroupDesc.HitGroupExport = getWStr(hitGroup.hitGroupName.getBuffer());

        D3D12_STATE_SUBOBJECT hitGroupSubObject = {};
        hitGroupSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        hitGroupSubObject.pDesc = hitGroups.add(hitGroupDesc);
        subObjects.add(hitGroupSubObject);
    }

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    // According to DXR spec, fixed function triangle intersections must use float2 as ray
    // attributes that defines the barycentric coordinates at intersection.
    shaderConfig.MaxAttributeSizeInBytes = desc.rayTracing.maxAttributeSizeInBytes;
    shaderConfig.MaxPayloadSizeInBytes = desc.rayTracing.maxRayPayloadSize;
    D3D12_STATE_SUBOBJECT shaderConfigSubObject = {};
    shaderConfigSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    shaderConfigSubObject.pDesc = &shaderConfig;
    subObjects.add(shaderConfigSubObject);

    D3D12_GLOBAL_ROOT_SIGNATURE globalSignatureDesc = {};
    globalSignatureDesc.pGlobalRootSignature = program->m_rootObjectLayout->m_rootSignature.get();
    D3D12_STATE_SUBOBJECT globalSignatureSubobject = {};
    globalSignatureSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    globalSignatureSubobject.pDesc = &globalSignatureDesc;
    subObjects.add(globalSignatureSubobject);

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = desc.rayTracing.maxRecursion;
    D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
    pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    pipelineConfigSubobject.pDesc = &pipelineConfig;
    subObjects.add(pipelineConfigSubobject);

    if (m_device->m_pipelineCreationAPIDispatcher)
    {
        m_device->m_pipelineCreationAPIDispatcher->beforeCreateRayTracingState(
            m_device, slangGlobalScope);
    }

    D3D12_STATE_OBJECT_DESC rtpsoDesc = {};
    rtpsoDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    rtpsoDesc.NumSubobjects = (UINT)subObjects.getCount();
    rtpsoDesc.pSubobjects = subObjects.getBuffer();
    SLANG_RETURN_ON_FAIL(
        m_device->m_device5->CreateStateObject(&rtpsoDesc, IID_PPV_ARGS(m_stateObject.writeRef())));

    if (m_device->m_pipelineCreationAPIDispatcher)
    {
        m_device->m_pipelineCreationAPIDispatcher->afterCreateRayTracingState(
            m_device, slangGlobalScope);
    }
    return SLANG_OK;
}

#endif

UInt ShaderObjectImpl::getEntryPointCount() { return 0; }

Result ShaderObjectImpl::getEntryPoint(UInt index, IShaderObject** outEntryPoint)
{
    *outEntryPoint = nullptr;
    return SLANG_OK;
}

const void* ShaderObjectImpl::getRawData() { return m_data.getBuffer(); }

size_t ShaderObjectImpl::getSize() { return (size_t)m_data.getCount(); }

Result ShaderObjectImpl::setData(ShaderOffset const& inOffset, void const* data, size_t inSize)
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

    m_isConstantBufferDirty = true;

    m_version++;

    return SLANG_OK;
}

Result ShaderObjectImpl::setObject(ShaderOffset const& offset, IShaderObject* object)
{
    SLANG_RETURN_ON_FAIL(Super::setObject(offset, object));
    if (m_isMutable)
    {
        auto subObjectIndex = getSubObjectIndex(offset);
        if (subObjectIndex >= m_subObjectVersions.getCount())
            m_subObjectVersions.setCount(subObjectIndex + 1);
        m_subObjectVersions[subObjectIndex] = static_cast<ShaderObjectImpl*>(object)->m_version;
        m_version++;
    }
    return SLANG_OK;
}

Result ShaderObjectImpl::setSampler(ShaderOffset const& offset, ISamplerState* sampler)
{
    if (offset.bindingRangeIndex < 0)
        return SLANG_E_INVALID_ARG;
    auto layout = getLayout();
    if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
        return SLANG_E_INVALID_ARG;
    auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
    auto samplerImpl = static_cast<SamplerStateImpl*>(sampler);
    ID3D12Device* d3dDevice = static_cast<DeviceImpl*>(getDevice())->m_device;
    d3dDevice->CopyDescriptorsSimple(
        1,
        m_descriptorSet.samplerTable.getCpuHandle(
            bindingRange.baseIndex + (int32_t)offset.bindingArrayIndex),
        samplerImpl->m_descriptor.cpuHandle,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    m_version++;
    return SLANG_OK;
}

Result ShaderObjectImpl::setCombinedTextureSampler(
    ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler)
{
#if 0
            if (offset.bindingRangeIndex < 0)
                return SLANG_E_INVALID_ARG;
            auto layout = getLayout();
            if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
                return SLANG_E_INVALID_ARG;
            auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);
            auto resourceViewImpl = static_cast<ResourceViewImpl*>(textureView);
            ID3D12Device* d3dDevice = static_cast<DeviceImpl*>(getDevice())->m_device;
            d3dDevice->CopyDescriptorsSimple(
                1,
                m_resourceHeap.getCpuHandle(
                    m_descriptorSet.m_resourceTable +
                    bindingRange.binding.offsetInDescriptorTable.resource +
                    (int32_t)offset.bindingArrayIndex),
                resourceViewImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            auto samplerImpl = static_cast<SamplerStateImpl*>(sampler);
            d3dDevice->CopyDescriptorsSimple(
                1,
                m_samplerHeap.getCpuHandle(
                    m_descriptorSet.m_samplerTable +
                    bindingRange.binding.offsetInDescriptorTable.sampler +
                    (int32_t)offset.bindingArrayIndex),
                samplerImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
#endif
    m_version++;
    return SLANG_OK;
}

Result ShaderObjectImpl::init(
    DeviceImpl* device,
    ShaderObjectLayoutImpl* layout,
    DescriptorHeapReference viewHeap,
    DescriptorHeapReference samplerHeap)
{
    m_device = device;

    m_layout = layout;

    m_cachedTransientHeap = nullptr;
    m_cachedTransientHeapVersion = 0;
    m_isConstantBufferDirty = true;

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
    m_rootArguments.setCount(layout->getOwnUserRootParameterCount());
    memset(
        m_rootArguments.getBuffer(),
        0,
        sizeof(D3D12_GPU_VIRTUAL_ADDRESS) * m_rootArguments.getCount());
    // Each shader object will own CPU descriptor heap memory
    // for any resource or sampler descriptors it might store
    // as part of its value.
    //
    // This allocate includes a reservation for any constant
    // buffer descriptor pertaining to the ordinary data,
    // but does *not* include any descriptors that are managed
    // as part of sub-objects.
    //
    if (auto resourceCount = layout->getResourceSlotCount())
    {
        m_descriptorSet.resourceTable.allocate(viewHeap, resourceCount);

        // We must also ensure that the memory for any resources
        // referenced by descriptors in this object does not get
        // freed while the object is still live.
        //
        m_boundResources.setCount(resourceCount);
    }
    if (auto samplerCount = layout->getSamplerSlotCount())
    {
        m_descriptorSet.samplerTable.allocate(samplerHeap, samplerCount);
    }

    // If the layout specifies that we have any sub-objects, then
    // we need to size the array to account for them.
    //
    Index subObjectCount = layout->getSubObjectSlotCount();
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

        auto& bindingRangeInfo = layout->getBindingRange(subObjectRangeInfo.bindingRangeIndex);
        for (uint32_t i = 0; i < bindingRangeInfo.count; ++i)
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

Result ShaderObjectImpl::_writeOrdinaryData(
    PipelineCommandEncoder* encoder,
    BufferResourceImpl* buffer,
    size_t offset,
    size_t destSize,
    ShaderObjectLayoutImpl* specializedLayout)
{
    auto src = m_data.getBuffer();
    auto srcSize = size_t(m_data.getCount());

    SLANG_ASSERT(srcSize <= destSize);

    uploadBufferDataImpl(
        encoder->m_device,
        encoder->m_d3dCmdList,
        encoder->m_transientHeap,
        buffer,
        offset,
        srcSize,
        src);

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

        for (uint32_t i = 0; i < count; ++i)
        {
            auto subObject = m_objects[bindingRangeInfo.subObjectIndex + i];

            RefPtr<ShaderObjectLayoutImpl> subObjectLayout;
            SLANG_RETURN_ON_FAIL(subObject->getSpecializedLayout(subObjectLayout.writeRef()));

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

bool ShaderObjectImpl::shouldAllocateConstantBuffer(TransientResourceHeapImpl* transientHeap)
{
    if (m_isConstantBufferDirty || m_cachedTransientHeap != transientHeap ||
        m_cachedTransientHeapVersion != transientHeap->getVersion())
    {
        return true;
    }
    return false;
}

/// Ensure that the `m_ordinaryDataBuffer` has been created, if it is needed

Result ShaderObjectImpl::_ensureOrdinaryDataBufferCreatedIfNeeded(
    PipelineCommandEncoder* encoder, ShaderObjectLayoutImpl* specializedLayout)
{
    // If data has been changed since last allocation/filling of constant buffer,
    // we will need to allocate a new one.
    //
    if (!shouldAllocateConstantBuffer(encoder->m_transientHeap))
    {
        return SLANG_OK;
    }
    m_isConstantBufferDirty = false;
    m_cachedTransientHeap = encoder->m_transientHeap;
    m_cachedTransientHeapVersion = encoder->m_transientHeap->getVersion();

    // Computing the size of the ordinary data buffer is *not* just as simple
    // as using the size of the `m_ordinayData` array that we store. The reason
    // for the added complexity is that interface-type fields may lead to the
    // storage being specialized such that it needs extra appended data to
    // store the concrete values that logically belong in those interface-type
    // fields but wouldn't fit in the fixed-size allocation we gave them.
    //
    m_constantBufferSize = specializedLayout->getTotalOrdinaryDataSize();
    if (m_constantBufferSize == 0)
    {
        return SLANG_OK;
    }

    // Once we have computed how large the buffer should be, we can allocate
    // it from the transient resource heap.
    //
    auto alignedConstantBufferSize = D3DUtil::calcAligned(m_constantBufferSize, 256);
    SLANG_RETURN_ON_FAIL(encoder->m_commandBuffer->m_transientHeap->allocateConstantBuffer(
        alignedConstantBufferSize, m_constantBufferWeakPtr, m_constantBufferOffset));

    // Once the buffer is allocated, we can use `_writeOrdinaryData` to fill it in.
    //
    // Note that `_writeOrdinaryData` is potentially recursive in the case
    // where this object contains interface/existential-type fields, so we
    // don't need or want to inline it into this call site.
    //
    SLANG_RETURN_ON_FAIL(_writeOrdinaryData(
        encoder,
        static_cast<BufferResourceImpl*>(m_constantBufferWeakPtr),
        m_constantBufferOffset,
        m_constantBufferSize,
        specializedLayout));

    {
        // We also create and store a descriptor for our root constant buffer
        // into the descriptor table allocation that was reserved for them.
        //
        // We always know that the ordinary data buffer will be the first descriptor
        // in the table of resource views.
        //
        auto descriptorTable = m_descriptorSet.resourceTable;
        D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
        viewDesc.BufferLocation = static_cast<BufferResourceImpl*>(m_constantBufferWeakPtr)
                                      ->m_resource.getResource()
                                      ->GetGPUVirtualAddress() +
                                  m_constantBufferOffset;
        viewDesc.SizeInBytes = (UINT)alignedConstantBufferSize;
        encoder->m_device->CreateConstantBufferView(&viewDesc, descriptorTable.getCpuHandle());
    }

    return SLANG_OK;
}

void ShaderObjectImpl::updateSubObjectsRecursive()
{
    if (!m_isMutable)
        return;
    auto& subObjectRanges = getLayout()->getSubObjectRanges();
    for (Slang::Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRanges.getCount();
         subObjectRangeIndex++)
    {
        auto const& subObjectRange = subObjectRanges[subObjectRangeIndex];
        auto const& bindingRange = getLayout()->getBindingRange(subObjectRange.bindingRangeIndex);
        Slang::Index count = bindingRange.count;

        for (Slang::Index subObjectIndexInRange = 0; subObjectIndexInRange < count;
             subObjectIndexInRange++)
        {
            Slang::Index objectIndex = bindingRange.subObjectIndex + subObjectIndexInRange;
            auto subObject = m_objects[objectIndex].Ptr();
            if (!subObject)
                continue;
            subObject->updateSubObjectsRecursive();
            if (m_subObjectVersions[objectIndex] != m_objects[objectIndex]->m_version)
            {
                ShaderOffset offset;
                offset.bindingRangeIndex = subObjectRange.bindingRangeIndex;
                offset.bindingArrayIndex = subObjectIndexInRange;
                setObject(offset, subObject);
            }
        }
    }
}

/// Prepare to bind this object as a parameter block.
///
/// This involves allocating and binding any descriptor tables necessary
/// to to store the state of the object. The function returns a descriptor
/// set formed from any table(s) allocated. In addition, the `ioOffset`
/// parameter will be adjusted to be correct for binding values into
/// the resulting descriptor set.
///
/// Returns:
///   SLANG_OK when successful,
///   SLANG_E_OUT_OF_MEMORY when descriptor heap is full.
///

Result ShaderObjectImpl::prepareToBindAsParameterBlock(
    BindingContext* context,
    BindingOffset& ioOffset,
    ShaderObjectLayoutImpl* specializedLayout,
    DescriptorSet& outDescriptorSet)
{
    auto transientHeap = context->transientHeap;
    auto submitter = context->submitter;

    // When writing into the new descriptor set, resource and sampler
    // descriptors will need to start at index zero in the respective
    // tables.
    //
    ioOffset.resource = 0;
    ioOffset.sampler = 0;

    // The index of the next root parameter to bind will be maintained,
    // but needs to be incremented by the number of descriptor tables
    // we allocate (zero or one resource table and zero or one sampler
    // table).
    //
    auto& rootParamIndex = ioOffset.rootParam;

    if (auto descriptorCount = specializedLayout->getTotalResourceDescriptorCount())
    {
        // There is a non-zero number of resource descriptors needed,
        // so we will allocate a table out of the appropriate heap,
        // and store it into the appropriate part of `descriptorSet`.
        //
        auto descriptorHeap = &transientHeap->getCurrentViewHeap();
        auto& table = outDescriptorSet.resourceTable;

        // Allocate the table.
        //
        if (!table.allocate(descriptorHeap, descriptorCount))
        {
            context->outOfMemoryHeap = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            return SLANG_E_OUT_OF_MEMORY;
        }

        // Bind the table to the pipeline, consuming the next available
        // root parameter.
        //
        auto tableRootParamIndex = rootParamIndex++;
        submitter->setRootDescriptorTable(tableRootParamIndex, table.getGpuHandle());
    }
    if (auto descriptorCount = specializedLayout->getTotalSamplerDescriptorCount())
    {
        // There is a non-zero number of sampler descriptors needed,
        // so we will allocate a table out of the appropriate heap,
        // and store it into the appropriate part of `descriptorSet`.
        //
        auto descriptorHeap = &transientHeap->getCurrentSamplerHeap();
        auto& table = outDescriptorSet.samplerTable;

        // Allocate the table.
        //
        if (!table.allocate(descriptorHeap, descriptorCount))
        {
            context->outOfMemoryHeap = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            return SLANG_E_OUT_OF_MEMORY;
        }

        // Bind the table to the pipeline, consuming the next available
        // root parameter.
        //
        auto tableRootParamIndex = rootParamIndex++;
        submitter->setRootDescriptorTable(tableRootParamIndex, table.getGpuHandle());
    }

    return SLANG_OK;
}

bool ShaderObjectImpl::checkIfCachedDescriptorSetIsValidRecursive(BindingContext* context)
{
    if (shouldAllocateConstantBuffer(context->transientHeap))
        return false;
    if (m_isMutable && m_version != m_cachedGPUDescriptorSetVersion)
        return false;
    if (m_cachedGPUDescriptorSet.resourceTable.getDescriptorCount() != 0 &&
        m_cachedGPUDescriptorSet.resourceTable.m_heap.ptr.linearHeap->getHeap() !=
            m_cachedTransientHeap->getCurrentViewHeap().getHeap())
        return false;
    if (m_cachedGPUDescriptorSet.samplerTable.getDescriptorCount() != 0 &&
        m_cachedGPUDescriptorSet.samplerTable.m_heap.ptr.linearHeap->getHeap() !=
            m_cachedTransientHeap->getCurrentSamplerHeap().getHeap())
        return false;

    auto& subObjectRanges = getLayout()->getSubObjectRanges();
    for (Slang::Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRanges.getCount();
         subObjectRangeIndex++)
    {
        auto const& subObjectRange = subObjectRanges[subObjectRangeIndex];
        auto const& bindingRange = getLayout()->getBindingRange(subObjectRange.bindingRangeIndex);
        if (bindingRange.bindingType != slang::BindingType::ParameterBlock)
            continue;
        Slang::Index count = bindingRange.count;

        for (Slang::Index subObjectIndexInRange = 0; subObjectIndexInRange < count;
             subObjectIndexInRange++)
        {
            Slang::Index objectIndex = bindingRange.subObjectIndex + subObjectIndexInRange;
            auto subObject = m_objects[objectIndex].Ptr();
            if (!subObject)
                continue;
            if (subObject->checkIfCachedDescriptorSetIsValidRecursive(context))
                return false;
        }
    }
    return true;
}

/// Bind this object as a `ParameterBlock<X>`

Result ShaderObjectImpl::bindAsParameterBlock(
    BindingContext* context, BindingOffset const& offset, ShaderObjectLayoutImpl* specializedLayout)
{
    if (checkIfCachedDescriptorSetIsValidRecursive(context))
    {
        // If we already have a valid gpu descriptor table in the current
        // heap, bind it.
        auto rootParamIndex = offset.rootParam;
        if (m_cachedGPUDescriptorSet.resourceTable.getDescriptorCount())
        {
            auto tableRootParamIndex = rootParamIndex++;
            context->submitter->setRootDescriptorTable(
                tableRootParamIndex, m_cachedGPUDescriptorSet.resourceTable.getGpuHandle());
        }
        if (m_cachedGPUDescriptorSet.samplerTable.getDescriptorCount())
        {
            auto tableRootParamIndex = rootParamIndex++;
            context->submitter->setRootDescriptorTable(
                tableRootParamIndex, m_cachedGPUDescriptorSet.samplerTable.getGpuHandle());
        }
        return SLANG_OK;
    }

    // The first step to binding an object as a parameter block is to allocate a descriptor
    // set (consisting of zero or one resource descriptor table and zero or one sampler
    // descriptor table) to represent its values.
    //
    BindingOffset subOffset = offset;
    SLANG_RETURN_ON_FAIL(prepareToBindAsParameterBlock(
        context, /* inout */ subOffset, specializedLayout, m_cachedGPUDescriptorSet));

    // Next we bind the object into that descriptor set as if it were being used
    // as a `ConstantBuffer<X>`.
    //
    SLANG_RETURN_ON_FAIL(
        bindAsConstantBuffer(context, m_cachedGPUDescriptorSet, subOffset, specializedLayout));

    m_cachedGPUDescriptorSetVersion = m_version;
    return SLANG_OK;
}

/// Bind this object as a `ConstantBuffer<X>`

Result ShaderObjectImpl::bindAsConstantBuffer(
    BindingContext* context,
    DescriptorSet const& descriptorSet,
    BindingOffset const& offset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    // If we are to bind as a constant buffer we first need to ensure that
    // the ordinary data buffer is created, if this object needs one.
    //
    SLANG_RETURN_ON_FAIL(
        _ensureOrdinaryDataBufferCreatedIfNeeded(context->encoder, specializedLayout));

    // Next, we need to bind all of the resource descriptors for this object
    // (including any ordinary data buffer) into the provided `descriptorSet`.
    //
    auto resourceCount = specializedLayout->getResourceSlotCount();
    if (resourceCount)
    {
        auto& dstTable = descriptorSet.resourceTable;
        auto& srcTable = m_descriptorSet.resourceTable;

        context->device->m_device->CopyDescriptorsSimple(
            UINT(resourceCount),
            dstTable.getCpuHandle(offset.resource),
            srcTable.getCpuHandle(),
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Finally, we delegate to `_bindImpl` to bind samplers and sub-objects,
    // since the logic is shared with the `bindAsValue()` case below.
    //
    SLANG_RETURN_ON_FAIL(_bindImpl(context, descriptorSet, offset, specializedLayout));
    return SLANG_OK;
}

/// Bind this object as a value (for an interface-type parameter)

Result ShaderObjectImpl::bindAsValue(
    BindingContext* context,
    DescriptorSet const& descriptorSet,
    BindingOffset const& offset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    // When binding a value for an interface-type field we do *not* want
    // to bind a buffer for the ordinary data (if there is any) because
    // ordinary data for interface-type fields gets allocated into the
    // parent object's ordinary data buffer.
    //
    // This CPU-memory descriptor table that holds resource descriptors
    // will have already been allocated to have space for an ordinary data
    // buffer (if needed), so we need to take care to skip over that
    // descriptor when copying descriptors from the CPU-memory set
    // to the GPU-memory `descriptorSet`.
    //
    auto skipResourceCount = specializedLayout->getOrdinaryDataBufferCount();
    auto resourceCount = specializedLayout->getResourceSlotCount() - skipResourceCount;
    if (resourceCount)
    {
        auto& dstTable = descriptorSet.resourceTable;
        auto& srcTable = m_descriptorSet.resourceTable;

        context->device->m_device->CopyDescriptorsSimple(
            UINT(resourceCount),
            dstTable.getCpuHandle(offset.resource),
            srcTable.getCpuHandle(skipResourceCount),
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Finally, we delegate to `_bindImpl` to bind samplers and sub-objects,
    // since the logic is shared with the `bindAsConstantBuffer()` case above.
    //
    // Note: Just like we had to do some subtle handling of the ordinary data buffer
    // above, here we need to contend with the fact that the `offset.resource` fields
    // computed for sub-object ranges were baked to take the ordinary data buffer
    // into account, so that if `skipResourceCount` is non-zero then they are all
    // too high by `skipResourceCount`.
    //
    // We will address the problem here by computing a modified offset that adjusts
    // for the ordinary data buffer that we have not bound after all.
    //
    BindingOffset subOffset = offset;
    subOffset.resource -= skipResourceCount;
    SLANG_RETURN_ON_FAIL(_bindImpl(context, descriptorSet, subOffset, specializedLayout));
    return SLANG_OK;
}

/// Shared logic for `bindAsConstantBuffer()` and `bindAsValue()`

Result ShaderObjectImpl::_bindImpl(
    BindingContext* context,
    DescriptorSet const& descriptorSet,
    BindingOffset const& offset,
    ShaderObjectLayoutImpl* specializedLayout)
{
    // We start by binding all the sampler decriptors, if needed.
    //
    // Note: resource descriptors were handled in either `bindAsConstantBuffer()`
    // or `bindAsValue()` before calling into `_bindImpl()`.
    //
    if (auto samplerCount = specializedLayout->getSamplerSlotCount())
    {
        auto& dstTable = descriptorSet.samplerTable;
        auto& srcTable = m_descriptorSet.samplerTable;

        context->device->m_device->CopyDescriptorsSimple(
            UINT(samplerCount),
            dstTable.getCpuHandle(offset.sampler),
            srcTable.getCpuHandle(),
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }

    // Next we iterate over the sub-object ranges and bind anything they require.
    //
    auto& subObjectRanges = specializedLayout->getSubObjectRanges();
    auto subObjectRangeCount = subObjectRanges.getCount();
    for (Index i = 0; i < subObjectRangeCount; i++)
    {
        auto& subObjectRange = specializedLayout->getSubObjectRange(i);
        auto& bindingRange = specializedLayout->getBindingRange(subObjectRange.bindingRangeIndex);
        auto subObjectIndex = bindingRange.subObjectIndex;
        auto subObjectLayout = subObjectRange.layout.Ptr();

        BindingOffset rangeOffset = offset;
        rangeOffset += subObjectRange.offset;

        BindingOffset rangeStride = subObjectRange.stride;

        switch (bindingRange.bindingType)
        {
        case slang::BindingType::ConstantBuffer:
            {
                auto objOffset = rangeOffset;
                for (uint32_t j = 0; j < bindingRange.count; j++)
                {
                    auto& object = m_objects[subObjectIndex + j];
                    SLANG_RETURN_ON_FAIL(object->bindAsConstantBuffer(
                        context, descriptorSet, objOffset, subObjectLayout));
                    objOffset += rangeStride;
                }
            }
            break;

        case slang::BindingType::ParameterBlock:
            {
                auto objOffset = rangeOffset;
                for (uint32_t j = 0; j < bindingRange.count; j++)
                {
                    auto& object = m_objects[subObjectIndex + j];
                    SLANG_RETURN_ON_FAIL(
                        object->bindAsParameterBlock(context, objOffset, subObjectLayout));
                    objOffset += rangeStride;
                }
            }
            break;

        case slang::BindingType::ExistentialValue:
            if (subObjectLayout)
            {
                auto objOffset = rangeOffset;
                for (uint32_t j = 0; j < bindingRange.count; j++)
                {
                    auto& object = m_objects[subObjectIndex + j];
                    SLANG_RETURN_ON_FAIL(
                        object->bindAsValue(context, descriptorSet, objOffset, subObjectLayout));
                    objOffset += rangeStride;
                }
            }
            break;
        }
    }

    return SLANG_OK;
}

Result ShaderObjectImpl::bindRootArguments(BindingContext* context, uint32_t& index)
{
    auto layoutImpl = getLayout();
    for (Index i = 0; i < m_rootArguments.getCount(); i++)
    {
        switch (layoutImpl->getRootParameterInfo(i).type)
        {
        case IResourceView::Type::ShaderResource:
        case IResourceView::Type::AccelerationStructure:
            context->submitter->setRootSRV(index, m_rootArguments[i]);
            break;
        case IResourceView::Type::UnorderedAccess:
            context->submitter->setRootUAV(index, m_rootArguments[i]);
            break;
        default:
            continue;
        }
        index++;
    }
    for (auto& subObject : m_objects)
    {
        if (subObject)
        {
            SLANG_RETURN_ON_FAIL(subObject->bindRootArguments(context, index));
        }
    }
    return SLANG_OK;
}

/// Get the layout of this shader object with specialization arguments considered
///
/// This operation should only be called after the shader object has been
/// fully filled in and finalized.
///

Result ShaderObjectImpl::getSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
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
/// This operation is virtual so that it can be customized by `RootShaderObject`.
///

Result ShaderObjectImpl::_createSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
{
    ExtendedShaderObjectType extendedType;
    SLANG_RETURN_ON_FAIL(getSpecializedShaderObjectType(&extendedType));

    auto renderer = getRenderer();
    RefPtr<ShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(renderer->getShaderObjectLayout(
        extendedType.slangType,
        m_layout->getContainerType(),
        (ShaderObjectLayoutBase**)layout.writeRef()));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result ShaderObjectImpl::setResource(ShaderOffset const& offset, IResourceView* resourceView)
{
    if (offset.bindingRangeIndex < 0)
        return SLANG_E_INVALID_ARG;
    auto layout = getLayout();
    if (offset.bindingRangeIndex >= layout->getBindingRangeCount())
        return SLANG_E_INVALID_ARG;

    m_version++;

    ID3D12Device* d3dDevice = static_cast<DeviceImpl*>(getDevice())->m_device;

    auto& bindingRange = layout->getBindingRange(offset.bindingRangeIndex);

    if (bindingRange.isRootParameter && resourceView)
    {
        auto& rootArg = m_rootArguments[bindingRange.baseIndex];
        switch (resourceView->getViewDesc()->type)
        {
        case IResourceView::Type::AccelerationStructure:
            {
                auto resourceViewImpl = static_cast<AccelerationStructureImpl*>(resourceView);
                rootArg = resourceViewImpl->getDeviceAddress();
            }
            break;
        case IResourceView::Type::ShaderResource:
        case IResourceView::Type::UnorderedAccess:
            {
                auto resourceViewImpl = static_cast<ResourceViewImpl*>(resourceView);
                if (resourceViewImpl->m_resource->isBuffer())
                {
                    rootArg = static_cast<BufferResourceImpl*>(resourceViewImpl->m_resource.Ptr())
                                  ->getDeviceAddress();
                }
                else
                {
                    getDebugCallback()->handleMessage(
                        DebugMessageType::Error,
                        DebugMessageSource::Layer,
                        "The shader parameter at the specified offset is a root parameter, and "
                        "therefore can only be a buffer view.");
                    return SLANG_FAIL;
                }
            }
            break;
        }
        return SLANG_OK;
    }

    if (resourceView == nullptr)
    {
        // Create null descriptor for the binding.
        auto destDescriptor = m_descriptorSet.resourceTable.getCpuHandle(
            bindingRange.baseIndex + (int32_t)offset.bindingArrayIndex);
        return createNullDescriptor(d3dDevice, destDescriptor, bindingRange);
    }

    ResourceViewInternalImpl* internalResourceView = nullptr;
    switch (resourceView->getViewDesc()->type)
    {
#if SLANG_GFX_HAS_DXR_SUPPORT
    case IResourceView::Type::AccelerationStructure:
        {
            auto asImpl = static_cast<AccelerationStructureImpl*>(resourceView);
            // Hold a reference to the resource to prevent its destruction.
            m_boundResources[bindingRange.baseIndex + offset.bindingArrayIndex] = asImpl->m_buffer;
            internalResourceView = asImpl;
        }
        break;
#endif
    default:
        {
            auto resourceViewImpl = static_cast<ResourceViewImpl*>(resourceView);
            // Hold a reference to the resource to prevent its destruction.
            m_boundResources[bindingRange.baseIndex + offset.bindingArrayIndex] =
                resourceViewImpl->m_resource;
            internalResourceView = resourceViewImpl;
        }
        break;
    }

    auto descriptorSlotIndex = bindingRange.baseIndex + (int32_t)offset.bindingArrayIndex;
    if (internalResourceView->m_descriptor.cpuHandle.ptr)
    {
        d3dDevice->CopyDescriptorsSimple(
            1,
            m_descriptorSet.resourceTable.getCpuHandle(
                bindingRange.baseIndex + (int32_t)offset.bindingArrayIndex),
            internalResourceView->m_descriptor.cpuHandle,
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
    else
    {
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "IShaderObject::setResource: the resource view cannot be set to this shader parameter. "
            "A possible reason is that the view is too large to be supported by D3D12.");
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

void PipelineStateImpl::init(const GraphicsPipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Graphics;
    pipelineDesc.graphics = inDesc;
    initializeBase(pipelineDesc);
}

void PipelineStateImpl::init(const ComputePipelineStateDesc& inDesc)
{
    PipelineStateDesc pipelineDesc;
    pipelineDesc.type = PipelineType::Compute;
    pipelineDesc.compute = inDesc;
    initializeBase(pipelineDesc);
}

Result PipelineStateImpl::getNativeHandle(InteropHandle* outHandle)
{
    SLANG_RETURN_ON_FAIL(ensureAPIPipelineStateCreated());
    outHandle->api = InteropHandleAPI::D3D12;
    outHandle->handleValue = reinterpret_cast<uint64_t>(m_pipelineState.get());
    return SLANG_OK;
}

Result PipelineStateImpl::ensureAPIPipelineStateCreated()
{
    if (m_pipelineState)
        return SLANG_OK;

    auto programImpl = static_cast<ShaderProgramImpl*>(m_program.Ptr());
    if (programImpl->m_shaders.getCount() == 0)
    {
        SLANG_RETURN_ON_FAIL(programImpl->compileShaders());
    }
    if (desc.type == PipelineType::Graphics)
    {
        // Only actually create a D3D12 pipeline state if the pipeline is fully specialized.
        auto inputLayoutImpl = (InputLayoutImpl*)desc.graphics.inputLayout;

        // Describe and create the graphics pipeline state object (PSO)
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

        psoDesc.pRootSignature = programImpl->m_rootObjectLayout->m_rootSignature;

        for (auto& shaderBin : programImpl->m_shaders)
        {
            switch (shaderBin.stage)
            {
            case SLANG_STAGE_VERTEX:
                psoDesc.VS = {shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount())};
                break;
            case SLANG_STAGE_FRAGMENT:
                psoDesc.PS = {shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount())};
                break;
            case SLANG_STAGE_DOMAIN:
                psoDesc.DS = {shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount())};
                break;
            case SLANG_STAGE_HULL:
                psoDesc.HS = {shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount())};
                break;
            case SLANG_STAGE_GEOMETRY:
                psoDesc.GS = {shaderBin.code.getBuffer(), SIZE_T(shaderBin.code.getCount())};
                break;
            default:
                getDebugCallback()->handleMessage(
                    DebugMessageType::Error,
                    DebugMessageSource::Layer,
                    "Unsupported shader stage.");
                return SLANG_E_NOT_AVAILABLE;
            }
        }

        if (inputLayoutImpl)
        {
            psoDesc.InputLayout = {
                inputLayoutImpl->m_elements.getBuffer(),
                UINT(inputLayoutImpl->m_elements.getCount())};
        }

        psoDesc.PrimitiveTopologyType = D3DUtil::getPrimitiveType(desc.graphics.primitiveType);

        {
            auto framebufferLayout =
                static_cast<FramebufferLayoutImpl*>(desc.graphics.framebufferLayout);
            const int numRenderTargets = int(framebufferLayout->m_renderTargets.getCount());

            if (framebufferLayout->m_hasDepthStencil)
            {
                psoDesc.DSVFormat = D3DUtil::getMapFormat(framebufferLayout->m_depthStencil.format);
                psoDesc.SampleDesc.Count = framebufferLayout->m_depthStencil.sampleCount;
            }
            else
            {
                psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
                if (framebufferLayout->m_renderTargets.getCount())
                {
                    psoDesc.SampleDesc.Count = framebufferLayout->m_renderTargets[0].sampleCount;
                }
            }
            psoDesc.NumRenderTargets = numRenderTargets;
            for (Int i = 0; i < numRenderTargets; i++)
            {
                psoDesc.RTVFormats[i] =
                    D3DUtil::getMapFormat(framebufferLayout->m_renderTargets[i].format);
            }

            psoDesc.SampleDesc.Quality = 0;
            psoDesc.SampleMask = UINT_MAX;
        }

        {
            auto& rs = psoDesc.RasterizerState;
            rs.FillMode = D3DUtil::getFillMode(desc.graphics.rasterizer.fillMode);
            rs.CullMode = D3DUtil::getCullMode(desc.graphics.rasterizer.cullMode);
            rs.FrontCounterClockwise =
                desc.graphics.rasterizer.frontFace == gfx::FrontFaceMode::CounterClockwise ? TRUE
                                                                                           : FALSE;
            rs.DepthBias = desc.graphics.rasterizer.depthBias;
            rs.DepthBiasClamp = desc.graphics.rasterizer.depthBiasClamp;
            rs.SlopeScaledDepthBias = desc.graphics.rasterizer.slopeScaledDepthBias;
            rs.DepthClipEnable = desc.graphics.rasterizer.depthClipEnable ? TRUE : FALSE;
            rs.MultisampleEnable = desc.graphics.rasterizer.multisampleEnable ? TRUE : FALSE;
            rs.AntialiasedLineEnable =
                desc.graphics.rasterizer.antialiasedLineEnable ? TRUE : FALSE;
            rs.ForcedSampleCount = desc.graphics.rasterizer.forcedSampleCount;
            rs.ConservativeRaster = desc.graphics.rasterizer.enableConservativeRasterization
                                        ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON
                                        : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        {
            D3D12_BLEND_DESC& blend = psoDesc.BlendState;
            blend.IndependentBlendEnable = FALSE;
            blend.AlphaToCoverageEnable = desc.graphics.blend.alphaToCoverageEnable ? TRUE : FALSE;
            blend.RenderTarget[0].RenderTargetWriteMask = (uint8_t)RenderTargetWriteMask::EnableAll;
            for (uint32_t i = 0; i < desc.graphics.blend.targetCount; i++)
            {
                auto& d3dDesc = blend.RenderTarget[i];
                d3dDesc.BlendEnable = desc.graphics.blend.targets[i].enableBlend ? TRUE : FALSE;
                d3dDesc.BlendOp = D3DUtil::getBlendOp(desc.graphics.blend.targets[i].color.op);
                d3dDesc.BlendOpAlpha = D3DUtil::getBlendOp(desc.graphics.blend.targets[i].alpha.op);
                d3dDesc.DestBlend =
                    D3DUtil::getBlendFactor(desc.graphics.blend.targets[i].color.dstFactor);
                d3dDesc.DestBlendAlpha =
                    D3DUtil::getBlendFactor(desc.graphics.blend.targets[i].alpha.dstFactor);
                d3dDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
                d3dDesc.LogicOpEnable = FALSE;
                d3dDesc.RenderTargetWriteMask = desc.graphics.blend.targets[i].writeMask;
                d3dDesc.SrcBlend =
                    D3DUtil::getBlendFactor(desc.graphics.blend.targets[i].color.srcFactor);
                d3dDesc.SrcBlendAlpha =
                    D3DUtil::getBlendFactor(desc.graphics.blend.targets[i].alpha.srcFactor);
            }
            for (uint32_t i = 1; i < desc.graphics.blend.targetCount; i++)
            {
                if (memcmp(
                        &desc.graphics.blend.targets[i],
                        &desc.graphics.blend.targets[0],
                        sizeof(desc.graphics.blend.targets[0])) != 0)
                {
                    blend.IndependentBlendEnable = TRUE;
                    break;
                }
            }
            for (uint32_t i = (uint32_t)desc.graphics.blend.targetCount;
                 i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
                 ++i)
            {
                blend.RenderTarget[i] = blend.RenderTarget[0];
            }
        }

        {
            auto& ds = psoDesc.DepthStencilState;

            ds.DepthEnable = desc.graphics.depthStencil.depthTestEnable;
            ds.DepthWriteMask = desc.graphics.depthStencil.depthWriteEnable
                                    ? D3D12_DEPTH_WRITE_MASK_ALL
                                    : D3D12_DEPTH_WRITE_MASK_ZERO;
            ds.DepthFunc = D3DUtil::getComparisonFunc(desc.graphics.depthStencil.depthFunc);
            ds.StencilEnable = desc.graphics.depthStencil.stencilEnable;
            ds.StencilReadMask = (UINT8)desc.graphics.depthStencil.stencilReadMask;
            ds.StencilWriteMask = (UINT8)desc.graphics.depthStencil.stencilWriteMask;
            ds.FrontFace = D3DUtil::translateStencilOpDesc(desc.graphics.depthStencil.frontFace);
            ds.BackFace = D3DUtil::translateStencilOpDesc(desc.graphics.depthStencil.backFace);
        }

        psoDesc.PrimitiveTopologyType = D3DUtil::getPrimitiveType(desc.graphics.primitiveType);

        if (m_device->m_pipelineCreationAPIDispatcher)
        {
            SLANG_RETURN_ON_FAIL(
                m_device->m_pipelineCreationAPIDispatcher->createGraphicsPipelineState(
                    m_device,
                    programImpl->linkedProgram.get(),
                    &psoDesc,
                    (void**)m_pipelineState.writeRef()));
        }
        else
        {
            SLANG_RETURN_ON_FAIL(m_device->m_device->CreateGraphicsPipelineState(
                &psoDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));
        }
    }
    else
    {

        // Only actually create a D3D12 pipeline state if the pipeline is fully specialized.
        ComPtr<ID3D12PipelineState> pipelineState;
        if (!programImpl->isSpecializable())
        {
            // Describe and create the compute pipeline state object
            D3D12_COMPUTE_PIPELINE_STATE_DESC computeDesc = {};
            computeDesc.pRootSignature =
                desc.compute.d3d12RootSignatureOverride
                    ? static_cast<ID3D12RootSignature*>(desc.compute.d3d12RootSignatureOverride)
                    : programImpl->m_rootObjectLayout->m_rootSignature;
            computeDesc.CS = {
                programImpl->m_shaders[0].code.getBuffer(),
                SIZE_T(programImpl->m_shaders[0].code.getCount())};

#ifdef GFX_NVAPI
            if (m_nvapi)
            {
                // Also fill the extension structure.
                // Use the same UAV slot index and register space that are declared in the shader.

                // For simplicities sake we just use u0
                NVAPI_D3D12_PSO_SET_SHADER_EXTENSION_SLOT_DESC extensionDesc;
                extensionDesc.baseVersion = NV_PSO_EXTENSION_DESC_VER;
                extensionDesc.version = NV_SET_SHADER_EXTENSION_SLOT_DESC_VER;
                extensionDesc.uavSlot = 0;
                extensionDesc.registerSpace = 0;

                // Put the pointer to the extension into an array - there can be multiple extensions
                // enabled at once.
                const NVAPI_D3D12_PSO_EXTENSION_DESC* extensions[] = {&extensionDesc};

                // Now create the PSO.
                const NvAPI_Status nvapiStatus = NvAPI_D3D12_CreateComputePipelineState(
                    m_device->m_device,
                    &computeDesc,
                    SLANG_COUNT_OF(extensions),
                    extensions,
                    m_pipelineState.writeRef());

                if (nvapiStatus != NVAPI_OK)
                {
                    return SLANG_FAIL;
                }
            }
            else
#endif
            {
                if (m_device->m_pipelineCreationAPIDispatcher)
                {
                    SLANG_RETURN_ON_FAIL(
                        m_device->m_pipelineCreationAPIDispatcher->createComputePipelineState(
                            m_device,
                            programImpl->linkedProgram.get(),
                            &computeDesc,
                            (void**)m_pipelineState.writeRef()));
                }
                else
                {
                    SLANG_RETURN_ON_FAIL(m_device->m_device->CreateComputePipelineState(
                        &computeDesc, IID_PPV_ARGS(m_pipelineState.writeRef())));
                }
            }
        }
    }

    return SLANG_OK;
}

// Swapchain Implementation

Result SwapchainImpl::init(
    DeviceImpl* renderer, const ISwapchain::Desc& swapchainDesc, WindowHandle window)
{
    m_queue = static_cast<CommandQueueImpl*>(swapchainDesc.queue)->m_d3dQueue;
    m_dxgiFactory = renderer->m_deviceInfo.m_dxgiFactory;
    SLANG_RETURN_ON_FAIL(
        D3DSwapchainBase::init(swapchainDesc, window, DXGI_SWAP_EFFECT_FLIP_DISCARD));
    renderer->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.writeRef()));

    SLANG_RETURN_ON_FAIL(m_swapChain->QueryInterface(m_swapChain3.writeRef()));
    for (uint32_t i = 0; i < swapchainDesc.imageCount; i++)
    {
        m_frameEvents.add(CreateEventEx(
            nullptr,
            false,
            CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET,
            EVENT_ALL_ACCESS));
    }
    return SLANG_OK;
}

Result SwapchainImpl::resize(uint32_t width, uint32_t height)
{
    for (auto evt : m_frameEvents)
        SetEvent(evt);
    SLANG_RETURN_ON_FAIL(D3DSwapchainBase::resize(width, height));
    return SLANG_OK;
}

void SwapchainImpl::createSwapchainBufferImages()
{
    m_images.clear();

    for (uint32_t i = 0; i < m_desc.imageCount; i++)
    {
        ComPtr<ID3D12Resource> d3dResource;
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(d3dResource.writeRef()));
        ITextureResource::Desc imageDesc = {};
        imageDesc.allowedStates = ResourceStateSet(
            ResourceState::Present, ResourceState::RenderTarget, ResourceState::CopyDestination);
        imageDesc.type = IResource::Type::Texture2D;
        imageDesc.arraySize = 0;
        imageDesc.format = m_desc.format;
        imageDesc.size.width = m_desc.width;
        imageDesc.size.height = m_desc.height;
        imageDesc.size.depth = 1;
        imageDesc.numMipLevels = 1;
        imageDesc.defaultState = ResourceState::Present;
        RefPtr<TextureResourceImpl> image = new TextureResourceImpl(imageDesc);
        image->m_resource.setResource(d3dResource.get());
        image->m_defaultState = D3D12_RESOURCE_STATE_PRESENT;
        m_images.add(image);
    }
    for (auto evt : m_frameEvents)
        SetEvent(evt);
}

int SwapchainImpl::acquireNextImage()
{
    auto result = (int)m_swapChain3->GetCurrentBackBufferIndex();
    WaitForSingleObject(m_frameEvents[result], INFINITE);
    ResetEvent(m_frameEvents[result]);
    return result;
}

Result SwapchainImpl::present()
{
    m_fence->SetEventOnCompletion(
        fenceValue, m_frameEvents[m_swapChain3->GetCurrentBackBufferIndex()]);
    SLANG_RETURN_ON_FAIL(D3DSwapchainBase::present());
    fenceValue++;
    m_queue->Signal(m_fence, fenceValue);
    return SLANG_OK;
}

bool SwapchainImpl::isOccluded()
{
    return (m_swapChain3->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED);
}

Result SwapchainImpl::setFullScreenMode(bool mode)
{
    return m_swapChain3->SetFullscreenState(mode, nullptr);
}

// CommandQueue implementation.

Result CommandQueueImpl::init(DeviceImpl* device, uint32_t queueIndex)
{
    m_queueIndex = queueIndex;
    m_renderer = device;
    m_device = device->m_device;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    SLANG_RETURN_ON_FAIL(
        m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_d3dQueue.writeRef())));
    SLANG_RETURN_ON_FAIL(
        m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.writeRef())));
    globalWaitHandle = CreateEventEx(
        nullptr, nullptr, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
    return SLANG_OK;
}

CommandQueueImpl::~CommandQueueImpl()
{
    waitOnHost();
    CloseHandle(globalWaitHandle);
    m_renderer->m_queueIndexAllocator.free((int)m_queueIndex, 1);
}

void CommandQueueImpl::executeCommandBuffers(
    uint32_t count, ICommandBuffer* const* commandBuffers, IFence* fence, uint64_t valueToSignal)
{
    ShortList<ID3D12CommandList*> commandLists;
    for (uint32_t i = 0; i < count; i++)
    {
        auto cmdImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
        commandLists.add(cmdImpl->m_cmdList);
    }
    if (count > 0)
    {
        m_d3dQueue->ExecuteCommandLists((UINT)count, commandLists.getArrayView().getBuffer());

        m_fenceValue++;

        for (uint32_t i = 0; i < count; i++)
        {
            if (i > 0 && commandBuffers[i] == commandBuffers[i - 1])
                continue;
            auto cmdImpl = static_cast<CommandBufferImpl*>(commandBuffers[i]);
            auto transientHeap = cmdImpl->m_transientHeap;
            auto& waitInfo = transientHeap->getQueueWaitInfo(m_queueIndex);
            waitInfo.waitValue = m_fenceValue;
            waitInfo.fence = m_fence;
            waitInfo.queue = m_d3dQueue;
        }
    }

    if (fence)
    {
        auto fenceImpl = static_cast<FenceImpl*>(fence);
        m_d3dQueue->Signal(fenceImpl->m_fence.get(), valueToSignal);
    }
}

void CommandQueueImpl::waitOnHost()
{
    m_fenceValue++;
    m_d3dQueue->Signal(m_fence, m_fenceValue);
    ResetEvent(globalWaitHandle);
    m_fence->SetEventOnCompletion(m_fenceValue, globalWaitHandle);
    WaitForSingleObject(globalWaitHandle, INFINITE);
}

Result CommandQueueImpl::waitForFenceValuesOnDevice(
    uint32_t fenceCount, IFence** fences, uint64_t* waitValues)
{
    for (uint32_t i = 0; i < fenceCount; ++i)
    {
        auto fenceImpl = static_cast<FenceImpl*>(fences[i]);
        m_d3dQueue->Wait(fenceImpl->m_fence.get(), waitValues[i]);
    }
    return SLANG_OK;
}

const CommandQueueImpl::Desc& CommandQueueImpl::getDesc() { return m_desc; }

ICommandQueue* CommandQueueImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandQueue)
        return static_cast<ICommandQueue*>(this);
    return nullptr;
}

Result CommandQueueImpl::getNativeHandle(InteropHandle* handle)
{
    handle->api = InteropHandleAPI::D3D12;
    handle->handleValue = (uint64_t)m_d3dQueue.get();
    return SLANG_OK;
}

ResourceViewInternalImpl::~ResourceViewInternalImpl()
{
    if (m_descriptor.cpuHandle.ptr)
        m_allocator->free(m_descriptor);
}

Result ResourceViewImpl::getNativeHandle(InteropHandle* outHandle)
{
    outHandle->api = InteropHandleAPI::D3D12CpuDescriptorHandle;
    outHandle->handleValue = m_descriptor.cpuHandle.ptr;
    return SLANG_OK;
}

void RenderPassLayoutImpl::init(const IRenderPassLayout::Desc& desc)
{
    SimpleRenderPassLayout::init(desc);
    m_framebufferLayout = static_cast<FramebufferLayoutImpl*>(desc.framebufferLayout);
    m_hasDepthStencil = m_framebufferLayout->m_hasDepthStencil;
}

ShaderObjectLayoutImpl::SubObjectRangeOffset::SubObjectRangeOffset(
    slang::VariableLayoutReflection* varLayout)
{
    if (auto pendingLayout = varLayout->getPendingDataLayout())
    {
        pendingOrdinaryData = (uint32_t)pendingLayout->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM);
    }
}

ShaderObjectLayoutImpl::SubObjectRangeStride::SubObjectRangeStride(
    slang::TypeLayoutReflection* typeLayout)
{
    if (auto pendingLayout = typeLayout->getPendingDataTypeLayout())
    {
        pendingOrdinaryData = (uint32_t)pendingLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
    }
}

bool ShaderObjectLayoutImpl::isBindingRangeRootParameter(
    SlangSession* globalSession,
    const char* rootParameterAttributeName,
    slang::TypeLayoutReflection* typeLayout,
    Index bindingRangeIndex)
{
    bool isRootParameter = false;
    if (rootParameterAttributeName)
    {
        if (auto leafVariable = typeLayout->getBindingRangeLeafVariable(bindingRangeIndex))
        {
            if (leafVariable->findUserAttributeByName(globalSession, rootParameterAttributeName))
            {
                isRootParameter = true;
            }
        }
    }
    return isRootParameter;
}

Result ShaderObjectLayoutImpl::createForElementType(
    RendererBase* renderer,
    slang::TypeLayoutReflection* elementType,
    ShaderObjectLayoutImpl** outLayout)
{
    Builder builder(renderer);
    builder.setElementTypeLayout(elementType);
    return builder.build(outLayout);
}

Result ShaderObjectLayoutImpl::init(Builder* builder)
{
    auto renderer = builder->m_renderer;

    initBase(renderer, builder->m_elementTypeLayout);

    m_containerType = builder->m_containerType;

    m_bindingRanges = _Move(builder->m_bindingRanges);
    m_subObjectRanges = _Move(builder->m_subObjectRanges);
    m_rootParamsInfo = _Move(builder->m_rootParamsInfo);

    m_ownCounts = builder->m_ownCounts;
    m_totalCounts = builder->m_totalCounts;
    m_subObjectCount = builder->m_subObjectCount;
    m_childRootParameterCount = builder->m_childRootParameterCount;
    m_totalOrdinaryDataSize = builder->m_totalOrdinaryDataSize;

    return SLANG_OK;
}

Result ShaderObjectLayoutImpl::Builder::setElementTypeLayout(
    slang::TypeLayoutReflection* typeLayout)
{
    typeLayout = _unwrapParameterGroups(typeLayout, m_containerType);
    m_elementTypeLayout = typeLayout;

    // If the type contains any ordinary data, then we must reserve a buffer
    // descriptor to hold it when binding as a parameter block.
    //
    m_totalOrdinaryDataSize = (uint32_t)typeLayout->getSize();
    if (m_totalOrdinaryDataSize != 0)
    {
        m_ownCounts.resource++;
    }

    // We will scan over the reflected Slang binding ranges and add them
    // to our array. There are two main things we compute along the way:
    //
    // * For each binding range we compute a `flatIndex` that can be
    //   used to identify where the values for the given range begin
    //   in the flattened arrays (e.g., `m_objects`) and descriptor
    //   tables that hold the state of a shader object.
    //
    // * We also update the various counters taht keep track of the number
    //   of sub-objects, resources, samplers, etc. that are being
    //   consumed. These counters will contribute to figuring out
    //   the descriptor table(s) that might be needed to represent
    //   the object.
    //
    SlangInt bindingRangeCount = typeLayout->getBindingRangeCount();
    for (SlangInt r = 0; r < bindingRangeCount; ++r)
    {
        slang::BindingType slangBindingType = typeLayout->getBindingRangeType(r);
        uint32_t count = (uint32_t)typeLayout->getBindingRangeBindingCount(r);
        slang::TypeLayoutReflection* slangLeafTypeLayout =
            typeLayout->getBindingRangeLeafTypeLayout(r);
        BindingRangeInfo bindingRangeInfo = {};
        bindingRangeInfo.bindingType = slangBindingType;
        bindingRangeInfo.resourceShape = slangLeafTypeLayout->getResourceShape();
        bindingRangeInfo.count = count;
        bindingRangeInfo.isRootParameter = isBindingRangeRootParameter(
            m_renderer->slangContext.globalSession,
            static_cast<DeviceImpl*>(m_renderer)->m_extendedDesc.rootParameterShaderAttributeName,
            typeLayout,
            r);
        if (bindingRangeInfo.isRootParameter)
        {
            RootParameterInfo rootInfo = {};
            switch (slangBindingType)
            {
            case slang::BindingType::RayTracingAccelerationStructure:
                rootInfo.type = IResourceView::Type::AccelerationStructure;
                break;
            case slang::BindingType::RawBuffer:
            case slang::BindingType::TypedBuffer:
                rootInfo.type = IResourceView::Type::ShaderResource;
                break;
            case slang::BindingType::MutableRawBuffer:
            case slang::BindingType::MutableTypedBuffer:
                rootInfo.type = IResourceView::Type::UnorderedAccess;
                break;
            }
            bindingRangeInfo.baseIndex = (uint32_t)m_rootParamsInfo.getCount();
            for (uint32_t i = 0; i < count; i++)
            {
                m_rootParamsInfo.add(rootInfo);
            }
        }
        else
        {
            switch (slangBindingType)
            {
            case slang::BindingType::ConstantBuffer:
            case slang::BindingType::ParameterBlock:
            case slang::BindingType::ExistentialValue:
                bindingRangeInfo.baseIndex = m_subObjectCount;
                bindingRangeInfo.subObjectIndex = m_subObjectCount;
                m_subObjectCount += count;
                break;
            case slang::BindingType::RawBuffer:
            case slang::BindingType::MutableRawBuffer:
                if (slangLeafTypeLayout->getType()->getElementType() != nullptr)
                {
                    // A structured buffer occupies both a resource slot and
                    // a sub-object slot.
                    bindingRangeInfo.subObjectIndex = m_subObjectCount;
                    m_subObjectCount += count;
                }
                bindingRangeInfo.baseIndex = m_ownCounts.resource;
                m_ownCounts.resource += count;
                break;
            case slang::BindingType::Sampler:
                bindingRangeInfo.baseIndex = m_ownCounts.sampler;
                m_ownCounts.sampler += count;
                break;

            case slang::BindingType::CombinedTextureSampler:
                // TODO: support this case...
                break;

            case slang::BindingType::VaryingInput:
            case slang::BindingType::VaryingOutput:
                break;

            default:
                bindingRangeInfo.baseIndex = m_ownCounts.resource;
                m_ownCounts.resource += count;
                break;
            }
        }
        m_bindingRanges.add(bindingRangeInfo);
    }

    // At this point we've computed the number of resources/samplers that
    // the type needs to represent its *own* state, and stored those counts
    // in `m_ownCounts`. Next we need to consider any resources/samplers
    // and root parameters needed to represent the state of the transitive
    // sub-objects of this objet, so that we can compute the total size
    // of the object when bound to the pipeline.

    m_totalCounts = m_ownCounts;

    SlangInt subObjectRangeCount = typeLayout->getSubObjectRangeCount();
    for (SlangInt r = 0; r < subObjectRangeCount; ++r)
    {
        SlangInt bindingRangeIndex = typeLayout->getSubObjectRangeBindingRangeIndex(r);
        auto slangBindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
        auto count = (uint32_t)typeLayout->getBindingRangeBindingCount(bindingRangeIndex);
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
        if (slangBindingType == slang::BindingType::ExistentialValue)
        {
            if (auto pendingTypeLayout = slangLeafTypeLayout->getPendingDataTypeLayout())
            {
                createForElementType(m_renderer, pendingTypeLayout, subObjectLayout.writeRef());
            }
        }
        else
        {
            createForElementType(
                m_renderer,
                slangLeafTypeLayout->getElementTypeLayout(),
                subObjectLayout.writeRef());
        }

        SubObjectRangeInfo subObjectRange;
        subObjectRange.bindingRangeIndex = bindingRangeIndex;
        subObjectRange.layout = subObjectLayout;

        // The Slang reflection API stors offset information for sub-object ranges,
        // and we care about *some* of that information: in particular, we need
        // the offset of sub-objects in terms of uniform/ordinary data for the
        // cases where we need to fill in "pending" data in our ordinary buffer.
        //
        subObjectRange.offset = SubObjectRangeOffset(typeLayout->getSubObjectRangeOffset(r));
        subObjectRange.stride = SubObjectRangeStride(slangLeafTypeLayout);

        // The remaining offset information is computed based on the counters
        // we are generating here, which depend only on the in-memory layout
        // decisions being made in our implementation. Remember that the
        // `register` and `space` values coming from DXBC/DXIL do *not*
        // dictate the in-memory layout we use.
        //
        // Note: One subtle point here is that the `.rootParam` offset we are computing
        // here does *not* include any root parameters that would be allocated
        // for the parent object type itself (e.g., for descriptor tables
        // used if it were bound as a parameter block). The later logic when
        // we actually go to bind things will need to apply those offsets.
        //
        // Note: An even *more* subtle point is that the `.resource` offset
        // being computed here *does* include the resource descriptor allocated
        // for holding the ordinary data buffer, if any. The implications of
        // this for later offset math is subtle.
        //
        subObjectRange.offset.rootParam = m_childRootParameterCount;
        subObjectRange.offset.resource = m_totalCounts.resource;
        subObjectRange.offset.sampler = m_totalCounts.sampler;

        // Along with the offset information, we also need to compute the
        // "stride" between consecutive sub-objects in the range. The actual
        // size/stride of a single object depends on the type of range we
        // are dealing with.
        //
        BindingOffset objectCounts;
        switch (slangBindingType)
        {
        default:
            {
                // We only treat buffers of interface types as actual sub-object binding
                // range.
                auto bindingRangeTypeLayout =
                    typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
                if (!bindingRangeTypeLayout)
                    continue;
                auto elementType = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex)
                                       ->getElementTypeLayout();
                if (!elementType)
                    continue;
                if (elementType->getKind() != slang::TypeReflection::Kind::Interface)
                {
                    continue;
                }
            }
            break;

        case slang::BindingType::ConstantBuffer:
            {
                SLANG_ASSERT(subObjectLayout);

                // The resource and sampler descriptors of a nested
                // constant buffer will "leak" into those of the
                // parent type, and we need to account for them
                // whenever we allocate storage.
                //
                objectCounts.resource = subObjectLayout->getTotalResourceDescriptorCount();
                objectCounts.sampler = subObjectLayout->getTotalSamplerDescriptorCount();
                objectCounts.rootParam = subObjectRange.layout->getChildRootParameterCount();
            }
            break;

        case slang::BindingType::ParameterBlock:
            {
                SLANG_ASSERT(subObjectLayout);

                // In contrast to a constant buffer, a parameter block can hide
                // the resource and sampler descriptor allocation it uses (since they
                // are allocated into the tables that make up the parameter block.
                //
                // The only resource usage that leaks into the surrounding context
                // is the number of root parameters consumed.
                //
                objectCounts.rootParam = subObjectRange.layout->getTotalRootTableParameterCount();
            }
            break;

        case slang::BindingType::ExistentialValue:
            // An unspecialized existential/interface value cannot consume any resources
            // as part of the parent object (it needs to fit inside the fixed-size
            // represnetation of existential types).
            //
            // However, if we are statically specializing to a type that doesn't "fit"
            // we may need to account for additional information that needs to be
            // allocaated.
            //
            if (subObjectLayout)
            {
                // The ordinary data for an existential-type value is allocated into
                // the same buffer as the parent object, so we only want to consider
                // the resource descriptors *other than* the ordinary data buffer.
                //
                // Otherwise the logic here is identical to the constant buffer case.
                //
                objectCounts.resource =
                    subObjectLayout->getTotalResourceDescriptorCountWithoutOrdinaryDataBuffer();
                objectCounts.sampler = subObjectLayout->getTotalSamplerDescriptorCount();
                objectCounts.rootParam = subObjectRange.layout->getChildRootParameterCount();

                // Note: In the implementation for some other graphics API (e.g.,
                // Vulkan) there needs to be more work done to handle the fact that
                // "pending" data from interface-type sub-objects get allocated to a
                // distinct offset after all the "primary" data. We are consciously
                // ignoring that issue here, and the physical layout of a shader object
                // into the D3D12 binding state may end up interleaving
                // resources/samplers for "primary" and "pending" data.
                //
                // If this choice ever causes issues, we can revisit the approach here.

                // An interface-type range that includes ordinary data can
                // increase the size of the ordinary data buffer we need to
                // allocate for the parent object.
                //
                uint32_t ordinaryDataEnd =
                    subObjectRange.offset.pendingOrdinaryData +
                    (uint32_t)count * subObjectRange.stride.pendingOrdinaryData;

                if (ordinaryDataEnd > m_totalOrdinaryDataSize)
                {
                    m_totalOrdinaryDataSize = ordinaryDataEnd;
                }
            }
            break;
        }

        // Once we've computed the usage for each object in the range, we can
        // easily compute the usage for the entire range.
        //
        auto rangeResourceCount = count * objectCounts.resource;
        auto rangeSamplerCount = count * objectCounts.sampler;
        auto rangeRootParamCount = count * objectCounts.rootParam;

        m_totalCounts.resource += rangeResourceCount;
        m_totalCounts.sampler += rangeSamplerCount;
        m_childRootParameterCount += rangeRootParamCount;

        m_subObjectRanges.add(subObjectRange);
    }

    // Once we have added up the resource usage from all the sub-objects
    // we can look at the total number of resources and samplers that
    // need to be bound as part of this objects descriptor tables and
    // that will allow us to decide whether we need to allocate a root
    // parameter for a resource table or not, ans similarly for a
    // sampler table.
    //
    if (m_totalCounts.resource)
        m_ownCounts.rootParam++;
    if (m_totalCounts.sampler)
        m_ownCounts.rootParam++;

    m_totalCounts.rootParam = m_ownCounts.rootParam + m_childRootParameterCount;

    return SLANG_OK;
}

Result ShaderObjectLayoutImpl::Builder::build(ShaderObjectLayoutImpl** outLayout)
{
    auto layout = RefPtr<ShaderObjectLayoutImpl>(new ShaderObjectLayoutImpl());
    SLANG_RETURN_ON_FAIL(layout->init(this));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

Result RootShaderObjectLayoutImpl::Builder::build(RootShaderObjectLayoutImpl** outLayout)
{
    RefPtr<RootShaderObjectLayoutImpl> layout = new RootShaderObjectLayoutImpl();
    SLANG_RETURN_ON_FAIL(layout->init(this));

    returnRefPtrMove(outLayout, layout);
    return SLANG_OK;
}

void RootShaderObjectLayoutImpl::Builder::addGlobalParams(
    slang::VariableLayoutReflection* globalsLayout)
{
    setElementTypeLayout(globalsLayout->getTypeLayout());
}

void RootShaderObjectLayoutImpl::Builder::addEntryPoint(
    SlangStage stage, ShaderObjectLayoutImpl* entryPointLayout)
{
    EntryPointInfo info;
    info.layout = entryPointLayout;

    info.offset.resource = m_totalCounts.resource;
    info.offset.sampler = m_totalCounts.sampler;
    info.offset.rootParam = m_childRootParameterCount;

    m_totalCounts.resource += entryPointLayout->getTotalResourceDescriptorCount();
    m_totalCounts.sampler += entryPointLayout->getTotalSamplerDescriptorCount();

    // TODO(tfoley): Check this to make sure it is reasonable...
    m_childRootParameterCount += entryPointLayout->getChildRootParameterCount();

    m_entryPoints.add(info);
}

Result RootShaderObjectLayoutImpl::RootSignatureDescBuilder::translateDescriptorRangeType(
    slang::BindingType c, D3D12_DESCRIPTOR_RANGE_TYPE* outType)
{
    switch (c)
    {
    case slang::BindingType::ConstantBuffer:
        *outType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        return SLANG_OK;
    case slang::BindingType::RawBuffer:
    case slang::BindingType::Texture:
    case slang::BindingType::TypedBuffer:
    case slang::BindingType::RayTracingAccelerationStructure:
        *outType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        return SLANG_OK;
    case slang::BindingType::MutableRawBuffer:
    case slang::BindingType::MutableTexture:
    case slang::BindingType::MutableTypedBuffer:
        *outType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        return SLANG_OK;
    case slang::BindingType::Sampler:
        *outType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        return SLANG_OK;
    default:
        return SLANG_FAIL;
    }
}

/// Add a new descriptor set to the layout being computed.
///
/// Note that a "descriptor set" in the layout may amount to
/// zero, one, or two different descriptor *tables* in the
/// final D3D12 root signature. Each descriptor set may
/// contain zero or more view ranges (CBV/SRV/UAV) and zero
/// or more sampler ranges. It maps to a view descriptor table
/// if the number of view ranges is non-zero and to a sampler
/// descriptor table if the number of sampler ranges is non-zero.
///

uint32_t RootShaderObjectLayoutImpl::RootSignatureDescBuilder::addDescriptorSet()
{
    auto result = (uint32_t)m_descriptorSets.getCount();
    m_descriptorSets.add(DescriptorSetLayout{});
    return result;
}

Result RootShaderObjectLayoutImpl::RootSignatureDescBuilder::addDescriptorRange(
    Index physicalDescriptorSetIndex,
    D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
    UINT registerIndex,
    UINT spaceIndex,
    UINT count,
    bool isRootParameter)
{
    if (isRootParameter)
    {
        D3D12_ROOT_PARAMETER rootParam = {};
        switch (rangeType)
        {
        case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            break;
        default:
            getDebugCallback()->handleMessage(
                DebugMessageType::Error,
                DebugMessageSource::Layer,
                "A shader parameter marked as root parameter is neither SRV nor UAV.");
            return SLANG_FAIL;
        }
        rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParam.Descriptor.RegisterSpace = spaceIndex;
        rootParam.Descriptor.ShaderRegister = registerIndex;
        m_rootParameters.add(rootParam);
        return SLANG_OK;
    }

    auto& descriptorSet = m_descriptorSets[physicalDescriptorSetIndex];

    D3D12_DESCRIPTOR_RANGE range = {};
    range.RangeType = rangeType;
    range.NumDescriptors = count;
    range.BaseShaderRegister = registerIndex;
    range.RegisterSpace = spaceIndex;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
    {
        descriptorSet.m_samplerRanges.add(range);
        descriptorSet.m_samplerCount += range.NumDescriptors;
    }
    else
    {
        descriptorSet.m_resourceRanges.add(range);
        descriptorSet.m_resourceCount += range.NumDescriptors;
    }

    return SLANG_OK;
}

/// Add one descriptor range as specified in Slang reflection information to the layout.
///
/// The layout information is taken from `typeLayout` for the descriptor
/// range with the given `descriptorRangeIndex` within the logical
/// descriptor set (reflected by Slang) with the given `logicalDescriptorSetIndex`.
///
/// The `physicalDescriptorSetIndex` is the index in the `m_descriptorSets` array of
/// the descriptor set that the range should be added to.
///
/// The `offset` encodes information about space and/or register offsets that
/// should be applied to descrptor ranges.
///
/// This operation can fail if the given descriptor range encodes a range that
/// doesn't map to anything directly supported by D3D12. Higher-level routines
/// will often want to ignore such failures.
///

Result RootShaderObjectLayoutImpl::RootSignatureDescBuilder::addDescriptorRange(
    slang::TypeLayoutReflection* typeLayout,
    Index physicalDescriptorSetIndex,
    BindingRegisterOffset const& containerOffset,
    BindingRegisterOffset const& elementOffset,
    Index logicalDescriptorSetIndex,
    Index descriptorRangeIndex,
    bool isRootParameter)
{
    auto bindingType = typeLayout->getDescriptorSetDescriptorRangeType(
        logicalDescriptorSetIndex, descriptorRangeIndex);
    auto count = typeLayout->getDescriptorSetDescriptorRangeDescriptorCount(
        logicalDescriptorSetIndex, descriptorRangeIndex);
    auto index = typeLayout->getDescriptorSetDescriptorRangeIndexOffset(
        logicalDescriptorSetIndex, descriptorRangeIndex);
    auto space = typeLayout->getDescriptorSetSpaceOffset(logicalDescriptorSetIndex);

    D3D12_DESCRIPTOR_RANGE_TYPE rangeType;
    SLANG_RETURN_ON_FAIL(translateDescriptorRangeType(bindingType, &rangeType));

    return addDescriptorRange(
        physicalDescriptorSetIndex,
        rangeType,
        (UINT)index + elementOffset[rangeType],
        (UINT)space + containerOffset.spaceOffset,
        (UINT)count,
        isRootParameter);
}

/// Add one binding range to the computed layout.
///
/// The layout information is taken from `typeLayout` for the binding
/// range with the given `bindingRangeIndex`.
///
/// The `physicalDescriptorSetIndex` is the index in the `m_descriptorSets` array of
/// the descriptor set that the range should be added to.
///
/// The `offset` encodes information about space and/or register offsets that
/// should be applied to descrptor ranges.
///
/// Note that a single binding range may encompass zero or more descriptor ranges.
///

void RootShaderObjectLayoutImpl::RootSignatureDescBuilder::addBindingRange(
    slang::TypeLayoutReflection* typeLayout,
    Index physicalDescriptorSetIndex,
    BindingRegisterOffset const& containerOffset,
    BindingRegisterOffset const& elementOffset,
    Index bindingRangeIndex)
{
    auto logicalDescriptorSetIndex =
        typeLayout->getBindingRangeDescriptorSetIndex(bindingRangeIndex);
    auto firstDescriptorRangeIndex =
        typeLayout->getBindingRangeFirstDescriptorRangeIndex(bindingRangeIndex);
    Index descriptorRangeCount = typeLayout->getBindingRangeDescriptorRangeCount(bindingRangeIndex);
    bool isRootParameter = isBindingRangeRootParameter(
        m_device->slangContext.globalSession,
        m_device->m_extendedDesc.rootParameterShaderAttributeName,
        typeLayout,
        bindingRangeIndex);
    for (Index i = 0; i < descriptorRangeCount; ++i)
    {
        auto descriptorRangeIndex = firstDescriptorRangeIndex + i;

        // Note: we ignore the `Result` returned by `addDescriptorRange()` because we
        // want to silently skip any ranges that represent kinds of bindings that
        // don't actually exist in D3D12.
        //
        addDescriptorRange(
            typeLayout,
            physicalDescriptorSetIndex,
            containerOffset,
            elementOffset,
            logicalDescriptorSetIndex,
            descriptorRangeIndex,
            isRootParameter);
    }
}

void RootShaderObjectLayoutImpl::RootSignatureDescBuilder::addAsValue(
    slang::VariableLayoutReflection* varLayout, Index physicalDescriptorSetIndex)
{
    BindingRegisterOffsetPair offset(varLayout);
    addAsValue(varLayout->getTypeLayout(), physicalDescriptorSetIndex, offset, offset);
}

/// Add binding ranges and parameter blocks to the root signature.
///
/// The layout information is taken from `typeLayout` which should
/// be a layout for either a program or an entry point.
///
/// The `physicalDescriptorSetIndex` is the index in the `m_descriptorSets` array of
/// the descriptor set that binding ranges not belonging to nested
/// parameter blocks should be added to.
///
/// The `offset` encodes information about space and/or register offsets that
/// should be applied to descrptor ranges.
///

void RootShaderObjectLayoutImpl::RootSignatureDescBuilder::addAsConstantBuffer(
    slang::TypeLayoutReflection* typeLayout,
    Index physicalDescriptorSetIndex,
    BindingRegisterOffsetPair const& containerOffset,
    BindingRegisterOffsetPair const& elementOffset)
{
    if (typeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) != 0)
    {
        auto descriptorRangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        auto& offsetForRangeType = containerOffset.primary.offsetForRangeType[descriptorRangeType];
        addDescriptorRange(
            physicalDescriptorSetIndex,
            descriptorRangeType,
            offsetForRangeType,
            containerOffset.primary.spaceOffset,
            1,
            false);
    }

    addAsValue(typeLayout, physicalDescriptorSetIndex, containerOffset, elementOffset);
}

void RootShaderObjectLayoutImpl::RootSignatureDescBuilder::addAsValue(
    slang::TypeLayoutReflection* typeLayout,
    Index physicalDescriptorSetIndex,
    BindingRegisterOffsetPair const& containerOffset,
    BindingRegisterOffsetPair const& elementOffset)
{
    // Our first task is to add the binding ranges for stuff that is
    // directly contained in `typeLayout` rather than via sub-objects.
    //
    // Our goal is to have the descriptors for directly-contained views/samplers
    // always be contiguous in CPU and GPU memory, so that we can write
    // to them easily with a single operaiton.
    //
    Index bindingRangeCount = typeLayout->getBindingRangeCount();
    for (Index bindingRangeIndex = 0; bindingRangeIndex < bindingRangeCount; bindingRangeIndex++)
    {
        // We will look at the type of each binding range and intentionally
        // skip those that represent sub-objects.
        //
        auto bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);
        switch (bindingType)
        {
        case slang::BindingType::ConstantBuffer:
        case slang::BindingType::ParameterBlock:
        case slang::BindingType::ExistentialValue:
            continue;

        default:
            break;
        }

        // For binding ranges that don't represent sub-objects, we will add
        // all of the descriptor ranges they encompass to the root signature.
        //
        addBindingRange(
            typeLayout,
            physicalDescriptorSetIndex,
            containerOffset.primary,
            elementOffset.primary,
            bindingRangeIndex);
    }

    // Next we need to recursively include everything bound via sub-objects
    Index subObjectRangeCount = typeLayout->getSubObjectRangeCount();
    for (Index subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount;
         subObjectRangeIndex++)
    {
        auto bindingRangeIndex =
            typeLayout->getSubObjectRangeBindingRangeIndex(subObjectRangeIndex);
        auto bindingType = typeLayout->getBindingRangeType(bindingRangeIndex);

        auto subObjectTypeLayout = typeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);

        BindingRegisterOffsetPair subObjectRangeContainerOffset = containerOffset;
        subObjectRangeContainerOffset +=
            BindingRegisterOffsetPair(typeLayout->getSubObjectRangeOffset(subObjectRangeIndex));
        BindingRegisterOffsetPair subObjectRangeElementOffset = elementOffset;
        subObjectRangeElementOffset +=
            BindingRegisterOffsetPair(typeLayout->getSubObjectRangeOffset(subObjectRangeIndex));

        switch (bindingType)
        {
        case slang::BindingType::ConstantBuffer:
            {
                auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                SLANG_ASSERT(containerVarLayout);

                auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                SLANG_ASSERT(elementVarLayout);

                auto elementTypeLayout = elementVarLayout->getTypeLayout();
                SLANG_ASSERT(elementTypeLayout);

                BindingRegisterOffsetPair containerOffset = subObjectRangeContainerOffset;
                containerOffset += BindingRegisterOffsetPair(containerVarLayout);

                BindingRegisterOffsetPair elementOffset = subObjectRangeElementOffset;
                elementOffset += BindingRegisterOffsetPair(elementVarLayout);

                addAsConstantBuffer(
                    elementTypeLayout, physicalDescriptorSetIndex, containerOffset, elementOffset);
            }
            break;

        case slang::BindingType::ParameterBlock:
            {
                auto containerVarLayout = subObjectTypeLayout->getContainerVarLayout();
                SLANG_ASSERT(containerVarLayout);

                auto elementVarLayout = subObjectTypeLayout->getElementVarLayout();
                SLANG_ASSERT(elementVarLayout);

                auto elementTypeLayout = elementVarLayout->getTypeLayout();
                SLANG_ASSERT(elementTypeLayout);

                BindingRegisterOffsetPair subDescriptorSetOffset;
                subDescriptorSetOffset.primary.spaceOffset =
                    subObjectRangeElementOffset.primary.spaceOffset;
                subDescriptorSetOffset.pending.spaceOffset =
                    subObjectRangeElementOffset.pending.spaceOffset;

                auto subPhysicalDescriptorSetIndex = addDescriptorSet();

                BindingRegisterOffsetPair containerOffset = subDescriptorSetOffset;
                containerOffset += BindingRegisterOffsetPair(containerVarLayout);

                BindingRegisterOffsetPair elementOffset = subDescriptorSetOffset;
                elementOffset += BindingRegisterOffsetPair(elementVarLayout);

                addAsConstantBuffer(
                    elementTypeLayout,
                    subPhysicalDescriptorSetIndex,
                    containerOffset,
                    elementOffset);
            }
            break;

        case slang::BindingType::ExistentialValue:
            {
                // Any nested binding ranges in the sub-object will "leak" into the
                // binding ranges for the surrounding context.
                //
                auto specializedTypeLayout = subObjectTypeLayout->getPendingDataTypeLayout();
                if (specializedTypeLayout)
                {
                    BindingRegisterOffsetPair pendingOffset;
                    pendingOffset.primary = subObjectRangeElementOffset.pending;

                    addAsValue(
                        specializedTypeLayout,
                        physicalDescriptorSetIndex,
                        pendingOffset,
                        pendingOffset);
                }
            }
            break;
        }
    }
}

D3D12_ROOT_SIGNATURE_DESC& RootShaderObjectLayoutImpl::RootSignatureDescBuilder::build()
{
    for (Index i = 0; i < m_descriptorSets.getCount(); i++)
    {
        auto& descriptorSet = m_descriptorSets[i];
        if (descriptorSet.m_resourceRanges.getCount())
        {
            D3D12_ROOT_PARAMETER rootParam = {};
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParam.DescriptorTable.NumDescriptorRanges =
                (UINT)descriptorSet.m_resourceRanges.getCount();
            rootParam.DescriptorTable.pDescriptorRanges =
                descriptorSet.m_resourceRanges.getBuffer();
            m_rootParameters.add(rootParam);
        }
        if (descriptorSet.m_samplerRanges.getCount())
        {
            D3D12_ROOT_PARAMETER rootParam = {};
            rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParam.DescriptorTable.NumDescriptorRanges =
                (UINT)descriptorSet.m_samplerRanges.getCount();
            rootParam.DescriptorTable.pDescriptorRanges = descriptorSet.m_samplerRanges.getBuffer();
            m_rootParameters.add(rootParam);
        }
    }

    m_rootSignatureDesc.NumParameters = UINT(m_rootParameters.getCount());
    m_rootSignatureDesc.pParameters = m_rootParameters.getBuffer();

    // TODO: static samplers should be reasonably easy to support...
    m_rootSignatureDesc.NumStaticSamplers = 0;
    m_rootSignatureDesc.pStaticSamplers = nullptr;

    // TODO: only set this flag if needed (requires creating root
    // signature at same time as pipeline state...).
    //
    m_rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    return m_rootSignatureDesc;
}

Result RootShaderObjectLayoutImpl::createRootSignatureFromSlang(
    DeviceImpl* device,
    RootShaderObjectLayoutImpl* rootLayout,
    slang::IComponentType* program,
    ID3D12RootSignature** outRootSignature,
    ID3DBlob** outError)
{
    // We are going to build up the root signature by adding
    // binding/descritpor ranges and nested parameter blocks
    // based on the computed layout information for `program`.
    //
    RootSignatureDescBuilder builder(device);
    auto layout = program->getLayout();

    // The layout information computed by Slang breaks up shader
    // parameters into what we can think of as "logical" descriptor
    // sets based on whether or not parameters have the same `space`.
    //
    // We want to basically ignore that decomposition and generate a
    // single descriptor set to hold all top-level parameters, and only
    // generate distinct descriptor sets when the shader has opted in
    // via explicit parameter blocks.
    //
    // To achieve this goal, we will manually allocate a default descriptor
    // set for root parameters in our signature, and then recursively
    // add all the binding/descriptor ranges implied by the global-scope
    // parameters.
    //
    auto rootDescriptorSetIndex = builder.addDescriptorSet();
    builder.addAsValue(layout->getGlobalParamsVarLayout(), rootDescriptorSetIndex);

    for (SlangUInt i = 0; i < layout->getEntryPointCount(); i++)
    {
        // Entry-point parameters should also be added to the default root
        // descriptor set.
        //
        // We add the parameters using the "variable layout" for the entry point
        // and not just its type layout, to ensure that any offset information is
        // applied correctly to the `register` and `space` information for entry-point
        // parameters.
        //
        // Note: When we start to support DXR we will need to handle entry-point parameters
        // differently because they will need to map to local root signatures rather than
        // being included in the global root signature as is being done here.
        //
        auto entryPoint = layout->getEntryPointByIndex(i);
        builder.addAsValue(entryPoint->getVarLayout(), rootDescriptorSetIndex);
    }

    auto& rootSignatureDesc = builder.build();

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    if (SLANG_FAILED(device->m_D3D12SerializeRootSignature(
            &rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            signature.writeRef(),
            error.writeRef())))
    {
        getDebugCallback()->handleMessage(
            DebugMessageType::Error,
            DebugMessageSource::Layer,
            "error: D3D12SerializeRootSignature failed");
        if (error)
        {
            getDebugCallback()->handleMessage(
                DebugMessageType::Error,
                DebugMessageSource::Driver,
                (const char*)error->GetBufferPointer());
            if (outError)
                returnComPtr(outError, error);
        }
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(device->m_device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(outRootSignature)));
    return SLANG_OK;
}

Result RootShaderObjectLayoutImpl::create(
    DeviceImpl* device,
    slang::IComponentType* program,
    slang::ProgramLayout* programLayout,
    RootShaderObjectLayoutImpl** outLayout,
    ID3DBlob** outError)
{
    RootShaderObjectLayoutImpl::Builder builder(device, program, programLayout);
    builder.addGlobalParams(programLayout->getGlobalParamsVarLayout());

    SlangInt entryPointCount = programLayout->getEntryPointCount();
    for (SlangInt e = 0; e < entryPointCount; ++e)
    {
        auto slangEntryPoint = programLayout->getEntryPointByIndex(e);
        RefPtr<ShaderObjectLayoutImpl> entryPointLayout;
        SLANG_RETURN_ON_FAIL(ShaderObjectLayoutImpl::createForElementType(
            device, slangEntryPoint->getTypeLayout(), entryPointLayout.writeRef()));
        builder.addEntryPoint(slangEntryPoint->getStage(), entryPointLayout);
    }

    RefPtr<RootShaderObjectLayoutImpl> layout;
    SLANG_RETURN_ON_FAIL(builder.build(layout.writeRef()));

    if (program->getSpecializationParamCount() == 0)
    {
        // For root object, we would like know the union of all binding slots
        // including all sub-objects in the shader-object hierarchy, so at
        // parameter binding time we can easily know how many GPU descriptor tables
        // to create without walking through the shader-object hierarchy again.
        // We build out this array along with root signature construction and store
        // it in `m_gpuDescriptorSetInfos`.
        SLANG_RETURN_ON_FAIL(createRootSignatureFromSlang(
            device, layout, program, layout->m_rootSignature.writeRef(), outError));
    }

    *outLayout = layout.detach();

    return SLANG_OK;
}

Result RootShaderObjectLayoutImpl::init(Builder* builder)
{
    auto renderer = builder->m_renderer;

    SLANG_RETURN_ON_FAIL(Super::init(builder));

    m_program = builder->m_program;
    m_programLayout = builder->m_programLayout;
    m_entryPoints = builder->m_entryPoints;
    return SLANG_OK;
}

Result ShaderProgramImpl::createShaderModule(
    slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode)
{
    ShaderBinary shaderBin;
    shaderBin.stage = entryPointInfo->getStage();
    shaderBin.entryPointInfo = entryPointInfo;
    shaderBin.code.addRange(
        reinterpret_cast<const uint8_t*>(kernelCode->getBufferPointer()),
        (Index)kernelCode->getBufferSize());
    m_shaders.add(_Move(shaderBin));
    return SLANG_OK;
}

Result ShaderObjectImpl::create(
    DeviceImpl* device, ShaderObjectLayoutImpl* layout, ShaderObjectImpl** outShaderObject)
{
    auto object = RefPtr<ShaderObjectImpl>(new ShaderObjectImpl());
    SLANG_RETURN_ON_FAIL(
        object->init(device, layout, device->m_cpuViewHeap.Ptr(), device->m_cpuSamplerHeap.Ptr()));
    returnRefPtrMove(outShaderObject, object);
    return SLANG_OK;
}

ShaderObjectImpl::~ShaderObjectImpl() { m_descriptorSet.freeIfSupported(); }

RootShaderObjectLayoutImpl* RootShaderObjectImpl::getLayout()
{
    return static_cast<RootShaderObjectLayoutImpl*>(m_layout.Ptr());
}

UInt RootShaderObjectImpl::getEntryPointCount() { return (UInt)m_entryPoints.getCount(); }

SlangResult RootShaderObjectImpl::getEntryPoint(UInt index, IShaderObject** outEntryPoint)
{
    returnComPtr(outEntryPoint, m_entryPoints[index]);
    return SLANG_OK;
}

Result RootShaderObjectImpl::collectSpecializationArgs(ExtendedShaderObjectTypeList& args)
{
    SLANG_RETURN_ON_FAIL(ShaderObjectImpl::collectSpecializationArgs(args));
    for (auto& entryPoint : m_entryPoints)
    {
        SLANG_RETURN_ON_FAIL(entryPoint->collectSpecializationArgs(args));
    }
    return SLANG_OK;
}

Result RootShaderObjectImpl::_createSpecializedLayout(ShaderObjectLayoutImpl** outLayout)
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

    if (diagnosticBlob && diagnosticBlob->getBufferSize())
    {
        getDebugCallback()->handleMessage(
            SLANG_FAILED(result) ? DebugMessageType::Error : DebugMessageType::Info,
            DebugMessageSource::Layer,
            (const char*)diagnosticBlob->getBufferPointer());
    }

    if (SLANG_FAILED(result))
        return result;

    ComPtr<ID3DBlob> d3dDiagnosticBlob;
    auto slangSpecializedLayout = specializedComponentType->getLayout();
    RefPtr<RootShaderObjectLayoutImpl> specializedLayout;
    auto rootLayoutResult = RootShaderObjectLayoutImpl::create(
        static_cast<DeviceImpl*>(getRenderer()),
        specializedComponentType,
        slangSpecializedLayout,
        specializedLayout.writeRef(),
        d3dDiagnosticBlob.writeRef());

    if (SLANG_FAILED(rootLayoutResult))
    {
        return rootLayoutResult;
    }

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

Result RootShaderObjectImpl::copyFrom(IShaderObject* object, ITransientResourceHeap* transientHeap)
{
    if (auto srcObj = dynamic_cast<MutableRootShaderObjectImpl*>(object))
    {
        *this = *srcObj;
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

Result RootShaderObjectImpl::bindAsRoot(
    BindingContext* context, RootShaderObjectLayoutImpl* specializedLayout)
{
    // Pull updates from sub-objects when this is a mutable root shader object.
    updateSubObjectsRecursive();

    // A root shader object always binds as if it were a parameter block,
    // insofar as it needs to allocate a descriptor set to hold the bindings
    // for its own state and any sub-objects.
    //
    // Note: We do not direclty use `bindAsParameterBlock` here because we also
    // need to bind the entry points into the same descriptor set that is
    // being used for the root object.

    BindingOffset rootOffset;

    // Bind all root parameters first.
    Super::bindRootArguments(context, rootOffset.rootParam);

    DescriptorSet descriptorSet;
    SLANG_RETURN_ON_FAIL(prepareToBindAsParameterBlock(
        context, /* inout */ rootOffset, specializedLayout, descriptorSet));

    SLANG_RETURN_ON_FAIL(
        Super::bindAsConstantBuffer(context, descriptorSet, rootOffset, specializedLayout));

    auto entryPointCount = m_entryPoints.getCount();
    for (Index i = 0; i < entryPointCount; ++i)
    {
        auto entryPoint = m_entryPoints[i];
        auto& entryPointInfo = specializedLayout->getEntryPoint(i);

        auto entryPointOffset = rootOffset;
        entryPointOffset += entryPointInfo.offset;

        entryPoint->updateSubObjectsRecursive();

        SLANG_RETURN_ON_FAIL(entryPoint->bindAsConstantBuffer(
            context, descriptorSet, entryPointOffset, entryPointInfo.layout));
    }

    return SLANG_OK;
}

Result RootShaderObjectImpl::resetImpl(
    DeviceImpl* device,
    RootShaderObjectLayoutImpl* layout,
    DescriptorHeapReference viewHeap,
    DescriptorHeapReference samplerHeap,
    bool isMutable)
{
    SLANG_RETURN_ON_FAIL(Super::init(device, layout, viewHeap, samplerHeap));
    m_isMutable = isMutable;
    m_specializedLayout = nullptr;
    m_entryPoints.clear();
    for (auto entryPointInfo : layout->getEntryPoints())
    {
        RefPtr<ShaderObjectImpl> entryPoint;
        SLANG_RETURN_ON_FAIL(
            ShaderObjectImpl::create(device, entryPointInfo.layout, entryPoint.writeRef()));
        entryPoint->m_isMutable = isMutable;
        m_entryPoints.add(entryPoint);
    }
    return SLANG_OK;
}

Result RootShaderObjectImpl::reset(
    DeviceImpl* device, RootShaderObjectLayoutImpl* layout, TransientResourceHeapImpl* heap)
{
    return resetImpl(
        device, layout, &heap->m_stagingCpuViewHeap, &heap->m_stagingCpuSamplerHeap, false);
}

RefPtr<BufferResource> ShaderTableImpl::createDeviceBuffer(
    PipelineStateBase* pipeline,
    TransientResourceHeapBase* transientHeap,
    IResourceCommandEncoder* encoder)
{
    uint32_t raygenTableSize = m_rayGenShaderCount * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    uint32_t missTableSize = m_missShaderCount * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    uint32_t hitgroupTableSize = m_hitGroupCount * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    m_rayGenTableOffset = 0;
    m_missTableOffset = (uint32_t)D3DUtil::calcAligned(
        raygenTableSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
    m_hitGroupTableOffset = (uint32_t)D3DUtil::calcAligned(
        m_missTableOffset + missTableSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
    uint32_t tableSize = m_hitGroupTableOffset + hitgroupTableSize;

    auto pipelineImpl = static_cast<RayTracingPipelineStateImpl*>(pipeline);
    ComPtr<IBufferResource> bufferResource;
    IBufferResource::Desc bufferDesc = {};
    bufferDesc.memoryType = gfx::MemoryType::DeviceLocal;
    bufferDesc.defaultState = ResourceState::General;
    bufferDesc.type = IResource::Type::Buffer;
    bufferDesc.sizeInBytes = tableSize;
    m_device->createBufferResource(bufferDesc, nullptr, bufferResource.writeRef());

    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    pipelineImpl->m_stateObject->QueryInterface(stateObjectProperties.writeRef());

    TransientResourceHeapImpl* transientHeapImpl =
        static_cast<TransientResourceHeapImpl*>(transientHeap);

    IBufferResource* stagingBuffer = nullptr;
    size_t stagingBufferOffset = 0;
    transientHeapImpl->allocateStagingBuffer(
        tableSize, stagingBuffer, stagingBufferOffset, MemoryType::Upload);

    assert(stagingBuffer);
    void* stagingPtr = nullptr;
    stagingBuffer->map(nullptr, &stagingPtr);

    auto copyShaderIdInto = [&](void* dest, String& name, const ShaderRecordOverwrite& overwrite)
    {
        if (name.getLength())
        {
            void* shaderId = stateObjectProperties->GetShaderIdentifier(name.toWString().begin());
            memcpy(dest, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }
        else
        {
            memset(dest, 0, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }
        if (overwrite.size)
        {
            memcpy((uint8_t*)dest + overwrite.offset, overwrite.data, overwrite.size);
        }
    };

    uint8_t* stagingBufferPtr = (uint8_t*)stagingPtr + stagingBufferOffset;
    for (uint32_t i = 0; i < m_rayGenShaderCount; i++)
    {
        copyShaderIdInto(
            stagingBufferPtr + m_rayGenTableOffset + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * i,
            m_shaderGroupNames[i],
            m_recordOverwrites[i]);
    }
    for (uint32_t i = 0; i < m_missShaderCount; i++)
    {
        copyShaderIdInto(
            stagingBufferPtr + m_missTableOffset + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * i,
            m_shaderGroupNames[m_rayGenShaderCount + i],
            m_recordOverwrites[m_rayGenShaderCount + i]);
    }
    for (uint32_t i = 0; i < m_hitGroupCount; i++)
    {
        copyShaderIdInto(
            stagingBufferPtr + m_hitGroupTableOffset + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * i,
            m_shaderGroupNames[m_rayGenShaderCount + m_missShaderCount + i],
            m_recordOverwrites[m_rayGenShaderCount + m_missShaderCount + i]);
    }

    stagingBuffer->unmap(nullptr);
    encoder->copyBuffer(bufferResource, 0, stagingBuffer, stagingBufferOffset, tableSize);
    encoder->bufferBarrier(
        1,
        bufferResource.readRef(),
        gfx::ResourceState::CopyDestination,
        gfx::ResourceState::ShaderResource);
    RefPtr<BufferResource> resultPtr = static_cast<BufferResource*>(bufferResource.get());
    return _Move(resultPtr);
}

// There are a pair of cyclic references between a `TransientResourceHeap` and
// a `CommandBuffer` created from the heap. We need to break the cycle upon
// the public reference count of a command buffer dropping to 0.

ICommandBuffer* CommandBufferImpl::getInterface(const Guid& guid)
{
    if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ICommandBuffer)
        return static_cast<ICommandBuffer*>(this);
    return nullptr;
}

Result CommandBufferImpl::getNativeHandle(InteropHandle* handle)
{
    handle->api = InteropHandleAPI::D3D12;
    handle->handleValue = (uint64_t)m_cmdList.get();
    return SLANG_OK;
}

void CommandBufferImpl::bindDescriptorHeaps()
{
    if (!m_descriptorHeapsBound)
    {
        ID3D12DescriptorHeap* heaps[] = {
            m_transientHeap->getCurrentViewHeap().getHeap(),
            m_transientHeap->getCurrentSamplerHeap().getHeap(),
        };
        m_cmdList->SetDescriptorHeaps(SLANG_COUNT_OF(heaps), heaps);
        m_descriptorHeapsBound = true;
    }
}

void CommandBufferImpl::reinit()
{
    invalidateDescriptorHeapBinding();
    m_rootShaderObject.init(m_renderer);
}

void CommandBufferImpl::init(
    DeviceImpl* renderer,
    ID3D12GraphicsCommandList* d3dCommandList,
    TransientResourceHeapImpl* transientHeap)
{
    m_transientHeap = transientHeap;
    m_renderer = renderer;
    m_cmdList = d3dCommandList;

    reinit();

#if SLANG_GFX_HAS_DXR_SUPPORT
    m_cmdList->QueryInterface<ID3D12GraphicsCommandList4>(m_cmdList4.writeRef());
    if (m_cmdList4)
    {
        m_cmdList1 = m_cmdList4;
        return;
    }
#endif
    m_cmdList->QueryInterface<ID3D12GraphicsCommandList1>(m_cmdList1.writeRef());
}

void CommandBufferImpl::encodeResourceCommands(IResourceCommandEncoder** outEncoder)
{
    m_resourceCommandEncoder.init(this);
    *outEncoder = &m_resourceCommandEncoder;
}

void CommandBufferImpl::encodeRenderCommands(
    IRenderPassLayout* renderPass, IFramebuffer* framebuffer, IRenderCommandEncoder** outEncoder)
{
    m_renderCommandEncoder.init(
        m_renderer,
        m_transientHeap,
        this,
        static_cast<RenderPassLayoutImpl*>(renderPass),
        static_cast<FramebufferImpl*>(framebuffer));
    *outEncoder = &m_renderCommandEncoder;
}

void CommandBufferImpl::encodeComputeCommands(IComputeCommandEncoder** outEncoder)
{
    m_computeCommandEncoder.init(m_renderer, m_transientHeap, this);
    *outEncoder = &m_computeCommandEncoder;
}

void CommandBufferImpl::encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder)
{
#if SLANG_GFX_HAS_DXR_SUPPORT
    m_rayTracingCommandEncoder.init(this);
    *outEncoder = &m_rayTracingCommandEncoder;
#else
    *outEncoder = nullptr;
#endif
}

void CommandBufferImpl::close() { m_cmdList->Close(); }

void ResourceCommandEncoderImpl::copyBuffer(
    IBufferResource* dst, size_t dstOffset, IBufferResource* src, size_t srcOffset, size_t size)
{
    auto dstBuffer = static_cast<BufferResourceImpl*>(dst);
    auto srcBuffer = static_cast<BufferResourceImpl*>(src);

    m_commandBuffer->m_cmdList->CopyBufferRegion(
        dstBuffer->m_resource.getResource(),
        dstOffset,
        srcBuffer->m_resource.getResource(),
        srcOffset,
        size);
}

void ResourceCommandEncoderImpl::uploadBufferData(
    IBufferResource* dst, size_t offset, size_t size, void* data)
{
    uploadBufferDataImpl(
        m_commandBuffer->m_renderer->m_device,
        m_commandBuffer->m_cmdList,
        m_commandBuffer->m_transientHeap,
        static_cast<BufferResourceImpl*>(dst),
        offset,
        size,
        data);
}

void ResourceCommandEncoderImpl::textureBarrier(
    size_t count, ITextureResource* const* textures, ResourceState src, ResourceState dst)
{
    ShortList<D3D12_RESOURCE_BARRIER> barriers;

    for (size_t i = 0; i < count; i++)
    {
        auto textureImpl = static_cast<TextureResourceImpl*>(textures[i]);
        auto d3dFormat = D3DUtil::getMapFormat(textureImpl->getDesc()->format);
        auto textureDesc = textureImpl->getDesc();
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        if (src == dst && src == ResourceState::UnorderedAccess)
        {
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.UAV.pResource = textureImpl->m_resource.getResource();
        }
        else
        {
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.StateBefore = D3DUtil::getResourceState(src);
            barrier.Transition.StateAfter = D3DUtil::getResourceState(dst);
            if (barrier.Transition.StateBefore == barrier.Transition.StateAfter)
                continue;
            barrier.Transition.pResource = textureImpl->m_resource.getResource();
            auto planeCount =
                D3DUtil::getPlaneSliceCount(D3DUtil::getMapFormat(textureImpl->getDesc()->format));
            auto arraySize = textureDesc->arraySize;
            if (arraySize == 0)
                arraySize = 1;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        }
        barriers.add(barrier);
    }
    if (barriers.getCount())
    {
        m_commandBuffer->m_cmdList->ResourceBarrier(
            (UINT)barriers.getCount(), barriers.getArrayView().getBuffer());
    }
}

void ResourceCommandEncoderImpl::bufferBarrier(
    size_t count, IBufferResource* const* buffers, ResourceState src, ResourceState dst)
{
    ShortList<D3D12_RESOURCE_BARRIER, 16> barriers;
    for (size_t i = 0; i < count; i++)
    {
        auto bufferImpl = static_cast<BufferResourceImpl*>(buffers[i]);

        D3D12_RESOURCE_BARRIER barrier = {};
        // If the src == dst, it must be a UAV barrier.
        barrier.Type = (src == dst && dst == ResourceState::UnorderedAccess)
                           ? D3D12_RESOURCE_BARRIER_TYPE_UAV
                           : D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_UAV)
        {
            barrier.UAV.pResource = bufferImpl->m_resource;
        }
        else
        {
            barrier.Transition.pResource = bufferImpl->m_resource;
            barrier.Transition.StateBefore = D3DUtil::getResourceState(src);
            barrier.Transition.StateAfter = D3DUtil::getResourceState(dst);
            barrier.Transition.Subresource = 0;
            if (barrier.Transition.StateAfter == barrier.Transition.StateBefore)
                continue;
        }
        barriers.add(barrier);
    }
    if (barriers.getCount())
    {
        m_commandBuffer->m_cmdList4->ResourceBarrier(
            (UINT)barriers.getCount(), barriers.getArrayView().getBuffer());
    }
}

void ResourceCommandEncoderImpl::writeTimestamp(IQueryPool* pool, SlangInt index)
{
    static_cast<QueryPoolImpl*>(pool)->writeTimestamp(m_commandBuffer->m_cmdList, index);
}

void ResourceCommandEncoderImpl::copyTexture(
    ITextureResource* dst,
    ResourceState dstState,
    SubresourceRange dstSubresource,
    ITextureResource::Offset3D dstOffset,
    ITextureResource* src,
    ResourceState srcState,
    SubresourceRange srcSubresource,
    ITextureResource::Offset3D srcOffset,
    ITextureResource::Size extent)
{
    auto dstTexture = static_cast<TextureResourceImpl*>(dst);
    auto srcTexture = static_cast<TextureResourceImpl*>(src);

    if (dstSubresource.layerCount == 0 && dstSubresource.mipLevelCount == 0 &&
        srcSubresource.layerCount == 0 && srcSubresource.mipLevelCount == 0)
    {
        m_commandBuffer->m_cmdList->CopyResource(
            dstTexture->m_resource.getResource(), srcTexture->m_resource.getResource());
        return;
    }

    auto d3dFormat = D3DUtil::getMapFormat(dstTexture->getDesc()->format);
    auto aspectMask = (int32_t)dstSubresource.aspectMask;
    if (dstSubresource.aspectMask == TextureAspect::Default)
        aspectMask = (int32_t)TextureAspect::Color;
    while (aspectMask)
    {
        auto aspect = Math::getLowestBit((int32_t)aspectMask);
        aspectMask &= ~aspect;
        auto planeIndex = D3DUtil::getPlaneSlice(d3dFormat, (TextureAspect)aspect);
        for (uint32_t layer = 0; layer < dstSubresource.layerCount; layer++)
        {
            for (uint32_t mipLevel = 0; mipLevel < dstSubresource.mipLevelCount; mipLevel++)
            {
                D3D12_TEXTURE_COPY_LOCATION dstRegion = {};

                dstRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dstRegion.pResource = dstTexture->m_resource.getResource();
                dstRegion.SubresourceIndex = D3DUtil::getSubresourceIndex(
                    dstSubresource.mipLevel + mipLevel,
                    dstSubresource.baseArrayLayer + layer,
                    planeIndex,
                    dstTexture->getDesc()->numMipLevels,
                    dstTexture->getDesc()->arraySize);

                D3D12_TEXTURE_COPY_LOCATION srcRegion = {};
                srcRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                srcRegion.pResource = srcTexture->m_resource.getResource();
                srcRegion.SubresourceIndex = D3DUtil::getSubresourceIndex(
                    srcSubresource.mipLevel + mipLevel,
                    srcSubresource.baseArrayLayer + layer,
                    planeIndex,
                    srcTexture->getDesc()->numMipLevels,
                    srcTexture->getDesc()->arraySize);

                D3D12_BOX srcBox = {};
                srcBox.left = srcOffset.x;
                srcBox.top = srcOffset.y;
                srcBox.front = srcOffset.z;
                srcBox.right = srcBox.left + extent.width;
                srcBox.bottom = srcBox.top + extent.height;
                srcBox.back = srcBox.front + extent.depth;

                m_commandBuffer->m_cmdList->CopyTextureRegion(
                    &dstRegion, dstOffset.x, dstOffset.y, dstOffset.z, &srcRegion, &srcBox);
            }
        }
    }
}

void ResourceCommandEncoderImpl::uploadTextureData(
    ITextureResource* dst,
    SubresourceRange subResourceRange,
    ITextureResource::Offset3D offset,
    ITextureResource::Size extent,
    ITextureResource::SubresourceData* subResourceData,
    size_t subResourceDataCount)
{
    auto dstTexture = static_cast<TextureResourceImpl*>(dst);
    auto baseSubresourceIndex = D3DUtil::getSubresourceIndex(
        subResourceRange.mipLevel,
        subResourceRange.baseArrayLayer,
        0,
        dstTexture->getDesc()->numMipLevels,
        dstTexture->getDesc()->arraySize);
    auto textureSize = dstTexture->getDesc()->size;
    FormatInfo formatInfo = {};
    gfxGetFormatInfo(dstTexture->getDesc()->format, &formatInfo);
    for (uint32_t i = 0; i < (uint32_t)subResourceDataCount; i++)
    {
        auto subresourceIndex = baseSubresourceIndex + i;
        // Get the footprint
        D3D12_RESOURCE_DESC texDesc = dstTexture->m_resource.getResource()->GetDesc();

        D3D12_TEXTURE_COPY_LOCATION dstRegion = {};

        dstRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstRegion.SubresourceIndex = subresourceIndex;
        dstRegion.pResource = dstTexture->m_resource.getResource();

        D3D12_TEXTURE_COPY_LOCATION srcRegion = {};
        srcRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint = srcRegion.PlacedFootprint;
        footprint.Offset = 0;
        footprint.Footprint.Format = texDesc.Format;
        uint32_t mipLevel =
            D3DUtil::getSubresourceMipLevel(subresourceIndex, dstTexture->getDesc()->numMipLevels);
        if (extent.width != ITextureResource::kRemainingTextureSize)
        {
            footprint.Footprint.Width = extent.width;
        }
        else
        {
            footprint.Footprint.Width = Math::Max(1, (textureSize.width >> mipLevel)) - offset.x;
        }
        if (extent.height != ITextureResource::kRemainingTextureSize)
        {
            footprint.Footprint.Height = extent.height;
        }
        else
        {
            footprint.Footprint.Height = Math::Max(1, (textureSize.height >> mipLevel)) - offset.y;
        }
        if (extent.depth != ITextureResource::kRemainingTextureSize)
        {
            footprint.Footprint.Depth = extent.depth;
        }
        else
        {
            footprint.Footprint.Depth = Math::Max(1, (textureSize.depth >> mipLevel)) - offset.z;
        }
        auto rowSize = (footprint.Footprint.Width + formatInfo.blockWidth - 1) /
                       formatInfo.blockWidth * formatInfo.blockSizeInBytes;
        auto rowCount =
            (footprint.Footprint.Height + formatInfo.blockHeight - 1) / formatInfo.blockHeight;
        footprint.Footprint.RowPitch =
            (UINT)D3DUtil::calcAligned(rowSize, (uint32_t)D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

        auto bufferSize = footprint.Footprint.RowPitch * rowCount * footprint.Footprint.Depth;

        IBufferResource* stagingBuffer;
        size_t stagingBufferOffset = 0;
        m_commandBuffer->m_transientHeap->allocateStagingBuffer(
            bufferSize, stagingBuffer, stagingBufferOffset, MemoryType::Upload, true);
        assert(stagingBufferOffset == 0);
        BufferResourceImpl* bufferImpl = static_cast<BufferResourceImpl*>(stagingBuffer);
        uint8_t* bufferData = nullptr;
        D3D12_RANGE mapRange = {0, 0};
        bufferImpl->m_resource.getResource()->Map(0, &mapRange, (void**)&bufferData);
        for (uint32_t z = 0; z < footprint.Footprint.Depth; z++)
        {
            auto imageStart = bufferData + footprint.Footprint.RowPitch * rowCount * (size_t)z;
            auto srcData = (uint8_t*)subResourceData->data + subResourceData->strideZ * z;
            for (uint32_t row = 0; row < rowCount; row++)
            {
                memcpy(
                    imageStart + row * (size_t)footprint.Footprint.RowPitch,
                    srcData + subResourceData->strideY * row,
                    rowSize);
            }
        }
        bufferImpl->m_resource.getResource()->Unmap(0, nullptr);
        srcRegion.pResource = bufferImpl->m_resource.getResource();
        m_commandBuffer->m_cmdList->CopyTextureRegion(
            &dstRegion, offset.x, offset.y, offset.z, &srcRegion, nullptr);
    }
}

void ResourceCommandEncoderImpl::clearResourceView(
    IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags)
{
    auto viewImpl = static_cast<ResourceViewImpl*>(view);
    switch (view->getViewDesc()->type)
    {
    case IResourceView::Type::RenderTarget:
        m_commandBuffer->m_cmdList->ClearRenderTargetView(
            viewImpl->m_descriptor.cpuHandle, clearValue->color.floatValues, 0, nullptr);
        break;
    case IResourceView::Type::DepthStencil:
        {
            D3D12_CLEAR_FLAGS clearFlags = (D3D12_CLEAR_FLAGS)0;
            if (flags & ClearResourceViewFlags::ClearDepth)
            {
                clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
            }
            if (flags & ClearResourceViewFlags::ClearStencil)
            {
                clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
            }
            m_commandBuffer->m_cmdList->ClearDepthStencilView(
                viewImpl->m_descriptor.cpuHandle,
                clearFlags,
                clearValue->depthStencil.depth,
                (UINT8)clearValue->depthStencil.stencil,
                0,
                nullptr);
            break;
        }
    case IResourceView::Type::UnorderedAccess:
        {
            ID3D12Resource* d3dResource = nullptr;
            switch (viewImpl->m_resource->getType())
            {
            case IResource::Type::Buffer:
                d3dResource = static_cast<BufferResourceImpl*>(viewImpl->m_resource.Ptr())
                                  ->m_resource.getResource();
                break;
            default:
                d3dResource = static_cast<TextureResourceImpl*>(viewImpl->m_resource.Ptr())
                                  ->m_resource.getResource();
                break;
            }
            auto gpuHandleIndex =
                m_commandBuffer->m_transientHeap->getCurrentViewHeap().allocate(1);
            if (gpuHandleIndex == -1)
            {
                m_commandBuffer->m_transientHeap->allocateNewViewDescriptorHeap(
                    m_commandBuffer->m_renderer);
                gpuHandleIndex = m_commandBuffer->m_transientHeap->getCurrentViewHeap().allocate(1);
                m_commandBuffer->bindDescriptorHeaps();
            }
            this->m_commandBuffer->m_renderer->m_device->CopyDescriptorsSimple(
                1,
                m_commandBuffer->m_transientHeap->getCurrentViewHeap().getCpuHandle(gpuHandleIndex),
                viewImpl->m_descriptor.cpuHandle,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            if (flags & ClearResourceViewFlags::FloatClearValues)
            {
                m_commandBuffer->m_cmdList->ClearUnorderedAccessViewFloat(
                    m_commandBuffer->m_transientHeap->getCurrentViewHeap().getGpuHandle(
                        gpuHandleIndex),
                    viewImpl->m_descriptor.cpuHandle,
                    d3dResource,
                    clearValue->color.floatValues,
                    0,
                    nullptr);
            }
            else
            {
                m_commandBuffer->m_cmdList->ClearUnorderedAccessViewUint(
                    m_commandBuffer->m_transientHeap->getCurrentViewHeap().getGpuHandle(
                        gpuHandleIndex),
                    viewImpl->m_descriptor.cpuHandle,
                    d3dResource,
                    clearValue->color.uintValues,
                    0,
                    nullptr);
            }
            break;
        }
    default:
        break;
    }
}

void ResourceCommandEncoderImpl::resolveResource(
    ITextureResource* source,
    ResourceState sourceState,
    SubresourceRange sourceRange,
    ITextureResource* dest,
    ResourceState destState,
    SubresourceRange destRange)
{
    auto srcTexture = static_cast<TextureResourceImpl*>(source);
    auto srcDesc = srcTexture->getDesc();
    auto dstTexture = static_cast<TextureResourceImpl*>(dest);
    auto dstDesc = dstTexture->getDesc();

    for (uint32_t layer = 0; layer < sourceRange.layerCount; ++layer)
    {
        for (uint32_t mip = 0; mip < sourceRange.mipLevelCount; ++mip)
        {
            auto srcSubresourceIndex = D3DUtil::getSubresourceIndex(
                mip + sourceRange.mipLevel,
                layer + sourceRange.baseArrayLayer,
                0,
                srcDesc->numMipLevels,
                srcDesc->arraySize);
            auto dstSubresourceIndex = D3DUtil::getSubresourceIndex(
                mip + destRange.mipLevel,
                layer + destRange.baseArrayLayer,
                0,
                dstDesc->numMipLevels,
                dstDesc->arraySize);

            DXGI_FORMAT format = D3DUtil::getMapFormat(srcDesc->format);

            m_commandBuffer->m_cmdList->ResolveSubresource(
                dstTexture->m_resource.getResource(),
                dstSubresourceIndex,
                srcTexture->m_resource.getResource(),
                srcSubresourceIndex,
                format);
        }
    }
}

void ResourceCommandEncoderImpl::resolveQuery(
    IQueryPool* queryPool, uint32_t index, uint32_t count, IBufferResource* buffer, uint64_t offset)
{
    auto queryBase = static_cast<QueryPoolBase*>(queryPool);
    switch (queryBase->m_desc.type)
    {
    case QueryType::AccelerationStructureCompactedSize:
    case QueryType::AccelerationStructureCurrentSize:
    case QueryType::AccelerationStructureSerializedSize:
        {
            auto queryPoolImpl = static_cast<PlainBufferProxyQueryPoolImpl*>(queryPool);
            auto bufferImpl = static_cast<BufferResourceImpl*>(buffer);
            auto srcQueryBuffer = queryPoolImpl->m_bufferResource->m_resource.getResource();

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barrier.Transition.pResource = srcQueryBuffer;
            m_commandBuffer->m_cmdList->ResourceBarrier(1, &barrier);

            m_commandBuffer->m_cmdList->CopyBufferRegion(
                bufferImpl->m_resource.getResource(),
                offset,
                srcQueryBuffer,
                index * sizeof(uint64_t),
                count * sizeof(uint64_t));

            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            barrier.Transition.pResource = srcQueryBuffer;
            m_commandBuffer->m_cmdList->ResourceBarrier(1, &barrier);
        }
        break;
    default:
        {
            auto queryPoolImpl = static_cast<QueryPoolImpl*>(queryPool);
            auto bufferImpl = static_cast<BufferResourceImpl*>(buffer);
            m_commandBuffer->m_cmdList->ResolveQueryData(
                queryPoolImpl->m_queryHeap.get(),
                queryPoolImpl->m_queryType,
                index,
                count,
                bufferImpl->m_resource.getResource(),
                offset);
        }
        break;
    }
}

void ResourceCommandEncoderImpl::copyTextureToBuffer(
    IBufferResource* dst,
    size_t dstOffset,
    size_t dstSize,
    size_t dstRowStride,
    ITextureResource* src,
    ResourceState srcState,
    SubresourceRange srcSubresource,
    ITextureResource::Offset3D srcOffset,
    ITextureResource::Size extent)
{
    assert(srcSubresource.mipLevelCount <= 1);

    auto srcTexture = static_cast<TextureResourceImpl*>(src);
    auto dstBuffer = static_cast<BufferResourceImpl*>(dst);
    auto baseSubresourceIndex = D3DUtil::getSubresourceIndex(
        srcSubresource.mipLevel,
        srcSubresource.baseArrayLayer,
        0,
        srcTexture->getDesc()->numMipLevels,
        srcTexture->getDesc()->arraySize);
    auto textureSize = srcTexture->getDesc()->size;
    FormatInfo formatInfo = {};
    gfxGetFormatInfo(srcTexture->getDesc()->format, &formatInfo);
    if (srcSubresource.mipLevelCount == 0)
        srcSubresource.mipLevelCount = srcTexture->getDesc()->numMipLevels;
    if (srcSubresource.layerCount == 0)
        srcSubresource.layerCount = srcTexture->getDesc()->arraySize;

    for (uint32_t layer = 0; layer < srcSubresource.layerCount; layer++)
    {
        // Get the footprint
        D3D12_RESOURCE_DESC texDesc = srcTexture->m_resource.getResource()->GetDesc();

        D3D12_TEXTURE_COPY_LOCATION dstRegion = {};
        dstRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dstRegion.pResource = dstBuffer->m_resource.getResource();
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT& footprint = dstRegion.PlacedFootprint;

        D3D12_TEXTURE_COPY_LOCATION srcRegion = {};
        srcRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcRegion.SubresourceIndex = D3DUtil::getSubresourceIndex(
            srcSubresource.mipLevel,
            layer + srcSubresource.baseArrayLayer,
            0,
            srcTexture->getDesc()->numMipLevels,
            srcTexture->getDesc()->arraySize);
        srcRegion.pResource = srcTexture->m_resource.getResource();

        footprint.Offset = dstOffset;
        footprint.Footprint.Format = texDesc.Format;
        uint32_t mipLevel = srcSubresource.mipLevel;
        if (extent.width != 0xFFFFFFFF)
        {
            footprint.Footprint.Width = extent.width;
        }
        else
        {
            footprint.Footprint.Width = Math::Max(1, (textureSize.width >> mipLevel)) - srcOffset.x;
        }
        if (extent.height != 0xFFFFFFFF)
        {
            footprint.Footprint.Height = extent.height;
        }
        else
        {
            footprint.Footprint.Height =
                Math::Max(1, (textureSize.height >> mipLevel)) - srcOffset.y;
        }
        if (extent.depth != 0xFFFFFFFF)
        {
            footprint.Footprint.Depth = extent.depth;
        }
        else
        {
            footprint.Footprint.Depth = Math::Max(1, (textureSize.depth >> mipLevel)) - srcOffset.z;
        }

        assert(dstRowStride % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT == 0);
        footprint.Footprint.RowPitch = (UINT)dstRowStride;

        auto bufferSize =
            footprint.Footprint.RowPitch * footprint.Footprint.Height * footprint.Footprint.Depth;

        D3D12_BOX srcBox = {};
        srcBox.left = srcOffset.x;
        srcBox.top = srcOffset.y;
        srcBox.front = srcOffset.z;
        srcBox.right = srcOffset.x + extent.width;
        srcBox.bottom = srcOffset.y + extent.height;
        srcBox.back = srcOffset.z + extent.depth;
        m_commandBuffer->m_cmdList->CopyTextureRegion(&dstRegion, 0, 0, 0, &srcRegion, &srcBox);
    }
}

void ResourceCommandEncoderImpl::textureSubresourceBarrier(
    ITextureResource* texture,
    SubresourceRange subresourceRange,
    ResourceState src,
    ResourceState dst)
{
    auto textureImpl = static_cast<TextureResourceImpl*>(texture);

    ShortList<D3D12_RESOURCE_BARRIER> barriers;
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    if (src == dst && src == ResourceState::UnorderedAccess)
    {
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = textureImpl->m_resource.getResource();
        barriers.add(barrier);
    }
    else
    {
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.StateBefore = D3DUtil::getResourceState(src);
        barrier.Transition.StateAfter = D3DUtil::getResourceState(dst);
        if (barrier.Transition.StateBefore == barrier.Transition.StateAfter)
            return;
        barrier.Transition.pResource = textureImpl->m_resource.getResource();
        auto d3dFormat = D3DUtil::getMapFormat(textureImpl->getDesc()->format);
        auto aspectMask = (int32_t)subresourceRange.aspectMask;
        if (subresourceRange.aspectMask == TextureAspect::Default)
            aspectMask = (int32_t)TextureAspect::Color;
        while (aspectMask)
        {
            auto aspect = Math::getLowestBit((int32_t)aspectMask);
            aspectMask &= ~aspect;
            auto planeIndex = D3DUtil::getPlaneSlice(d3dFormat, (TextureAspect)aspect);
            for (uint32_t layer = 0; layer < subresourceRange.layerCount; layer++)
            {
                for (uint32_t mip = 0; mip < subresourceRange.mipLevelCount; mip++)
                {
                    barrier.Transition.Subresource = D3DUtil::getSubresourceIndex(
                        mip + subresourceRange.mipLevel,
                        layer + subresourceRange.baseArrayLayer,
                        planeIndex,
                        textureImpl->getDesc()->numMipLevels,
                        textureImpl->getDesc()->arraySize);
                    barriers.add(barrier);
                }
            }
        }
    }
    m_commandBuffer->m_cmdList->ResourceBarrier(
        (UINT)barriers.getCount(), barriers.getArrayView().getBuffer());
}

void ResourceCommandEncoderImpl::beginDebugEvent(const char* name, float rgbColor[3])
{
    auto beginEvent = m_commandBuffer->m_renderer->m_BeginEventOnCommandList;
    if (beginEvent)
    {
        beginEvent(
            m_commandBuffer->m_cmdList,
            0xff000000 | (uint8_t(rgbColor[0] * 255.0f) << 16) |
                (uint8_t(rgbColor[1] * 255.0f) << 8) | uint8_t(rgbColor[2] * 255.0f),
            name);
    }
}

void ResourceCommandEncoderImpl::endDebugEvent()
{
    auto endEvent = m_commandBuffer->m_renderer->m_EndEventOnCommandList;
    if (endEvent)
    {
        endEvent(m_commandBuffer->m_cmdList);
    }
}

void RenderCommandEncoderImpl::init(
    DeviceImpl* renderer,
    TransientResourceHeapImpl* transientHeap,
    CommandBufferImpl* cmdBuffer,
    RenderPassLayoutImpl* renderPass,
    FramebufferImpl* framebuffer)
{
    PipelineCommandEncoder::init(cmdBuffer);
    m_preCmdList = nullptr;
    m_renderPass = renderPass;
    m_framebuffer = framebuffer;
    m_transientHeap = transientHeap;
    m_boundVertexBuffers.clear();
    m_boundIndexBuffer = nullptr;
    m_primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_boundIndexFormat = DXGI_FORMAT_UNKNOWN;
    m_boundIndexOffset = 0;
    m_currentPipeline = nullptr;

    // Set render target states.
    if (!framebuffer)
    {
        return;
    }
    m_d3dCmdList->OMSetRenderTargets(
        (UINT)framebuffer->renderTargetViews.getCount(),
        framebuffer->renderTargetDescriptors.getArrayView().getBuffer(),
        FALSE,
        framebuffer->depthStencilView ? &framebuffer->depthStencilDescriptor : nullptr);

    // Issue clear commands based on render pass set up.
    for (Index i = 0; i < framebuffer->renderTargetViews.getCount(); i++)
    {
        if (i >= renderPass->m_renderTargetAccesses.getCount())
            continue;

        auto& access = renderPass->m_renderTargetAccesses[i];

        // Transit resource states.
        {
            D3D12BarrierSubmitter submitter(m_d3dCmdList);
            auto resourceViewImpl = framebuffer->renderTargetViews[i].Ptr();
            if (resourceViewImpl)
            {
                auto textureResource =
                    static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
                if (textureResource)
                {
                    D3D12_RESOURCE_STATES initialState;
                    if (access.initialState == ResourceState::Undefined)
                    {
                        initialState = textureResource->m_defaultState;
                    }
                    else
                    {
                        initialState = D3DUtil::getResourceState(access.initialState);
                    }
                    textureResource->m_resource.transition(
                        initialState, D3D12_RESOURCE_STATE_RENDER_TARGET, submitter);
                }
            }
        }
        // Clear.
        if (access.loadOp == IRenderPassLayout::AttachmentLoadOp::Clear)
        {
            m_d3dCmdList->ClearRenderTargetView(
                framebuffer->renderTargetDescriptors[i],
                framebuffer->renderTargetClearValues[i].values,
                0,
                nullptr);
        }
    }

    if (renderPass->m_hasDepthStencil)
    {
        // Transit resource states.
        {
            D3D12BarrierSubmitter submitter(m_d3dCmdList);
            auto resourceViewImpl = framebuffer->depthStencilView.Ptr();
            auto textureResource =
                static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
            D3D12_RESOURCE_STATES initialState;
            if (renderPass->m_depthStencilAccess.initialState == ResourceState::Undefined)
            {
                initialState = textureResource->m_defaultState;
            }
            else
            {
                initialState =
                    D3DUtil::getResourceState(renderPass->m_depthStencilAccess.initialState);
            }
            textureResource->m_resource.transition(
                initialState, D3D12_RESOURCE_STATE_DEPTH_WRITE, submitter);
        }
        // Clear.
        uint32_t clearFlags = 0;
        if (renderPass->m_depthStencilAccess.loadOp == IRenderPassLayout::AttachmentLoadOp::Clear)
        {
            clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        }
        if (renderPass->m_depthStencilAccess.stencilLoadOp ==
            IRenderPassLayout::AttachmentLoadOp::Clear)
        {
            clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        }
        if (clearFlags)
        {
            m_d3dCmdList->ClearDepthStencilView(
                framebuffer->depthStencilDescriptor,
                (D3D12_CLEAR_FLAGS)clearFlags,
                framebuffer->depthStencilClearValue.depth,
                framebuffer->depthStencilClearValue.stencil,
                0,
                nullptr);
        }
    }
}

Result RenderCommandEncoderImpl::bindPipeline(IPipelineState* state, IShaderObject** outRootObject)
{
    return bindPipelineImpl(state, outRootObject);
}

Result RenderCommandEncoderImpl::bindPipelineWithRootObject(
    IPipelineState* state, IShaderObject* rootObject)
{
    return bindPipelineWithRootObjectImpl(state, rootObject);
}

void RenderCommandEncoderImpl::setViewports(uint32_t count, const Viewport* viewports)
{
    static const int kMaxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    assert(count <= kMaxViewports && count <= kMaxRTVCount);
    for (UInt ii = 0; ii < count; ++ii)
    {
        auto& inViewport = viewports[ii];
        auto& dxViewport = m_viewports[ii];

        dxViewport.TopLeftX = inViewport.originX;
        dxViewport.TopLeftY = inViewport.originY;
        dxViewport.Width = inViewport.extentX;
        dxViewport.Height = inViewport.extentY;
        dxViewport.MinDepth = inViewport.minZ;
        dxViewport.MaxDepth = inViewport.maxZ;
    }
    m_d3dCmdList->RSSetViewports(UINT(count), m_viewports);
}

void RenderCommandEncoderImpl::setScissorRects(uint32_t count, const ScissorRect* rects)
{
    static const int kMaxScissorRects = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    assert(count <= kMaxScissorRects && count <= kMaxRTVCount);

    for (UInt ii = 0; ii < count; ++ii)
    {
        auto& inRect = rects[ii];
        auto& dxRect = m_scissorRects[ii];

        dxRect.left = LONG(inRect.minX);
        dxRect.top = LONG(inRect.minY);
        dxRect.right = LONG(inRect.maxX);
        dxRect.bottom = LONG(inRect.maxY);
    }

    m_d3dCmdList->RSSetScissorRects(UINT(count), m_scissorRects);
}

void RenderCommandEncoderImpl::setPrimitiveTopology(PrimitiveTopology topology)
{
    m_primitiveTopologyType = D3DUtil::getPrimitiveType(topology);
    m_primitiveTopology = D3DUtil::getPrimitiveTopology(topology);
}

void RenderCommandEncoderImpl::setVertexBuffers(
    uint32_t startSlot,
    uint32_t slotCount,
    IBufferResource* const* buffers,
    const uint32_t* offsets)
{
    {
        const Index num = startSlot + slotCount;
        if (num > m_boundVertexBuffers.getCount())
        {
            m_boundVertexBuffers.setCount(num);
        }
    }

    for (UInt i = 0; i < slotCount; i++)
    {
        BufferResourceImpl* buffer = static_cast<BufferResourceImpl*>(buffers[i]);

        BoundVertexBuffer& boundBuffer = m_boundVertexBuffers[startSlot + i];
        boundBuffer.m_buffer = buffer;
        boundBuffer.m_offset = int(offsets[i]);
    }
}

void RenderCommandEncoderImpl::setIndexBuffer(
    IBufferResource* buffer, Format indexFormat, uint32_t offset)
{
    m_boundIndexBuffer = (BufferResourceImpl*)buffer;
    m_boundIndexFormat = D3DUtil::getMapFormat(indexFormat);
    m_boundIndexOffset = offset;
}

void RenderCommandEncoderImpl::prepareDraw()
{
    auto pipelineState = m_currentPipeline.Ptr();
    if (!pipelineState || (pipelineState->desc.type != PipelineType::Graphics))
    {
        assert(!"No graphics pipeline state set");
        return;
    }

    // Submit - setting for graphics
    {
        GraphicsSubmitter submitter(m_d3dCmdList);
        RefPtr<PipelineStateBase> newPipeline;
        if (SLANG_FAILED(_bindRenderState(&submitter, newPipeline)))
        {
            assert(!"Failed to bind render state");
        }
    }

    m_d3dCmdList->IASetPrimitiveTopology(m_primitiveTopology);

    // Set up vertex buffer views
    {
        auto inputLayout = (InputLayoutImpl*)pipelineState->inputLayout.Ptr();
        if (inputLayout)
        {
            int numVertexViews = 0;
            D3D12_VERTEX_BUFFER_VIEW vertexViews[16];
            for (Index i = 0; i < m_boundVertexBuffers.getCount(); i++)
            {
                const BoundVertexBuffer& boundVertexBuffer = m_boundVertexBuffers[i];
                BufferResourceImpl* buffer = boundVertexBuffer.m_buffer;
                if (buffer)
                {
                    D3D12_VERTEX_BUFFER_VIEW& vertexView = vertexViews[numVertexViews++];
                    vertexView.BufferLocation =
                        buffer->m_resource.getResource()->GetGPUVirtualAddress() +
                        boundVertexBuffer.m_offset;
                    vertexView.SizeInBytes =
                        UINT(buffer->getDesc()->sizeInBytes - boundVertexBuffer.m_offset);
                    vertexView.StrideInBytes = inputLayout->m_vertexStreamStrides[i];
                }
            }
            m_d3dCmdList->IASetVertexBuffers(0, numVertexViews, vertexViews);
        }
    }
    // Set up index buffer
    if (m_boundIndexBuffer)
    {
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
        indexBufferView.BufferLocation =
            m_boundIndexBuffer->m_resource.getResource()->GetGPUVirtualAddress() +
            m_boundIndexOffset;
        indexBufferView.SizeInBytes =
            UINT(m_boundIndexBuffer->getDesc()->sizeInBytes - m_boundIndexOffset);
        indexBufferView.Format = m_boundIndexFormat;

        m_d3dCmdList->IASetIndexBuffer(&indexBufferView);
    }
}

void RenderCommandEncoderImpl::draw(uint32_t vertexCount, uint32_t startVertex)
{
    prepareDraw();
    m_d3dCmdList->DrawInstanced(vertexCount, 1, startVertex, 0);
}

void RenderCommandEncoderImpl::drawIndexed(
    uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex)
{
    prepareDraw();
    m_d3dCmdList->DrawIndexedInstanced(indexCount, 1, startIndex, baseVertex, 0);
}

void RenderCommandEncoderImpl::endEncoding()
{
    PipelineCommandEncoder::endEncodingImpl();
    if (!m_framebuffer)
        return;
    // Issue clear commands based on render pass set up.
    for (Index i = 0; i < m_renderPass->m_renderTargetAccesses.getCount(); i++)
    {
        auto& access = m_renderPass->m_renderTargetAccesses[i];

        // Transit resource states.
        {
            D3D12BarrierSubmitter submitter(m_d3dCmdList);
            auto resourceViewImpl = m_framebuffer->renderTargetViews[i].Ptr();
            if (!resourceViewImpl)
                continue;
            auto textureResource =
                static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
            if (textureResource)
            {
                textureResource->m_resource.transition(
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    D3DUtil::getResourceState(access.finalState),
                    submitter);
            }
        }
    }

    if (m_renderPass->m_hasDepthStencil)
    {
        // Transit resource states.
        D3D12BarrierSubmitter submitter(m_d3dCmdList);
        auto resourceViewImpl = m_framebuffer->depthStencilView.Ptr();
        auto textureResource =
            static_cast<TextureResourceImpl*>(resourceViewImpl->m_resource.Ptr());
        textureResource->m_resource.transition(
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            D3DUtil::getResourceState(m_renderPass->m_depthStencilAccess.finalState),
            submitter);
    }
    m_framebuffer = nullptr;
}

void RenderCommandEncoderImpl::setStencilReference(uint32_t referenceValue)
{
    m_d3dCmdList->OMSetStencilRef((UINT)referenceValue);
}

void RenderCommandEncoderImpl::drawIndirect(
    uint32_t maxDrawCount,
    IBufferResource* argBuffer,
    uint64_t argOffset,
    IBufferResource* countBuffer,
    uint64_t countOffset)
{
    prepareDraw();

    auto argBufferImpl = static_cast<BufferResourceImpl*>(argBuffer);
    auto countBufferImpl = static_cast<BufferResourceImpl*>(countBuffer);

    m_d3dCmdList->ExecuteIndirect(
        m_renderer->drawIndirectCmdSignature,
        maxDrawCount,
        argBufferImpl->m_resource,
        argOffset,
        countBufferImpl ? countBufferImpl->m_resource.getResource() : nullptr,
        countOffset);
}

void RenderCommandEncoderImpl::drawIndexedIndirect(
    uint32_t maxDrawCount,
    IBufferResource* argBuffer,
    uint64_t argOffset,
    IBufferResource* countBuffer,
    uint64_t countOffset)
{
    prepareDraw();

    auto argBufferImpl = static_cast<BufferResourceImpl*>(argBuffer);
    auto countBufferImpl = static_cast<BufferResourceImpl*>(countBuffer);

    m_d3dCmdList->ExecuteIndirect(
        m_renderer->drawIndexedIndirectCmdSignature,
        maxDrawCount,
        argBufferImpl->m_resource,
        argOffset,
        countBufferImpl ? countBufferImpl->m_resource.getResource() : nullptr,
        countOffset);
}

Result RenderCommandEncoderImpl::setSamplePositions(
    uint32_t samplesPerPixel, uint32_t pixelCount, const SamplePosition* samplePositions)
{
    if (m_commandBuffer->m_cmdList1)
    {
        m_commandBuffer->m_cmdList1->SetSamplePositions(
            samplesPerPixel, pixelCount, (D3D12_SAMPLE_POSITION*)samplePositions);
        return SLANG_OK;
    }
    return SLANG_E_NOT_AVAILABLE;
}

void RenderCommandEncoderImpl::drawInstanced(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t startVertex,
    uint32_t startInstanceLocation)
{
    prepareDraw();
    m_d3dCmdList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstanceLocation);
}

void RenderCommandEncoderImpl::drawIndexedInstanced(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t startIndexLocation,
    int32_t baseVertexLocation,
    uint32_t startInstanceLocation)
{
    prepareDraw();
    m_d3dCmdList->DrawIndexedInstanced(
        indexCount, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void ComputeCommandEncoderImpl::endEncoding() { PipelineCommandEncoder::endEncodingImpl(); }

void ComputeCommandEncoderImpl::init(
    DeviceImpl* renderer, TransientResourceHeapImpl* transientHeap, CommandBufferImpl* cmdBuffer)
{
    PipelineCommandEncoder::init(cmdBuffer);
    m_preCmdList = nullptr;
    m_transientHeap = transientHeap;
    m_currentPipeline = nullptr;
}

Result ComputeCommandEncoderImpl::bindPipeline(IPipelineState* state, IShaderObject** outRootObject)
{
    return bindPipelineImpl(state, outRootObject);
}

Result ComputeCommandEncoderImpl::bindPipelineWithRootObject(
    IPipelineState* state, IShaderObject* rootObject)
{
    return bindPipelineWithRootObjectImpl(state, rootObject);
}

void ComputeCommandEncoderImpl::dispatchCompute(int x, int y, int z)
{
    // Submit binding for compute
    {
        ComputeSubmitter submitter(m_d3dCmdList);
        RefPtr<PipelineStateBase> newPipeline;
        if (SLANG_FAILED(_bindRenderState(&submitter, newPipeline)))
        {
            assert(!"Failed to bind render state");
        }
    }
    m_d3dCmdList->Dispatch(x, y, z);
}

void ComputeCommandEncoderImpl::dispatchComputeIndirect(IBufferResource* argBuffer, uint64_t offset)
{
    // Submit binding for compute
    {
        ComputeSubmitter submitter(m_d3dCmdList);
        RefPtr<PipelineStateBase> newPipeline;
        if (SLANG_FAILED(_bindRenderState(&submitter, newPipeline)))
        {
            assert(!"Failed to bind render state");
        }
    }
    auto argBufferImpl = static_cast<BufferResourceImpl*>(argBuffer);

    m_d3dCmdList->ExecuteIndirect(
        m_renderer->dispatchIndirectCmdSignature, 1, argBufferImpl->m_resource, offset, nullptr, 0);
}

FenceImpl::~FenceImpl()
{
    if (m_waitEvent)
        CloseHandle(m_waitEvent);
}

HANDLE FenceImpl::getWaitEvent()
{
    if (m_waitEvent)
        return m_waitEvent;
    m_waitEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    return m_waitEvent;
}

Result FenceImpl::init(DeviceImpl* device, const IFence::Desc& desc)
{
    SLANG_RETURN_ON_FAIL(device->m_device->CreateFence(
        desc.initialValue,
        desc.isShared ? D3D12_FENCE_FLAG_SHARED : D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(m_fence.writeRef())));
    return SLANG_OK;
}

Result FenceImpl::getCurrentValue(uint64_t* outValue)
{
    *outValue = m_fence->GetCompletedValue();
    return SLANG_OK;
}

Result FenceImpl::setCurrentValue(uint64_t value)
{
    SLANG_RETURN_ON_FAIL(m_fence->Signal(value));
    return SLANG_OK;
}

Result FenceImpl::getSharedHandle(InteropHandle* outHandle)
{
    // Check if a shared handle already exists.
    if (sharedHandle.handleValue != 0)
    {
        *outHandle = sharedHandle;
        return SLANG_OK;
    }

    ComPtr<ID3D12Device> devicePtr;
    m_fence->GetDevice(IID_PPV_ARGS(devicePtr.writeRef()));
    SLANG_RETURN_ON_FAIL(devicePtr->CreateSharedHandle(
        m_fence, NULL, GENERIC_ALL, nullptr, (HANDLE*)&outHandle->handleValue));
    outHandle->api = InteropHandleAPI::D3D12;
    sharedHandle = *outHandle;
    return SLANG_OK;
}

Result FenceImpl::getNativeHandle(InteropHandle* outNativeHandle)
{
    outNativeHandle->api = gfx::InteropHandleAPI::D3D12;
    outNativeHandle->handleValue = (uint64_t)m_fence.get();
    return SLANG_OK;
}

} // namespace d3d12

Result SLANG_MCALL createD3D12Device(const IDevice::Desc* desc, IDevice** outDevice)
{
    RefPtr<d3d12::DeviceImpl> result = new d3d12::DeviceImpl();
    SLANG_RETURN_ON_FAIL(result->initialize(*desc));
    returnComPtr(outDevice, result);
    return SLANG_OK;
}
} // namespace gfx
