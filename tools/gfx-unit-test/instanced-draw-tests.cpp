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
    // Testing only code used to dump images to visually confirm correctness.
    // Will be removed once all draw tests are complete.
//     Slang::Result writeImage(
//         const char* filename,
//         ISlangBlob* pixels,
//         uint32_t width,
//         uint32_t height)
//     {
//         int stbResult =
//             stbi_write_hdr(filename, width, height, 4, (float*)pixels->getBufferPointer());
// 
//         return stbResult ? SLANG_OK : SLANG_FAIL;
//     }

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
        { { 0,  0, 0 }, { 1, 0, 0 } },
        { { 0, -1, 0 }, { 0, 0, 1 } },
    };

    static const int kIndexCount = 6;
    static const uint32_t kIndexData[kIndexCount] =
    {
        0, 2, 5,
        0, 1, 2,
    };

    const int kWidth = 256;
    const int kHeight = 256;
    const Format format = Format::R32G32B32A32_FLOAT;

    ComPtr<IBufferResource> createVertexBuffer(IDevice* device)
    {
        IBufferResource::Desc vertexBufferDesc;
        vertexBufferDesc.type = IResource::Type::Buffer;
        vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        vertexBufferDesc.allowedStates = ResourceState::VertexBuffer;
        ComPtr<IBufferResource> vertexBuffer = device->createBufferResource(vertexBufferDesc, &kVertexData[0]);
        SLANG_CHECK_ABORT(vertexBuffer != nullptr);
        return vertexBuffer;
    }

    ComPtr<IBufferResource> createInstanceBuffer(IDevice* device)
    {
        IBufferResource::Desc instanceBufferDesc;
        instanceBufferDesc.type = IResource::Type::Buffer;
        instanceBufferDesc.sizeInBytes = kInstanceCount * sizeof(Instance);
        instanceBufferDesc.defaultState = ResourceState::VertexBuffer;
        instanceBufferDesc.allowedStates = ResourceState::VertexBuffer;
        ComPtr<IBufferResource> instanceBuffer = device->createBufferResource(instanceBufferDesc, &kInstanceData[0]);
        SLANG_CHECK_ABORT(instanceBuffer != nullptr);
        return instanceBuffer;
    }

    ComPtr<IBufferResource> createIndexBuffer(IDevice* device)
    {
        IBufferResource::Desc indexBufferDesc;
        indexBufferDesc.type = IResource::Type::Buffer;
        indexBufferDesc.sizeInBytes = kIndexCount * sizeof(uint32_t);
        indexBufferDesc.defaultState = ResourceState::IndexBuffer;
        indexBufferDesc.allowedStates = ResourceState::IndexBuffer;
        ComPtr<IBufferResource> indexBuffer = device->createBufferResource(indexBufferDesc, &kIndexData[0]);
        SLANG_CHECK_ABORT(indexBuffer != nullptr);
        return indexBuffer;
    }

    ComPtr<ITextureResource> createColorBuffer(IDevice* device)
    {
        gfx::ITextureResource::Desc colorBufferDesc;
        colorBufferDesc.type = IResource::Type::Texture2D;
        colorBufferDesc.size.width = kWidth;
        colorBufferDesc.size.height = kHeight;
        colorBufferDesc.size.depth = 1;
        colorBufferDesc.numMipLevels = 1;
        colorBufferDesc.format = format;
        colorBufferDesc.defaultState = ResourceState::RenderTarget;
        colorBufferDesc.allowedStates = { ResourceState::RenderTarget, ResourceState::CopySource };
        ComPtr<ITextureResource> colorBuffer = device->createTextureResource(colorBufferDesc, nullptr);
        SLANG_CHECK_ABORT(colorBuffer != nullptr);
        return colorBuffer;
    }

    class BaseDrawTest
    {
    public:
        ComPtr<IDevice> device;
        UnitTestContext* context;

        ComPtr<ITransientResourceHeap> transientHeap;
        ComPtr<IPipelineState> pipelineState;
        ComPtr<IRenderPassLayout> renderPass;
        ComPtr<IFramebuffer> framebuffer;

        ComPtr<IBufferResource> vertexBuffer;
        ComPtr<IBufferResource> instanceBuffer;
        ComPtr<ITextureResource> colorBuffer;

        void init(IDevice* device, UnitTestContext* context)
        {
            this->device = device;
            this->context = context;
        }

        void createRequiredResources()
        {
            InputElementDesc inputElements[] = {
                // Vertex buffer data
                { "POSITIONA", 0, Format::R32G32B32_FLOAT, offsetof(Vertex, position), InputSlotClass::PerVertex, 0 },

                // Instance buffer data
                { "POSITIONB", 0, Format::R32G32B32_FLOAT, offsetof(Instance, position), InputSlotClass::PerInstance, 1, 1 },
                { "COLOR",     0, Format::R32G32B32_FLOAT, offsetof(Instance, color),    InputSlotClass::PerInstance, 1, 1 },
            };
            auto inputLayout = device->createInputLayout(inputElements, SLANG_COUNT_OF(inputElements));
            SLANG_CHECK_ABORT(inputLayout != nullptr);

            vertexBuffer = createVertexBuffer(device);
            instanceBuffer = createInstanceBuffer(device);
            colorBuffer = createColorBuffer(device);

            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

            ComPtr<IShaderProgram> shaderProgram;
            slang::ProgramLayout* slangReflection;
            GFX_CHECK_CALL_ABORT(loadGraphicsProgram(device, shaderProgram, "graphics-smoke", "vertexMain", "fragmentMain", slangReflection));

            IFramebufferLayout::AttachmentLayout attachmentLayout;
            attachmentLayout.format = format;
            attachmentLayout.sampleCount = 1;

            IFramebufferLayout::Desc framebufferLayoutDesc;
            framebufferLayoutDesc.renderTargetCount = 1;
            framebufferLayoutDesc.renderTargets = &attachmentLayout;
            ComPtr<gfx::IFramebufferLayout> framebufferLayout = device->createFramebufferLayout(framebufferLayoutDesc);
            SLANG_CHECK_ABORT(framebufferLayout != nullptr);

            GraphicsPipelineStateDesc pipelineDesc = {};
            pipelineDesc.program = shaderProgram.get();
            pipelineDesc.inputLayout = inputLayout;
            pipelineDesc.framebufferLayout = framebufferLayout;
            pipelineDesc.depthStencil.depthTestEnable = false;
            pipelineDesc.depthStencil.depthWriteEnable = false;
            GFX_CHECK_CALL_ABORT(
                device->createGraphicsPipelineState(pipelineDesc, pipelineState.writeRef()));

            IRenderPassLayout::Desc renderPassDesc = {};
            renderPassDesc.framebufferLayout = framebufferLayout;
            renderPassDesc.renderTargetCount = 1;
            IRenderPassLayout::AttachmentAccessDesc renderTargetAccess = {};
            renderTargetAccess.loadOp = IRenderPassLayout::AttachmentLoadOp::Clear;
            renderTargetAccess.storeOp = IRenderPassLayout::AttachmentStoreOp::Store;
            renderTargetAccess.initialState = ResourceState::Undefined;
            renderTargetAccess.finalState = ResourceState::CopySource;
            renderPassDesc.renderTargetAccess = &renderTargetAccess;
            GFX_CHECK_CALL_ABORT(device->createRenderPassLayout(renderPassDesc, renderPass.writeRef()));

            gfx::IResourceView::Desc colorBufferViewDesc;
            memset(&colorBufferViewDesc, 0, sizeof(colorBufferViewDesc));
            colorBufferViewDesc.format = format;
            colorBufferViewDesc.renderTarget.shape = gfx::IResource::Type::Texture2D;
            colorBufferViewDesc.type = gfx::IResourceView::Type::RenderTarget;
            auto rtv = device->createTextureView(colorBuffer, colorBufferViewDesc);

            gfx::IFramebuffer::Desc framebufferDesc;
            framebufferDesc.renderTargetCount = 1;
            framebufferDesc.depthStencilView = nullptr;
            framebufferDesc.renderTargetViews = rtv.readRef();
            framebufferDesc.layout = framebufferLayout;
            GFX_CHECK_CALL_ABORT(device->createFramebuffer(framebufferDesc, framebuffer.writeRef()));
        }

        void checkTestResults(int pixelCount, int channelCount, const int* testXCoords, const int* testYCoords, float* testResults)
        {
            // Read texture values back from four specific pixels located within the triangles
            // and compare against expected values (because testing every single pixel will be too long and tedious
            // and requires maintaining reference images).
            ComPtr<ISlangBlob> resultBlob;
            size_t rowPitch = 0;
            size_t pixelSize = 0;
            GFX_CHECK_CALL_ABORT(device->readTextureResource(
                colorBuffer, ResourceState::CopySource, resultBlob.writeRef(), &rowPitch, &pixelSize));
            auto result = (float*)resultBlob->getBufferPointer();

            // Dump image to disk so we can visually confirm the output
            //writeImage("C:/Users/lucchen/Documents/dump.hdr", resultBlob, kWidth, kHeight);

            int cursor = 0;
            for (int i = 0; i < pixelCount; ++i)
            {
                auto x = testXCoords[i];
                auto y = testYCoords[i];
                auto pixelPtr = result + x * channelCount + y * rowPitch / sizeof(float);
                for (int j = 0; j < channelCount; ++j)
                {
                    testResults[cursor] = pixelPtr[j];
                    cursor++;
                }
            }

            float expectedResult[] = { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                                       0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
            compareComputeResultFuzzy(testResults, expectedResult, sizeof(expectedResult));
        }
    };

    struct DrawInstancedTest : BaseDrawTest
    {
        void setUpAndDraw()
        {
            createRequiredResources();

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);
            auto commandBuffer = transientHeap->createCommandBuffer();

            auto encoder = commandBuffer->encodeRenderCommands(renderPass, framebuffer);
            auto rootObject = encoder->bindPipeline(pipelineState);

            gfx::Viewport viewport = {};
            viewport.maxZ = 1.0f;
            viewport.extentX = kWidth;
            viewport.extentY = kHeight;
            encoder->setViewportAndScissor(viewport);

            uint32_t startVertex = 0;
            uint32_t startInstanceLocation = 0;

            encoder->setVertexBuffer(0, vertexBuffer, sizeof(Vertex));
            encoder->setVertexBuffer(1, instanceBuffer, sizeof(Instance));
            encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

            encoder->drawInstanced(kVertexCount, kInstanceCount, startVertex, startInstanceLocation);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void run()
        {
            setUpAndDraw();

            const int kPixelCount = 4;
            const int kChannelCount = 4;
            int testXCoords[kPixelCount] = { 64, 192, 64, 192 };
            int testYCoords[kPixelCount] = { 100, 100, 250, 250 };
            float testResults[kPixelCount * kChannelCount];

            checkTestResults(kPixelCount, kChannelCount, testXCoords, testYCoords, testResults);
        }
    };

    struct DrawIndexedInstancedTest : BaseDrawTest
    {
        ComPtr<IBufferResource> indexBuffer;

        void setUpAndDraw()
        {
            createRequiredResources();

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);
            auto commandBuffer = transientHeap->createCommandBuffer();

            auto encoder = commandBuffer->encodeRenderCommands(renderPass, framebuffer);
            auto rootObject = encoder->bindPipeline(pipelineState);

            gfx::Viewport viewport = {};
            viewport.maxZ = 1.0f;
            viewport.extentX = kWidth;
            viewport.extentY = kHeight;
            encoder->setViewportAndScissor(viewport);

            uint32_t startIndex = 0;
            int32_t startVertex = 0;
            uint32_t startInstanceLocation = 0;

            encoder->setVertexBuffer(0, vertexBuffer, sizeof(Vertex));
            encoder->setVertexBuffer(1, instanceBuffer, sizeof(Instance));
            encoder->setIndexBuffer(indexBuffer, Format::R32_UINT);
            encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

            encoder->drawIndexedInstanced(kIndexCount, kInstanceCount, startIndex, startVertex, startInstanceLocation);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void run()
        {
            indexBuffer = createIndexBuffer(device);

            setUpAndDraw();

            const int kPixelCount = 4;
            const int kChannelCount = 4;
            int testXCoords[kPixelCount] = { 64, 192, 64, 192 };
            int testYCoords[kPixelCount] = { 32, 100, 150, 250 };
            float testResults[kPixelCount * kChannelCount];

            checkTestResults(kPixelCount, kChannelCount, testXCoords, testYCoords, testResults);
        }
    };

    struct DrawIndirectTest : BaseDrawTest
    {
        ComPtr<IBufferResource> indirectBuffer;

        struct IndirectArgData
        {
            float padding; // Ensure args and count don't start at 0 offset for testing purposes
            IndirectDrawArguments args;
            uint32_t count;
        };

        ComPtr<IBufferResource> createIndirectBuffer(IDevice* device)
        {
            static const IndirectArgData kIndirectData =
            {
                42.0f, // padding
                {6, 2, 0, 0}, // args
                1, // count
            };

            IBufferResource::Desc indirectBufferDesc;
            indirectBufferDesc.type = IResource::Type::Buffer;
            indirectBufferDesc.sizeInBytes = sizeof(IndirectArgData);
            indirectBufferDesc.defaultState = ResourceState::IndirectArgument;
            indirectBufferDesc.allowedStates = ResourceState::IndirectArgument;
            ComPtr<IBufferResource> indexBuffer = device->createBufferResource(indirectBufferDesc, &kIndirectData);
            SLANG_CHECK_ABORT(indexBuffer != nullptr);
            return indexBuffer;
        }

        void setUpAndDraw()
        {
            createRequiredResources();

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);
            auto commandBuffer = transientHeap->createCommandBuffer();

            auto encoder = commandBuffer->encodeRenderCommands(renderPass, framebuffer);
            auto rootObject = encoder->bindPipeline(pipelineState);

            gfx::Viewport viewport = {};
            viewport.maxZ = 1.0f;
            viewport.extentX = kWidth;
            viewport.extentY = kHeight;
            encoder->setViewportAndScissor(viewport);

            encoder->setVertexBuffer(0, vertexBuffer, sizeof(Vertex));
            encoder->setVertexBuffer(1, instanceBuffer, sizeof(Instance));
            encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

            uint32_t maxDrawCount = 1;
            uint64_t argOffset = offsetof(IndirectArgData, args);
            uint64_t countOffset = offsetof(IndirectArgData, count);

            encoder->drawIndirect(maxDrawCount, indirectBuffer, argOffset, indirectBuffer, countOffset);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void run()
        {
            indirectBuffer = createIndirectBuffer(device);

            setUpAndDraw();

            const int kPixelCount = 4;
            const int kChannelCount = 4;
            int testXCoords[kPixelCount] = { 64, 192, 64, 192 };
            int testYCoords[kPixelCount] = { 100, 100, 250, 250 };
            float testResults[kPixelCount * kChannelCount];

            checkTestResults(kPixelCount, kChannelCount, testXCoords, testYCoords, testResults);
        }
    };

    struct DrawIndexedIndirectTest : BaseDrawTest
    {
        ComPtr<IBufferResource> indexBuffer;
        ComPtr<IBufferResource> indirectBuffer;

        struct IndexedIndirectArgData
        {
            float padding; // Ensure args and count don't start at 0 offset for testing purposes
            IndirectDrawIndexedArguments args;
            uint32_t count;
        };

        ComPtr<IBufferResource> createIndirectBuffer(IDevice* device)
        {
            static const IndexedIndirectArgData kIndexedIndirectData =
            {
                42.0f, // padding
                {6, 2, 0, 0, 0}, // args
                1, // count
            };

            IBufferResource::Desc indirectBufferDesc;
            indirectBufferDesc.type = IResource::Type::Buffer;
            indirectBufferDesc.sizeInBytes = sizeof(IndexedIndirectArgData);
            indirectBufferDesc.defaultState = ResourceState::IndirectArgument;
            indirectBufferDesc.allowedStates = ResourceState::IndirectArgument;
            ComPtr<IBufferResource> indexBuffer = device->createBufferResource(indirectBufferDesc, &kIndexedIndirectData);
            SLANG_CHECK_ABORT(indexBuffer != nullptr);
            return indexBuffer;
        }

        void setUpAndDraw()
        {
            createRequiredResources();

            ICommandQueue::Desc queueDesc = { ICommandQueue::QueueType::Graphics };
            auto queue = device->createCommandQueue(queueDesc);
            auto commandBuffer = transientHeap->createCommandBuffer();

            auto encoder = commandBuffer->encodeRenderCommands(renderPass, framebuffer);
            auto rootObject = encoder->bindPipeline(pipelineState);

            gfx::Viewport viewport = {};
            viewport.maxZ = 1.0f;
            viewport.extentX = kWidth;
            viewport.extentY = kHeight;
            encoder->setViewportAndScissor(viewport);

            encoder->setVertexBuffer(0, vertexBuffer, sizeof(Vertex));
            encoder->setVertexBuffer(1, instanceBuffer, sizeof(Instance));
            encoder->setIndexBuffer(indexBuffer, Format::R32_UINT);
            encoder->setPrimitiveTopology(PrimitiveTopology::TriangleList);

            uint32_t maxDrawCount = 1;
            uint64_t argOffset = offsetof(IndexedIndirectArgData, args);
            uint64_t countOffset = offsetof(IndexedIndirectArgData, count);

            encoder->drawIndexedIndirect(maxDrawCount, indirectBuffer, argOffset, indirectBuffer, countOffset);
            encoder->endEncoding();
            commandBuffer->close();
            queue->executeCommandBuffer(commandBuffer);
            queue->waitOnHost();
        }

        void run()
        {
            indexBuffer = createIndexBuffer(device);
            indirectBuffer = createIndirectBuffer(device);

            setUpAndDraw();

            const int kPixelCount = 4;
            const int kChannelCount = 4;
            int testXCoords[kPixelCount] = { 64, 192, 64, 192 };
            int testYCoords[kPixelCount] = { 32, 100, 150, 250 };
            float testResults[kPixelCount * kChannelCount];

            checkTestResults(kPixelCount, kChannelCount, testXCoords, testYCoords, testResults);
        }
    };

    void drawInstancedTestImpl(IDevice* device, UnitTestContext* context)
    {
        DrawInstancedTest test;
        test.init(device, context);
        test.run();
    }

    void drawIndexedInstancedTestImpl(IDevice* device, UnitTestContext* context)
    {
        DrawIndexedInstancedTest test;
        test.init(device, context);
        test.run();
    }

    void drawIndirectTestImpl(IDevice* device, UnitTestContext* context)
    {
        DrawIndirectTest test;
        test.init(device, context);
        test.run();
    }

    void drawIndexedIndirectTestImpl(IDevice* device, UnitTestContext* context)
    {
        DrawIndexedIndirectTest test;
        test.init(device, context);
        test.run();
    }

    SLANG_UNIT_TEST(drawInstancedD3D12)
    {
        runTestImpl(drawInstancedTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(drawIndexedInstancedD3D12)
    {
        runTestImpl(drawIndexedInstancedTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(drawIndirectD3D12)
    {
        runTestImpl(drawIndirectTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(drawIndexedIndirectD3D12)
    {
        runTestImpl(drawIndexedIndirectTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

#if 0
    SLANG_UNIT_TEST(drawInstancedVulkan)
    {
        runTestImpl(drawInstancedTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(drawIndexedInstancedVulkan)
    {
        runTestImpl(drawIndexedInstancedTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(drawIndirectVulkan)
    {
        runTestImpl(drawIndirectTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

    SLANG_UNIT_TEST(drawIndexedIndirectVulkan)
    {
        runTestImpl(drawIndexedIndirectTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }
#endif
    }
