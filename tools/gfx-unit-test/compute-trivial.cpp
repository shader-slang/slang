#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
void computeTrivialTestImpl(IDevice* device, UnitTestContext* context)
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

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        {
            auto encoder = commandEncoder->beginComputePass();
            auto rootObject = encoder->bindPipeline(pipelineState);

            // Bind buffer directly to the entry point.
            ShaderCursor(rootObject).getPath("buffer").setBinding(Binding(numbersBuffer));

            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }

        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{1.0f, 2.0f, 3.0f, 4.0f});
}

SLANG_UNIT_TEST(computeTrivialD3D12)
{
    runTestImpl(computeTrivialTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(computeTrivialD3D11)
{
    runTestImpl(computeTrivialTestImpl, unitTestContext, DeviceType::D3D11);
}

SLANG_UNIT_TEST(computeTrivialVulkan)
{
    runTestImpl(computeTrivialTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test