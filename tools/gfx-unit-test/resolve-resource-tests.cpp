#if 0
// Disabled: slang-rhi doesn't have resolveResource API.

#include "core/slang-basic.h"
#include "gfx-test-util.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace Slang;
using namespace rhi;

namespace
{
using namespace gfx_test;

struct Vertex
{
    float position[3];
    float color[3];
};

static const int kVertexCount = 12;
static const Vertex kVertexData[kVertexCount] = {
    // Triangle 1
    {{0, 0, 0.5}, {1, 0, 0}},
    {{1, 1, 0.5}, {1, 0, 0}},
    {{-1, 1, 0.5}, {1, 0, 0}},

    // Triangle 2
    {{-1, 1, 0.5}, {0, 1, 0}},
    {{0, 0, 0.5}, {0, 1, 0}},
    {{-1, -1, 0.5}, {0, 1, 0}},

    // Triangle 3
    {{-1, -1, 0.5}, {0, 0, 1}},
    {{0, 0, 0.5}, {0, 0, 1}},
    {{1, -1, 0.5}, {0, 0, 1}},

    // Triangle 4
    {{1, -1, 0.5}, {0, 0, 0}},
    {{0, 0, 0.5}, {0, 0, 0}},
    {{1, 1, 0.5}, {0, 0, 0}},
};

const int kWidth = 256;
const int kHeight = 256;
Format format = Format::RGBA32Float;

ComPtr<IBuffer> createVertexBuffer(IDevice* device)
{
            BufferDesc vertexBufferDesc;
        vertexBufferDesc.size = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        vertexBufferDesc.usage = BufferUsage::VertexBuffer;
    ComPtr<IBuffer> vertexBuffer = device->createBuffer(vertexBufferDesc, &kVertexData[0]);
    SLANG_CHECK_ABORT(vertexBuffer != nullptr);
    return vertexBuffer;
}

struct BaseResolveResourceTest
{
    IDevice* device;
    UnitTestContext* context;

    ComPtr<ITexture> msaaTexture;
    ComPtr<ITexture> dstTexture;

    ComPtr<IRenderPipeline> pipelineState;

    ComPtr<IBuffer> vertexBuffer;

    struct TextureInfo
    {
        Extent3D extent;
        int numMipLevels;
        int arraySize;
        const SubresourceData* initData;
    };

    void init(IDevice* device, UnitTestContext* context)
    {
        this->device = device;
        this->context = context;
    }

    void createRequiredResources(
        TextureInfo msaaTextureInfo,
        TextureInfo dstTextureInfo,
        Format format)
    {
        VertexStreamDesc vertexStreams[] = {
            {sizeof(Vertex), InputSlotClass::PerVertex, 0},
        };

        InputElementDesc inputElements[] = {
            // Vertex buffer data
            {"POSITION", 0, Format::RGB32Float, offsetof(Vertex, position), 0},
            {"COLOR", 0, Format::RGB32Float, offsetof(Vertex, color), 0},
        };

        TextureDesc msaaTexDesc = {};
        msaaTexDesc.type = TextureType::Texture2D;
        msaaTexDesc.mipCount = dstTextureInfo.numMipLevels;
        msaaTexDesc.arrayLength = dstTextureInfo.arraySize;
        msaaTexDesc.size = dstTextureInfo.extent;
        msaaTexDesc.defaultState = ResourceState::RenderTarget;
        msaaTexDesc.usage = TextureUsage::RenderTarget;
        msaaTexDesc.format = format;
        msaaTexDesc.sampleCount = 4;

        msaaTexture = device->createTexture(msaaTexDesc, msaaTextureInfo.initData);
        SLANG_CHECK_ABORT(msaaTexture);

        TextureDesc dstTexDesc = {};
        dstTexDesc.type = TextureType::Texture2D;
        dstTexDesc.mipCount = dstTextureInfo.numMipLevels;
        dstTexDesc.arrayLength = dstTextureInfo.arraySize;
        dstTexDesc.size = dstTextureInfo.extent;
        dstTexDesc.defaultState = ResourceState::CopyDestination;
        dstTexDesc.usage = TextureUsage::CopyDestination | TextureUsage::CopySource;
        dstTexDesc.format = format;

        dstTexture = device->createTexture(dstTexDesc, dstTextureInfo.initData);
        SLANG_CHECK_ABORT(dstTexture);

        InputLayoutDesc inputLayoutDesc = {};
        inputLayoutDesc.inputElementCount = SLANG_COUNT_OF(inputElements);
        inputLayoutDesc.inputElements = inputElements;
        inputLayoutDesc.vertexStreamCount = SLANG_COUNT_OF(vertexStreams);
        inputLayoutDesc.vertexStreams = vertexStreams;
        auto inputLayout = device->createInputLayout(inputLayoutDesc);
        SLANG_CHECK_ABORT(inputLayout != nullptr);

        vertexBuffer = createVertexBuffer(device);

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadGraphicsProgram(
            device,
            shaderProgram,
            "resolve-resource-shader",
            "vertexMain",
            "fragmentMain",
            slangReflection));

        ColorTargetDesc colorTarget = {};
        colorTarget.format = format;
        
        RenderPipelineDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        pipelineDesc.inputLayout = inputLayout;
        pipelineDesc.targets = &colorTarget;
        pipelineDesc.targetCount = 1;
        pipelineDesc.primitiveTopology = PrimitiveTopology::TriangleList;
        pipelineDesc.depthStencil.depthTestEnable = false;
        pipelineDesc.depthStencil.depthWriteEnable = false;
        pipelineState = device->createRenderPipeline(pipelineDesc);
        SLANG_CHECK_ABORT(pipelineState);
    }

