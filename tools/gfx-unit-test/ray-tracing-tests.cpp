#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "tools/platform/vector-math.h"
#include "source/core/slang-basic.h"

#include <chrono>

#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"

using namespace gfx;
using namespace Slang;

namespace gfx_test
{
    Slang::Result writeImage(
        const char* filename,
        ISlangBlob* pixels,
        uint32_t width,
        uint32_t height)
    {
        int stbResult =
            stbi_write_hdr(filename, width, height, 4, (float*)pixels->getBufferPointer());

        return stbResult ? SLANG_OK : SLANG_FAIL;
    }

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
    static const Vertex kVertexData[kVertexCount] =
    {
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
    static const int kIndexData[kIndexCount] =
    {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };

    struct Primitive
    {
        float data[4];
        float color[4];
    };
    static const int kPrimitiveCount = 12;
    static const Primitive kPrimitiveData[kPrimitiveCount] =
    {
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

    struct BaseRayTracingTest
    {
        IDevice* device;
        UnitTestContext* context;

        Uniforms gUniforms = {};

        ComPtr<IFramebufferLayout> gFramebufferLayout;
        ComPtr<ITransientResourceHeap> gTransientHeap;
        ComPtr<ICommandQueue> gQueue;

        ComPtr<IPipelineState> gPresentPipelineState;
        ComPtr<IPipelineState> gRenderPipelineState;
        ComPtr<IBufferResource> gFullScreenVertexBuffer;
        ComPtr<IBufferResource> gVertexBuffer;
        ComPtr<IBufferResource> gIndexBuffer;
        ComPtr<IBufferResource> gTransformBuffer;
        ComPtr<IBufferResource> gInstanceBuffer;
        ComPtr<IBufferResource> gBLASBuffer;
        ComPtr<IAccelerationStructure> gBLAS;
        ComPtr<IBufferResource> gTLASBuffer;
        ComPtr<IAccelerationStructure> gTLAS;
        ComPtr<ITextureResource> gResultTexture;
        ComPtr<IResourceView> gResultTextureUAV;
        ComPtr<IShaderTable> gShaderTable;

        uint32_t width = 2;
        uint32_t height = 2;

        void init(IDevice* device, UnitTestContext* context)
        {
            this->device = device;
            this->context = context;
        }

        // Load and compile shader code from source.
        gfx::Result loadShaderProgram(gfx::IDevice* device, gfx::IShaderProgram** outProgram)
        {
            ComPtr<slang::ISession> slangSession;
            slangSession = device->getSlangSession();

            ComPtr<slang::IBlob> diagnosticsBlob;
            slang::IModule* module = slangSession->loadModule("ray-tracing-test-shader", diagnosticsBlob.writeRef());
            //diagnoseIfNeeded(diagnosticsBlob);
            if (!module)
                return SLANG_FAIL;

            Slang::List<slang::IComponentType*> componentTypes;
            componentTypes.add(module);
            ComPtr<slang::IEntryPoint> entryPoint;
            SLANG_RETURN_ON_FAIL(module->findEntryPointByName("rayGenShader", entryPoint.writeRef()));
            componentTypes.add(entryPoint);
            SLANG_RETURN_ON_FAIL(module->findEntryPointByName("missShader", entryPoint.writeRef()));
            componentTypes.add(entryPoint);
            SLANG_RETURN_ON_FAIL(
                module->findEntryPointByName("closestHitShader", entryPoint.writeRef()));
            componentTypes.add(entryPoint);

            ComPtr<slang::IComponentType> linkedProgram;
            SlangResult result = slangSession->createCompositeComponentType(
                componentTypes.getBuffer(),
                componentTypes.getCount(),
                linkedProgram.writeRef(),
                diagnosticsBlob.writeRef());
            //diagnoseIfNeeded(diagnosticsBlob);
            SLANG_RETURN_ON_FAIL(result);

            gfx::IShaderProgram::Desc programDesc = {};
            programDesc.slangGlobalScope = linkedProgram;
            SLANG_RETURN_ON_FAIL(device->createProgram(programDesc, outProgram));

            return SLANG_OK;
        }

        void createResultTexture()
        {
            ITextureResource::Desc resultTextureDesc = {};
            resultTextureDesc.type = IResource::Type::Texture2D;
            resultTextureDesc.numMipLevels = 1;
            resultTextureDesc.size.width = width;
            resultTextureDesc.size.height = height;
            resultTextureDesc.size.depth = 1;
            resultTextureDesc.defaultState = ResourceState::UnorderedAccess;
            resultTextureDesc.format = Format::R32G32B32A32_FLOAT;
            gResultTexture = device->createTextureResource(resultTextureDesc);
            IResourceView::Desc resultUAVDesc = {};
            resultUAVDesc.format = resultTextureDesc.format;
            resultUAVDesc.type = IResourceView::Type::UnorderedAccess;
            gResultTextureUAV = device->createTextureView(gResultTexture, resultUAVDesc);
        }

        void createRequiredResources()
        {
            ICommandQueue::Desc queueDesc = {};
            queueDesc.type = ICommandQueue::QueueType::Graphics;
            gQueue = device->createCommandQueue(queueDesc);

            IBufferResource::Desc vertexBufferDesc;
            vertexBufferDesc.type = IResource::Type::Buffer;
            vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
            vertexBufferDesc.defaultState = ResourceState::ShaderResource;
            gVertexBuffer = device->createBufferResource(vertexBufferDesc, &kVertexData[0]);
            SLANG_CHECK_ABORT(gVertexBuffer != nullptr);

            IBufferResource::Desc indexBufferDesc;
            indexBufferDesc.type = IResource::Type::Buffer;
            indexBufferDesc.sizeInBytes = kIndexCount * sizeof(int32_t);
            indexBufferDesc.defaultState = ResourceState::ShaderResource;
            gIndexBuffer = device->createBufferResource(indexBufferDesc, &kIndexData[0]);
            SLANG_CHECK_ABORT(gIndexBuffer != nullptr);

            IBufferResource::Desc transformBufferDesc;
            transformBufferDesc.type = IResource::Type::Buffer;
            transformBufferDesc.sizeInBytes = sizeof(float) * 12;
            transformBufferDesc.defaultState = ResourceState::ShaderResource;
            float transformData[12] = {
                1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
            gTransformBuffer = device->createBufferResource(transformBufferDesc, &transformData);
            SLANG_CHECK_ABORT(gTransformBuffer != nullptr);

            createResultTexture();

            IFramebufferLayout::TargetLayout renderTargetLayout = { Format::R8G8B8A8_UNORM, 1 };
            IFramebufferLayout::TargetLayout depthLayout = { gfx::Format::D32_FLOAT, 1 };
            IFramebufferLayout::Desc framebufferLayoutDesc;
            framebufferLayoutDesc.renderTargetCount = 1;
            framebufferLayoutDesc.renderTargets = &renderTargetLayout;
            framebufferLayoutDesc.depthStencil = &depthLayout;
            GFX_CHECK_CALL_ABORT(
                device->createFramebufferLayout(framebufferLayoutDesc, gFramebufferLayout.writeRef()));

            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096 * 1024;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, gTransientHeap.writeRef()));

            // Build bottom level acceleration structure.
            {
                IAccelerationStructure::BuildInputs accelerationStructureBuildInputs;
                IAccelerationStructure::PrebuildInfo accelerationStructurePrebuildInfo;
                accelerationStructureBuildInputs.descCount = 1;
                accelerationStructureBuildInputs.kind = IAccelerationStructure::Kind::BottomLevel;
                accelerationStructureBuildInputs.flags =
                    IAccelerationStructure::BuildFlags::AllowCompaction;
                IAccelerationStructure::GeometryDesc geomDesc;
                geomDesc.flags = IAccelerationStructure::GeometryFlags::Opaque;
                geomDesc.type = IAccelerationStructure::GeometryType::Triangles;
                geomDesc.content.triangles.indexCount = kIndexCount;
                geomDesc.content.triangles.indexData = gIndexBuffer->getDeviceAddress();
                geomDesc.content.triangles.indexFormat = Format::R32_UINT;
                geomDesc.content.triangles.vertexCount = kVertexCount;
                geomDesc.content.triangles.vertexData = gVertexBuffer->getDeviceAddress();
                geomDesc.content.triangles.vertexFormat = Format::R32G32B32_FLOAT;
                geomDesc.content.triangles.vertexStride = sizeof(Vertex);
                geomDesc.content.triangles.transform3x4 = gTransformBuffer->getDeviceAddress();
                accelerationStructureBuildInputs.geometryDescs = &geomDesc;

                // Query buffer size for acceleration structure build.
                GFX_CHECK_CALL_ABORT(device->getAccelerationStructurePrebuildInfo(
                    accelerationStructureBuildInputs, &accelerationStructurePrebuildInfo));
                // Allocate buffers for acceleration structure.
                IBufferResource::Desc asDraftBufferDesc;
                asDraftBufferDesc.type = IResource::Type::Buffer;
                asDraftBufferDesc.defaultState = ResourceState::AccelerationStructure;
                asDraftBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.resultDataMaxSize;
                ComPtr<IBufferResource> draftBuffer = device->createBufferResource(asDraftBufferDesc);
                IBufferResource::Desc scratchBufferDesc;
                scratchBufferDesc.type = IResource::Type::Buffer;
                scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
                scratchBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.scratchDataSize;
                ComPtr<IBufferResource> scratchBuffer = device->createBufferResource(scratchBufferDesc);

                // Build acceleration structure.
                ComPtr<IQueryPool> compactedSizeQuery;
                IQueryPool::Desc queryPoolDesc;
                queryPoolDesc.count = 1;
                queryPoolDesc.type = QueryType::AccelerationStructureCompactedSize;
                GFX_CHECK_CALL_ABORT(
                    device->createQueryPool(queryPoolDesc, compactedSizeQuery.writeRef()));

                ComPtr<IAccelerationStructure> draftAS;
                IAccelerationStructure::CreateDesc draftCreateDesc;
                draftCreateDesc.buffer = draftBuffer;
                draftCreateDesc.kind = IAccelerationStructure::Kind::BottomLevel;
                draftCreateDesc.offset = 0;
                draftCreateDesc.size = accelerationStructurePrebuildInfo.resultDataMaxSize;
                GFX_CHECK_CALL_ABORT(
                    device->createAccelerationStructure(draftCreateDesc, draftAS.writeRef()));

                compactedSizeQuery->reset();

                auto commandBuffer = gTransientHeap->createCommandBuffer();
                auto encoder = commandBuffer->encodeRayTracingCommands();
                IAccelerationStructure::BuildDesc buildDesc = {};
                buildDesc.dest = draftAS;
                buildDesc.inputs = accelerationStructureBuildInputs;
                buildDesc.scratchData = scratchBuffer->getDeviceAddress();
                AccelerationStructureQueryDesc compactedSizeQueryDesc = {};
                compactedSizeQueryDesc.queryPool = compactedSizeQuery;
                compactedSizeQueryDesc.queryType = QueryType::AccelerationStructureCompactedSize;
                encoder->buildAccelerationStructure(buildDesc, 1, &compactedSizeQueryDesc);
                encoder->endEncoding();
                commandBuffer->close();
                gQueue->executeCommandBuffer(commandBuffer);
                gQueue->waitOnHost();

                uint64_t compactedSize = 0;
                compactedSizeQuery->getResult(0, 1, &compactedSize);
                IBufferResource::Desc asBufferDesc;
                asBufferDesc.type = IResource::Type::Buffer;
                asBufferDesc.defaultState = ResourceState::AccelerationStructure;
                asBufferDesc.sizeInBytes = (size_t)compactedSize;
                gBLASBuffer = device->createBufferResource(asBufferDesc);
                IAccelerationStructure::CreateDesc createDesc;
                createDesc.buffer = gBLASBuffer;
                createDesc.kind = IAccelerationStructure::Kind::BottomLevel;
                createDesc.offset = 0;
                createDesc.size = (size_t)compactedSize;
                device->createAccelerationStructure(createDesc, gBLAS.writeRef());

                commandBuffer = gTransientHeap->createCommandBuffer();
                encoder = commandBuffer->encodeRayTracingCommands();
                encoder->copyAccelerationStructure(gBLAS, draftAS, AccelerationStructureCopyMode::Compact);
                encoder->endEncoding();
                commandBuffer->close();
                gQueue->executeCommandBuffer(commandBuffer);
                gQueue->waitOnHost();
            }

            // Build top level acceleration structure.
            {
                List<IAccelerationStructure::InstanceDesc> instanceDescs;
                instanceDescs.setCount(1);
                instanceDescs[0].accelerationStructure = gBLAS->getDeviceAddress();
                instanceDescs[0].flags =
                    IAccelerationStructure::GeometryInstanceFlags::TriangleFacingCullDisable;
                instanceDescs[0].instanceContributionToHitGroupIndex = 0;
                instanceDescs[0].instanceID = 0;
                instanceDescs[0].instanceMask = 0xFF;
                float transformMatrix[] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
                memcpy(&instanceDescs[0].transform[0][0], transformMatrix, sizeof(float) * 12);

                IBufferResource::Desc instanceBufferDesc;
                instanceBufferDesc.type = IResource::Type::Buffer;
                instanceBufferDesc.sizeInBytes =
                    instanceDescs.getCount() * sizeof(IAccelerationStructure::InstanceDesc);
                instanceBufferDesc.defaultState = ResourceState::ShaderResource;
                gInstanceBuffer = device->createBufferResource(instanceBufferDesc, instanceDescs.getBuffer());
                SLANG_CHECK_ABORT(gInstanceBuffer != nullptr);

                IAccelerationStructure::BuildInputs accelerationStructureBuildInputs = {};
                IAccelerationStructure::PrebuildInfo accelerationStructurePrebuildInfo = {};
                accelerationStructureBuildInputs.descCount = 1;
                accelerationStructureBuildInputs.kind = IAccelerationStructure::Kind::TopLevel;
                accelerationStructureBuildInputs.instanceDescs = gInstanceBuffer->getDeviceAddress();

                // Query buffer size for acceleration structure build.
                GFX_CHECK_CALL_ABORT(device->getAccelerationStructurePrebuildInfo(
                    accelerationStructureBuildInputs, &accelerationStructurePrebuildInfo));

                IBufferResource::Desc asBufferDesc;
                asBufferDesc.type = IResource::Type::Buffer;
                asBufferDesc.defaultState = ResourceState::AccelerationStructure;
                asBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.resultDataMaxSize;
                gTLASBuffer = device->createBufferResource(asBufferDesc);

                IBufferResource::Desc scratchBufferDesc;
                scratchBufferDesc.type = IResource::Type::Buffer;
                scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
                scratchBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.scratchDataSize;
                ComPtr<IBufferResource> scratchBuffer = device->createBufferResource(scratchBufferDesc);

                IAccelerationStructure::CreateDesc createDesc;
                createDesc.buffer = gTLASBuffer;
                createDesc.kind = IAccelerationStructure::Kind::TopLevel;
                createDesc.offset = 0;
                createDesc.size = (size_t)accelerationStructurePrebuildInfo.resultDataMaxSize;
                GFX_CHECK_CALL_ABORT(device->createAccelerationStructure(createDesc, gTLAS.writeRef()));

                auto commandBuffer = gTransientHeap->createCommandBuffer();
                auto encoder = commandBuffer->encodeRayTracingCommands();
                IAccelerationStructure::BuildDesc buildDesc = {};
                buildDesc.dest = gTLAS;
                buildDesc.inputs = accelerationStructureBuildInputs;
                buildDesc.scratchData = scratchBuffer->getDeviceAddress();
                encoder->buildAccelerationStructure(buildDesc, 0, nullptr);
                encoder->endEncoding();
                commandBuffer->close();
                gQueue->executeCommandBuffer(commandBuffer);
                gQueue->waitOnHost();
            }

            IBufferResource::Desc fullScreenVertexBufferDesc;
            fullScreenVertexBufferDesc.type = IResource::Type::Buffer;
            fullScreenVertexBufferDesc.sizeInBytes =
                FullScreenTriangle::kVertexCount * sizeof(FullScreenTriangle::Vertex);
            fullScreenVertexBufferDesc.defaultState = ResourceState::VertexBuffer;
            gFullScreenVertexBuffer = device->createBufferResource(
                fullScreenVertexBufferDesc, &FullScreenTriangle::kVertices[0]);
            SLANG_CHECK_ABORT(gFullScreenVertexBuffer != nullptr);

            InputElementDesc inputElements[] = {
                {"POSITION", 0, Format::R32G32_FLOAT, offsetof(FullScreenTriangle::Vertex, position)},
            };
            auto inputLayout = device->createInputLayout(sizeof(FullScreenTriangle::Vertex), &inputElements[0], SLANG_COUNT_OF(inputElements));
            SLANG_CHECK_ABORT(inputLayout != nullptr);

            ComPtr<IShaderProgram> shaderProgram;
            GFX_CHECK_CALL_ABORT(loadShaderProgram(device, shaderProgram.writeRef()));
            GraphicsPipelineStateDesc desc;
            desc.inputLayout = inputLayout;
            desc.program = shaderProgram;
            desc.framebufferLayout = gFramebufferLayout;
            gPresentPipelineState = device->createGraphicsPipelineState(desc);
            SLANG_CHECK_ABORT(gPresentPipelineState != nullptr);

            const char* hitgroupNames[] = { "hitgroup" };

            ComPtr<IShaderProgram> rayTracingProgram;
            GFX_CHECK_CALL_ABORT(
                loadShaderProgram(device, rayTracingProgram.writeRef()));
            RayTracingPipelineStateDesc rtpDesc = {};
            rtpDesc.program = rayTracingProgram;
            rtpDesc.hitGroupCount = 1;
            HitGroupDesc hitGroups[1];
            hitGroups[0].closestHitEntryPoint = "closestHitShader";
            hitGroups[0].hitGroupName = hitgroupNames[0];
            rtpDesc.hitGroups = hitGroups;
            rtpDesc.maxRayPayloadSize = 64;
            rtpDesc.maxRecursion = 2;
            GFX_CHECK_CALL_ABORT(
                device->createRayTracingPipelineState(rtpDesc, gRenderPipelineState.writeRef()));
            SLANG_CHECK_ABORT(gRenderPipelineState != nullptr);

            IShaderTable::Desc shaderTableDesc = {};
            const char* raygenName = "rayGenShader";
            const char* missName = "missShader";
            shaderTableDesc.program = rayTracingProgram;
            shaderTableDesc.hitGroupCount = 1;
            shaderTableDesc.hitGroupNames = hitgroupNames;
            shaderTableDesc.rayGenShaderCount = 1;
            shaderTableDesc.rayGenShaderEntryPointNames = &raygenName;
            shaderTableDesc.missShaderCount = 1;
            shaderTableDesc.missShaderEntryPointNames = &missName;
            GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, gShaderTable.writeRef()));
        }

