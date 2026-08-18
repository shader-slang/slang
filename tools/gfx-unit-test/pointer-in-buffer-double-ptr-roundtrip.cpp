#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{

// Runtime validation of double-pointer (int**) in a StorageBuffer.
//
// Builds a three-level pointer chain entirely from the host side:
//   ptrBuffer -> midBuffer -> leafBuffer -> {42}
//
// The shader reads int** from StructuredBuffer<int**> (lowered to ulong
// in StorageBuffer address space), casts it to int* via CastIntToPtr,
// dereferences to get int*, casts again, and dereferences to get the
// final int value. This validates that both levels of the CastIntToPtr
// chain produce correct GPU addresses.
void pointerInBufferDoublePtrRoundtripTestImpl(IDevice* device, UnitTestContext* context)
{
    ComPtr<IShaderProgram> shaderProgram;
    slang::ProgramLayout* slangReflection;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        shaderProgram,
        "pointer-in-buffer-double-ptr-roundtrip",
        "computeMain",
        slangReflection));

    ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram.get();
    ComPtr<IComputePipeline> pipeline;
    GFX_CHECK_CALL_ABORT(device->createComputePipeline(pipelineDesc, pipeline.writeRef()));

    auto makeBuffer = [&](size_t size,
                          uint32_t elementSize,
                          const void* data,
                          BufferUsage usage,
                          ResourceState state) -> ComPtr<IBuffer>
    {
        BufferDesc desc = {};
        desc.size = size;
        desc.elementSize = elementSize;
        desc.format = Format::Undefined;
        desc.usage = usage | BufferUsage::CopyDestination;
        desc.defaultState = state;
        desc.memoryType = MemoryType::DeviceLocal;
        ComPtr<IBuffer> buf;
        GFX_CHECK_CALL_ABORT(device->createBuffer(desc, data, buf.writeRef()));
        return buf;
    };

    // Level 0: the final int value.
    int32_t leafData[] = {42};
    auto leafBuffer = makeBuffer(
        sizeof(leafData),
        sizeof(int32_t),
        leafData,
        BufferUsage::ShaderResource,
        ResourceState::ShaderResource);

    // Level 1: holds the address of leafBuffer (the int* value).
    uint64_t leafAddr = leafBuffer->getDeviceAddress();
    auto midBuffer = makeBuffer(
        sizeof(uint64_t),
        sizeof(uint64_t),
        &leafAddr,
        BufferUsage::ShaderResource,
        ResourceState::ShaderResource);

    // Level 2: holds the address of midBuffer (the int** value).
    // This is what the shader reads from StructuredBuffer<int**>.
    uint64_t midAddr = midBuffer->getDeviceAddress();
    auto ptrBuffer = makeBuffer(
        sizeof(uint64_t),
        sizeof(uint64_t),
        &midAddr,
        BufferUsage::ShaderResource,
        ResourceState::ShaderResource);

    // Output buffer.
    int32_t outputInit[] = {0};
    auto outputBuffer = makeBuffer(
        sizeof(int32_t),
        sizeof(int32_t),
        outputInit,
        BufferUsage::ShaderResource | BufferUsage::UnorderedAccess | BufferUsage::CopySource,
        ResourceState::UnorderedAccess);

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

SLANG_UNIT_TEST(pointerInBufferDoublePtrRoundtripVulkan)
{
    runTestImpl(pointerInBufferDoublePtrRoundtripTestImpl, unitTestContext, DeviceType::Vulkan);
}

SLANG_UNIT_TEST(pointerInBufferDoublePtrRoundtripMetal)
{
    runTestImpl(pointerInBufferDoublePtrRoundtripTestImpl, unitTestContext, DeviceType::Metal);
}

} // namespace gfx_test
