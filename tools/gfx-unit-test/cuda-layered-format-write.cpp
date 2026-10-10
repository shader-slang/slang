#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
// Creates a zero-filled two-layer RGBA8Unorm 1D or 2D array texture that a shader can write.
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
    desc.usage =
        TextureUsage::UnorderedAccess | TextureUsage::CopySource | TextureUsage::CopyDestination;
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

// Checks that the RGBA8 texel at (x, 0) of `layer` matches `expected` within one unit per channel,
// since the rounding of a float that lands exactly between two unorm8 values is up to the hardware.
static void checkTexel(
    IDevice* device,
    ITexture* texture,
    uint32_t layer,
    uint32_t x,
    std::array<int, 4> expected)
{
    ComPtr<ISlangBlob> blob;
    SubresourceLayout layout;
    GFX_CHECK_CALL_ABORT(device->readTexture(texture, layer, 0, blob.writeRef(), &layout));
    const uint8_t* texel = (const uint8_t*)blob->getBufferPointer() + x * layout.colPitch;
    for (int i = 0; i < 4; ++i)
    {
        SLANG_CHECK(abs(int(texel[i]) - expected[i]) <= 1);
    }
}

// Verifies that a store through a `[format("rgba8")]` RWTexture1DArray / RWTexture2DArray of
// `float4` converts the texel to unorm8 on CUDA (issue #13554). We read the texels back on the host
// after the dispatch completes, because CUDA has no formatted layered surface read and a kernel
// cannot rely on reading a surface it has just written.
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

    // Only texel x = 1 of layer 1 was written; its neighbours and layer 0 stay zero.
    checkTexel(device, texture2DArray, 1, 1, {64, 128, 191, 255});
    checkTexel(device, texture2DArray, 1, 0, {0, 0, 0, 0});
    checkTexel(device, texture2DArray, 0, 1, {0, 0, 0, 0});

    checkTexel(device, texture1DArray, 1, 1, {255, 191, 128, 64});
    checkTexel(device, texture1DArray, 1, 0, {0, 0, 0, 0});
    checkTexel(device, texture1DArray, 0, 1, {0, 0, 0, 0});
}

SLANG_UNIT_TEST(cudaLayeredFormatWrite)
{
    runTestImpl(cudaLayeredFormatWriteTestImpl, unitTestContext, DeviceType::CUDA);
}

} // namespace gfx_test
