// Duplicated: This this test is identical to slang-rhi\tests\test-uint16-buffer.cpp

#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
void uint16BufferTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(
        loadComputeProgram(device, shaderProgram, "uint16-buffer", "computeMain", slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    const int numberCount = 4;
    uint16_t initialData[] = {0, 1, 2, 3};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(uint16_t);
    bufferDesc.format = rhi::Format::Undefined;

    bufferDesc.elementSize = 0; // Let RHI derive from reflection
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        auto computePassEncoder = commandEncoder->beginComputePass();

        auto rootObject = computePassEncoder->bindPipeline(pipelineState);

        // Bind buffer to the entry point.
        ShaderCursor(rootObject).getPath("buffer").setBinding(Binding(numbersBuffer));

        computePassEncoder->dispatchCompute(1, 1, 1);
        computePassEncoder->end();
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array<uint16_t, 4>{1, 2, 3, 4});
}

SLANG_UNIT_TEST(uint16BufferTestD3D12)
{
    runTestImpl(uint16BufferTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(uint16BufferTestVulkan)
{
    runTestImpl(uint16BufferTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
