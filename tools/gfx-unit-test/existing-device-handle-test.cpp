#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    void existingDeviceHandleTestImpl(IDevice* device, UnitTestContext* context)
    {
        Slang::ComPtr<ITransientResourceHeap> transientHeap;
        ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096;
        GFX_CHECK_CALL_ABORT(
            device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadShaderProgram(device, shaderProgram, "compute-smoke", slangReflection));

        ComputePipelineStateDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<gfx::IPipelineState> pipelineState;
        GFX_CHECK_CALL_ABORT(
            device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

        const int numberCount = 4;
        float initialData[] = { 0.0f, 1.0f, 2.0f, 3.0f };
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.sizeInBytes = numberCount * sizeof(float);
        bufferDesc.format = gfx::Format::Unknown;
        bufferDesc.elementSize = sizeof(float);
        bufferDesc.allowedStates = ResourceStateSet(
            ResourceState::ShaderResource,
            ResourceState::UnorderedAccess,
            ResourceState::CopyDestination,
            ResourceState::CopySource);
        bufferDesc.defaultState = ResourceState::UnorderedAccess;
        bufferDesc.cpuAccessFlags = AccessFlag::Write | AccessFlag::Read;

        ComPtr<IBufferResource> numbersBuffer;
        GFX_CHECK_CALL_ABORT(device->createBufferResource(
            bufferDesc,
            (void*)initialData,
            numbersBuffer.writeRef()));

        ComPtr<IResourceView> bufferView;
        IResourceView::Desc viewDesc = {};
        viewDesc.type = IResourceView::Type::UnorderedAccess;
        viewDesc.format = Format::Unknown;
        GFX_CHECK_CALL_ABORT(device->createBufferView(numbersBuffer, viewDesc, bufferView.writeRef()));

        // We have done all the set up work, now it is time to start recording a command buffer for
        // GPU execution.
        {
            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeComputeCommands();

            auto rootObject = encoder->bindPipeline(pipelineState);

            slang::TypeReflection* addTransformerType =
                slangReflection->findTypeByName("AddTransformer");

            // Now we can use this type to create a shader object that can be bound to the root object.
            ComPtr<IShaderObject> transformer;
            GFX_CHECK_CALL_ABORT(device->createShaderObject(
                addTransformerType, ShaderObjectContainerType::None, transformer.writeRef()));
            // Set the `c` field of the `AddTransformer`.
            float c = 1.0f;
            ShaderCursor(transformer).getPath("c").setData(&c, sizeof(float));

            ShaderCursor entryPointCursor(
                rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.
            // Bind buffer view to the entry point.
            entryPointCursor.getPath("buffer").setResource(bufferView);

            // Bind the previously created transformer object to root object.
            entryPointCursor.getPath("transformer").setObject(transformer);

            encoder->dispatchCompute(1, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->wait();
        }

        compareComputeResult(
            device,
            numbersBuffer,
            Slang::makeArray<float>(11.0f, 12.0f, 13.0f, 14.0f));
    }

    void existingDeviceHandleTestAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
    {
        if ((api & context->enabledApis) == 0)
        {
            SLANG_IGNORE_TEST;
        }
        Slang::ComPtr<IDevice> device;
        IDevice::Desc deviceDesc = {};
        switch (api)
        {
        case Slang::RenderApiFlag::D3D11:
            deviceDesc.deviceType = gfx::DeviceType::DirectX11;
            break;
        case Slang::RenderApiFlag::D3D12:
            deviceDesc.deviceType = gfx::DeviceType::DirectX12;
            break;
        case Slang::RenderApiFlag::Vulkan:
            deviceDesc.deviceType = gfx::DeviceType::Vulkan;
            break;
        default:
            SLANG_IGNORE_TEST;
        }
        deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
        const char* searchPaths[] = { "", "../../tools/gfx-unit-test", "tools/gfx-unit-test" };
        deviceDesc.slang.searchPathCount = (SlangInt)SLANG_COUNT_OF(searchPaths);
        deviceDesc.slang.searchPaths = searchPaths;
        auto createDeviceResult = gfxCreateDevice(&deviceDesc, device.writeRef());
        if (SLANG_FAILED(createDeviceResult))
        {
            SLANG_IGNORE_TEST;
        }

        IDevice::NativeHandle handle = {};
        GFX_CHECK_CALL_ABORT(device->getNativeHandle(&handle));
        Slang::ComPtr<IDevice> testDevice;
        IDevice::Desc testDeviceDesc = deviceDesc;
        testDeviceDesc.existingDeviceHandles = handle;
        auto createTestDeviceResult = gfxCreateDevice(&testDeviceDesc, testDevice.writeRef());
        if (SLANG_FAILED(createTestDeviceResult))
        {
            SLANG_IGNORE_TEST;
        }

        existingDeviceHandleTestImpl(testDevice, context);
    }

    SLANG_UNIT_TEST(existingDeviceHandleD3D12)
    {
        return existingDeviceHandleTestAPI(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(existingDeviceHandleVulkan)
    {
        return existingDeviceHandleTestAPI(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

}
