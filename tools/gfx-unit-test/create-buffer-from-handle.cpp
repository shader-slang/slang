#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
void createBufferFromHandleTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "compute-trivial",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    const int numberCount = 4;
    float initialData[] = {0.0f, 1.0f, 2.0f, 3.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> originalNumbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, originalNumbersBuffer.writeRef()));

    NativeHandle handle;
    originalNumbersBuffer->getNativeHandle(&handle);
    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBufferFromNativeHandle(handle, bufferDesc, numbersBuffer.writeRef()));
    compareComputeResult(device, numbersBuffer, std::array{0.0f, 1.0f, 2.0f, 3.0f});

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        {
            auto encoder = commandEncoder->beginComputePass();
            auto rootObject = encoder->bindPipeline(pipelineState);

            ShaderCursor rootCursor(rootObject);
            // Bind buffer directly to the entry point.
            rootCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }

        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{1.0f, 2.0f, 3.0f, 4.0f});
}

SLANG_UNIT_TEST(createBufferFromHandleD3D12)
{
    runTestImpl(createBufferFromHandleTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(createBufferFromHandleVulkan)
{
    runTestImpl(createBufferFromHandleTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
