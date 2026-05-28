#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{

// Runtime validation of Metal pointer-in-buffer lowering.
//
// The host writes a real device address (as uint64_t) into a structured
// buffer, and the shader reads it back as `int*` through the lowered
// ulong representation. This exercises the CastIntToPtr / CastPtrToInt
// roundtrip with actual GPU addresses — something the TEST_INPUT-based
// filecheck tests cannot do because they can't express cross-buffer
// address references.
void pointerInBufferRoundtripTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "pointer-in-buffer-roundtrip",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipeline.writeRef()));

    // Buffer holding the int values we want to read through a pointer.
    int32_t dataValues[] = {42, 100};
    BufferDesc dataDesc = {};
    dataDesc.size = sizeof(dataValues);
    dataDesc.elementSize = sizeof(int32_t);
    dataDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                     BufferUsage::CopyDestination | BufferUsage::CopySource;
    dataDesc.defaultState = ResourceState::UnorderedAccess;
    dataDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> dataBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(dataDesc, (void*)dataValues, dataBuffer.writeRef()));

    // Get the GPU address of dataBuffer and store it as a uint64_t.
    // This is the pointer value that the shader will interpret as `int*`.
    DeviceAddress dataAddr = dataBuffer->getDeviceAddress();
    uint64_t ptrValue = static_cast<uint64_t>(dataAddr);

    // Buffer holding a single pointer (as uint64_t / ulong on Metal).
    // The shader sees this as StructuredBuffer<int*>.
    BufferDesc ptrDesc = {};
    ptrDesc.size = sizeof(uint64_t);
    ptrDesc.elementSize = sizeof(uint64_t);
    ptrDesc.usage = BufferUsage::ShaderResource | BufferUsage::CopyDestination;
    ptrDesc.defaultState = ResourceState::ShaderResource;
    ptrDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> ptrBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(ptrDesc, (void*)&ptrValue, ptrBuffer.writeRef()));

    // Output buffer.
    int32_t outputInit[] = {0};
    BufferDesc outputDesc = {};
    outputDesc.size = sizeof(int32_t);
    outputDesc.elementSize = sizeof(int32_t);
    outputDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess |
                       BufferUsage::CopyDestination | BufferUsage::CopySource;
    outputDesc.defaultState = ResourceState::UnorderedAccess;
    outputDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> outputBuffer;
    GFX_CHECK_CALL_ABORT(
        device->createBuffer(outputDesc, (void*)outputInit, outputBuffer.writeRef()));

    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        {
            auto encoder = commandEncoder->beginComputePass();
            auto rootObject = encoder->bindPipeline(pipeline);
            ShaderCursor rootCursor(rootObject);

            rootCursor.getPath("ptrs").setBinding(Binding(ptrBuffer));
            rootCursor.getPath("output").setBinding(Binding(outputBuffer));

            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    compareComputeResult(device, outputBuffer, std::array{42});
}

SLANG_UNIT_TEST(pointerInBufferRoundtripVulkan)
{
    runTestImpl(pointerInBufferRoundtripTestImpl, unitTestContext, DeviceType::Vulkan);
}

SLANG_UNIT_TEST(pointerInBufferRoundtripMetal)
{
    runTestImpl(pointerInBufferRoundtripTestImpl, unitTestContext, DeviceType::Metal);
}

} // namespace gfx_test
