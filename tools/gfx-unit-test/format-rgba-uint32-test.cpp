#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    void FormatRGBAUInt32TestImpl(IDevice* device, UnitTestContext* context)
    {
        Slang::ComPtr<ITransientResourceHeap> transientHeap;
        ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096;
        GFX_CHECK_CALL_ABORT(
            device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadShaderProgram(device, shaderProgram, "compute-smoke-uint", slangReflection));

        ComputePipelineStateDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<gfx::IPipelineState> pipelineState;
        GFX_CHECK_CALL_ABORT(
            device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

        uint32_t texData[] = { 244u, 0u, 0u, 244u, 0u, 244u, 0u, 244u, 0u, 0u, 244u, 244u, 244u, 244u, 244u, 244u,
                               184u, 0u, 0u, 244u, 0u, 184u, 0u, 244u, 0u, 0u, 184u, 244u, 184u, 184u, 184u, 244u,
                               124u, 0u, 0u, 244u, 0u, 124u, 0u, 244u, 0u, 0u, 124u, 244u, 124u, 124u, 124u, 244u,
                               64u, 0u, 0u, 244u, 0u, 64u, 0u, 244u, 0u, 0u, 64u, 244u, 64u, 64u, 64u, 244u };
        ITextureResource::Desc texDesc = {};
        texDesc.type = IResource::Type::Texture2D;
        texDesc.numMipLevels = 1;
        texDesc.arraySize = 1;
        texDesc.size.width = 4;
        texDesc.size.height = 4;
        texDesc.size.depth = 1;
        texDesc.defaultState = ResourceState::ShaderResource;
        texDesc.format = Format::RGBA_UInt32;

        ITextureResource::SubresourceData subData = {};
        subData.data = (void*)texData;
        subData.strideY = texDesc.size.width * 4 * sizeof(uint32_t);

        ComPtr<ITextureResource> inTex;
        GFX_CHECK_CALL_ABORT(device->createTextureResource(
            texDesc,
            &subData,
            inTex.writeRef()));

        ComPtr<IResourceView> texView;
        IResourceView::Desc texViewDesc = {};
        texViewDesc.type = IResourceView::Type::ShaderResource;
        texViewDesc.format = Format::RGBA_UInt32;
        GFX_CHECK_CALL_ABORT(device->createTextureView(inTex, texViewDesc, texView.writeRef()));

        const int numberCount = 64;
        uint32_t initialData[numberCount] = { 0u };
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.sizeInBytes = numberCount * sizeof(uint32_t);
        bufferDesc.format = gfx::Format::Unknown;
        bufferDesc.elementSize = sizeof(uint32_t);
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
            Slang::makeArray<uint32_t>(244u, 0u, 0u, 244u, 0u, 244u, 0u, 244u, 0u, 0u, 244u, 244u, 244u, 244u, 244u, 244u,
                                       184u, 0u, 0u, 244u, 0u, 184u, 0u, 244u, 0u, 0u, 184u, 244u, 184u, 184u, 184u, 244u,
                                       124u, 0u, 0u, 244u, 0u, 124u, 0u, 244u, 0u, 0u, 124u, 244u, 124u, 124u, 124u, 244u,
                                       64u, 0u, 0u, 244u, 0u, 64u, 0u, 244u, 0u, 0u, 64u, 244u, 64u, 64u, 64u, 244u));
    }

    void FormatRGBAUInt32TestAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
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

        FormatRGBAUInt32TestImpl(device, context);
    }

    SLANG_UNIT_TEST(FormatRGBAUInt32D3D12)
    {
        FormatRGBAUInt32TestAPI(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(FormatRGBAUInt32Vulkan)
    {
        FormatRGBAUInt32TestAPI(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

}
