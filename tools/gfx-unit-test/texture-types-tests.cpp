#if 0
// Duplicated: This test is identical to slang-rhi\tests\test-texture-types.cpp

#include "core/slang-basic.h"
#include "gfx-test-texture-util.h"
#include "gfx-test-util.h"
#include "unit-test/slang-unit-test.h"

#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

#if SLANG_WINDOWS_FAMILY
#include <d3d12.h>
#endif

using namespace Slang;
using namespace rhi;

namespace gfx_test
{
struct BaseTextureViewTest
{
    IDevice* device;
    UnitTestContext* context;

    TextureViewType viewType;
    size_t alignedRowStride;

    RefPtr<TextureInfo> textureInfo;
    RefPtr<ValidationTextureFormatBase> validationFormat;

    ComPtr<ITexture> texture;
    ComPtr<ITextureView> textureView;
    ComPtr<IBuffer> resultsBuffer;
    ComPtr<IBufferView> bufferView;

    ComPtr<ISampler> sampler;

    const void* expectedTextureData;

    void init(
        IDevice* device,
        UnitTestContext* context,
        Format format,
        RefPtr<ValidationTextureFormatBase> validationFormat,
        TextureViewType viewType,
        TextureType type)
    {
        this->device = device;
        this->context = context;
        this->validationFormat = validationFormat;
        this->viewType = viewType;

        this->textureInfo = new TextureInfo();
        this->textureInfo->format = format;
        this->textureInfo->textureType = type;
    }

    ResourceState getDefaultResourceStateForViewType(TextureViewType type)
    {
        switch (type)
        {
        case TextureViewType::RenderTarget:
            return ResourceState::RenderTarget;
        case TextureViewType::DepthStencil:
            return ResourceState::DepthWrite;
        case TextureViewType::ShaderResource:
            return ResourceState::ShaderResource;
        case TextureViewType::UnorderedAccess:
            return ResourceState::UnorderedAccess;
        default:
            return ResourceState::Undefined;
        }
    }

    String getShaderEntryPoint()
    {
        String base = "resourceViewTest";
        String shape;
        String view;

        switch (textureInfo->textureType)
        {
        case TextureType::Texture1D:
            shape = "1D";
            break;
        case TextureType::Texture2D:
            shape = "2D";
            break;
        case TextureType::Texture3D:
            shape = "3D";
            break;
        case TextureType::TextureCube:
            shape = "Cube";
            break;
        default:
            assert(!"Invalid texture shape");
            SLANG_CHECK_ABORT(false);
        }

        switch (viewType)
        {
        case TextureViewType::RenderTarget:
            view = "Render";
            break;
        case TextureViewType::DepthStencil:
            view = "Depth";
            break;
        case TextureViewType::ShaderResource:
            view = "Shader";
            break;
        case TextureViewType::UnorderedAccess:
            view = "Unordered";
            break;
        default:
            assert(!"Invalid resource view");
            SLANG_CHECK_ABORT(false);
        }

        return base + shape + view;
    }
};

// used for shaderresource and unorderedaccess
struct ShaderAndUnorderedTests : BaseTextureViewTest
{
    void createRequiredResources()
    {
        TextureDesc textureDesc = {};
        textureDesc.type = textureInfo->textureType;
        textureDesc.numMipLevels = textureInfo->mipLevelCount;
        textureDesc.arrayLength = textureInfo->arrayLayerCount;
        textureDesc.size = textureInfo->extents;
        textureDesc.defaultState = getDefaultResourceStateForViewType(viewType);
        textureDesc.allowedStates = ResourceStateSet(
            textureDesc.defaultState,
            ResourceState::CopySource,
            ResourceState::CopyDestination);
        textureDesc.format = textureInfo->format;

        texture = device->createTexture(textureDesc, textureInfo->subresourceDatas.getBuffer());
        SLANG_CHECK_ABORT(texture);

        TextureViewDesc textureViewDesc = {};
        textureViewDesc.type = viewType;
        textureViewDesc.format = textureDesc.format;
        textureView = device->createTextureView(texture, textureViewDesc);
        SLANG_CHECK_ABORT(textureView);

        auto texelSize = getTexelSize(textureInfo->format);
        size_t alignment;
        device->getTextureRowAlignment(&alignment);
        alignedRowStride =
            (textureInfo->extents.width * texelSize + alignment - 1) & ~(alignment - 1);
        BufferDesc bufferDesc = {};
        // All of the values read back from the shader will be uint32_t
        bufferDesc.size = textureDesc.size.width * textureDesc.size.height *
                         textureDesc.size.depth * texelSize * sizeof(uint32_t);
        bufferDesc.format = Format::Unknown;
        bufferDesc.elementSize = sizeof(uint32_t);
        bufferDesc.defaultState = ResourceState::UnorderedAccess;
        bufferDesc.allowedStates = ResourceStateSet(
            bufferDesc.defaultState,
            ResourceState::CopyDestination,
            ResourceState::CopySource);
        bufferDesc.memoryType = MemoryType::DeviceLocal;

        resultsBuffer = device->createBuffer(bufferDesc, nullptr);
        SLANG_CHECK_ABORT(resultsBuffer);

        BufferViewDesc bufferViewDesc = {};
        bufferViewDesc.type = BufferViewType::UnorderedAccess;
        bufferViewDesc.format = Format::Unknown;
        bufferView = device->createBufferView(resultsBuffer, nullptr, bufferViewDesc);
        SLANG_CHECK_ABORT(bufferView);
    }

