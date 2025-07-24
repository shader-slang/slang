#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

using namespace rhi;

namespace gfx_test
{
void mutableShaderObjectTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "mutable-shader-object",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipelineState;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipelineState.writeRef()));

    float initialData[] = {0.0f, 1.0f, 2.0f, 3.0f};
    const int numberCount = SLANG_COUNT_OF(initialData);
    BufferDesc bufferDesc = {};
    bufferDesc.size = sizeof(initialData);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    {
        slang::TypeReflection* addTransformerType =
            slangReflection->findTypeByName("AddTransformer");

        ComPtr<IShaderObject> transformer;
        GFX_CHECK_CALL_ABORT(device->createShaderObject(
            addTransformerType,
            ShaderObjectContainerType::None,
            transformer.writeRef()));

        // Set the `c` field of the `AddTransformer`.
        float c = 1.0f;
        ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));

        ComPtr<ICommandQueue> queue;
        GFX_CHECK_CALL_ABORT(device->getQueue(QueueType::Graphics, queue.writeRef()));

        // Create root shader object
        ComPtr<IShaderObject> rootObject;
        GFX_CHECK_CALL_ABORT(device->createRootShaderObject(shaderProgram, rootObject.writeRef()));

        auto commandEncoder = queue->createCommandEncoder();
        auto computeEncoder = commandEncoder->beginComputePass();

        // Bind pipeline with our root object
        computeEncoder->bindPipeline(pipelineState, rootObject);

        auto entryPointCursor = ShaderCursor(rootObject->getEntryPoint(0));

        entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

        // Bind the transformer object to root object.
        entryPointCursor.getPath("transformer").setObject(transformer);

        computeEncoder->dispatchCompute(1, 1, 1);
        computeEncoder->end();

        // Set buffer state to ensure writes are visible
        commandEncoder->setBufferState(numbersBuffer, ResourceState::UnorderedAccess);

        computeEncoder = commandEncoder->beginComputePass();

        // Bind pipeline with our root object again
        computeEncoder->bindPipeline(pipelineState, rootObject);

        // Mutate `transformer` object and run again.
        c = 2.0f;
        ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));
        entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));
        entryPointCursor.getPath("transformer").setObject(transformer);
        computeEncoder->dispatchCompute(1, 1, 1);
        computeEncoder->end();

        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{3.0f, 4.0f, 5.0f, 6.0f});
}

// SLANG_UNIT_TEST(mutableShaderObjectCPU)
//{
//     runTestImpl(mutableShaderObjectTestImpl, unitTestContext, Slang::RenderApiFlag::CPU);
// }

SLANG_UNIT_TEST(mutableShaderObjectD3D11)
{
    runTestImpl(mutableShaderObjectTestImpl, unitTestContext, DeviceType::D3D11);
}

SLANG_UNIT_TEST(mutableShaderObjectD3D12)
{
    runTestImpl(mutableShaderObjectTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(mutableShaderObjectVulkan)
{
    runTestImpl(mutableShaderObjectTestImpl, unitTestContext, DeviceType::Vulkan);
}
} // namespace gfx_test
