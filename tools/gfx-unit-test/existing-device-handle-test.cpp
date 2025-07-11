#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{
void existingDeviceHandleTestImpl(IDevice* device, UnitTestContext* context)
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

            ShaderCursor rootCursor(rootObject);
            // Bind buffer directly to the root.
            rootCursor.getPath("buffer").setBinding(Binding(numbersBuffer));

            encoder->dispatchCompute(1, 1, 1);
            encoder->end();
        }

        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    compareComputeResult(device, numbersBuffer, std::array{1.0f, 2.0f, 3.0f, 4.0f});
}

void existingDeviceHandleTestAPI(UnitTestContext* context, DeviceType deviceType)
{
    if (!deviceTypeInEnabledApis(deviceType, context->enabledApis))
    {
        SLANG_IGNORE_TEST
    }
    Slang::ComPtr<IDevice> device;
    DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = deviceType;
    deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
    const char* searchPaths[] = {"", "../../tools/gfx-unit-test", "tools/gfx-unit-test"};
    deviceDesc.slang.searchPathCount = (SlangInt)SLANG_COUNT_OF(searchPaths);
    deviceDesc.slang.searchPaths = searchPaths;
    auto createDeviceResult = getRHI()->createDevice(deviceDesc, device.writeRef());
    if (SLANG_FAILED(createDeviceResult) || !device)
    {
        SLANG_IGNORE_TEST;
    }

    DeviceNativeHandles handles;
    GFX_CHECK_CALL_ABORT(device->getNativeDeviceHandles(&handles));
    Slang::ComPtr<IDevice> testDevice;
    DeviceDesc testDeviceDesc = deviceDesc;
    testDeviceDesc.existingDeviceHandles.handles[0] = handles.handles[0];
    if (deviceType == DeviceType::Vulkan)
    {
        testDeviceDesc.existingDeviceHandles.handles[1] = handles.handles[1];
        testDeviceDesc.existingDeviceHandles.handles[2] = handles.handles[2];
    }
    auto createTestDeviceResult = getRHI()->createDevice(testDeviceDesc, testDevice.writeRef());
    if (SLANG_FAILED(createTestDeviceResult) || !testDevice)
    {
        SLANG_IGNORE_TEST;
    }

    existingDeviceHandleTestImpl(testDevice, context);
}

SLANG_UNIT_TEST(existingDeviceHandleD3D12)
{
    return existingDeviceHandleTestAPI(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(existingDeviceHandleVulkan)
{
    return existingDeviceHandleTestAPI(unitTestContext, DeviceType::Vulkan);
}
#if SLANG_WIN64
SLANG_UNIT_TEST(existingDeviceHandleCUDA)
{
    return existingDeviceHandleTestAPI(unitTestContext, DeviceType::CUDA);
}
#endif
} // namespace gfx_test