    void submitShaderWork(const char* entryPoint)
    {
        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadComputeProgram(
            device,
            shaderProgram,
            "trivial-copy-textures",
            entryPoint,
            slangReflection));

        ComputePipelineDesc pipelineDesc = {};
        pipelineDesc.program = shaderProgram.get();
        ComPtr<IComputePipeline> pipelineState;
        pipelineState = device->createComputePipeline(pipelineDesc);
        SLANG_CHECK_ABORT(pipelineState);

        // We have done all the set up work, now it is time to start recording a command buffer for
        // GPU execution.
        {
            auto queue = device->getQueue(QueueType::Graphics);

            ComPtr<ICommandEncoder> encoder = queue->createCommandEncoder();
            ComPtr<IComputeCommandEncoder> computeEncoder = encoder->beginComputePass();

            ComPtr<IShaderObject> rootObject = computeEncoder->bindPipeline(pipelineState);

            ShaderCursor entryPointCursor(
                rootObject->getEntryPoint(0)); // get a cursor the the first entry-point.

            auto width = textureInfo->extents.width;
            auto height = textureInfo->extents.height;
            auto depth = textureInfo->extents.depth;

            entryPointCursor["width"].setData(width);
            entryPointCursor["height"].setData(height);
            entryPointCursor["depth"].setData(depth);

            // Bind texture view to the entry point
            entryPointCursor["resourceView"].setResource(textureView);
            entryPointCursor["testResults"].setResource(bufferView);

            if (sampler)
                entryPointCursor["sampler"].setSampler(sampler);

            auto bufferElementCount = width * height * depth;
            DispatchArguments args = {};
            args.threadsPerAxis[0] = bufferElementCount;
            args.threadsPerAxis[1] = 1;
            args.threadsPerAxis[2] = 1;
            computeEncoder->dispatchCompute(args);
            computeEncoder->end();
            
            ComPtr<ICommandBuffer> commandBuffer;
            encoder->finish(commandBuffer.writeRef());
            queue->submit(commandBuffer);
            queue->waitOnHost();
        }
    }

    void validateTextureValues(ValidationTextureData actual, ValidationTextureData original)
    {
        // TODO: needs to be extended to cover mip levels and array layers
        for (GfxIndex x = 0; x < actual.extents.width; ++x)
        {
            for (GfxIndex y = 0; y < actual.extents.height; ++y)
            {
                for (GfxIndex z = 0; z < actual.extents.depth; ++z)
                {
                    auto actualBlock = (uint8_t*)actual.getBlockAt(x, y, z);
                    for (Int i = 0; i < 4; ++i)
                    {
                        SLANG_CHECK(actualBlock[i] == 1);
                    }
                }
            }
        }
    }

