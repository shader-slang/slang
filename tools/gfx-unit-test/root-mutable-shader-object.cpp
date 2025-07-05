#if 0
// Duplicated: This test is identical to slang-rhi\tests\test-mutable-shader-object.cpp
// TODO: This test failed
// The result buffer is still {0.0f, 1.0f, 2.0f, 3.0f}. Not incremented by the shader

#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

using namespace rhi;

namespace gfx_test
{
void mutableRootShaderObjectTestImpl(IDevice* device, UnitTestContext* context)
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
    ComPtr<IComputePipeline> computePipeline;
    GFX_CHECK_CALL_ABORT(
        device->createComputePipeline(pipelineDesc, computePipeline.writeRef()));

    float initialData[] = {0.0f, 1.0f, 2.0f, 3.0f};
    const int numberCount = SLANG_COUNT_OF(initialData);
    BufferDesc bufferDesc = {};
    bufferDesc.size = sizeof(initialData);
    bufferDesc.format = Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::UnorderedAccess | BufferUsage::ShaderResource | BufferUsage::CopySource | BufferUsage::CopyDestination;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(bufferDesc, (void*)initialData, numbersBuffer.writeRef()));

    ComPtr<IShaderObject> rootObject;
    device->createRootShaderObject(shaderProgram, rootObject.writeRef());
    auto entryPointCursor = ShaderCursor(rootObject->getEntryPoint(0));
    entryPointCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

    slang::TypeReflection* addTransformerType = slangReflection->findTypeByName("AddTransformer");
    ComPtr<IShaderObject> transformer;
    GFX_CHECK_CALL_ABORT(device->createShaderObject(
        addTransformerType,
        ShaderObjectContainerType::None,
        transformer.writeRef()));
    entryPointCursor.getPath("transformer").setObject(transformer);

    // Set the `c` field of the `AddTransformer`.
    float c = 1.0f;
    ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));

    {
        auto queue = device->getQueue(QueueType::Graphics);

        auto commandEncoder = queue->createCommandEncoder();
        {
            auto encoder = commandEncoder->beginComputePass();
            encoder->bindPipeline(computePipeline, rootObject);
            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }

        // Set buffer state to ensure writes are visible
        commandEncoder->setBufferState(numbersBuffer, ResourceState::UnorderedAccess);

        // Mutate `transformer` object and run again.
        c = 2.0f;
        ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));
        {
            auto encoder = commandEncoder->beginComputePass();
            encoder->bindPipeline(computePipeline, rootObject);
            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }

        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{3.0f, 4.0f, 5.0f, 6.0f});
}

SLANG_UNIT_TEST(mutableRootShaderObjectD3D12)
{
    runTestImpl(mutableRootShaderObjectTestImpl, unitTestContext, DeviceType::D3D12, {});
}

/*SLANG_UNIT_TEST(mutableRootShaderObjectVulkan)
{
    runTestImpl(mutableRootShaderObjectTestImpl, unitTestContext, DeviceType::Vulkan, {});
}*/
} // namespace gfx_test

#endif