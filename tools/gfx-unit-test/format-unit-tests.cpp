#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    void setUpAndRunTest(IDevice* device, ComPtr<IResourceView> texView, ComPtr<IResourceView> bufferView, const char* entryPoint, ComPtr<ISamplerState> sampler = nullptr)
    {
        Slang::ComPtr<ITransientResourceHeap> transientHeap;
        ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096;
        GFX_CHECK_CALL_ABORT(
            device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadComputeProgram(device, shaderProgram, "format-test-shaders", entryPoint, slangReflection));

        ComputePipelineStateDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<gfx::IPipelineState> pipelineState;
        GFX_CHECK_CALL_ABORT(
            device->createComputePipelineState(pipelineDesc, pipelineState.writeRef()));

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

            if (sampler) entryPointCursor.getPath("sampler").setSampler(sampler);

            // Bind buffer view to the entry point.
            entryPointCursor.getPath("buffer").setResource(bufferView);

            encoder->dispatchCompute(1, 1, 1);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->wait();
        }
    }

    ComPtr<IResourceView> createTexView(IDevice* device, ITextureResource::Size size, gfx::Format format, ITextureResource::SubresourceData* data, int mips = 1)
    {
        ITextureResource::Desc texDesc = {};
        texDesc.type = IResource::Type::Texture2D;
        texDesc.numMipLevels = mips;
        texDesc.arraySize = 1;
        texDesc.size = size;
        texDesc.defaultState = ResourceState::ShaderResource;
        texDesc.format = format;

        ComPtr<ITextureResource> inTex;
        GFX_CHECK_CALL_ABORT(device->createTextureResource(
            texDesc,
            data,
            inTex.writeRef()));
        
        ComPtr<IResourceView> texView;
        IResourceView::Desc texViewDesc = {};
        texViewDesc.type = IResourceView::Type::ShaderResource;
        texViewDesc.format = inTex->getDesc()->format;
        GFX_CHECK_CALL_ABORT(device->createTextureView(inTex, texViewDesc, texView.writeRef()));
        return texView;
    }

    template <typename T>
    ComPtr<IBufferResource> createBuffer(IDevice* device, int size, void* initialData)
    {
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.sizeInBytes = size * sizeof(T);
        bufferDesc.format = gfx::Format::Unknown;
        bufferDesc.elementSize = sizeof(T);
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
            initialData,
            outBuffer.writeRef()));
        return outBuffer;
    }

    ComPtr<IResourceView> createBufferView(IDevice* device, ComPtr<IBufferResource> outBuffer)
    {
        ComPtr<IResourceView> bufferView;
        IResourceView::Desc viewDesc = {};
        viewDesc.type = IResourceView::Type::UnorderedAccess;
        viewDesc.format = Format::Unknown;
        GFX_CHECK_CALL_ABORT(device->createBufferView(outBuffer, viewDesc, bufferView.writeRef()));
        return bufferView;
    }

    void FormatTestsImpl(IDevice* device, UnitTestContext* context)
    {
        // BC1 Unorm
        {
            uint8_t texData[] = { 31, 0, 0, 0, 0, 0, 0, 0, 31, 0, 0, 0, 0, 0, 0, 0,
                              31, 0, 0, 0, 0, 0, 0, 0, 31, 0, 0, 0, 0, 0, 0, 0,
                              255, 255, 255, 255, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData[] = {
                ITextureResource::SubresourceData {(void*)texData, 16, 0},
                ITextureResource::SubresourceData {(void*)(texData + 32), 8, 0}
            };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            ITextureResource::Size size = {};
            size.width = 8;
            size.height = 8;
            size.depth = 1;

            auto texView = createTexView(device, size, gfx::Format::BC1_Unorm, subData, 2);
            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);

            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
        }
        
        // RGBA8 Unorm Srgb
        {
            uint8_t texData[] = { 0, 0, 0, 255, 127, 127, 127, 255, 255, 255, 255, 255, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = {};
            subData.data = (void*)texData;
            subData.strideY = 2 * 4 * sizeof(uint8_t);
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            ITextureResource::Size size = {};
            size.width = 2;
            size.height = 2;
            size.depth = 1;

            auto texView = createTexView(device, size, gfx::Format::RGBA_Unorm_UInt8_Srgb, &subData);
            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");

            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.0f, 1.0f, 0.211914062f, 0.211914062f, 0.211914062f,
                                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        }
    }

    void FormatTestsAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum api)
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

        FormatTestsImpl(device, context);
    }

    SLANG_UNIT_TEST(FormatTestsD3D12)
    {
        FormatTestsAPI(unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(FormatTestsVulkan)
    {
        FormatTestsAPI(unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

}