    void checkTestResults()
    {
        // Shader resources are read-only, so we don't need to check that writes to the resource
        // were correct.
        if (viewType != TextureViewType::ShaderResource)
        {
            ComPtr<ISlangBlob> textureBlob;
            size_t rowPitch;
            size_t pixelSize;
            GFX_CHECK_CALL_ABORT(device->readTexture(
                texture,
                ResourceState::CopySource,
                textureBlob.writeRef(),
                &rowPitch,
                &pixelSize));
            auto textureValues = (uint8_t*)textureBlob->getBufferPointer();

            ValidationTextureData textureResults;
            textureResults.extents = textureInfo->extents;
            textureResults.textureData = textureValues;
            textureResults.strides.x = (uint32_t)pixelSize;
            textureResults.strides.y = (uint32_t)rowPitch;
            textureResults.strides.z = textureResults.extents.height * textureResults.strides.y;

            ValidationTextureData originalData;
            originalData.extents = textureInfo->extents;
            originalData.textureData = textureInfo->subresourceDatas.getBuffer();
            originalData.strides.x = (uint32_t)pixelSize;
            originalData.strides.y = textureInfo->extents.width * originalData.strides.x;
            originalData.strides.z = textureInfo->extents.height * originalData.strides.y;

            validateTextureValues(textureResults, originalData);
        }

        ComPtr<ISlangBlob> bufferBlob;
        GFX_CHECK_CALL_ABORT(device->readBuffer(
            resultsBuffer,
            0,
            resultsBuffer->getDesc().size,
            bufferBlob.writeRef()));
        auto results = (uint32_t*)bufferBlob->getBufferPointer();

        auto elementCount = textureInfo->extents.width * textureInfo->extents.height *
                            textureInfo->extents.depth * 4;
        auto castedTextureData = (uint8_t*)expectedTextureData;
        for (Int i = 0; i < elementCount; ++i)
        {
            SLANG_CHECK(results[i] == castedTextureData[i]);
        }
    }

    void run()
    {
        // TODO: Should test with samplers
        //             SamplerDesc samplerDesc;
        //             sampler = device->createSampler(samplerDesc);

        // TODO: Should test multiple mip levels and array layers
        textureInfo->extents.width = 4;
        textureInfo->extents.height =
            (textureInfo->textureType == TextureType::Texture1D) ? 1 : 4;
        textureInfo->extents.depth =
            (textureInfo->textureType != TextureType::Texture3D) ? 1 : 2;
        textureInfo->mipLevelCount = 1;
        textureInfo->arrayLayerCount = 1;
        generateTextureData(textureInfo, validationFormat);

        // We need to save the pointer to the original texture data for results checking because the
        // texture will be overwritten during testing (if the texture can be written to).
        expectedTextureData = textureInfo->subresourceDatas[getSubresourceIndex(0, 1, 0)].data;

        createRequiredResources();
        auto entryPointName = getShaderEntryPoint();
        // printf("%s\n", entryPointName.getBuffer());
        submitShaderWork(entryPointName.getBuffer());

        checkTestResults();
    }
};

// used for rendertarget and depthstencil
struct RenderTargetTests : BaseTextureViewTest
{
    struct Vertex
    {
        float position[3];
        float color[3];
    };

