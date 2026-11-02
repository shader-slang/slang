#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
static const uint32_t kWidth = 4;
static const uint32_t kHeight = 4;
static const uint32_t kLayerCount = 4;

static ComPtr<ITexture> createLayeredTexture(IDevice* device, TextureType type, Format format)
{
    const uint32_t height = type == TextureType::Texture2DArray ? kHeight : 1;

    TextureDesc desc = {};
    desc.type = type;
    desc.size = {kWidth, height, 1};
    desc.arrayLength = kLayerCount;
    desc.mipCount = 1;
    desc.format = format;
    desc.usage =
        TextureUsage::UnorderedAccess | TextureUsage::CopySource | TextureUsage::CopyDestination;
    desc.defaultState = ResourceState::UnorderedAccess;

    uint32_t zeros[kWidth * kHeight] = {};
    SubresourceData initData[kLayerCount];
    for (auto& data : initData)
    {
        data.data = zeros;
        data.rowPitch = kWidth * sizeof(uint32_t);
        data.slicePitch = kWidth * height * sizeof(uint32_t);
    }

    ComPtr<ITexture> texture;
    GFX_CHECK_CALL_ABORT(device->createTexture(desc, initData, texture.writeRef()));
    return texture;
}

// `tolerance` is the allowed per-channel error. Unorm8 needs one code, because 0.25, 0.5 and 0.75
// scale to 63.75, 127.5 and 191.25, and the rounding is up to the hardware.
static void checkSingleTexelWritten(
    IDevice* device,
    ITexture* texture,
    uint32_t x,
    uint32_t y,
    uint32_t layer,
    std::array<int, 4> expected,
    int tolerance)
{
    for (uint32_t l = 0; l < kLayerCount; ++l)
    {
        ComPtr<ISlangBlob> blob;
        SubresourceLayout layout;
        GFX_CHECK_CALL_ABORT(device->readTexture(texture, l, 0, blob.writeRef(), &layout));
        for (uint32_t ty = 0; ty < layout.size.height; ++ty)
        {
            for (uint32_t tx = 0; tx < layout.size.width; ++tx)
            {
                const uint8_t* texel = (const uint8_t*)blob->getBufferPointer() +
                                       ty * layout.rowPitch + tx * layout.colPitch;
                const bool isWritten = tx == x && ty == y && l == layer;
                for (int i = 0; i < 4; ++i)
                {
                    SLANG_CHECK(abs(int(texel[i]) - (isWritten ? expected[i] : 0)) <= tolerance);
                }
            }
        }
    }
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

    ComPtr<ITexture> texture2DArray =
        createLayeredTexture(device, TextureType::Texture2DArray, Format::RGBA8Unorm);
    ComPtr<ITexture> texture1DArray =
        createLayeredTexture(device, TextureType::Texture1DArray, Format::RGBA8Unorm);
    ComPtr<ITexture> uintTexture1DArray =
        createLayeredTexture(device, TextureType::Texture1DArray, Format::RGBA8Uint);

    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto encoder = commandEncoder->beginComputePass();
        auto rootObject = encoder->bindPipeline(pipelineState);
        ShaderCursor cursor(rootObject);
        cursor["texture2DArray"].setBinding(Binding(texture2DArray));
        cursor["texture1DArray"].setBinding(Binding(texture1DArray));
        cursor["uintTexture1DArray"].setBinding(Binding(uintTexture1DArray));
        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    checkSingleTexelWritten(device, texture2DArray, 2, 1, 3, {64, 128, 191, 255}, 1);
    checkSingleTexelWritten(device, texture1DArray, 2, 0, 3, {255, 191, 128, 64}, 1);
    checkSingleTexelWritten(device, uintTexture1DArray, 2, 0, 3, {1, 2, 3, 4}, 0);
}

SLANG_UNIT_TEST(cudaLayeredFormatWriteCUDA)
{
    runTestImpl(cudaLayeredFormatWriteTestImpl, unitTestContext, DeviceType::CUDA);
}

} // namespace gfx_test
