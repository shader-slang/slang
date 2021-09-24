#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    SlangResult existingDeviceHandleTestImpl(IDevice* device, slang::UnitTestContext* context)
    {
        Slang::ComPtr<ITransientResourceHeap> transientHeap;
        ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096;
        SLANG_RETURN_ON_FAIL(
            device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        SLANG_RETURN_ON_FAIL(loadShaderProgram(device, shaderProgram, context->outputWriter, "compute-smoke", slangReflection));

        ComputePipelineStateDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<gfx::IPipelineState> pipelineState;
        SLANG_RETURN_ON_FAIL(
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
        SLANG_RETURN_ON_FAIL(device->createBufferResource(
            bufferDesc,
            (void*)initialData,
            numbersBuffer.writeRef()));

        ComPtr<IResourceView> bufferView;
        IResourceView::Desc viewDesc = {};
        viewDesc.type = IResourceView::Type::UnorderedAccess;
        viewDesc.format = Format::Unknown;
        SLANG_RETURN_ON_FAIL(device->createBufferView(numbersBuffer, viewDesc, bufferView.writeRef()));

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
            SLANG_RETURN_ON_FAIL(device->createShaderObject(
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

        return compareComputeResult(
            device,
            numbersBuffer,
            Slang::makeArray<float>(11.0f, 12.0f, 13.0f, 14.0f));
    }

    SlangResult existingDeviceHandleTestAPI(slang::UnitTestContext* context, Slang::RenderApiFlag::Enum api)
    {
        if ((api & context->enabledApis) == 0)
        {
            return SLANG_E_NOT_AVAILABLE;
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
            return SLANG_E_NOT_AVAILABLE;
        }
        auto createDeviceResult = gfxCreateDevice(&deviceDesc, device.writeRef());
        if (SLANG_FAILED(createDeviceResult))
        {
            return SLANG_E_NOT_AVAILABLE;
        }

        IDevice::NativeHandle handle = {};
        SLANG_RETURN_ON_FAIL(device->getNativeHandle(&handle));
        Slang::ComPtr<IDevice> testDevice;
        IDevice::Desc testDeviceDesc = {};
        switch (api)
        {
        case Slang::RenderApiFlag::D3D11:
            testDeviceDesc.deviceType = gfx::DeviceType::DirectX11;
            break;
        case Slang::RenderApiFlag::D3D12:
            testDeviceDesc.deviceType = gfx::DeviceType::DirectX12;
            break;
        case Slang::RenderApiFlag::Vulkan:
            testDeviceDesc.deviceType = gfx::DeviceType::Vulkan;
            break;
        default:
            return SLANG_E_NOT_AVAILABLE;
        }
        testDeviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
        const char* searchPaths[] = { "", "../../tools/gfx-test", "tools/gfx-test" };
        testDeviceDesc.slang.searchPathCount = (SlangInt)SLANG_COUNT_OF(searchPaths);
        testDeviceDesc.slang.searchPaths = searchPaths;
        testDeviceDesc.existingDeviceHandles = handle;
        auto createTestDeviceResult = gfxCreateDevice(&testDeviceDesc, testDevice.writeRef());
        if (SLANG_FAILED(createTestDeviceResult))
        {
            return SLANG_E_NOT_AVAILABLE;
        }

        SLANG_RETURN_ON_FAIL(existingDeviceHandleTestImpl(testDevice, context));
        return SLANG_OK;
    }

    SLANG_UNIT_TEST(existingDeviceHandleD3D12)
    {
        return existingDeviceHandleTestAPI(context, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(existingDeviceHandleVulkan)
    {
        return existingDeviceHandleTestAPI(context, Slang::RenderApiFlag::Vulkan);
    }

}