    const int kVertexCount = 12;
    const Vertex kVertexData[12] = {
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

    int sampleCount = 1;

    ComPtr<IRenderPipeline> pipelineState;

    ComPtr<ITexture> sampledTexture;
    ComPtr<IBuffer> vertexBuffer;

    void createRequiredResources()
    {
        BufferDesc vertexBufferDesc;
        vertexBufferDesc.size = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        vertexBufferDesc.allowedStates = ResourceState::VertexBuffer;
        vertexBufferDesc.usage = BufferUsage::VertexBuffer;
        vertexBuffer = device->createBuffer(vertexBufferDesc, &kVertexData[0]);
        SLANG_CHECK_ABORT(vertexBuffer != nullptr);

        VertexStreamDesc vertexStreams[] = {
            {sizeof(Vertex), InputSlotClass::PerVertex, 0},
        };

        InputElementDesc inputElements[] = {
            // Vertex buffer data
            {"POSITION", 0, Format::RGB32Float, offsetof(Vertex, position), 0},
            {"COLOR", 0, Format::RGB32Float, offsetof(Vertex, color), 0},
        };

        TextureDesc sampledTexDesc = {};
        sampledTexDesc.type = textureInfo->textureType;
        sampledTexDesc.numMipLevels = textureInfo->mipLevelCount;
        sampledTexDesc.arrayLength = textureInfo->arrayLayerCount;
        sampledTexDesc.size = textureInfo->extents;
        sampledTexDesc.defaultState = getDefaultResourceStateForViewType(viewType);
        sampledTexDesc.allowedStates = ResourceStateSet(
            sampledTexDesc.defaultState,
            ResourceState::ResolveSource,
            ResourceState::CopySource);
        sampledTexDesc.format = textureInfo->format;
        sampledTexDesc.sampleCount = sampleCount;

        sampledTexture = device->createTexture(
            sampledTexDesc,
            textureInfo->subresourceDatas.getBuffer());
        SLANG_CHECK_ABORT(sampledTexture);

        TextureDesc texDesc = {};
        texDesc.type = textureInfo->textureType;
        texDesc.numMipLevels = textureInfo->mipLevelCount;
        texDesc.arrayLength = textureInfo->arrayLayerCount;
        texDesc.size = textureInfo->extents;
        texDesc.defaultState = ResourceState::ResolveDestination;
        texDesc.allowedStates =
            ResourceStateSet(ResourceState::ResolveDestination, ResourceState::CopySource);
        texDesc.format = textureInfo->format;

        texture = device->createTexture(
            texDesc,
            textureInfo->subresourceDatas.getBuffer());
        SLANG_CHECK_ABORT(texture);

        InputLayoutDesc inputLayoutDesc = {};
        inputLayoutDesc.inputElementCount = SLANG_COUNT_OF(inputElements);
        inputLayoutDesc.inputElements = inputElements;
        inputLayoutDesc.vertexStreamCount = SLANG_COUNT_OF(vertexStreams);
        inputLayoutDesc.vertexStreams = vertexStreams;
        auto inputLayout = device->createInputLayout(inputLayoutDesc);
        SLANG_CHECK_ABORT(inputLayout != nullptr);

        ComPtr<IShaderProgram> shaderProgram;
        slang::ProgramLayout* slangReflection;
        GFX_CHECK_CALL_ABORT(loadGraphicsProgram(
            device,
            shaderProgram,
            "trivial-copy-textures",
            "vertexMain",
            "fragmentMain",
            slangReflection));

        ColorTargetDesc colorTarget = {};
        colorTarget.format = textureInfo->format;
        
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

        auto texelSize = getTexelSize(textureInfo->format);
        size_t alignment;
        device->getTextureRowAlignment(&alignment);
        alignedRowStride =
            (textureInfo->extents.width * texelSize + alignment - 1) & ~(alignment - 1);
    }

    void submitShaderWork(const char* entryPointName)
    {
        auto queue = device->getQueue(QueueType::Graphics);

        ComPtr<ICommandEncoder> encoder = queue->createCommandEncoder();
        
        // Create render target view
        TextureViewDesc rtvDesc = {};
        rtvDesc.type = TextureViewType::RenderTarget;
        rtvDesc.format = textureInfo->format;
        auto rtv = device->createTextureView(sampledTexture, rtvDesc);

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
        viewport.maxZ = (float)textureInfo->extents.depth;
        viewport.width = (float)textureInfo->extents.width;
        viewport.height = (float)textureInfo->extents.height;
        
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

        if (sampleCount > 1)
        {
            SubresourceRange msaaSubresource = {};
            msaaSubresource.aspectMask = TextureAspect::Color;
            msaaSubresource.mipLevel = 0;
            msaaSubresource.mipLevelCount = 1;
            msaaSubresource.baseArrayLayer = 0;
            msaaSubresource.layerCount = 1;

            SubresourceRange dstSubresource = {};
            dstSubresource.aspectMask = TextureAspect::Color;
            dstSubresource.mipLevel = 0;
            dstSubresource.mipLevelCount = 1;
            dstSubresource.baseArrayLayer = 0;
            dstSubresource.layerCount = 1;

            encoder->resolveResource(
                sampledTexture,
                ResourceState::ResolveSource,
                msaaSubresource,
                texture,
                ResourceState::ResolveDestination,
                dstSubresource);
            encoder->textureBarrier(
                texture,
                ResourceState::ResolveDestination,
                ResourceState::CopySource);
        }
        else
        {
            encoder->textureBarrier(
                sampledTexture,
                ResourceState::ResolveSource,
                ResourceState::CopySource);
        }
        
        ComPtr<ICommandBuffer> commandBuffer;
        encoder->finish(commandBuffer.writeRef());
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    // TODO: Should take a value indicating the slice that was rendered into
    // TODO: Needs to handle either the correct slice or array layer (will not always check z)
    void validateTextureValues(ValidationTextureData actual)
    {
        for (GfxIndex x = 0; x < actual.extents.width; ++x)
        {
            for (GfxIndex y = 0; y < actual.extents.height; ++y)
            {
                for (GfxIndex z = 0; z < actual.extents.depth; ++z)
                {
                    auto actualBlock = (float*)actual.getBlockAt(x, y, z);
                    for (Int i = 0; i < 4; ++i)
                    {
                        if (z == 0)
                        {
                            // Slice being rendered into
                            SLANG_CHECK(actualBlock[i] == (float)i + 1);
                        }
                        else
                        {
                            SLANG_CHECK(actualBlock[i] == 0.0f);
                        }
                    }
                }
            }
        }
    }

    void checkTestResults()
    {
        ComPtr<ISlangBlob> textureBlob;
        size_t rowPitch;
        size_t pixelSize;
        if (sampleCount > 1)
        {
            GFX_CHECK_CALL_ABORT(device->readTexture(
                texture,
                ResourceState::CopySource,
                textureBlob.writeRef(),
                &rowPitch,
                &pixelSize));
        }
        else
        {
            GFX_CHECK_CALL_ABORT(device->readTexture(
                sampledTexture,
                ResourceState::CopySource,
                textureBlob.writeRef(),
                &rowPitch,
                &pixelSize));
        }
        auto textureValues = (float*)textureBlob->getBufferPointer();

        ValidationTextureData textureResults;
        textureResults.extents = textureInfo->extents;
        textureResults.textureData = textureValues;
        textureResults.strides.x = (uint32_t)pixelSize;
        textureResults.strides.y = (uint32_t)rowPitch;
        textureResults.strides.z = textureResults.extents.height * textureResults.strides.y;

        validateTextureValues(textureResults);
    }

    void run()
    {
        auto entryPointName = getShaderEntryPoint();
        //             printf("%s\n", entryPointName.getBuffer());

        // TODO: Sampler state and null state?
        //             ISamplerState::Desc samplerDesc;
        //             sampler = device->createSamplerState(samplerDesc);

        textureInfo->extents.width = 4;
        textureInfo->extents.height =
            (textureInfo->textureType == TextureType::Texture1D) ? 1 : 4;
        textureInfo->extents.depth =
            (textureInfo->textureType != TextureType::Texture3D) ? 1 : 2;
        textureInfo->mipLevelCount = 1;
        textureInfo->arrayLayerCount = 1;
        generateTextureData(textureInfo, validationFormat);

        // We need to save the pointer to the original texture data for results checking because the
        // texture will be overwritten during testing (if the texture can be written to).
        expectedTextureData = textureInfo->subresourceDatas[getSubresourceIndex(0, 1, 0)].data;

        createRequiredResources();
        submitShaderWork(entryPointName.getBuffer());

        checkTestResults();
    }
};

void shaderAndUnorderedTestImpl(IDevice* device, UnitTestContext* context)
{
    // TODO: Buffer and TextureCube
    for (Int i = 2; i < (int32_t)TextureType::TextureCube; ++i)
    {
        for (Int j = 3; j < (int32_t)TextureViewType::AccelerationStructure; ++j)
        {
            auto shape = (TextureType)i;
            auto view = (TextureViewType)j;
            auto format = Format::R8G8B8A8_UINT;
            auto validationFormat = getValidationTextureFormat(format);
            if (!validationFormat)
                SLANG_CHECK_ABORT(false);

            ShaderAndUnorderedTests test;
            test.init(device, context, format, validationFormat, view, shape);
            test.run();
        }
    }
}

void renderTargetTestImpl(IDevice* device, UnitTestContext* context)
{
    // TODO: Buffer and TextureCube
    for (Int i = 2; i < (int32_t)TextureType::TextureCube; ++i)
    {
        auto shape = (TextureType)i;
        auto view = TextureViewType::RenderTarget;
        auto format = Format::R32G32B32A32_FLOAT;
        auto validationFormat = getValidationTextureFormat(format);
        if (!validationFormat)
            SLANG_CHECK_ABORT(false);

        RenderTargetTests test;
        test.init(device, context, format, validationFormat, view, shape);
        test.run();
    }
}

SLANG_UNIT_TEST(shaderAndUnorderedAccessTests)
{
    runTestImpl(shaderAndUnorderedTestImpl, unitTestContext, DeviceType::D3D12);
    runTestImpl(shaderAndUnorderedTestImpl, unitTestContext, DeviceType::Vulkan);
}

SLANG_UNIT_TEST(renderTargetTests)
{
    runTestImpl(renderTargetTestImpl, unitTestContext, DeviceType::D3D12);
    runTestImpl(renderTargetTestImpl, unitTestContext, DeviceType::Vulkan);
}
} // namespace gfx_test

#endif // Disabled test file

// 1D + array + multisample, ditto for 2D, ditto for 3D
// one test with something bound, one test with nothing bound, one test with subset of layers (set
// values in SubresourceRange and assign in desc)
