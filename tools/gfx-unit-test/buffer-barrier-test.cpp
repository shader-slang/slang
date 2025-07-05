#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi/shader-cursor.h>

using namespace rhi;

namespace gfx_test
{
struct Shader
{
    ComPtr<IShaderProgram> program;
    slang::ProgramLayout* reflection = nullptr;
    ComputePipelineDesc pipelineDesc = {};
    ComPtr<IComputePipeline> pipeline;
};

struct Buffer
{
    BufferDesc desc;
    ComPtr<IBuffer> buffer;
    ComPtr<ITextureView> view;
};

ComPtr<IBuffer> createFloatBuffer(
    IDevice* device,
    bool unorderedAccess,
    size_t elementCount,
    float* initialData = nullptr)
{
    BufferDesc desc = {};
    desc.size = elementCount * sizeof(float);
    desc.elementSize = sizeof(float);
    desc.format = Format::Undefined;
    desc.memoryType = MemoryType::DeviceLocal;
    desc.usage =
        BufferUsage::ShaderResource | BufferUsage::CopyDestination | BufferUsage::CopySource;
    if (unorderedAccess)
        desc.usage |= BufferUsage::UnorderedAccess;

    ComPtr<IBuffer> buffer;
    GFX_CHECK_CALL_ABORT(device->createBuffer(desc, (void*)initialData, buffer.writeRef()));
    return buffer;
}

void barrierTestImpl(IDevice* device, UnitTestContext* context)
{
    Shader programA;
    Shader programB;
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        programA.program,
        "buffer-barrier-test",
        "computeA",
        programA.reflection));
    GFX_CHECK_CALL_ABORT(loadComputeProgram(
        device,
        programB.program,
        "buffer-barrier-test",
        "computeB",
        programB.reflection));
    programA.pipelineDesc.program = programA.program.get();
    programB.pipelineDesc.program = programB.program.get();
    GFX_CHECK_CALL_ABORT(
        device->createComputePipeline(programA.pipelineDesc, programA.pipeline.writeRef()));

    GFX_CHECK_CALL_ABORT(
        device->createComputePipeline(programB.pipelineDesc, programB.pipeline.writeRef()));

    float initialData[] = {1.0f, 2.0f, 3.0f, 4.0f};
    ComPtr<IBuffer> inputBuffer = createFloatBuffer(device, false, 4, initialData);
    ComPtr<IBuffer> intermediateBuffer = createFloatBuffer(device, true, 4, nullptr);
    ComPtr<IBuffer> outputBuffer = createFloatBuffer(device, true, 4, nullptr);

    // We have done all the set up work, now it is time to start recording a command buffer for
    // GPU execution.
    {
        auto queue = device->getQueue(QueueType::Graphics);
        auto commandEncoder = queue->createCommandEncoder();

        // Write inputBuffer data to intermediateBuffer
        {
            auto passEncoder = commandEncoder->beginComputePass();
            auto rootObject = passEncoder->bindPipeline(programA.pipeline);

            ShaderCursor cursor(rootObject->getEntryPoint(0));
            cursor["inBuffer"].setBinding(inputBuffer);
            cursor["outBuffer"].setBinding(intermediateBuffer);
            passEncoder->dispatchCompute(1, 1, 1);
            passEncoder->end();
        }

        // Resource transition is automatically handled.

        // Write intermediateBuffer data to outputBuffer

        {
            auto passEncoder = commandEncoder->beginComputePass();
            auto rootObject = passEncoder->bindPipeline(programB.pipeline);
            ShaderCursor cursor(rootObject->getEntryPoint(0));
            cursor["inBuffer"].setBinding(intermediateBuffer);
            cursor["outBuffer"].setBinding(outputBuffer);
            passEncoder->dispatchCompute(1, 1, 1);
            passEncoder->end();
        }


        queue->submit(commandEncoder->finish());
        queue->waitOnHost();
    }


    compareComputeResult(device, outputBuffer, makeArray<float>(11.0f, 12.0f, 13.0f, 14.0f));
}

void barrierTestAPI(UnitTestContext* context, DeviceType deviceType)
{
    Slang::List<const char*> searchPaths = {"", "../../tools/gfx-unit-test", "tools/gfx-unit-test"};
    auto device = createTestingDevice(context, deviceType, searchPaths);

    if (!device)
    {
        SLANG_IGNORE_TEST
    }

    barrierTestImpl(device.get(), context);
}

SLANG_UNIT_TEST(bufferBarrierVulkan)
{
    barrierTestAPI(unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
