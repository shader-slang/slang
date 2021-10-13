#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    void setUpAndRunTest()
    {

    }

    void FormatBC1UnormTestImpl(IDevice* device, UnitTestContext* context)
    {
        Slang::ComPtr<ITransientResourceHeap> transientHeap;
        ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096;
        GFX_CHECK_CALL_ABORT(
            device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "format-test-shaders", "sampleTexFloat", slangReflection));

        ComputePipelineStateDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<gfx::IPipelineState> pipelineState;
        GFX_CHECK_CALL_ABORT(
            device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

        uint8_t texData[] = { 31, 0, 0, 0, 0, 0, 0, 0, 31, 0, 0, 0, 0, 0, 0, 0,
                              31, 0, 0, 0, 0, 0, 0, 0, 31, 0, 0, 0, 0, 0, 0, 0,
                              255, 255, 255, 255, 0, 0, 0, 0 };
        ITextureResource::Desc texDesc = {};
        texDesc.type = IResource::Type::Texture2D;
        texDesc.numMipLevels = 2;
        texDesc.arraySize = 1;
        texDesc.size.width = 8;
        texDesc.size.height = 8;
        texDesc.size.depth = 1;
        texDesc.defaultState = ResourceState::ShaderResource;
        texDesc.format = Format::BC1_Unorm;

        ITextureResource::SubresourceData subData[] = {
            ITextureResource::SubresourceData {(void*)texData, 16, 0},
            ITextureResource::SubresourceData {(void*)(texData + 32), 8, 0}
        };

        ComPtr<ITextureResource> inTex;
        GFX_CHECK_CALL_ABORT(device->createTextureResource(
            texDesc,
            subData,
            inTex.writeRef()));

        ComPtr<IResourceView> texView;
        IResourceView::Desc texViewDesc = {};
        texViewDesc.type = IResourceView::Type::ShaderResource;
        texViewDesc.format = Format::BC1_Unorm;
        GFX_CHECK_CALL_ABORT(device->createTextureView(inTex, texViewDesc, texView.writeRef()));

        const int numberCount = 64;
        float initialData[numberCount] = { 0.0f };
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

        ComPtr<IBufferResource> outBuffer;
        GFX_CHECK_CALL_ABORT(device->createBufferResource(
            bufferDesc,
            (void*)initialData,
            outBuffer.writeRef()));

        ComPtr<IResourceView> bufferView;
        IResourceView::Desc viewDesc = {};
        viewDesc.type = IResourceView::Type::UnorderedAccess;
        viewDesc.format = Format::Unknown;
        GFX_CHECK_CALL_ABORT(device->createBufferView(outBuffer, viewDesc, bufferView.writeRef()));

        ISamplerState::Desc samplerDesc;
        auto sampler = device->createSamplerState(samplerDesc);

        // We have done all the set up work, now it is time to start recording a command buffer for
        // GPU execution.
        {
            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);

            auto commandBuffer = transientHeap->createCommandBuffer();
            auto encoder = commandBuffer->encodeComputeCommands();

            auto rootObject = encoder->bindPipeline(pipelineState);

            ShaderCursor entryPointCursor(
                rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.

            // Bind texture view to the entry point
            entryPointCursor.getPath("tex").setResource(texView);

            entryPointCursor.getPath("sampler").setSampler(sampler);

            // Bind buffer view to the entry point.
            entryPointCursor.getPath("buffer").setResource(bufferView);

            encoder->dispatchCompute(1, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->wait();
        }

        compareComputeResult(
            device,
            outBuffer,
            Slang::makeArray<float>(0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
    }

    void FormatBC1UnormTestAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
    {
        if ((api & context->enabledApis) == 0)
        {
            SLANG_IGNORE_TEST
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
            SLANG_IGNORE_TEST
        }
        deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
        const char* searchPaths[] = { "", "../../tools/gfx-unit-test", "tools/gfx-unit-test" };
        deviceDesc.slang.searchPathCount = (SlangInt)SLANG_COUNT_OF(searchPaths);
        deviceDesc.slang.searchPaths = searchPaths;
        auto createDeviceResult = gfxCreateDevice(&deviceDesc, device.writeRef());
        if (SLANG_FAILED(createDeviceResult))
        {
            SLANG_IGNORE_TEST
        }

        FormatBC1UnormTestImpl(device, context);
    }

    SLANG_UNIT_TEST(FormatBC1UnormD3D12)
    {
        FormatBC1UnormTestAPI(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(FormatBC1UnormVulkan)
    {
        FormatBC1UnormTestAPI(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

}
