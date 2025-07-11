#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
void sharedBufferTestImpl(IDevice* srcDevice, IDevice* dstDevice, UnitTestContext* context)
{
    // Create a shareable buffer using srcDevice, get its handle, then create a buffer using the
    // handle using dstDevice. Read back the buffer and check that its contents are correct.
    const int numberCount = 4;
    float initialData[] = {0.0f, 1.0f, 2.0f, 3.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopySource | BufferUsage::CopyDestination | BufferUsage::Shared;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> srcBuffer;
    GFX_CHECK_CALL_ABORT(
        srcDevice->createBuffer(bufferDesc, (void*)initialData, srcBuffer.writeRef()));

    NativeHandle sharedHandle;
    GFX_CHECK_CALL_ABORT(srcBuffer->getSharedHandle(&sharedHandle));
    ComPtr<IBuffer> dstBuffer;
    GFX_CHECK_CALL_ABORT(
        dstDevice->createBufferFromSharedHandle(sharedHandle, bufferDesc, dstBuffer.writeRef()));
    // Reading back the buffer from srcDevice to make sure it's been filled in before reading
    // anything back from dstDevice
    // TODO: Implement actual synchronization (and not this hacky solution)
    compareComputeResult(srcDevice, srcBuffer, std::array{0.0f, 1.0f, 2.0f, 3.0f});

    NativeHandle testHandle;
    GFX_CHECK_CALL_ABORT(dstBuffer->getNativeHandle(&testHandle));
    const BufferDesc& testDesc = dstBuffer->getDesc();
    SLANG_CHECK(testDesc.elementSize == sizeof(float));
    SLANG_CHECK(testDesc.size == numberCount * sizeof(float));
    compareComputeResult(dstDevice, dstBuffer, std::array{0.0f, 1.0f, 2.0f, 3.0f});

    // Check that dstBuffer can be successfully used in a compute dispatch using dstDevice.
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        dstDevice,
        shaderProgram,
        "compute-trivial",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(dstDevice->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    auto queue = dstDevice->getQueue(QueueType::Graphics);
    auto commandEncoder = queue->createCommandEncoder();
    auto computePassEncoder = commandEncoder->beginComputePass();

    auto rootObject = computePassEncoder->bindPipeline(pipelineState);

    ShaderCursor rootCursor(rootObject);
    // Bind buffer to the entry point.
    rootCursor.getPath("buffer").setBinding(Binding(dstBuffer));

    computePassEncoder->dispatchCompute(1, 1, 1);
    computePassEncoder->end();
    auto commandBuffer = commandEncoder->finish();
    queue->submit(commandBuffer);
    queue->waitOnHost();

    compareComputeResult(dstDevice, dstBuffer, std::array{1.0f, 2.0f, 3.0f, 4.0f});
}

void sharedBufferTestAPI(UnitTestContext* context, DeviceType srcApi, DeviceType dstApi)
{
    auto srcDevice = createTestingDevice(context, srcApi);
    auto dstDevice = createTestingDevice(context, dstApi);
    if (!srcDevice || !dstDevice)
    {
        SLANG_IGNORE_TEST;
    }

    sharedBufferTestImpl(srcDevice, dstDevice, context);
}
#if SLANG_WIN64
SLANG_UNIT_TEST(sharedBufferD3D12ToCUDA)
{
    sharedBufferTestAPI(unitTestContext, DeviceType::D3D12, DeviceType::CUDA);
}

SLANG_UNIT_TEST(sharedBufferVulkanToCUDA)
{
    sharedBufferTestAPI(unitTestContext, DeviceType::Vulkan, DeviceType::CUDA);
}
#endif
} // namespace gfx_test
