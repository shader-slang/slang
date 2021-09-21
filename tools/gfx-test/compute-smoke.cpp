#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    SlangResult computeSmokeTestImpl(IDevice* device, slang::UnitTestContext* context)
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

    SLANG_UNIT_TEST(computeSmoke)
    {
        Slang::ComPtr<IDevice> device;
        IDevice::Desc deviceDesc = {};
        deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
        const char* searchPaths[] = { "", "../../tools/gfx-test", "tools/gfx-test" };
        deviceDesc.slang.searchPathCount = (SlangInt)(sizeof(searchPaths) / sizeof(const char*));
        deviceDesc.slang.searchPaths = searchPaths;
        auto createDeviceResult = gfxCreateDevice(&deviceDesc, device.writeRef());
        if (SLANG_FAILED(createDeviceResult))
            return SLANG_E_NOT_AVAILABLE;

        return computeSmokeTestImpl(device, context);
    }

}
