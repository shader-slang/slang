// main.cpp

// This file implements an example of hardware ray-tracing using
// Slang shaders and the `slang-rhi` graphics API.

#include "core/slang-basic.h"
#include "examples/example-base/example-base.h"
#include "platform/vector-math.h"
#include "platform/window.h"
#include "slang-com-ptr.h"
#include "slang-rhi.h"
#include "slang-rhi/acceleration-structure-utils.h"
#include "slang-rhi/shader-cursor.h"
#include "slang.h"

using namespace rhi;
using namespace Slang;

static const ExampleResources resourceBase("ray-tracing");

struct Uniforms
{
    float screenWidth, screenHeight;
    float focalLength = 24.0f, frameHeight = 24.0f;
    float cameraDir[4];
    float cameraUp[4];
    float cameraRight[4];
    float cameraPosition[4];
    float lightDir[4];
};

struct Vertex
{
    float position[3];
};

// Define geometry data for our test scene.
// The scene contains a floor plane, and a cube placed on top of it at the center.
static const int kVertexCount = 24;
static const Vertex kVertexData[kVertexCount] = {
    // Floor plane
    {{-100.0f, 0, 100.0f}},
    {{100.0f, 0, 100.0f}},
    {{100.0f, 0, -100.0f}},
    {{-100.0f, 0, -100.0f}},
    // Cube face (+y).
    {{-1.0f, 2.0, 1.0f}},
    {{1.0f, 2.0, 1.0f}},
    {{1.0f, 2.0, -1.0f}},
    {{-1.0f, 2.0, -1.0f}},
    // Cube face (+z).
    {{-1.0f, 0.0, 1.0f}},
    {{1.0f, 0.0, 1.0f}},
    {{1.0f, 2.0, 1.0f}},
    {{-1.0f, 2.0, 1.0f}},
    // Cube face (-z).
    {{-1.0f, 0.0, -1.0f}},
    {{-1.0f, 2.0, -1.0f}},
    {{1.0f, 2.0, -1.0f}},
    {{1.0f, 0.0, -1.0f}},
    // Cube face (-x).
    {{-1.0f, 0.0, -1.0f}},
    {{-1.0f, 0.0, 1.0f}},
    {{-1.0f, 2.0, 1.0f}},
    {{-1.0f, 2.0, -1.0f}},
    // Cube face (+x).
    {{1.0f, 2.0, -1.0f}},
    {{1.0f, 2.0, 1.0f}},
    {{1.0f, 0.0, 1.0f}},
    {{1.0f, 0.0, -1.0f}},
};
static const int kIndexCount = 36;
static const int kIndexData[kIndexCount] = {0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,
                                            8,  9,  10, 8,  10, 11, 12, 13, 14, 12, 14, 15,
                                            16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};