    void submitGPUWork(
        SubresourceRange msaaSubresource,
        SubresourceRange dstSubresource,
        Extent3D extent)
    {
        auto queue = device->getQueue(QueueType::Graphics);

        ComPtr<ICommandEncoder> encoder = queue->createCommandEncoder();
        
        // Create render target view
        TextureViewDesc rtvDesc = {};
        rtvDesc.format = format;
        auto rtv = device->createTextureView(msaaTexture, rtvDesc);

        RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = rtv;
        colorAttachment.loadOp = LoadOp::Clear;
        colorAttachment.storeOp = StoreOp::Store;
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        memcpy(colorAttachment.clearValue, clearColor, sizeof(clearColor));

        RenderPassDesc passDesc = {};
        passDesc.colorAttachments = &colorAttachment;
        passDesc.colorAttachmentCount = 1;

        auto renderEncoder = encoder->beginRenderPass(passDesc);
        auto rootObject = renderEncoder->bindPipeline(pipelineState);

        Viewport viewport = {};
        viewport.maxZ = 1.0f;
        viewport.extentX = kWidth;
        viewport.extentY = kHeight;
        
        RenderState state = {};
        state.viewports[0] = viewport;
        state.viewportCount = 1;
        state.vertexBuffers[0] = BufferOffsetPair(vertexBuffer, 0);
        state.vertexBufferCount = 1;
        renderEncoder->setRenderState(state);

        DrawArguments drawArgs = {};
        drawArgs.vertexCount = kVertexCount;
        drawArgs.startVertexLocation = 0;
        renderEncoder->draw(drawArgs);
        renderEncoder->end();

        // Note: slang-rhi doesn't have a direct resolveResource function
        // For MSAA resolve, we would typically use a resolve render pass or blit operation
        // For this test, we'll use a simple copy operation instead
        encoder->copyTexture(
            dstTexture,
            dstSubresource,
            Offset3D{0, 0, 0},
            msaaTexture,
            msaaSubresource,
            Offset3D{0, 0, 0},
            extent);
        encoder->setTextureState(
            dstTexture,
            dstSubresource,
            ResourceState::CopySource);
            
        queue->submit(encoder->finish());
        queue->waitOnHost();
    }

    void checkTestResults(
        int pixelCount,
        int channelCount,
        const int* testXCoords,
        const int* testYCoords,
        float* testResults)
    {
        // Read texture values back from four specific pixels located within the triangles
        // and compare against expected values (because testing every single pixel will be too long
        // and tedious and requires maintaining reference images).
        ComPtr<ISlangBlob> resultBlob;
        size_t rowPitch = 0;
        size_t pixelSize = 0;
        GFX_CHECK_CALL_ABORT(device->readTexture(
            dstTexture,
            0, // layer
            0, // mip
            resultBlob.writeRef(),
            nullptr)); // SubresourceLayout output is optional
        auto result = (float*)resultBlob->getBufferPointer();

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

        float expectedResult[] = {0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f,
                                  1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f,
                                  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f};
        SLANG_CHECK(memcmp(testResults, expectedResult, 128) == 0);
    }
};

struct ResolveResourceSimple : BaseResolveResourceTest
{
    void run()
    {
        Extent3D extent = {};
        extent.width = kWidth;
        extent.height = kHeight;
        extent.depth = 1;

        TextureInfo msaaTextureInfo = {extent, 1, 1, nullptr};
        TextureInfo dstTextureInfo = {extent, 1, 1, nullptr};

        createRequiredResources(msaaTextureInfo, dstTextureInfo, format);

        SubresourceRange msaaSubresource = {};
        msaaSubresource.layer = 0;
        msaaSubresource.layerCount = 1;
        msaaSubresource.mip = 0;
        msaaSubresource.mipCount = 1;

        SubresourceRange dstSubresource = {};
        dstSubresource.layer = 0;
        dstSubresource.layerCount = 1;
        dstSubresource.mip = 0;
        dstSubresource.mipCount = 1;

        submitGPUWork(msaaSubresource, dstSubresource, extent);

        const int kPixelCount = 8;
        const int kChannelCount = 4;
        int testXCoords[kPixelCount] = {64, 127, 191, 64, 191, 64, 127, 191};
        int testYCoords[kPixelCount] = {64, 64, 64, 127, 127, 191, 191, 191};
        float testResults[kPixelCount * kChannelCount];

        checkTestResults(kPixelCount, kChannelCount, testXCoords, testYCoords, testResults);
    }
};

template<typename T>
void resolveResourceTestImpl(IDevice* device, UnitTestContext* context)
{
    T test;
    test.init(device, context);
    test.run();
}
} // namespace

namespace gfx_test
{
SLANG_UNIT_TEST(resolveResourceSimpleD3D12)
{
    runTestImpl(
        resolveResourceTestImpl<ResolveResourceSimple>,
        unitTestContext,
        DeviceType::D3D12);
}

SLANG_UNIT_TEST(resolveResourceSimpleVulkan)
{
    runTestImpl(
        resolveResourceTestImpl<ResolveResourceSimple>,
        unitTestContext,
        DeviceType::Vulkan);
}
} // namespace gfx_test

#endif
