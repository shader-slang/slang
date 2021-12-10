#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"

using namespace gfx;

namespace gfx_test
{
    /* static */ Slang::Result writeImage(
        const char* filename,
        ISlangBlob* pixels,
        uint32_t width,
        uint32_t height)
    {
        int stbResult =
            stbi_write_hdr(filename, width, height, 4, (float*)pixels->getBufferPointer());

        return stbResult ? SLANG_OK : SLANG_FAIL;
    }

    struct Vertex
    {
        float position[3];
    };

    struct Instance
    {
        float position[3];
        float color[3];
    };

    static const int kVertexCount = 6;
    static const Vertex kVertexData[kVertexCount] =
    {
        // Triangle 1
        { 0, 0, 0.5 },
        { 1, 0, 0.5 },
        { 0, 1, 0.5 },

        // Triangle 2
        { -1, 0, 0.5 },
        {  0, 0, 0.5 },
        { -1, 1, 0.5 },
    };

    static const int kInstanceCount = 2;
    static const Instance kInstanceData[kInstanceCount] =
    {
        { { 0,  0, 0 }, {1, 0, 0} },
        { { 0, -1, 0 }, {0, 0, 1} },
    };

    void drawInstancedTestImpl(IDevice* device, UnitTestContext* context)
    {
        Slang::ComPtr<ITransientResourceHeap> transientHeap;
        ITransientResourceHeap::Desc transientHeapDesc = {};
        transientHeapDesc.constantBufferSize = 4096;
        GFX_CHECK_CALL_ABORT(
            device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadGraphicsProgram(device, shaderProgram, "graphics-smoke", "vertexMain", "fragmentMain", slangReflection));

        Format format = Format::R32G32B32A32_FLOAT;

        InputElementDesc inputElements[] = {
            // Vertex buffer data
            { "POSITIONA", 0, Format::R32G32B32_FLOAT, offsetof(Vertex, position), InputSlotClass::PerVertex, 0 },

            // Instance buffer data
            { "POSITIONB", 0, Format::R32G32B32_FLOAT, offsetof(Instance, position), InputSlotClass::PerInstance, 1, 1 },
            { "COLOR",     0, Format::R32G32B32_FLOAT, offsetof(Instance, color),    InputSlotClass::PerInstance, 1, 1 },
        };
        ComPtr<gfx::IInputLayout> inputLayout = device->createInputLayout(inputElements, 3);
        SLANG_CHECK_ABORT(inputLayout != nullptr);

        IBufferResource::Desc vertexBufferDesc;
        vertexBufferDesc.type = IResource::Type::Buffer;
        vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        vertexBufferDesc.allowedStates = ResourceState::VertexBuffer;
        ComPtr<IBufferResource> vertexBuffer = device->createBufferResource(vertexBufferDesc, &kVertexData[0]);
        SLANG_CHECK_ABORT(vertexBuffer != nullptr);

        IBufferResource::Desc instanceBufferDesc;
        instanceBufferDesc.type = IResource::Type::Buffer;
        instanceBufferDesc.sizeInBytes = kInstanceCount * sizeof(Instance);
        instanceBufferDesc.defaultState = ResourceState::VertexBuffer;
        instanceBufferDesc.allowedStates = ResourceState::VertexBuffer;
        ComPtr<IBufferResource> instanceBuffer = device->createBufferResource(instanceBufferDesc, &kInstanceData[0]);
        SLANG_CHECK_ABORT(instanceBuffer != nullptr);

        IFramebufferLayout::AttachmentLayout attachmentLayout;
        attachmentLayout.format = format;
        attachmentLayout.sampleCount = 1;

        IFramebufferLayout::Desc framebufferLayoutDesc;
        framebufferLayoutDesc.renderTargetCount = 1;
        framebufferLayoutDesc.renderTargets = &attachmentLayout;
        ComPtr<gfx::IFramebufferLayout> framebufferLayout = device->createFramebufferLayout(framebufferLayoutDesc);

        GraphicsPipelineStateDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        pipelineDesc.inputLayout = inputLayout;
        pipelineDesc.framebufferLayout = framebufferLayout;
        pipelineDesc.depthStencil.depthTestEnable = false;
        pipelineDesc.depthStencil.depthWriteEnable = false;
        ComPtr<gfx::IPipelineState> pipelineState;
        GFX_CHECK_CALL_ABORT(
            device->createGraphicsPipelineState(pipelineDesc, pipelineState.writeRef()));

        ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
        auto queue = device->createCommandQueue(queueDesc);
        auto commandBuffer = transientHeap->createCommandBuffer();

        IRenderPassLayout::Desc renderPassDesc = {};
        renderPassDesc.framebufferLayout = framebufferLayout;
        renderPassDesc.renderTargetCount = 1;
        IRenderPassLayout::AttachmentAccessDesc renderTargetAccess = {};
        renderTargetAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
        renderTargetAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
        renderTargetAccess.initialState = ResourceState::Undefined;
        renderTargetAccess.finalState = ResourceState::CopySource;
        renderPassDesc.renderTargetAccess = &renderTargetAccess;
        ComPtr<IRenderPassLayout> renderPass = device->createRenderPassLayout(renderPassDesc);

        const int width = 256;
        const int height = 256;

        gfx::ITextureResource::Desc colorBufferDesc;
        colorBufferDesc.type = IResource::Type::Texture2D;
        colorBufferDesc.size.width = width;
        colorBufferDesc.size.height = height;
        colorBufferDesc.size.depth = 1;
        colorBufferDesc.numMipLevels = 1;
        colorBufferDesc.format = format;
        colorBufferDesc.defaultState = ResourceState::RenderTarget;
        colorBufferDesc.allowedStates = { ResourceState::RenderTarget, ResourceState::CopySource };
        ComPtr<ITextureResource> colorBuffer = device->createTextureResource(colorBufferDesc, nullptr);

        gfx::IResourceView::Desc colorBufferViewDesc;
        memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
        colorBufferViewDesc.format = format;
        colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
        colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
        ComPtr<gfx::IResourceView> rtv =
            device->createTextureView(colorBuffer.get(), colorBufferViewDesc);

        gfx::IFramebuffer::Desc framebufferDesc;
        framebufferDesc.renderTargetCount = 1;
        framebufferDesc.depthStencilView = nullptr;
        framebufferDesc.renderTargetViews = rtv.readRef();
        framebufferDesc.layout = framebufferLayout;
        ComPtr<gfx::IFramebuffer> framebuffer = device->createFramebuffer(framebufferDesc);

        auto encoder = commandBuffer->encodeRenderCommands(renderPass, framebuffer);
        auto rootObject = encoder->bindPipeline(pipelineState);

        gfx::Viewport viewport = {};
        viewport.maxZ = 1.0f;
        viewport.extentX = width;
        viewport.extentY = height;
        encoder->setViewportAndScissor(viewport);

        UInt startVertex = 0;
        UInt startInstanceLocation = 0;

        encoder->setVertexBuffer(0, vertexBuffer, sizeof(Vertex));
        encoder->setVertexBuffer(1, instanceBuffer, sizeof(Instance));
        encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);
        encoder->drawInstanced(kVertexCount, kInstanceCount, startVertex, startInstanceLocation);
        encoder->endEncoding();
        commandBuffer->close();
        queue->executeCommandBuffer(commandBuffer);
        queue->waitOnHost();

