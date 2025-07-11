#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
void computeSmokeTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(
        loadComputeProgram(device, shaderProgram, "compute-smoke", "computeMain", slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    const int numberCount = 4;
    float initialData[] = {0.0f, 1.0f, 2.0f, 3.0f};
    BufferDesc bufferDesc = {};
    bufferDesc.size = numberCount * sizeof(float);
    bufferDesc.format = rhi::Format::Undefined;
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
        auto encoder = commandEncoder->beginComputePass();

        auto rootObject = encoder->bindPipeline(pipelineState);

        slang::TypeReflection* addTransformerType =
            slangReflection->findTypeByName("AddTransformer");

        // Now we can use this type to create a shader object that can be bound to the root object.
        ComPtr<IShaderObject> transformer;
        GFX_CHECK_CALL_ABORT(device->createShaderObject(
            addTransformerType,
            ShaderObjectContainerType::None,
            transformer.writeRef()));
        // Set the `c` field of the `AddTransformer`.
        float c = 1.0f;
        ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));

        ShaderCursor entryPointCursor(
            rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
        // Bind buffer to the entry point.
        entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

        // Bind the previously created transformer object to root object.
        entryPointCursor.getPath("transformer").setObject(transformer);

        encoder->dispatchCompute(1, 1, 1);
        encoder->end();
        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{11.0f, 12.0f, 13.0f, 14.0f});
}

SLANG_UNIT_TEST(computeSmokeD3D12)
{
    runTestImpl(computeSmokeTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(computeSmokeD3D11)
{
    runTestImpl(computeSmokeTestImpl, unitTestContext, DeviceType::D3D11);
}

SLANG_UNIT_TEST(computeSmokeVulkan)
{
    runTestImpl(computeSmokeTestImpl, unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
