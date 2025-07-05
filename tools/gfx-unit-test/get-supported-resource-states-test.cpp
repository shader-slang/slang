/*
 * This test has been disabled because the slang-rhi API
 * does not provide equivalent functionality for querying format-supported
 * resource states. The old gfx API's getFormatSupportedResourceStates() is
 * replaced by with IDevice::getFormatSupport.
 */

#if 0
// Disabled: no equivalent API in slang-rhi

#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace Slang;
using namespace rhi;

namespace
{
using namespace gfx_test;

struct GetSupportedResourceStatesBase
{
    IDevice* device;
    UnitTestContext* context;

    ResourceStateSet formatSupportedStates;
    ResourceStateSet textureAllowedStates;
    ResourceStateSet bufferAllowedStates;

    ComPtr<ITexture> texture;
    ComPtr<IBuffer> buffer;

    void init(IDevice* device, UnitTestContext* context)
    {
        this->device = device;
        this->context = context;
    }

    void checkResult()
    {
        SLANG_CHECK_ABORT(formatSupportedStates.isSubsetOf(bufferAllowedStates));
        SLANG_CHECK_ABORT(formatSupportedStates.isSubsetOf(textureAllowedStates));

        auto queue = device->getQueue(QueueType::Graphics);
        ComPtr<ICommandEncoder> encoder = queue->createCommandEncoder();

        encoder->setBufferState(buffer, ResourceState::UnorderedAccess);

        encoder->setTextureState(texture, SubresourceRange{}, ResourceState::UnorderedAccess);

        ComPtr<ICommandBuffer> commandBuffer;
        encoder->finish(commandBuffer.writeRef());
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    void run()
    {
        switch (format)
        {
        case Format::R8Unorm:
        case Format::RG8Unorm:
        case Format::RGBA8Unorm:
        case Format::RGBA8UnormSrgb:
        case Format::B8G8R8A8Unorm:
        case Format::B8G8R8A8UnormSrgb:
        case Format::R16Float:
        case Format::RG16Float:
        case Format::RGB16Float:
        case Format::RGBA16Float:
        case Format::R32Float:
        case Format::RG32Float:
        case Format::RGB32Float:
        case Format::RGBA32Float:
        case Format::R8G8Typeless:
        case Format::R8Typeless:
        case Format::B8G8R8A8Typeless:
        case Format::R10G10B10A2Typeless:
        case Format::Undefined:
            break;
        }

        auto formatInfo = getFormatInfo(format);

        if (!isTypelessFormat(format))
        {
            GFX_CHECK_CALL_ABORT(device->getFormatSupportedResourceStates(format, &formatSupportedStates));
        }

        textureAllowedStates = ResourceStateSet(
            ResourceState::ShaderResource, ResourceState::UnorderedAccess, ResourceState::RenderTarget);

        BufferDesc bufferDesc = {};
        bufferDesc.size = 256;
        bufferDesc.format = format;
        bufferDesc.defaultState = ResourceState::UnorderedAccess;
        bufferDesc.usage = BufferUsage::UnorderedAccess;

        buffer = device->createBuffer(bufferDesc, nullptr);

        TextureDesc textureDesc = {};
        textureDesc.type = TextureType::Texture2D;
        textureDesc.mipCount = dstTextureInfo.numMipLevels;
        textureDesc.arrayLength = dstTextureInfo.arraySize;
        textureDesc.size = extent;
        textureDesc.defaultState = ResourceState::UnorderedAccess;
        textureDesc.usage = TextureUsage::UnorderedAccess;
        textureDesc.format = format;
        textureDesc.format = (format != Format::Undefined) ? format : Format::Undefined;

        texture = device->createTexture(textureDesc, nullptr);

        checkResult();
    }
};

template<typename T>
void getSupportedResourceStatesTestImpl(IDevice* device, UnitTestContext* context)
{
    T test;
    test.init(device, context);
    test.run();
}
} // namespace

namespace gfx_test
{
SLANG_UNIT_TEST(getSupportedResourceStatesD3D12)
{
    runTestImpl(
        getSupportedResourceStatesTestImpl<GetSupportedResourceStatesBase>,
        unitTestContext,
        DeviceType::D3D12);
}

SLANG_UNIT_TEST(getSupportedResourceStatesVulkan)
{
    runTestImpl(
        getSupportedResourceStatesTestImpl<GetSupportedResourceStatesBase>,
        unitTestContext,
        DeviceType::Vulkan);
}
} // namespace gfx_test
#endif