        // Read texture values back from four specific pixels located within the triangles
        // and compare against expected values (because testing every single pixel will be too long and tedious
        // and requires maintaining reference images).
        ComPtr<ISlangBlob> resultBlob;
        size_t rowPitch = 0;
        size_t pixelSize = 0;
        GFX_CHECK_CALL_ABORT(device->readTextureResource(
            colorBuffer, ResourceState::CopySource, resultBlob.writeRef(), &rowPitch, &pixelSize));
        auto result = (float*)resultBlob->getBufferPointer();

        const int kPixelCount = 4;
        const int kChannelCount = 4;
        int testXCoords[kPixelCount] = { 64, 192, 64, 192 };
        int testYCoords[kPixelCount] = { 100, 100, 250, 250 };
        float testResults[kPixelCount * kChannelCount];

        int cursor = 0;
        for (int i = 0; i < kPixelCount; ++i)
        {
            auto x = testXCoords[i];
            auto y = testYCoords[i];
            auto pixelPtr = result + x * kChannelCount + y * rowPitch / sizeof(float);
            for (int j = 0; j < kChannelCount; ++j)
            {
                testResults[cursor] = pixelPtr[j];
                cursor++;
            }
        }

        float expectedResult[] = { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                   0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
        compareComputeResultFuzzy(testResults, expectedResult, sizeof(expectedResult));
    }

    SLANG_UNIT_TEST(drawInstancedD3D12)
    {
        runTestImpl(drawInstancedTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

#if 0
    SLANG_UNIT_TEST(drawInstancedVulkan)
    {
        runTestImpl(drawInstancedTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
#endif
    }
