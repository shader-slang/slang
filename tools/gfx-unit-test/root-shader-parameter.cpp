// Duplicated: This test is identical slang-rhi\tests\test-root-shader-parameter.cpp

#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
static ComPtr<IBuffer> createBuffer(IDevice* device, uint32_t content)
{
    ComPtr<IBuffer> buffer;
    BufferDesc bufferDesc = {};
    bufferDesc.size = sizeof(uint32_t);
    bufferDesc.format = rhi::Format::Undefined;
    bufferDesc.elementSize = sizeof(float);
    bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    bufferDesc.defaultState = ResourceState::UnorderedAccess;
    bufferDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> numbersBuffer;
    GFX_CHECK_CALL_ABORT(device->createBuffer(bufferDesc, (void*)&content, buffer.writeRef()));

    return buffer;
}
void rootShaderParameterTestImpl(IDevice* device, UnitTestContext* context)
{
    if (!device->hasFeature(Feature::ParameterBlock))
    {
        SLANG_CHECK("no support for parameter blocks");
    }

    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "root-shader-parameter",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<rhi::IComputePipeline> pipeline = device->createComputePipeline(pipelineDesc);

    Slang::List<ComPtr<IBuffer>> buffers;

    for (uint32_t i = 0; i < 9; i++)
    {
        buffers.add(createBuffer(device, i == 0 ? 10 : i));
    }

    ComPtr<IShaderObject> rootObject;
    device->createRootShaderObject(shaderProgram, rootObject.writeRef());

    ComPtr<IShaderObject> g, s1, s2;
    device->createShaderObject(
        slangReflection->findTypeByName("S0"),
        ShaderObjectContainerType::None,
        g.writeRef());
    device->createShaderObject(
        slangReflection->findTypeByName("S1"),
        ShaderObjectContainerType::None,
        s1.writeRef());
    device->createShaderObject(
        slangReflection->findTypeByName("S1"),
        ShaderObjectContainerType::None,
        s2.writeRef());

    {
        auto cursor = ShaderCursor(s1);
        cursor["c0"].setBinding(buffers[2]);
        cursor["c1"].setBinding(buffers[3]);
        cursor["c2"].setBinding(buffers[4]);
    }
    {
        auto cursor = ShaderCursor(s2);
        cursor["c0"].setBinding(buffers[5]);
        cursor["c1"].setBinding(buffers[6]);
        cursor["c2"].setBinding(buffers[7]);
    }
    {
        auto cursor = ShaderCursor(g);
        cursor["b0"].setBinding(buffers[0]);
        cursor["b1"].setBinding(buffers[1]);
        cursor["s1"].setObject(s1);
        cursor["s2"].setObject(s2);
    }
    {
        auto cursor = ShaderCursor(rootObject);
        cursor["g"].setObject(g);
        cursor["buffer"].setBinding(buffers[8]);
    }

    {
        auto queue = device->getQueue(QueueType::Graphics);

        auto commandBuffer = queue->createCommandEncoder();
        {
            auto encoder = commandBuffer->beginComputePass();
            encoder->bindPipeline(pipeline, rootObject);
            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }

        queue->submit(commandBuffer->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, buffers[8], std::array{10 - 1 + 2 - 3 + 4 + 5 - 6 + 7});
}

SLANG_UNIT_TEST(rootShaderParameterD3D12)
{
    runTestImpl(rootShaderParameterTestImpl, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(rootShaderParameterVulkan)
{
    runTestImpl(rootShaderParameterTestImpl, unitTestContext, DeviceType::Vulkan);
}
} // namespace gfx_test
