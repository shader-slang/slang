#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{

// Runtime validation of pointer fields inside a ParameterBlock on Metal.
//
// ParameterBlock structs go through three lowering passes on Metal:
// 1. MetalParameterBlock (early) — converts resource fields to DescriptorHandle,
//    decorates the storage type with [PhysicalType].
// 2. Default (main) — lowers matrix/bool fields in all buffer types.
//    Also decorates with [PhysicalType].
// 3. MetalPointerLowering (late) — re-processes [PhysicalType]-decorated types
//    (via shouldSkipPhysicalTypes() override) to lower multi-level pointer fields.
// This test's struct has only pointer and scalar fields, so pass 2 is a no-op.
//
// Single-level int* fields should be preserved as typed device pointers since
// Metal accepts one level of pointer indirection in argument-buffer-bound structs.
// This test validates that the preserved pointer actually works at runtime.
void pointerInParamBlockRoundtripTestImpl(IDevice* device, UnitTestContext* context)
{
    if (!device->hasFeature(Feature::ParameterBlock))
    {
        SLANG_IGNORE_TEST
    }

    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "pointer-in-paramblock-roundtrip",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipeline.writeRef()));

    // Data buffer: the int values that the shader will read through the pointer.
    int32_t dataValues[] = {7, 3};
    BufferDesc dataDesc = {};
    dataDesc.size = sizeof(dataValues);
    dataDesc.elementSize = sizeof(int32_t);
    dataDesc.usage = BufferUsage::ShaderResource | BufferUsage::CopyDestination;
    dataDesc.defaultState = ResourceState::ShaderResource;
    dataDesc.memoryType = MemoryType::DeviceLocal;

    ComPtr<IBuffer> dataBuffer;
    GFX_CHECK_CALL_ABORT(device->createBuffer(dataDesc, (void*)dataValues, dataBuffer.writeRef()));

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

    // Create root shader object and ParameterBlock shader object.
    ComPtr<IShaderObject> rootObject;
    GFX_CHECK_CALL_ABORT(device->createRootShaderObject(shaderProgram, rootObject.writeRef()));

    ComPtr<IShaderObject> paramsObject;
    GFX_CHECK_CALL_ABORT(device->createShaderObject(
        slangReflection->findTypeByName("PtrParams"),
        ShaderObjectContainerType::None,
        paramsObject.writeRef()));

    // Bind the pointer field and scalar field inside the ParameterBlock.
    // Pointer-typed fields are raw device addresses (not resource descriptors),
    // so we write the address via setData rather than setBinding.
    {
        auto cursor = ShaderCursor(paramsObject);
        uint64_t dataAddr = dataBuffer->getDeviceAddress();
        cursor["data"].setData(&dataAddr, sizeof(dataAddr));
        int32_t scale = 5;
        cursor["scale"].setData(&scale, sizeof(scale));
    }

    // Bind the ParameterBlock and output buffer to the root object.
    {
        auto cursor = ShaderCursor(rootObject);
        cursor["params"].setObject(paramsObject);
        cursor["output"].setBinding(Binding(outputBuffer));
    }

    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();
        {
            auto encoder = commandEncoder->beginComputePass();
            encoder->bindPipeline(pipeline, rootObject);
            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }

    // data[0] * scale + data[1] = 7 * 5 + 3 = 38
    compareComputeResult(device, outputBuffer, std::array{38});
}

SLANG_UNIT_TEST(pointerInParamBlockRoundtripVulkan)
{
    runTestImpl(pointerInParamBlockRoundtripTestImpl, unitTestContext, DeviceType::Vulkan);
}

SLANG_UNIT_TEST(pointerInParamBlockRoundtripMetal)
{
    runTestImpl(pointerInParamBlockRoundtripTestImpl, unitTestContext, DeviceType::Metal);
}

} // namespace gfx_test
