#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    gfx::Format convertTypelessFormat(gfx::Format format)
    {
        switch (format)
        {
        case gfx::Format::RGBA_Typeless32:
            return gfx::Format::RGBA_Float32;
        case gfx::Format::RGB_Typeless32:
            return gfx::Format::RGB_Float32;
        case gfx::Format::RG_Typeless32:
            return gfx::Format::RG_Float32;
        case gfx::Format::R_Typeless32:
            return gfx::Format::R_Float32;
        case gfx::Format::RGBA_Typeless16:
            return gfx::Format::RGBA_Float16;
        case gfx::Format::RG_Typeless16:
            return gfx::Format::RG_Float16;
        case gfx::Format::R_Typeless16:
            return gfx::Format::R_Float16;
        case gfx::Format::RGBA_Typeless8:
            return gfx::Format::RGBA_Unorm_UInt8;
        case gfx::Format::RG_Typeless8:
            return gfx::Format::RG_Unorm_UInt8;
        case gfx::Format::R_Typeless8:
            return gfx::Format::R_Unorm_UInt8;
        case gfx::Format::BGRA_Typeless8:
            return gfx::Format::BGRA_Unorm_UInt8;
        default:
            return gfx::Format::Unknown;
        }
    }

    void setUpAndRunTest(
        IDevice* device,
        ComPtr<IResourceView> texView,
        ComPtr<IResourceView> bufferView,
        const char* entryPoint,
        ComPtr<ISamplerState> sampler = nullptr)
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

    ComPtr<IResourceView> createTexView(
        IDevice* device,
        ITextureResource::Size size,
        gfx::Format format,
        ITextureResource::SubresourceData* data,
        int mips = 1)
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
        texViewDesc.format = gfxIsTypelessFormat(format) ? convertTypelessFormat(format) : format;
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

    void formatTestsImpl(IDevice* device, UnitTestContext* context)
    {
        ISamplerState::Desc samplerDesc;
        auto sampler = device->createSamplerState(samplerDesc);

        // Note: D32_FLOAT and D16_UNORM are not directly tested as they are only used for raster. These
        // are the same as R32_FLOAT and R16_UNORM, respectively, when passed to a shader.
        {
            float texData[] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f };
            ITextureResource::SubresourceData subData = { (void*)texData, 32, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_Float32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f));

            texView = createTexView(device, size, gfx::Format::RGBA_Typeless32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f));
        }

        {
            float texData[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f };
            ITextureResource::SubresourceData subData = { (void*)texData, 24, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 12;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGB_Float32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat3");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f));

            texView = createTexView(device, size, gfx::Format::RGB_Typeless32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat3");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f));
        }

        {
            float texData[] = { 1.0f, 0.0f, 0.0f, 1.0f,
                                1.0f, 1.0f, 0.5f, 0.5f };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_Float32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f,
                                        1.0f, 1.0f, 0.5f, 0.5f));

            texView = createTexView(device, size, gfx::Format::RG_Typeless32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f,
                                        1.0f, 1.0f, 0.5f, 0.5f));
        }

        {
            float texData[] = { 1.0f, 0.0f, 0.5f, 0.25f };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_Float32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.5f, 0.25f));

            texView = createTexView(device, size, gfx::Format::R_Typeless32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.5f, 0.25f));
        }

        {
            uint16_t texData[] = { 15360u, 0u, 0u, 15360u, 0u, 15360u, 0u, 15360u,
                                   0u, 0u, 15360u, 15360u, 14336u, 14336u, 14336u, 15360u };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_Float16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f));

            texView = createTexView(device, size, gfx::Format::RGBA_Typeless16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f));
        }

        {
            uint16_t texData[] = { 15360u, 0u, 0u, 15360u,
                                   15360u, 15360u, 14336u, 14336u };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_Float16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f,
                                        1.0f, 1.0f, 0.5f, 0.5f));

            texView = createTexView(device, size, gfx::Format::RG_Typeless16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f,
                                        1.0f, 1.0f, 0.5f, 0.5f));
        }

        {
            uint16_t texData[] = { 15360u, 0u, 14336u, 13312u };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_Float16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.5f, 0.25f));

            texView = createTexView(device, size, gfx::Format::R_Typeless16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.5f, 0.25f));
        }

        {
            uint32_t texData[] = { 255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                   0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u };
            ITextureResource::SubresourceData subData = { (void*)texData, 32, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_UInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                           0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u));
        }

        {
            uint32_t texData[] = { 255u, 0u, 0u, 0u, 255u, 0u,
                                   0u, 0u, 255u, 127u, 127u, 127u };
            ITextureResource::SubresourceData subData = { (void*)texData, 24, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 12;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGB_UInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint3");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 0u, 255u, 0u,
                                           0u, 0u, 255u, 127u, 127u, 127u));
        }

        {
            uint32_t texData[] = { 255u, 0u, 0u, 255u,
                                   255u, 255u, 127u, 127u };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 12;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_UInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u,
                                           255u, 255u, 127u, 127u));
        }

        {
            uint32_t texData[] = { 255u, 0u, 127u, 73u };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_UInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 127u, 73u));
        }

        {
            uint16_t texData[] = { 255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                   0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_UInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                           0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u));
        }

        {
            uint16_t texData[] = { 255u, 0u, 0u, 255u,
                                   255u, 255u, 127u, 127u };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_UInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u,
                                           255u, 255u, 127u, 127u));
        }

        {
            uint16_t texData[] = { 255u, 0u, 127u, 73u };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_UInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 127u, 73u));
        }

        {
            uint8_t texData[] = { 255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                  0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_UInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                           0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u));
        }

        {
            uint8_t texData[] = { 255u, 0u, 0u, 255u,
                                  255u, 255u, 127u, 127u };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_UInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u,
                                           255u, 255u, 127u, 127u));
        }

        {
            uint8_t texData[] = { 255u, 0u, 127u, 73u };
            ITextureResource::SubresourceData subData = { (void*)texData, 2, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_UInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 127u, 73u));
        }

        {
            int32_t texData[] = { 255, 0, 0, 255, 0, 255, 0, 255,
                                  0, 0, 255, 255, 127, 127, 127, 255 };
            ITextureResource::SubresourceData subData = { (void*)texData, 32, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_SInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                           0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u));
        }

        {
            int32_t texData[] = { 255, 0, 0, 0, 255, 0,
                                  0, 0, 255, 127, 127, 127 };
            ITextureResource::SubresourceData subData = { (void*)texData, 24, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 12;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGB_SInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint3");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 0u, 255u, 0u,
                                           0u, 0u, 255u, 127u, 127u, 127u));
        }

        {
            int32_t texData[] = { 255, 0, 0, 255,
                                  255, 255, 127, 127 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 12;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_SInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u,
                                           255u, 255u, 127u, 127u));
        }

        {
            int32_t texData[] = { 255, 0, 127, 73 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_SInt32, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 127u, 73u));
        }

        {
            int16_t texData[] = { 255, 0, 0, 255, 0, 255, 0, 255,
                                  0, 0, 255, 255, 127, 127, 127, 255 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_SInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
                                           0u, 0u, 255u, 255u, 127u, 127u, 127u, 255u));
        }

        {
            int16_t texData[] = { 255, 0, 0, 255,
                                  255, 255, 127, 127 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_SInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 0u, 255u,
                                           255u, 255u, 127u, 127u));
        }

        {
            int16_t texData[] = { 255, 0, 127, 73 };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_SInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(255u, 0u, 127u, 73u));
        }

        {
            int8_t texData[] = { 127, 0, 0, 127, 0, 127, 0, 127,
                                 0, 0, 127, 127, 0, 0, 0, 127 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_SInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(127u, 0u, 0u, 127u, 0u, 127u, 0u, 127u,
                                           0u, 0u, 127u, 127u, 0u, 0u, 0u, 127u));
        }

        {
            int8_t texData[] = { 127, 0, 0, 127,
                                 127, 127, 73, 73 };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_SInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(127u, 0u, 0u, 127u,
                                           127u, 127u, 73u, 73u));
        }

        {
            int8_t texData[] = { 127, 0, 73, 25 };
            ITextureResource::SubresourceData subData = { (void*)texData, 2, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            uint32_t initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<uint32_t>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_SInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexUint");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<uint32_t>(127u, 0u, 73u, 25u));
        }

        {
            uint16_t texData[] = { 65535u, 0u, 0u, 65535u, 0u, 65535u, 0u, 65535u,
                                   0u, 0u, 65535u, 65535u, 32767u, 32767u, 32767u, 32767u };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_Unorm_UInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f, 0.499992371f, 0.499992371f, 0.499992371f, 0.499992371f));
        }

        {
            uint16_t texData[] = { 65535u, 0u, 0u, 65535u,
                                   65535u, 65535u, 32767u, 32767u };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_Unorm_UInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f,
                                        1.0f, 1.0f, 0.499992371f, 0.499992371f));
        }

        {
            uint16_t texData[] = { 65535u, 0u, 32767u, 16383u };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0u };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_Unorm_UInt16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.499992371f, 0.249988556f));
        }

        {
            uint8_t texData[] = { 0, 0, 0, 255, 127, 127, 127, 255, 255, 255, 255, 255, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0};
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_Typeless8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.0f, 1.0f, 0.498039216f, 0.498039216f, 0.498039216f, 1.0f,
                                        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));

            texView = createTexView(device, size, gfx::Format::RGBA_Unorm_UInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.0f, 1.0f, 0.498039216f, 0.498039216f, 0.498039216f, 1.0f,
                                        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));

            texView = createTexView(device, size, gfx::Format::RGBA_Unorm_UInt8_Srgb, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.0f, 1.0f, 0.211914062f, 0.211914062f, 0.211914062f,
                                        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        }

        {
            uint8_t texData[] = { 255, 0, 0, 255, 255, 255, 127, 127 };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_Typeless8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.498039216f, 0.498039216f));

            texView = createTexView(device, size, gfx::Format::RG_Unorm_UInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.498039216f, 0.498039216f));
        }

        {
            uint8_t texData[] = { 255, 0, 127, 63};
            ITextureResource::SubresourceData subData = { (void*)texData, 2, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_Typeless8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.498039216f, 0.247058824f));

            texView = createTexView(device, size, gfx::Format::R_Unorm_UInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.498039216f, 0.247058824f));
        }

        {
            uint8_t texData[] = { 0, 0, 0, 255, 127, 127, 127, 255, 255, 255, 255, 255, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BGRA_Typeless8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.0f, 1.0f, 0.498039216f, 0.498039216f, 0.498039216f, 1.0f,
                                        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));

            texView = createTexView(device, size, gfx::Format::BGRA_Unorm_UInt8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.0f, 1.0f, 0.498039216f, 0.498039216f, 0.498039216f, 1.0f,
                                        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        }

        {
            int16_t texData[] = { 32767, 0, 0, 32767, 0, 32767, 0, 32767,
                                  0, 0, 32767, 32767, -32768, -32768, 0, 32767 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_Snorm_Int16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f));
        }

        {
            int16_t texData[] = { 32767, 0, 0, 32767,
                                  32767, 32767, -32768, -32768 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_Snorm_Int16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f));
        }

        {
            int16_t texData[] = { 32767, 0, -32768, 0};
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_Snorm_Int16, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, -1.0f, 0.0f));
        }

        {
            int8_t texData[] = { 127, 0, 0, 127, 0, 127, 0, 127,
                                 0, 0, 127, 127, -128, -128, 0, 127 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RGBA_Snorm_Int8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f, 1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f));
        }

        {
            int8_t texData[] = { 127, 0, 0, 127,
                                 127, 127, -128, -128 };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::RG_Snorm_Int8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat2");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f));
        }

        {
            int8_t texData[] = { 127, 0, -128, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 2, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::R_Snorm_Int8, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, -1.0f, 0.0f));
        }

        {
            uint8_t texData[] = { 15, 240, 240, 240, 0, 255, 119, 119 };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BGRA_Unorm4, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                    1.0f, 0.0f, 0.0f, 1.0f, 0.466666669f, 0.466666669f, 0.466666669f, 0.466666669f));
        }

        {
            uint16_t texData[] = { 31, 2016, 63488, 31727 };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 12;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::B5G6R5_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat3");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                                        1.0f, 0.0f, 0.0f, 0.482352942f, 0.490196079f, 0.482352942f));
        }

        {
            uint16_t texData[] = { 31, 2016, 63488, 31727 };
            ITextureResource::SubresourceData subData = { (void*)texData, 4, 0 };
            ITextureResource::Size size = { 2, 2, 1 };
            const int resultCount = 16;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::B5G5R5A1_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "copyTexFloat4");
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 1.0f, 0.0f, 0.0313725509f, 1.0f, 0.0f, 0.0f,
                                        0.968627453f, 0.0f, 0.0f, 1.0f, 0.968627453f, 1.0f, 0.482352942f, 0.0f));
        }

        // These BC1 tests also check that mipmaps are working correctly for compressed formats.
        {
            uint8_t texData[] = { 16, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0,
                                  16, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0,
                                  255, 255, 255, 255, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData[] = {
                ITextureResource::SubresourceData {(void*)texData, 16, 32},
                ITextureResource::SubresourceData {(void*)(texData + 32), 8, 0}
            };
            ITextureResource::Size size = { 8, 8, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC1_Unorm, subData, 2);
            setUpAndRunTest(device, texView, bufferView, "sampleMips", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.517647088f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC1_Unorm_Srgb, subData, 2);
            setUpAndRunTest(device, texView, bufferView, "sampleMips", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.230468750f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
        }

        {
            uint8_t texData[] = { 255, 255, 255, 255, 255, 255, 255, 255,
                                  16, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC2_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.517647088f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC2_Unorm_Srgb, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.230468750f, 1.0f));
        }

        {
            uint8_t texData[] = { 0, 255, 255, 255, 255, 255, 255, 255,
                                  16, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC3_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.517647088f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC3_Unorm_Srgb, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.0f, 0.230468750f, 1.0f));
        }

        {
            uint8_t texData[] = { 127, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 8, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC4_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.498039216f, 0.0f, 0.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC4_Snorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 0.0f, 0.0f, 1.0f));
        }

        {
            uint8_t texData[] = { 127, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 8;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC5_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.498039216f, 0.498039216f, 0.0f, 1.0f, 0.498039216f, 0.498039216f, 0.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC5_Snorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f));
        }

        // BC6H_UF16 and BC6H_SF16 are tested separately due to requiring different texture data.
        {
            uint8_t texData[] = { 98, 238, 232, 77, 240, 66, 148, 31, 124, 95, 2, 224, 255, 107, 77, 250 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC6_Unsigned, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.336669922f, 0.911132812f, 2.13867188f, 1.0f));
        }

        {
            uint8_t texData[] = { 107, 238, 232, 77, 240, 71, 128, 127, 1, 0, 255, 255, 170, 218, 221, 254 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC6_Signed, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.336914062f, 0.910644531f, 2.14062500f, 1.0f));
        }

        {
            uint8_t texData[] = { 104, 0, 0, 0, 64, 163, 209, 104, 0, 0, 0, 0, 0, 0, 0, 0 };
            ITextureResource::SubresourceData subData = { (void*)texData, 16, 0 };
            ITextureResource::Size size = { 4, 4, 1 };
            const int resultCount = 4;
            float initialData[resultCount] = { 0.0f };

            auto outBuffer = createBuffer<float>(device, resultCount, initialData);
            auto bufferView = createBufferView(device, outBuffer);

            auto texView = createTexView(device, size, gfx::Format::BC7_Unorm, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
            compareComputeResult(
                device,
                outBuffer,
                Slang::makeArray<float>(0.0f, 0.101960786f, 0.0f, 1.0f));

            texView = createTexView(device, size, gfx::Format::BC7_Unorm_Srgb, &subData);
            setUpAndRunTest(device, texView, bufferView, "sampleTex", sampler);
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

        formatTestsImpl(device, context);
    }

    SLANG_UNIT_TEST(FormatTestsD3D11)
    {
        FormatTestsAPI(unitTestContext, Slang::RenderApiFlag::D3D11);
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