        void renderFrame()
        {
            {
                ComPtr<ICommandBuffer> renderCommandBuffer =
                    gTransientHeap->createCommandBuffer();
                auto renderEncoder = renderCommandBuffer->encodeRayTracingCommands();
                IShaderObject* rootObject = nullptr;
                renderEncoder->bindPipeline(gRenderPipelineState, &rootObject);
                auto cursor = ShaderCursor(rootObject);
                cursor["resultTexture"].setResource(gResultTextureUAV);
                cursor["sceneBVH"].setResource(gTLAS);
                renderEncoder->dispatchRays(0, gShaderTable, width, height, 1);
                renderEncoder->endEncoding();
                renderCommandBuffer->close();
                gQueue->executeCommandBuffer(renderCommandBuffer);
                gQueue->waitOnHost();
            }
        }

        void checkTestResults()
        {
            ComPtr<ISlangBlob> resultBlob;
            size_t rowPitch = 0;
            size_t pixelSize = 0;
            GFX_CHECK_CALL_ABORT(device->readTextureResource(
                gResultTexture, ResourceState::CopySource, resultBlob.writeRef(), &rowPitch, &pixelSize));

            writeImage("C:/Users/lucchen/Documents/test.hdr", resultBlob, width, height);
        }

        void run()
        {
            createRequiredResources();
            renderFrame();
            checkTestResults();
        }
    };

    void simpleRayTracingTestImpl(IDevice* device, UnitTestContext* context)
    {
        BaseRayTracingTest test;
        test.init(device, context);
        test.run();
    }

    SLANG_UNIT_TEST(simpleRayTracingD3D12)
    {
        runTestImpl(simpleRayTracingTestImpl, unitTestContext, Slang::RenderApiFlag::D3D12);
    }

    SLANG_UNIT_TEST(simpleRayTracingVulkan)
    {
        runTestImpl(simpleRayTracingTestImpl, unitTestContext, Slang::RenderApiFlag::Vulkan);
    }

}