struct Primitive
{
    float data[4];
    float color[4];
};
static const int kPrimitiveCount = 12;
static const Primitive kPrimitiveData[kPrimitiveCount] = {
    {{0.0f, 1.0f, 0.0f, 0.0f}, {0.75f, 0.8f, 0.85f, 1.0f}},
    {{0.0f, 1.0f, 0.0f, 0.0f}, {0.75f, 0.8f, 0.85f, 1.0f}},
    {{0.0f, 1.0f, 0.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{0.0f, 1.0f, 0.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{0.0f, 0.0f, 1.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{0.0f, 0.0f, 1.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{0.0f, 0.0f, -1.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{0.0f, 0.0f, -1.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{-1.0f, 0.0f, 0.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{-1.0f, 0.0f, 0.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{1.0f, 0.0f, 0.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
    {{1.0f, 0.0f, 0.0f, 0.0f}, {0.95f, 0.85f, 0.05f, 1.0f}},
};


// We need to use a rasterization pipeline to copy the ray-traced image
// to the swapchain. To do so we need to render a full-screen triangle.
// We will define a small helper type that defines the data for such a triangle.
//
struct FullScreenTriangle
{
    struct Vertex
    {
        float position[2];
    };

    enum
    {
        kVertexCount = 3
    };

    static const Vertex kVertices[kVertexCount];
};
const FullScreenTriangle::Vertex FullScreenTriangle::kVertices[FullScreenTriangle::kVertexCount] = {
    {{-1, -1}},
    {{-1, 3}},
    {{3, -1}},
};

// The example application will be implemented as a `struct`, so that
// we can scope the resources it allocates without using global variables.
//
struct RayTracing : public WindowedAppBase
{


    Uniforms gUniforms = {};


    // Many Slang API functions return detailed diagnostic information
    // (error messages, warnings, etc.) as a "blob" of data, or return
    // a null blob pointer instead if there were no issues.
    //
    // For convenience, we define a subroutine that will dump the information
    // in a diagnostic blob if one is produced, and skip it otherwise.
    //
    void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            printf("%s", (const char*)diagnosticsBlob->getBufferPointer());
#ifdef _WIN32
            _Win32OutputDebugString((const char*)diagnosticsBlob->getBufferPointer());
#endif
        }
    }

    // Load and compile shader code from souce.
    Result loadShaderProgram(IDevice* device, bool isComputePipeline, IShaderProgram** outProgram)
    {
        ComPtr<slang::ISession> slangSession;
        slangSession = device->getSlangSession();

        ComPtr<slang::IBlob> diagnosticsBlob;
        Slang::String path = resourceBase.resolveResource("shaders.slang");
        slang::IModule* module =
            slangSession->loadModule(path.getBuffer(), diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!module)
            return SLANG_FAIL;

        Slang::List<slang::IComponentType*> componentTypes;
        componentTypes.add(module);
        if (isComputePipeline)
        {
            ComPtr<slang::IEntryPoint> computeEntryPoint;
            SLANG_RETURN_ON_FAIL(
                module->findEntryPointByName("computeMain", computeEntryPoint.writeRef()));
            componentTypes.add(computeEntryPoint);
        }
        else
        {
            ComPtr<slang::IEntryPoint> entryPoint;
            SLANG_RETURN_ON_FAIL(module->findEntryPointByName("vertexMain", entryPoint.writeRef()));
            componentTypes.add(entryPoint);
            SLANG_RETURN_ON_FAIL(
                module->findEntryPointByName("fragmentMain", entryPoint.writeRef()));
            componentTypes.add(entryPoint);
        }

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

    ComPtr<IRenderPipeline> gPresentPipeline;
    ComPtr<IComputePipeline> gRenderPipeline;
    ComPtr<IBuffer> gFullScreenVertexBuffer;
    ComPtr<IBuffer> gVertexBuffer;
    ComPtr<IBuffer> gIndexBuffer;
    ComPtr<IBuffer> gPrimitiveBuffer;
    ComPtr<IBuffer> gTransformBuffer;
    ComPtr<IBuffer> gInstanceBuffer;
    ComPtr<IAccelerationStructure> gBLAS;
    ComPtr<IAccelerationStructure> gTLAS;
    ComPtr<ITexture> gResultTexture;

    uint64_t lastTime = 0;

    // glm::vec3 lightDir = normalize(glm::vec3(10, 10, 10));
    // glm::vec3 lightColor = glm::vec3(1, 1, 1);

    glm::vec3 cameraPosition = glm::vec3(-2.53f, 2.72f, 4.3f);
    float cameraOrientationAngles[2] = {-0.475f, -0.35f}; // Spherical angles (theta, phi).

    float translationScale = 0.5f;
    float rotationScale = 0.01f;

    // In order to control camera movement, we will
    // use good old WASD
    bool wPressed = false;
    bool aPressed = false;
    bool sPressed = false;
    bool dPressed = false;

    bool isMouseDown = false;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;

    void setKeyState(platform::KeyCode key, bool state)
    {
        switch (key)
        {
        default:
            break;
        case platform::KeyCode::W:
            wPressed = state;
            break;
        case platform::KeyCode::A:
            aPressed = state;
            break;
        case platform::KeyCode::S:
            sPressed = state;
            break;
        case platform::KeyCode::D:
            dPressed = state;
            break;
        }
    }
    void onKeyDown(platform::KeyEventArgs args) { setKeyState(args.key, true); }
    void onKeyUp(platform::KeyEventArgs args) { setKeyState(args.key, false); }

    void onMouseDown(platform::MouseEventArgs args)
    {
        isMouseDown = true;
        lastMouseX = (float)args.x;
        lastMouseY = (float)args.y;
    }

    void onMouseMove(platform::MouseEventArgs args)
    {
        if (isMouseDown)
        {
            float deltaX = args.x - lastMouseX;
            float deltaY = args.y - lastMouseY;

            cameraOrientationAngles[0] += -deltaX * rotationScale;
            cameraOrientationAngles[1] += -deltaY * rotationScale;
            lastMouseX = (float)args.x;
            lastMouseY = (float)args.y;
        }
    }
    void onMouseUp(platform::MouseEventArgs args) { isMouseDown = false; }

    Slang::Result initialize()
    {
        SLANG_RETURN_ON_FAIL(initializeBase("Ray Tracing", 1024, 768, getDeviceType()));

        if (!isTestMode())
        {
            gWindow->events.mouseMove = [this](const platform::MouseEventArgs& e)
            { onMouseMove(e); };
            gWindow->events.mouseUp = [this](const platform::MouseEventArgs& e) { onMouseUp(e); };
            gWindow->events.mouseDown = [this](const platform::MouseEventArgs& e)
            { onMouseDown(e); };
            gWindow->events.keyDown = [this](const platform::KeyEventArgs& e) { onKeyDown(e); };
            gWindow->events.keyUp = [this](const platform::KeyEventArgs& e) { onKeyUp(e); };
        }

        BufferDesc vertexBufferDesc;
        vertexBufferDesc.size = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.usage = BufferUsage::AccelerationStructureBuildInput;
        vertexBufferDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
        gVertexBuffer = gDevice->createBuffer(vertexBufferDesc, &kVertexData[0]);
        if (!gVertexBuffer)
            return SLANG_FAIL;

        BufferDesc indexBufferDesc;
        indexBufferDesc.size = kIndexCount * sizeof(int32_t);
        indexBufferDesc.usage = BufferUsage::AccelerationStructureBuildInput;
        indexBufferDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
        gIndexBuffer = gDevice->createBuffer(indexBufferDesc, &kIndexData[0]);
        if (!gIndexBuffer)
            return SLANG_FAIL;

        BufferDesc primitiveBufferDesc;
        primitiveBufferDesc.size = kPrimitiveCount * sizeof(Primitive);
        primitiveBufferDesc.elementSize = sizeof(Primitive);
        primitiveBufferDesc.usage = BufferUsage::ShaderResource;
        primitiveBufferDesc.defaultState = ResourceState::ShaderResource;
        gPrimitiveBuffer = gDevice->createBuffer(primitiveBufferDesc, &kPrimitiveData[0]);
        if (!gPrimitiveBuffer)
            return SLANG_FAIL;

        BufferDesc transformBufferDesc;
        transformBufferDesc.size = sizeof(float) * 12;
        transformBufferDesc.usage = BufferUsage::AccelerationStructureBuildInput;
        transformBufferDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
        float transformData[12] =
            {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        gTransformBuffer = gDevice->createBuffer(transformBufferDesc, &transformData);
        if (!gTransformBuffer)
            return SLANG_FAIL;
        // Build bottom level acceleration structure.
        {
            AccelerationStructureBuildInput buildInput = {};
            buildInput.type = AccelerationStructureBuildInputType::Triangles;
            buildInput.triangles.vertexBuffers[0] = gVertexBuffer;
            buildInput.triangles.vertexBufferCount = 1;
            buildInput.triangles.vertexFormat = Format::RGB32Float;
            buildInput.triangles.vertexCount = kVertexCount;
            buildInput.triangles.vertexStride = sizeof(Vertex);
            buildInput.triangles.indexBuffer = gIndexBuffer;
            buildInput.triangles.indexFormat = IndexFormat::Uint32;
            buildInput.triangles.indexCount = kIndexCount;
            buildInput.triangles.preTransformBuffer = gTransformBuffer;
            buildInput.triangles.flags = AccelerationStructureGeometryFlags::Opaque;

            AccelerationStructureBuildDesc buildDesc = {};
            buildDesc.inputs = &buildInput;
            buildDesc.inputCount = 1;
            buildDesc.flags = AccelerationStructureBuildFlags::AllowCompaction;

            // Query buffer size for acceleration structure build.
            AccelerationStructureSizes sizes;
            SLANG_RETURN_ON_FAIL(gDevice->getAccelerationStructureSizes(buildDesc, &sizes));

            // Allocate buffers for acceleration structure.
            BufferDesc scratchBufferDesc;
            scratchBufferDesc.usage = BufferUsage::UnorderedAccess;
            scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
            scratchBufferDesc.size = sizes.scratchSize;
            ComPtr<IBuffer> scratchBuffer = gDevice->createBuffer(scratchBufferDesc);
            if (!scratchBuffer)
                return SLANG_FAIL;

            // Build acceleration structure.
            ComPtr<IQueryPool> compactedSizeQuery;
            QueryPoolDesc queryPoolDesc;
            queryPoolDesc.count = 1;
            queryPoolDesc.type = QueryType::AccelerationStructureCompactedSize;
            SLANG_RETURN_ON_FAIL(
                gDevice->createQueryPool(queryPoolDesc, compactedSizeQuery.writeRef()));

            ComPtr<IAccelerationStructure> draftAS;
            AccelerationStructureDesc draftCreateDesc;
            draftCreateDesc.size = sizes.accelerationStructureSize;
            SLANG_RETURN_ON_FAIL(
                gDevice->createAccelerationStructure(draftCreateDesc, draftAS.writeRef()));

            compactedSizeQuery->reset();

            auto commandEncoder = gQueue->createCommandEncoder();
            AccelerationStructureQueryDesc compactedSizeQueryDesc = {};
            compactedSizeQueryDesc.queryPool = compactedSizeQuery;
            compactedSizeQueryDesc.queryType = QueryType::AccelerationStructureCompactedSize;
            commandEncoder->buildAccelerationStructure(
                buildDesc,
                draftAS,
                nullptr,
                scratchBuffer,
                1,
                &compactedSizeQueryDesc);
            gQueue->submit(commandEncoder->finish());
            gQueue->waitOnHost();

            uint64_t compactedSize = 0;
            compactedSizeQuery->getResult(0, 1, &compactedSize);
            AccelerationStructureDesc createDesc;
            createDesc.size = compactedSize;
            gDevice->createAccelerationStructure(createDesc, gBLAS.writeRef());

            commandEncoder = gQueue->createCommandEncoder();
            commandEncoder->copyAccelerationStructure(
                gBLAS,
                draftAS,
                AccelerationStructureCopyMode::Compact);
            gQueue->submit(commandEncoder->finish());
            gQueue->waitOnHost();
        }

        // Build top level acceleration structure.
        {
            AccelerationStructureInstanceDescType nativeInstanceDescType =
                getAccelerationStructureInstanceDescType(gDevice);
            Size nativeInstanceDescSize =
                getAccelerationStructureInstanceDescSize(nativeInstanceDescType);

            std::vector<AccelerationStructureInstanceDescGeneric> instanceDescs;
            instanceDescs.resize(1);
            float transformMatrix[] =
                {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
            memcpy(&instanceDescs[0].transform[0][0], transformMatrix, sizeof(float) * 12);

            instanceDescs[0].instanceID = 0;
            instanceDescs[0].instanceMask = 0xFF;
            instanceDescs[0].instanceContributionToHitGroupIndex = 0;
            instanceDescs[0].flags = AccelerationStructureInstanceFlags::TriangleFacingCullDisable;
            instanceDescs[0].accelerationStructure = gBLAS->getHandle();

            std::vector<uint8_t> nativeInstanceDescs(instanceDescs.size() * nativeInstanceDescSize);
            convertAccelerationStructureInstanceDescs(
                instanceDescs.size(),
                nativeInstanceDescType,
                nativeInstanceDescs.data(),
                nativeInstanceDescSize,
                instanceDescs.data(),
                sizeof(AccelerationStructureInstanceDescGeneric));

            BufferDesc instanceBufferDesc;
            instanceBufferDesc.size =
                instanceDescs.size() * sizeof(AccelerationStructureInstanceDescGeneric);
            instanceBufferDesc.usage = BufferUsage::ShaderResource;
            instanceBufferDesc.defaultState = ResourceState::ShaderResource;
            gInstanceBuffer = gDevice->createBuffer(instanceBufferDesc, nativeInstanceDescs.data());
            if (!gInstanceBuffer)
                return SLANG_FAIL;

            AccelerationStructureBuildInput buildInput = {};
            buildInput.type = AccelerationStructureBuildInputType::Instances;
            buildInput.instances.instanceBuffer = gInstanceBuffer;
            buildInput.instances.instanceCount = 1;
            buildInput.instances.instanceStride = nativeInstanceDescSize;

            AccelerationStructureBuildDesc buildDesc = {};
            buildDesc.inputs = &buildInput;
            buildDesc.inputCount = 1;

            // Query buffer size for acceleration structure build.
            AccelerationStructureSizes sizes;
            SLANG_RETURN_ON_FAIL(gDevice->getAccelerationStructureSizes(buildDesc, &sizes));

            BufferDesc scratchBufferDesc;
            scratchBufferDesc.usage = BufferUsage::UnorderedAccess;
            scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
            scratchBufferDesc.size = sizes.scratchSize;
            ComPtr<IBuffer> scratchBuffer = gDevice->createBuffer(scratchBufferDesc);

            AccelerationStructureDesc createDesc;
            createDesc.size = sizes.accelerationStructureSize;
            SLANG_RETURN_ON_FAIL(
                gDevice->createAccelerationStructure(createDesc, gTLAS.writeRef()));

            auto commandEncoder = gQueue->createCommandEncoder();
            commandEncoder
                ->buildAccelerationStructure(buildDesc, gTLAS, nullptr, scratchBuffer, 0, nullptr);
            gQueue->submit(commandEncoder->finish());
            gQueue->waitOnHost();
        }

        BufferDesc fullScreenVertexBufferDesc;
        fullScreenVertexBufferDesc.size =
            FullScreenTriangle::kVertexCount * sizeof(FullScreenTriangle::Vertex);
        fullScreenVertexBufferDesc.usage = BufferUsage::VertexBuffer;
        fullScreenVertexBufferDesc.defaultState = ResourceState::VertexBuffer;
        gFullScreenVertexBuffer =
            gDevice->createBuffer(fullScreenVertexBufferDesc, &FullScreenTriangle::kVertices[0]);
        if (!gFullScreenVertexBuffer)
            return SLANG_FAIL;

        InputElementDesc inputElements[] = {
            {"POSITION", 0, Format::RG32Float, offsetof(FullScreenTriangle::Vertex, position)},
        };
        auto inputLayout = gDevice->createInputLayout(
            sizeof(FullScreenTriangle::Vertex),
            &inputElements[0],
            SLANG_COUNT_OF(inputElements));
        if (!inputLayout)
            return SLANG_FAIL;

        ComPtr<IShaderProgram> shaderProgram;
        SLANG_RETURN_ON_FAIL(loadShaderProgram(gDevice, false, shaderProgram.writeRef()));
        ColorTargetDesc colorTarget;
        colorTarget.format = Format::RGBA16Float;
        RenderPipelineDesc desc;
        desc.inputLayout = inputLayout;
        desc.program = shaderProgram;
        desc.targetCount = 1;
        desc.targets = &colorTarget;
        desc.depthStencil.depthTestEnable = false;
        desc.depthStencil.depthWriteEnable = false;
        desc.primitiveTopology = PrimitiveTopology::TriangleList;
        gPresentPipeline = gDevice->createRenderPipeline(desc);
        if (!gPresentPipeline)
            return SLANG_FAIL;

        ComPtr<IShaderProgram> computeProgram;
        SLANG_RETURN_ON_FAIL(loadShaderProgram(gDevice, true, computeProgram.writeRef()));
        ComputePipelineDesc computeDesc;
        computeDesc.program = computeProgram;
        gRenderPipeline = gDevice->createComputePipeline(computeDesc);
        if (!gRenderPipeline)
            return SLANG_FAIL;

        createResultTexture();
        return SLANG_OK;
    }

    void createResultTexture()
    {
        TextureDesc resultTextureDesc = {};
        resultTextureDesc.type = TextureType::Texture2D;
        resultTextureDesc.mipCount = 1;
        resultTextureDesc.size.width = windowWidth;
        resultTextureDesc.size.height = windowHeight;
        resultTextureDesc.size.depth = 1;
        resultTextureDesc.usage = TextureUsage::UnorderedAccess | TextureUsage::ShaderResource;
        resultTextureDesc.defaultState = ResourceState::UnorderedAccess;
        resultTextureDesc.format = Format::RGBA16Float;
        gResultTexture = gDevice->createTexture(resultTextureDesc);
    }

    virtual void windowSizeChanged() override
    {
        WindowedAppBase::windowSizeChanged();
        createResultTexture();
    }

    glm::vec3 getVectorFromSphericalAngles(float theta, float phi)
    {
        auto sinTheta = sin(theta);
        auto cosTheta = cos(theta);
        auto sinPhi = sin(phi);
        auto cosPhi = cos(phi);
        return glm::vec3(-sinTheta * cosPhi, sinPhi, -cosTheta * cosPhi);
    }
    void updateUniforms()
    {
        gUniforms.screenWidth = (float)windowWidth;
        gUniforms.screenHeight = (float)windowHeight;
        if (!lastTime)
            lastTime = getCurrentTime();
        uint64_t currentTime = getCurrentTime();
        float deltaTime = float(double(currentTime - lastTime) / double(getTimerFrequency()));
        lastTime = currentTime;

        auto camDir =
            getVectorFromSphericalAngles(cameraOrientationAngles[0], cameraOrientationAngles[1]);
        auto camUp = getVectorFromSphericalAngles(
            cameraOrientationAngles[0],
            cameraOrientationAngles[1] + glm::pi<float>() * 0.5f);
        auto camRight = glm::cross(camDir, camUp);

        glm::vec3 movement = glm::vec3(0);
        if (wPressed)
            movement += camDir;
        if (sPressed)
            movement -= camDir;
        if (aPressed)
            movement -= camRight;
        if (dPressed)
            movement += camRight;

        cameraPosition += deltaTime * translationScale * movement;

        memcpy(gUniforms.cameraDir, &camDir, sizeof(float) * 3);
        memcpy(gUniforms.cameraUp, &camUp, sizeof(float) * 3);
        memcpy(gUniforms.cameraRight, &camRight, sizeof(float) * 3);
        memcpy(gUniforms.cameraPosition, &cameraPosition, sizeof(float) * 3);
        auto lightDir = glm::normalize(glm::vec3(1.0f, 3.0f, 2.0f));
        memcpy(gUniforms.lightDir, &lightDir, sizeof(float) * 3);
    }

    virtual void renderFrame(ITexture* texture) override
    {
        updateUniforms();
        {
            auto commandEncoder = gQueue->createCommandEncoder();
            auto computePassEncoder = commandEncoder->beginComputePass();
            auto rootObject = computePassEncoder->bindPipeline(gRenderPipeline);
            auto cursor = ShaderCursor(rootObject);
            cursor["resultTexture"].setBinding(gResultTexture);
            cursor["uniforms"].setData(&gUniforms, sizeof(Uniforms));
            cursor["sceneBVH"].setBinding(gTLAS);
            cursor["primitiveBuffer"].setBinding(gPrimitiveBuffer);
            computePassEncoder->dispatchCompute(
                (windowWidth + 15) / 16,
                (windowHeight + 15) / 16,
                1);
            computePassEncoder->end();
            gQueue->submit(commandEncoder->finish());
        }

        {
            auto commandEncoder = gQueue->createCommandEncoder();

            ComPtr<ITextureView> textureView = gDevice->createTextureView(texture, {});
            RenderPassColorAttachment colorAttachment = {};
            colorAttachment.view = textureView;
            colorAttachment.loadOp = LoadOp::Clear;

            RenderPassDesc renderPassDesc = {};
            renderPassDesc.colorAttachments = &colorAttachment;
            renderPassDesc.colorAttachmentCount = 1;

            auto renderPassEncoder = commandEncoder->beginRenderPass(renderPassDesc);

            RenderState renderState = {};
            renderState.viewports[0] = Viewport::fromSize(windowWidth, windowHeight);
            renderState.viewportCount = 1;
            renderState.scissorRects[0] = ScissorRect::fromSize(windowWidth, windowHeight);
            renderState.scissorRectCount = 1;
            renderState.vertexBuffers[0] = gFullScreenVertexBuffer;
            renderState.vertexBufferCount = 1;
            renderPassEncoder->setRenderState(renderState);

            auto rootObject = renderPassEncoder->bindPipeline(gPresentPipeline);
            auto cursor = ShaderCursor(rootObject);
            cursor["t"].setBinding(gResultTexture);

            DrawArguments drawArgs = {};
            drawArgs.vertexCount = 3;
            renderPassEncoder->draw(drawArgs);
            renderPassEncoder->end();
            gQueue->submit(commandEncoder->finish());
        }

        if (!isTestMode())
        {
            // With that, we are done drawing for one frame, and ready for the next.
            //
            gSurface->present();
        }
    }
};

// This macro instantiates an appropriate main function to
// run the application defined above.
EXAMPLE_MAIN(innerMain<RayTracing>);
