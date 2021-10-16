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
        // RGBA_UInt32
        {
            uint32_t texData[] = { 244u, 0u, 0u, 244u, 0u, 244u, 0u, 244u, 0u, 0u, 244u, 244u, 244u, 244u, 244u, 244u,
                               184u, 0u, 0u, 244u, 0u, 184u, 0u, 244u, 0u, 0u, 184u, 244u, 184u, 184u, 184u, 244u,
                               124u, 0u, 0u, 244u, 0u, 124u, 0u, 244u, 0u, 0u, 124u, 244u, 124u, 124u, 124u, 244u,
                               64u, 0u, 0u, 244u, 0u, 64u, 0u, 244u, 0u, 0u, 64u, 244u, 64u, 64u, 64u, 244u };
            ITextureResource::SubresourceData subData = {};
            subData.data = (void*)texData;
            subData.strideY = 4 * 4 * sizeof(uint32_t);
            const int resultCount = 64;
            uint32_t initialData[resultCount] = { 0u };

            ITextureResource::Size size = { 4, 4, 1 };

            auto texView = createTexView(device, size, gfx::Format::RGBA_UInt32, &subData);
            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            setUpAndRunTest(device, texView, bufferView, "copyTexUint");

            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(244u, 0u, 0u, 244u, 0u, 244u, 0u, 244u, 0u, 0u, 244u, 244u, 244u, 244u, 244u, 244u,
                                           184u, 0u, 0u, 244u, 0u, 184u, 0u, 244u, 0u, 0u, 184u, 244u, 184u, 184u, 184u, 244u,
                                           124u, 0u, 0u, 244u, 0u, 124u, 0u, 244u, 0u, 0u, 124u, 244u, 124u, 124u, 124u, 244u,
                                           64u, 0u, 0u, 244u, 0u, 64u, 0u, 244u, 0u, 0u, 64u, 244u, 64u, 64u, 64u, 244u));
        }

        // RGBA_Unorm_UInt8_Srgb
        {
            uint8_t texData[] = { 0, 0, 0, 255, 127, 127, 127, 255, 255, 255, 255, 255, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = {};
            subData.data = (void*)texData;
            subData.strideY = 2 * 4 * sizeof(uint8_t);
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            ITextureResource::Size size = { 2, 2, 1 };

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

        // BC1_Unorm, BC1_Unorm_Srgb - These tests also check that mipmaps are working correctly for compressed formats.
        {
            uint8_t texData[] = { 16, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0,
                                  16, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0,
                                  255, 255, 255, 255, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData[] = {
                ITextureResource::SubresourceData {(void*)texData, 16, 0},
                ITextureResource::SubresourceData {(void*)(texData + 32), 8, 0}
            };
            ITextureResource::Size size = { 8, 8, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC1_Unorm, subData, 2);
            setUpAndRunTest(device, texView, bufferView, "sampleMipsFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.517647088f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC1_Unorm_Srgb, subData, 2);
            setUpAndRunTest(device, texView, bufferView, "sampleMipsFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.230468750f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
        }

        // BC2_Unorm, BC2_Unorm_Srgb
        {
            uint8_t texData[] = { 255, 255, 255, 255, 255, 255, 255, 255,
                                  16, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC2_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.517647088f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC2_Unorm_Srgb, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.230468750f, 1.0f));
        }

        // BC3_Unorm, BC3_Unorm_Srgb
        {
            uint8_t texData[] = { 0, 255, 255, 255, 255, 255, 255, 255,
                                  16, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC3_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.517647088f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC3_Unorm_Srgb, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.230468750f, 1.0f));
        }

        // BC4_Unorm, BC4_Snorm
        {
            uint8_t texData[] = { 127, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { texData, 8, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC4_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.498039216f, 0.0f, 0.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC4_Snorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f));
        }

        // BC5_Unorm, BC5_Snorm
        {
            uint8_t texData[] = { 127, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC5_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.498039216f, 0.498039216f, 0.0f, 1.0f, 0.498039216f, 0.498039216f, 0.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC5_Snorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f));
        }

        // BC6_Unsigned
        {
            uint8_t texData[] = { 98, 238, 232, 77, 240, 66, 148, 31, 124, 95, 2, 224, 255, 107, 77, 250 };
            ITextureResource::SubresourceData subData = { texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC6_Unsigned, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.336669922f, 0.911132812f, 2.13867188f, 1.0f));
        }

        // BC6_Signed
        {
            uint8_t texData[] = { 107, 238, 232, 77, 240, 71, 128, 127, 1, 0, 255, 255, 170, 218, 221, 254 };
            ITextureResource::SubresourceData subData = { texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC6_Signed, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.336914062f, 0.910644531f, 2.14062500f, 1.0f));
        }

        // BC7_Unorm, BC7_Unorm_Srgb
        {
            uint8_t texData[] = { 104, 0, 0, 0, 64, 163, 209, 104, 0, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            ISamplerState::Desc samplerDesc;
            auto sampler = device->createSamplerState(samplerDesc);

            auto texView = createTexView(device, size, gfx::Format::BC7_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.101960786f, 0.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC7_Unorm_Srgb, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTexFloat", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0103149414f, 0.0f, 1.0f));
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
