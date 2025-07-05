#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "platform/vector-math.h"
#include "platform/window.h"
#include "slang-com-ptr.h"
#include "slang-rhi.h"
#include "slang-rhi/shader-cursor.h"
#include "slang.h"

using namespace rhi;
using namespace Slang;

static const ExampleResources resourceBase("autodiff-texture");

struct Vertex
{
    float position[3];
};

static const int kVertexCount = 4;
static const Vertex kVertexData[kVertexCount] = {
    {{0, 0, 0}},
    {{0, 1, 0}},
    {{1, 0, 0}},
    {{1, 1, 0}},
};
float clearValue[] = {0.0f, 0.0f, 0.0f, 0.0f};


struct AutoDiffTexture : public WindowedAppBase
{

    List<uint32_t> mipMapOffset;
    int textureWidth;
    int textureHeight;

    void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            printf("%s", (const char*)diagnosticsBlob->getBufferPointer());
        }
    }

    Result loadRenderProgram(
        IDevice* device,
        const char* fileName,
        const char* fragmentShader,
        IShaderProgram** outProgram)
    {
        ComPtr<slang::ISession> slangSession;
        slangSession = device->getSlangSession();

        ComPtr<slang::IBlob> diagnosticsBlob;
        Slang::String path = resourceBase.resolveResource(fileName);
        slang::IModule* module =
            slangSession->loadModule(path.getBuffer(), diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!module)
            return SLANG_FAIL;

        ComPtr<slang::IEntryPoint> vertexEntryPoint;
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName("vertexMain", vertexEntryPoint.writeRef()));
        ComPtr<slang::IEntryPoint> fragmentEntryPoint;
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName(fragmentShader, fragmentEntryPoint.writeRef()));

        Slang::List<slang::IComponentType*> componentTypes;
        componentTypes.add(module);
        int entryPointCount = 0;
        int vertexEntryPointIndex = entryPointCount++;
        componentTypes.add(vertexEntryPoint);

        int fragmentEntryPointIndex = entryPointCount++;
        componentTypes.add(fragmentEntryPoint);

        ComPtr<slang::IComponentType> linkedProgram;
        SlangResult result = slangSession->createCompositeComponentType(
            componentTypes.getBuffer(),
            componentTypes.getCount(),
            linkedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);

        if (isTestMode())
        {
            printEntrypointHashes(componentTypes.getCount() - 1, 1, linkedProgram);
        }

        ShaderProgramDesc programDesc = {};
        programDesc.slangGlobalScope = linkedProgram;
        SLANG_RETURN_ON_FAIL(device->createShaderProgram(programDesc, outProgram));

        return SLANG_OK;
    }

    Result loadComputeProgram(IDevice* device, const char* fileName, IShaderProgram** outProgram)
    {
        ComPtr<slang::ISession> slangSession;
        slangSession = device->getSlangSession();

        ComPtr<slang::IBlob> diagnosticsBlob;
        Slang::String path = resourceBase.resolveResource(fileName);
        slang::IModule* module =
            slangSession->loadModule(path.getBuffer(), diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!module)
            return SLANG_FAIL;

        Slang::List<slang::IComponentType*> componentTypes;
        componentTypes.add(module);
        ComPtr<slang::IEntryPoint> computeEntryPoint;
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName("computeMain", computeEntryPoint.writeRef()));
        componentTypes.add(computeEntryPoint);

        ComPtr<slang::IComponentType> linkedProgram;
        SlangResult result = slangSession->createCompositeComponentType(
            componentTypes.getBuffer(),
            componentTypes.getCount(),
            linkedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);

        if (isTestMode())
        {
            printEntrypointHashes(componentTypes.getCount() - 1, 1, linkedProgram);
        }

        ShaderProgramDesc programDesc = {};
        programDesc.slangGlobalScope = linkedProgram;
        SLANG_RETURN_ON_FAIL(device->createShaderProgram(programDesc, outProgram));

        return SLANG_OK;
    }

    ComPtr<IRenderPipeline> gRefPipeline;
    ComPtr<IRenderPipeline> gIterPipeline;
    ComPtr<IComputePipeline> gReconstructPipeline;
    ComPtr<IComputePipeline> gConvertPipeline;
    ComPtr<IComputePipeline> gBuildMipPipeline;
    ComPtr<IComputePipeline> gLearnMipPipeline;
    ComPtr<IRenderPipeline> gDrawQuadPipeline;

    ComPtr<ITexture> gLearningTexture;
    ComPtr<ITextureView> gLearningTextureSRV;
    List<ComPtr<ITextureView>> gLearningTextureUAVs;

    ComPtr<ITexture> gDiffTexture;
    ComPtr<ITextureView> gDiffTextureSRV;
    List<ComPtr<ITextureView>> gDiffTextureUAVs;

    ComPtr<IBuffer> gVertexBuffer;
    ComPtr<ITextureView> gTexView;
    ComPtr<ISampler> gSampler;

    ComPtr<ITexture> gDepthTexture;
    ComPtr<ITextureView> gDepthTextureView;

    ComPtr<ITexture> gIterImage;
    ComPtr<ITextureView> gIterImageSRV;

    ComPtr<ITexture> gRefImage;
    ComPtr<ITextureView> gRefImageSRV;

    ComPtr<IBuffer> gAccumulateBuffer;
    ComPtr<ITextureView> gAccumulateBufferView;

    ComPtr<IBuffer> gReconstructBuffer;
    ComPtr<ITextureView> gReconstructBufferView;


    bool resetLearntTexture = false;

    ComPtr<ITexture> createRenderTargetTexture(Format format, int w, int h, int levels)
    {
        TextureDesc textureDesc = {};
        textureDesc.format = format;
        textureDesc.size.width = w;
        textureDesc.size.height = h;
        textureDesc.size.depth = 1;
        textureDesc.mipCount = levels;
        textureDesc.usage = TextureUsage::ShaderResource | TextureUsage::UnorderedAccess |
                            TextureUsage::RenderTarget;
        textureDesc.defaultState = ResourceState::RenderTarget;
        return gDevice->createTexture(textureDesc);
    }
    ComPtr<ITexture> createDepthTexture()
    {
        TextureDesc textureDesc = {};
        textureDesc.format = Format::D32Float;
        textureDesc.size.width = windowWidth;
        textureDesc.size.height = windowHeight;
        textureDesc.size.depth = 1;
        textureDesc.mipCount = 1;
        textureDesc.usage = TextureUsage::DepthStencil;
        textureDesc.defaultState = ResourceState::DepthWrite;
        return gDevice->createTexture(textureDesc);
    }
    ComPtr<ITextureView> createRTV(ITexture* tex, Format f)
    {
        TextureViewDesc rtvDesc = {};
        rtvDesc.format = f;
        rtvDesc.subresourceRange.mipCount = 1;
        return gDevice->createTextureView(tex, rtvDesc);
    }
    ComPtr<ITextureView> createDSV(ITexture* tex)
    {
        TextureViewDesc dsvDesc = {};
        dsvDesc.format = Format::D32Float;
        dsvDesc.subresourceRange.mipCount = 1;
        return gDevice->createTextureView(tex, dsvDesc);
    }
    ComPtr<ITextureView> createSRV(ITexture* tex)
    {
        TextureViewDesc srvDesc = {};
        return gDevice->createTextureView(tex, srvDesc);
    }
    ComPtr<IRenderPipeline> createRenderPipeline(IInputLayout* inputLayout, IShaderProgram* program)
    {
        ColorTargetDesc colorTarget;
        colorTarget.format = Format::RGBA8Unorm;
        RenderPipelineDesc desc;
        desc.inputLayout = inputLayout;
        desc.program = program;
        desc.targetCount = 1;
        desc.targets = &colorTarget;
        desc.depthStencil.depthTestEnable = true;
        desc.depthStencil.depthWriteEnable = true;
        desc.depthStencil.format = Format::D32Float;
        desc.rasterizer.cullMode = CullMode::None;
        desc.primitiveTopology = PrimitiveTopology::TriangleStrip;
        return gDevice->createRenderPipeline(desc);
    }
    ComPtr<IComputePipeline> createComputePipeline(IShaderProgram* program)
    {
        ComputePipelineDesc desc = {};
        desc.program = program;
        return gDevice->createComputePipeline(desc);
    }
    ComPtr<ITextureView> createUAV(ITexture* texture, int level)
    {
        TextureViewDesc desc = {};
        SubresourceRange textureViewRange = {};
        textureViewRange.mipCount = 1;
        textureViewRange.mip = level;    // Fixed: should be level, not 0
        textureViewRange.layerCount = 1; // Fixed: should be 1, not level
        textureViewRange.layer = 0;
        desc.subresourceRange = textureViewRange;
        return gDevice->createTextureView(texture, desc);
    }
    Slang::Result initialize()
    {
        SLANG_RETURN_ON_FAIL(initializeBase("autodiff-texture", 1024, 768, DeviceType::Default));
        srand(20421);

        if (!isTestMode())
        {
            gWindow->events.keyPress = [this](platform::KeyEventArgs& e)
            {
                if (e.keyChar == 'R' || e.keyChar == 'r')
                    resetLearntTexture = true;
            };
        }

        platform::Rect clientRect{};
        if (isTestMode())
        {
            clientRect.width = 1024;
            clientRect.height = 768;
        }
        else
        {
            clientRect = getWindow()->getClientRect();
        }

        windowWidth = clientRect.width;
        windowHeight = clientRect.height;

        InputElementDesc inputElements[] = {
            {"POSITION", 0, Format::RGB32Float, offsetof(Vertex, position)}};
        auto inputLayout = gDevice->createInputLayout(sizeof(Vertex), &inputElements[0], 1);
        if (!inputLayout)
            return SLANG_FAIL;

        BufferDesc vertexBufferDesc;
        vertexBufferDesc.size = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.elementSize = sizeof(Vertex);
        vertexBufferDesc.usage = BufferUsage::VertexBuffer;
        gVertexBuffer = gDevice->createBuffer(vertexBufferDesc, &kVertexData[0]);
        if (!gVertexBuffer)
            return SLANG_FAIL;

        {
            ComPtr<IShaderProgram> shaderProgram;
            SLANG_RETURN_ON_FAIL(loadRenderProgram(
                gDevice,
                "train.slang",
                "fragmentMain",
                shaderProgram.writeRef()));
            gRefPipeline = createRenderPipeline(inputLayout, shaderProgram);
        }
        {
            ComPtr<IShaderProgram> shaderProgram;
            SLANG_RETURN_ON_FAIL(loadRenderProgram(
                gDevice,
                "train.slang",
                "diffFragmentMain",
                shaderProgram.writeRef()));
            gIterPipeline = createRenderPipeline(inputLayout, shaderProgram);
        }
        {
            ComPtr<IShaderProgram> shaderProgram;
            SLANG_RETURN_ON_FAIL(loadRenderProgram(
                gDevice,
                "draw-quad.slang",
                "fragmentMain",
                shaderProgram.writeRef()));
            gDrawQuadPipeline = createRenderPipeline(inputLayout, shaderProgram);
        }
        {
            ComPtr<IShaderProgram> shaderProgram;
            SLANG_RETURN_ON_FAIL(
                loadComputeProgram(gDevice, "reconstruct.slang", shaderProgram.writeRef()));
            gReconstructPipeline = createComputePipeline(shaderProgram);
        }
        {
            ComPtr<IShaderProgram> shaderProgram;
            SLANG_RETURN_ON_FAIL(
                loadComputeProgram(gDevice, "convert.slang", shaderProgram.writeRef()));
            gConvertPipeline = createComputePipeline(shaderProgram);
        }
        {
            ComPtr<IShaderProgram> shaderProgram;
            SLANG_RETURN_ON_FAIL(
                loadComputeProgram(gDevice, "buildmip.slang", shaderProgram.writeRef()));
            gBuildMipPipeline = createComputePipeline(shaderProgram);
        }
        {
            ComPtr<IShaderProgram> shaderProgram;
            SLANG_RETURN_ON_FAIL(
                loadComputeProgram(gDevice, "learnmip.slang", shaderProgram.writeRef()));
            gLearnMipPipeline = createComputePipeline(shaderProgram);
        }

        // Load texture from file - this would need to be adapted to use slang-rhi texture loading
        Slang::String imagePath = resourceBase.resolveResource("checkerboard.jpg");
        gTexView = createTextureFromFile(imagePath.getBuffer(), textureWidth, textureHeight);
        textureWidth = 512; // Placeholder values
        textureHeight = 512;
        initMipOffsets(textureWidth, textureHeight);

        BufferDesc bufferDesc = {};
        bufferDesc.size = mipMapOffset.getLast() * sizeof(uint32_t);
        bufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::UnorderedAccess;

        gAccumulateBuffer = gDevice->createBuffer(bufferDesc);
        if (!gAccumulateBuffer)
        {
            printf("ERROR: Failed to create accumulate buffer!\n");
            return SLANG_FAIL;
        }

        gReconstructBuffer = gDevice->createBuffer(bufferDesc);
        if (!gReconstructBuffer)
        {
            printf("ERROR: Failed to create reconstruct buffer!\n");
            return SLANG_FAIL;
        }

        int mipCount = 1 + Math::Log2Ceil(Math::Max(textureWidth, textureHeight));
        SubresourceData initialData = {};
        initialData.data = gLearningTexture =
            createRenderTargetTexture(Format::RGBA32Float, textureWidth, textureHeight, mipCount);
        gLearningTextureSRV = createSRV(gLearningTexture);
        for (int i = 0; i < mipCount; i++)
            gLearningTextureUAVs.add(createUAV(gLearningTexture, i));

        gDiffTexture =
            createRenderTargetTexture(Format::RGBA32Float, textureWidth, textureHeight, mipCount);
        gDiffTextureSRV = createSRV(gDiffTexture);
        for (int i = 0; i < mipCount; i++)
            gDiffTextureUAVs.add(createUAV(gDiffTexture, i));

        SamplerDesc samplerDesc = {};
        gSampler = gDevice->createSampler(samplerDesc);

        gDepthTexture = createDepthTexture();
        gDepthTextureView = createDSV(gDepthTexture);

        gRefImage = createRenderTargetTexture(Format::RGBA8Unorm, windowWidth, windowHeight, 1);
        gRefImageSRV = createSRV(gRefImage);

        gIterImage = createRenderTargetTexture(Format::RGBA8Unorm, windowWidth, windowHeight, 1);
        gIterImageSRV = createSRV(gIterImage);

        // Initialize textures
        {
            auto commandEncoder = gQueue->createCommandEncoder();
            // Clear learning and diff textures
            commandEncoder->clearTextureFloat(gLearningTexture, kEntireTexture, clearValue);
            commandEncoder->clearTextureFloat(gDiffTexture, kEntireTexture, clearValue);

            gQueue->submit(commandEncoder->finish());
        }

        return SLANG_OK;
    }

    void initMipOffsets(int w, int h)
    {
        int layers = 1 + Math::Log2Ceil(Math::Max(w, h));
        uint32_t offset = 0;
        for (int i = 0; i < layers; i++)
        {
            auto lw = Math::Max(1, w >> i);
            auto lh = Math::Max(1, h >> i);
            mipMapOffset.add(offset);
            offset += lw * lh * 4;
        }
        mipMapOffset.add(offset);
    }

    glm::mat4x4 getTransformMatrix()
    {
        float rotX = (rand() / (float)RAND_MAX) * 0.3f;
        float rotY = (rand() / (float)RAND_MAX) * 0.2f;
        glm::mat4x4 matProj = glm::perspectiveRH_ZO(
            glm::radians(60.0f),
            (float)windowWidth / (float)windowHeight,
            0.1f,
            1000.0f);
        auto identity = glm::mat4(1.0f);
        auto translate = glm::translate(
            identity,
            glm::vec3(
                -0.6f + 0.2f * (rand() / (float)RAND_MAX),
                -0.6f + 0.2f * (rand() / (float)RAND_MAX),
                -1.0f));
        auto rot = glm::rotate(translate, -glm::pi<float>() * rotX, glm::vec3(1.0f, 0.0f, 0.0f));
        rot = glm::rotate(rot, -glm::pi<float>() * rotY, glm::vec3(0.0f, 1.0f, 0.0f));
        auto transformMatrix = matProj * rot;
        transformMatrix = glm::transpose(transformMatrix);
        return transformMatrix;
    }

    template<typename SetupPipelineFunc>
    void renderImage(ITexture* renderTarget, const SetupPipelineFunc& setupPipeline)
    {
        auto commandEncoder = gQueue->createCommandEncoder();

        ComPtr<ITextureView> renderTargetView = createRTV(renderTarget, Format::RGBA8Unorm);
        RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = renderTargetView;
        colorAttachment.loadOp = LoadOp::Clear;
        colorAttachment.clearValue[0] = 0.3f;
        colorAttachment.clearValue[1] = 0.5f;
        colorAttachment.clearValue[2] = 0.7f;
        colorAttachment.clearValue[3] = 1.0f;

        RenderPassDepthStencilAttachment depthAttachment = {};
        depthAttachment.view = gDepthTextureView;
        depthAttachment.depthLoadOp = LoadOp::Clear;
        depthAttachment.depthClearValue = 1.0f;

        RenderPassDesc renderPass = {};
        renderPass.colorAttachments = &colorAttachment;
        renderPass.colorAttachmentCount = 1;
        renderPass.depthStencilAttachment = &depthAttachment;

        auto renderEncoder = commandEncoder->beginRenderPass(renderPass);

        RenderState renderState = {};
        renderState.viewports[0] = Viewport::fromSize(windowWidth, windowHeight);
        renderState.viewportCount = 1;
        renderState.scissorRects[0] = ScissorRect::fromSize(windowWidth, windowHeight);
        renderState.scissorRectCount = 1;
        renderState.vertexBuffers[0] = gVertexBuffer;
        renderState.vertexBufferCount = 1;

        setupPipeline(renderEncoder);


        renderEncoder->setRenderState(renderState);

        DrawArguments drawArgs = {};
        drawArgs.vertexCount = 4;
        renderEncoder->draw(drawArgs);
        renderEncoder->end();
        gQueue->submit(commandEncoder->finish());
    }

    void renderReferenceImage(glm::mat4x4 transformMatrix)
    {
        renderImage(
            gRefImage,
            [&](IRenderPassEncoder* encoder)
            {
                auto rootObject =
                    encoder->bindPipeline(static_cast<IRenderPipeline*>(gRefPipeline.get()));
                ShaderCursor rootCursor(rootObject);
                rootCursor["Uniforms"]["modelViewProjection"].setData(
                    &transformMatrix,
                    sizeof(float) * 16);
                rootCursor["Uniforms"]["bwdTexture"]["texture"].setBinding(gTexView);
                rootCursor["Uniforms"]["sampler"].setBinding(gSampler);
                rootCursor["Uniforms"]["mipOffset"].setData(
                    mipMapOffset.getBuffer(),
                    sizeof(uint32_t) * mipMapOffset.getCount());
                rootCursor["Uniforms"]["texRef"].setBinding(gTexView);
                rootCursor["Uniforms"]["bwdTexture"]["accumulateBuffer"].setBinding(
                    gAccumulateBuffer);
            });
    }

    virtual void renderFrame(ITexture* texture) override
    {
        static uint32_t frameCount = 0;
        frameCount++;
        auto transformMatrix = getTransformMatrix();
        renderReferenceImage(transformMatrix);

        // Clear buffers
        {
            auto commandEncoder = gQueue->createCommandEncoder();
            commandEncoder->clearBuffer(gAccumulateBuffer, 0, gAccumulateBuffer->getDesc().size);
            commandEncoder->clearBuffer(gReconstructBuffer, 0, gReconstructBuffer->getDesc().size);

            if (resetLearntTexture)
            {
                commandEncoder->clearTextureFloat(gLearningTexture, kEntireTexture, clearValue);
                resetLearntTexture = false;
            }
            gQueue->submit(commandEncoder->finish());
        }

        // Render image using backward propagate shader to obtain texture-space gradients.
        renderImage(
            gIterImage,
            [&](IRenderPassEncoder* encoder)
            {
                auto rootObject = encoder->bindPipeline(gIterPipeline.get());
                ShaderCursor rootCursor(rootObject);

                rootCursor["Uniforms"]["modelViewProjection"].setData(
                    &transformMatrix,
                    sizeof(float) * 16);
                rootCursor["Uniforms"]["bwdTexture"]["texture"].setBinding(gLearningTextureSRV);
                rootCursor["Uniforms"]["sampler"].setBinding(gSampler);
                rootCursor["Uniforms"]["mipOffset"].setData(
                    mipMapOffset.getBuffer(),
                    sizeof(uint32_t) * mipMapOffset.getCount());
                rootCursor["Uniforms"]["texRef"].setBinding(gRefImageSRV);
                rootCursor["Uniforms"]["bwdTexture"]["accumulateBuffer"].setBinding(
                    gAccumulateBuffer);
                rootCursor["Uniforms"]["bwdTexture"]["minLOD"].setData(5.0);
            });

        // Propagete gradients through mip map layers from top (lowest res) to bottom (highest res).
        {
            auto commandEncoder = gQueue->createCommandEncoder();
            auto encoder = commandEncoder->beginComputePass();
            auto rootObject = encoder->bindPipeline(gReconstructPipeline.get());
            for (int i = (int)mipMapOffset.getCount() - 2; i >= 0; i--)
            {
                ShaderCursor rootCursor(rootObject);
                rootCursor["Uniforms"]["mipOffset"].setData(
                    mipMapOffset.getBuffer(),
                    sizeof(uint32_t) * mipMapOffset.getCount());
                rootCursor["Uniforms"]["dstLayer"].setData(i);
                rootCursor["Uniforms"]["layerCount"].setData(mipMapOffset.getCount() - 1);
                rootCursor["Uniforms"]["width"].setData(textureWidth);
                rootCursor["Uniforms"]["height"].setData(textureHeight);
                rootCursor["Uniforms"]["accumulateBuffer"].setBinding(gAccumulateBuffer);
                rootCursor["Uniforms"]["dstBuffer"].setBinding(gReconstructBuffer);

                encoder->dispatchCompute(
                    ((textureWidth >> i) + 15) / 16,
                    ((textureHeight >> i) + 15) / 16,
                    1);
            }
            encoder->end();
            gQueue->submit(commandEncoder->finish());

            commandEncoder = gQueue->createCommandEncoder();
            // Convert bottom layer mip from buffer to texture
            {
                auto encoder = commandEncoder->beginComputePass();
                auto rootObject = encoder->bindPipeline(gConvertPipeline.get());
                ShaderCursor rootCursor(rootObject);
                rootCursor["Uniforms"]["mipOffset"].setData(
                    mipMapOffset.getBuffer(),
                    sizeof(uint32_t) * mipMapOffset.getCount());
                rootCursor["Uniforms"]["dstLayer"].setData(0);
                rootCursor["Uniforms"]["width"].setData(textureWidth);
                rootCursor["Uniforms"]["height"].setData(textureHeight);
                rootCursor["Uniforms"]["srcBuffer"].setBinding(gReconstructBuffer);
                rootCursor["Uniforms"]["dstTexture"].setBinding(gDiffTextureUAVs[0]);
                encoder->dispatchCompute((textureWidth + 15) / 16, (textureHeight + 15) / 16, 1);
                encoder->end();
            }

            // Build higher level mip map layers
            encoder = commandEncoder->beginComputePass();
            rootObject = encoder->bindPipeline(gBuildMipPipeline.get());
            for (int i = 1; i < (int)mipMapOffset.getCount() - 1; i++)
            {

                ShaderCursor rootCursor(rootObject);
                rootCursor["Uniforms"]["dstWidth"].setData(textureWidth >> i);
                rootCursor["Uniforms"]["dstHeight"].setData(textureHeight >> i);
                rootCursor["Uniforms"]["srcTexture"].setBinding(gDiffTextureUAVs[i - 1]);
                rootCursor["Uniforms"]["dstTexture"].setBinding(gDiffTextureUAVs[i]);
                encoder->dispatchCompute(
                    ((textureWidth >> i) + 15) / 16,
                    ((textureHeight >> i) + 15) / 16,
                    1);
            }
            encoder->end();

            // Accumulate gradients to learnt texture
            encoder = commandEncoder->beginComputePass();
            rootObject = encoder->bindPipeline(gLearnMipPipeline.get());
            for (int i = 0; i < (int)mipMapOffset.getCount() - 1; i++)
            {
                ShaderCursor rootCursor(rootObject);
                rootCursor["Uniforms"]["dstWidth"].setData(textureWidth >> i);
                rootCursor["Uniforms"]["dstHeight"].setData(textureHeight >> i);
                rootCursor["Uniforms"]["learningRate"].setData(0.1f);
                rootCursor["Uniforms"]["srcTexture"].setBinding(gDiffTextureUAVs[i]);
                rootCursor["Uniforms"]["dstTexture"].setBinding(gLearningTextureUAVs[i]);
                encoder->dispatchCompute(
                    ((textureWidth >> i) + 15) / 16,
                    ((textureHeight >> i) + 15) / 16,
                    1);
            }
            encoder->end();

            gQueue->submit(commandEncoder->finish());
        }

        // Draw currently learnt texture
        {
            auto commandEncoder = gQueue->createCommandEncoder();

            ComPtr<ITextureView> textureView = gDevice->createTextureView(texture, {});
            RenderPassColorAttachment colorAttachment = {};
            colorAttachment.view = textureView;
            colorAttachment.loadOp = LoadOp::Clear;

            RenderPassDesc renderPass = {};
            renderPass.colorAttachments = &colorAttachment;
            renderPass.colorAttachmentCount = 1;

            auto renderEncoder = commandEncoder->beginRenderPass(renderPass);

            drawTexturedQuad(renderEncoder, 0, 0, textureWidth, textureHeight, gLearningTextureSRV);

            int refImageWidth = windowWidth - textureWidth - 10;
            int refImageHeight = refImageWidth * windowHeight / windowWidth;
            drawTexturedQuad(
                renderEncoder,
                textureWidth + 10,
                0,
                refImageWidth,
                refImageHeight,
                gRefImageSRV);

            drawTexturedQuad(
                renderEncoder,
                textureWidth + 10,
                refImageHeight + 10,
                refImageWidth,
                refImageHeight,
                gIterImageSRV);
            renderEncoder->end();
            gQueue->submit(commandEncoder->finish());
        }

        if (!isTestMode())
        {
            gSurface->present();
        }
    }

    void drawTexturedQuad(
        IRenderPassEncoder* renderEncoder,
        int x,
        int y,
        int w,
        int h,
        ITextureView* srv)
    {
        RenderState renderState = {};
        renderState.viewports[0] = Viewport::fromSize(windowWidth, windowHeight);
        renderState.viewportCount = 1;
        renderState.scissorRects[0] = ScissorRect::fromSize(windowWidth, windowHeight);
        renderState.scissorRectCount = 1;
        renderState.vertexBuffers[0] = gVertexBuffer;
        renderState.vertexBufferCount = 1;
        renderEncoder->setRenderState(renderState);

        auto root =
            renderEncoder->bindPipeline(static_cast<IRenderPipeline*>(gDrawQuadPipeline.get()));
        ShaderCursor rootCursor(root);
        rootCursor["Uniforms"]["x"].setData(x);
        rootCursor["Uniforms"]["y"].setData(y);
        rootCursor["Uniforms"]["width"].setData(w);
        rootCursor["Uniforms"]["height"].setData(h);
        rootCursor["Uniforms"]["viewWidth"].setData(windowWidth);
        rootCursor["Uniforms"]["viewHeight"].setData(windowHeight);
        rootCursor["Uniforms"]["texture"].setBinding(srv);
        rootCursor["Uniforms"]["sampler"].setBinding(gSampler);

        DrawArguments drawArgs = {};
        drawArgs.vertexCount = 4;
        renderEncoder->draw(drawArgs);
    }
};

EXAMPLE_MAIN(innerMain<AutoDiffTexture>);
