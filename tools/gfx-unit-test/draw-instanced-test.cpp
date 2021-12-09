#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    struct Vertex
    {
        float position[3];
        float color[3];
    };

    static const int kVertexCount = 3;
    static const Vertex kVertexData[kVertexCount] =
    {
        { { -1, -1, 0.5 }, { 1, 0, 0 } },
        { { -1,  3, 0.5 }, { 1, 0, 0 } },
        { {  3, -1, 0.5 }, { 1, 0, 0 } },
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
            { "POSITION", 0, Format::R32G32B32_FLOAT, offsetof(Vertex, position) },
            { "COLOR",    0, Format::R32G32B32_FLOAT, offsetof(Vertex, color) },
        };
        ComPtr<gfx::IInputLayout> inputLayout = device->createInputLayout(inputElements, 2);
        SLANG_CHECK_ABORT(inputLayout != nullptr);

        IBufferResource::Desc vertexBufferDesc;
        vertexBufferDesc.type = IResource::Type::Buffer;
        vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        vertexBufferDesc.allowedStates = ResourceState::VertexBuffer;
        ComPtr<IBufferResource> vertexBuffer = device->createBufferResource(vertexBufferDesc, &kVertexData[0]);
        SLANG_CHECK_ABORT(vertexBuffer != nullptr);

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

        const int width = 2;
        const int height = 2;

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

        UInt vertexCount = 3;
        UInt instanceCount = 1;
        UInt startVertex = 0;
        UInt startInstanceLocation = 0;

        gfx::Viewport viewport = {};
        viewport.maxZ = 1.0f;
        viewport.extentX = width;
        viewport.extentY = height;
        encoder->setViewportAndScissor(viewport);

        encoder->setVertexBuffer(0, vertexBuffer, sizeof(Vertex));
        encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);
        encoder->drawInstanced(vertexCount, instanceCount, startVertex, startInstanceLocation);
        encoder->endEncoding();
        commandBuffer->close();
        queue->executeCommandBuffer(commandBuffer);
        queue->waitOnHost();

        float expectedResult[] = { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                   1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f };
        compareComputeResult(device, colorBuffer, ResourceState::CopySource, expectedResult, 32, 2);
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
