#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
// Verifies that a store through a `[format("rgba8")]` RWTexture1DArray / RWTexture2DArray of
// `float4` converts the texel to unorm8 on CUDA (issue #13554). The texel is read back on the host
// after the dispatch completes, because a kernel cannot portably read back a surface it has just
// written, and CUDA has no formatted layered surface read.
static ComPtr<ITexture> createLayeredTexture(IDevice* device, TextureType type)
{
    const uint32_t width = 2;
    const uint32_t height = type == TextureType::Texture2DArray ? 2 : 1;
    const uint32_t layerCount = 2;

    TextureDesc desc = {};
    desc.type = type;
    desc.size = {width, height, 1};
    desc.arrayLength = layerCount;
    desc.mipCount = 1;
    desc.format = Format::RGBA8Unorm;
    desc.usage = TextureUsage::UnorderedAccess | TextureUsage::CopySource |
                 TextureUsage::CopyDestination;
    desc.defaultState = ResourceState::UnorderedAccess;

    uint32_t zeros[width * height] = {};
    SubresourceData initData[layerCount];
    for (auto& data : initData)
    {
        data.data = zeros;
        data.rowPitch = width * sizeof(uint32_t);
        data.slicePitch = width * height * sizeof(uint32_t);
    }

    ComPtr<ITexture> texture;
    GFX_CHECK_CALL_ABORT(device->createTexture(desc, initData, texture.writeRef()));
    return texture;
}

// Returns the texel at (x, y) of `layer`, packed as RGBA8 with R in the low byte.
static uint32_t readTexel(IDevice* device, ITexture* texture, uint32_t layer, uint32_t x, uint32_t y)
{
    ComPtr<ISlangBlob> blob;
    SubresourceLayout layout;
    GFX_CHECK_CALL_ABORT(device->readTexture(texture, layer, 0, blob.writeRef(), &layout));
    const uint8_t* texel = (const uint8_t*)blob->getBufferPointer() + y * layout.rowPitch +
                           x * layout.colPitch;
    uint32_t value = 0;
    ::memcpy(&value, texel, sizeof(value));
    return value;
}

void cudaLayeredFormatWriteTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "cuda-layered-format-write",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    ComPtr<ITexture> texture2DArray = createLayeredTexture(device, TextureType::Texture2DArray);
    ComPtr<ITexture> texture1DArray = createLayeredTexture(device, TextureType::Texture1DArray);

    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto encoder = commandEncoder->beginComputePass();
        auto rootObject = encoder->bindPipeline(pipelineState);
        ShaderCursor cursor(rootObject);
        cursor["texture2DArray"].setBinding(Binding(texture2DArray));
        cursor["texture1DArray"].setBinding(Binding(texture1DArray));
        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    // float4(0.25, 0.5, 0.75, 1.0) as unorm8 is (64, 128, 191, 255).
    SLANG_CHECK(readTexel(device, texture2DArray, 1, 1, 0) == 0xFFBF8040u);
    SLANG_CHECK(readTexel(device, texture2DArray, 0, 1, 0) == 0u);
    SLANG_CHECK(readTexel(device, texture2DArray, 1, 0, 0) == 0u);

    // float4(1.0, 0.75, 0.5, 0.25) as unorm8 is (255, 191, 128, 64).
    SLANG_CHECK(readTexel(device, texture1DArray, 1, 1, 0) == 0x4080BFFFu);
    SLANG_CHECK(readTexel(device, texture1DArray, 0, 1, 0) == 0u);
    SLANG_CHECK(readTexel(device, texture1DArray, 1, 0, 0) == 0u);
}

SLANG_UNIT_TEST(cudaLayeredFormatWrite)
{
    runTestImpl(cudaLayeredFormatWriteTestImpl, unitTestContext, DeviceType::CUDA);
}

} // namespace gfx_test
