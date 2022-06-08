#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "gfx-test-texture-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "tools/platform/vector-math.h"
#include "source/core/slang-basic.h"

#include <chrono>

using namespace gfx;
using namespace Slang;

namespace gfx_test
{
    struct Vertex
    {
        float position[3];
    };

    static const int kVertexCount = 9;
    static const Vertex kVertexData[kVertexCount] =
    {
        // Triangle 1
        {  0,  0, 1 },
        {  4,  0, 1 },
        {  0,  4, 1 },

        // Triangle 2
        { -4,  0, 1 },
        {  0,  0, 1 },
        {  0,  4, 1 },

        // Triangle 3
        {  0,  0, 1 },
        {  4,  0, 1 },
        {  0, -4, 1 },
    };
    static const int kIndexCount = 9;
    static const uint32_t kIndexData[kIndexCount] =
    {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
    };

    struct BaseRayTracingTest
    {
        IDevice* device;
        UnitTestContext* context;

        ComPtr<IFramebufferLayout> framebufferLayout;
        ComPtr<ITransientResourceHeap> transientHeap;
        ComPtr<ICommandQueue> queue;

        ComPtr<IPipelineState> renderPipelineState;
        ComPtr<IBufferResource> vertexBuffer;
        ComPtr<IBufferResource> indexBuffer;
        ComPtr<IBufferResource> transformBuffer;
        ComPtr<IBufferResource> instanceBuffer;
        ComPtr<IBufferResource> BLASBuffer;
        ComPtr<IAccelerationStructure> BLAS;
        ComPtr<IBufferResource> TLASBuffer;
        ComPtr<IAccelerationStructure> TLAS;
        ComPtr<ITextureResource> resultTexture;
        ComPtr<IResourceView> resultTextureUAV;
        ComPtr<IShaderTable> shaderTable;

        uint32_t width = 2;
        uint32_t height = 2;

        void init(IDevice* device, UnitTestContext* context)
        {
            if (!device->hasFeature("ray-tracing"))
                SLANG_IGNORE_TEST;

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
            resultTexture = device->createTextureResource(resultTextureDesc);
            IResourceView::Desc resultUAVDesc = {};
            resultUAVDesc.format = resultTextureDesc.format;
            resultUAVDesc.type = IResourceView::Type::UnorderedAccess;
            resultTextureUAV = device->createTextureView(resultTexture, resultUAVDesc);
        }

        void createRequiredResources()
        {
            ICommandQueue::Desc queueDesc = {};
            queueDesc.type = ICommandQueue::QueueType::Graphics;
            queue = device->createCommandQueue(queueDesc);

            IBufferResource::Desc vertexBufferDesc;
            vertexBufferDesc.type = IResource::Type::Buffer;
            vertexBufferDesc.sizeInBytes = kVertexCount * sizeof(Vertex);
            vertexBufferDesc.defaultState = ResourceState::ShaderResource;
            vertexBuffer = device->createBufferResource(vertexBufferDesc, &kVertexData[0]);
            SLANG_CHECK_ABORT(vertexBuffer != nullptr);

            IBufferResource::Desc indexBufferDesc;
            indexBufferDesc.type = IResource::Type::Buffer;
            indexBufferDesc.sizeInBytes = kIndexCount * sizeof(int32_t);
            indexBufferDesc.defaultState = ResourceState::ShaderResource;
            indexBuffer = device->createBufferResource(indexBufferDesc, &kIndexData[0]);
            SLANG_CHECK_ABORT(indexBuffer != nullptr);

            IBufferResource::Desc transformBufferDesc;
            transformBufferDesc.type = IResource::Type::Buffer;
            transformBufferDesc.sizeInBytes = sizeof(float) * 12;
            transformBufferDesc.defaultState = ResourceState::ShaderResource;
            float transformData[12] = {
                1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
            transformBuffer = device->createBufferResource(transformBufferDesc, &transformData);
            SLANG_CHECK_ABORT(transformBuffer != nullptr);

            createResultTexture();

            IFramebufferLayout::TargetLayout renderTargetLayout = { Format::R8G8B8A8_UNORM, 1 };
            IFramebufferLayout::TargetLayout depthLayout = { gfx::Format::D32_FLOAT, 1 };
            IFramebufferLayout::Desc framebufferLayoutDesc;
            framebufferLayoutDesc.renderTargetCount = 1;
            framebufferLayoutDesc.renderTargets = &renderTargetLayout;
            framebufferLayoutDesc.depthStencil = &depthLayout;
            GFX_CHECK_CALL_ABORT(
                device->createFramebufferLayout(framebufferLayoutDesc, framebufferLayout.writeRef()));

            ITransientResourceHeap::Desc transientHeapDesc = {};
            transientHeapDesc.constantBufferSize = 4096 * 1024;
            GFX_CHECK_CALL_ABORT(
                device->createTransientResourceHeap(transientHeapDesc, transientHeap.writeRef()));

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
                geomDesc.content.triangles.indexData = indexBuffer->getDeviceAddress();
                geomDesc.content.triangles.indexFormat = Format::R32_UINT;
                geomDesc.content.triangles.vertexCount = kVertexCount;
                geomDesc.content.triangles.vertexData = vertexBuffer->getDeviceAddress();
                geomDesc.content.triangles.vertexFormat = Format::R32G32B32_FLOAT;
                geomDesc.content.triangles.vertexStride = sizeof(Vertex);
                geomDesc.content.triangles.transform3x4 = transformBuffer->getDeviceAddress();
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

                auto commandBuffer = transientHeap->createCommandBuffer();
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
                queue->executeCommandBuffer(commandBuffer);
                queue->waitOnHost();

                uint64_t compactedSize = 0;
                compactedSizeQuery->getResult(0, 1, &compactedSize);
                IBufferResource::Desc asBufferDesc;
                asBufferDesc.type = IResource::Type::Buffer;
                asBufferDesc.defaultState = ResourceState::AccelerationStructure;
                asBufferDesc.sizeInBytes = (size_t)compactedSize;
                BLASBuffer = device->createBufferResource(asBufferDesc);
                IAccelerationStructure::CreateDesc createDesc;
                createDesc.buffer = BLASBuffer;
                createDesc.kind = IAccelerationStructure::Kind::BottomLevel;
                createDesc.offset = 0;
                createDesc.size = (size_t)compactedSize;
                device->createAccelerationStructure(createDesc, BLAS.writeRef());

                commandBuffer = transientHeap->createCommandBuffer();
                encoder = commandBuffer->encodeRayTracingCommands();
                encoder->copyAccelerationStructure(BLAS, draftAS, AccelerationStructureCopyMode::Compact);
                encoder->endEncoding();
                commandBuffer->close();
                queue->executeCommandBuffer(commandBuffer);
                queue->waitOnHost();
            }

            // Build top level acceleration structure.
            {
                List<IAccelerationStructure::InstanceDesc> instanceDescs;
                instanceDescs.setCount(1);
                instanceDescs[0].accelerationStructure = BLAS->getDeviceAddress();
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
                instanceBuffer = device->createBufferResource(instanceBufferDesc, instanceDescs.getBuffer());
                SLANG_CHECK_ABORT(instanceBuffer != nullptr);

                IAccelerationStructure::BuildInputs accelerationStructureBuildInputs = {};
                IAccelerationStructure::PrebuildInfo accelerationStructurePrebuildInfo = {};
                accelerationStructureBuildInputs.descCount = 1;
                accelerationStructureBuildInputs.kind = IAccelerationStructure::Kind::TopLevel;
                accelerationStructureBuildInputs.instanceDescs = instanceBuffer->getDeviceAddress();

                // Query buffer size for acceleration structure build.
                GFX_CHECK_CALL_ABORT(device->getAccelerationStructurePrebuildInfo(
                    accelerationStructureBuildInputs, &accelerationStructurePrebuildInfo));

                IBufferResource::Desc asBufferDesc;
                asBufferDesc.type = IResource::Type::Buffer;
                asBufferDesc.defaultState = ResourceState::AccelerationStructure;
                asBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.resultDataMaxSize;
                TLASBuffer = device->createBufferResource(asBufferDesc);

                IBufferResource::Desc scratchBufferDesc;
                scratchBufferDesc.type = IResource::Type::Buffer;
                scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
                scratchBufferDesc.sizeInBytes = (size_t)accelerationStructurePrebuildInfo.scratchDataSize;
                ComPtr<IBufferResource> scratchBuffer = device->createBufferResource(scratchBufferDesc);

                IAccelerationStructure::CreateDesc createDesc;
                createDesc.buffer = TLASBuffer;
                createDesc.kind = IAccelerationStructure::Kind::TopLevel;
                createDesc.offset = 0;
                createDesc.size = (size_t)accelerationStructurePrebuildInfo.resultDataMaxSize;
                GFX_CHECK_CALL_ABORT(device->createAccelerationStructure(createDesc, TLAS.writeRef()));

                auto commandBuffer = transientHeap->createCommandBuffer();
                auto encoder = commandBuffer->encodeRayTracingCommands();
                IAccelerationStructure::BuildDesc buildDesc = {};
                buildDesc.dest = TLAS;
                buildDesc.inputs = accelerationStructureBuildInputs;
                buildDesc.scratchData = scratchBuffer->getDeviceAddress();
                encoder->buildAccelerationStructure(buildDesc, 0, nullptr);
                encoder->endEncoding();
                commandBuffer->close();
                queue->executeCommandBuffer(commandBuffer);
                queue->waitOnHost();
            }

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
                device->createRayTracingPipelineState(rtpDesc, renderPipelineState.writeRef()));
            SLANG_CHECK_ABORT(renderPipelineState != nullptr);

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
            GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));
        }

        void renderFrame()
        {
            ComPtr<ICommandBuffer> renderCommandBuffer =
                transientHeap->createCommandBuffer();
            auto renderEncoder = renderCommandBuffer->encodeRayTracingCommands();
            IShaderObject* rootObject = nullptr;
            renderEncoder->bindPipeline(renderPipelineState, &rootObject);
            auto cursor = ShaderCursor(rootObject);
            cursor["resultTexture"].setResource(resultTextureUAV);
            cursor["sceneBVH"].setResource(TLAS);
            renderEncoder->dispatchRays(0, shaderTable, width, height, 1);
            renderEncoder->endEncoding();
            renderCommandBuffer->close();
            queue->executeCommandBuffer(renderCommandBuffer);
            queue->waitOnHost();
        }

        void checkTestResults()
        {
            ComPtr<ISlangBlob> resultBlob;
            size_t rowPitch = 0;
            size_t pixelSize = 0;
            GFX_CHECK_CALL_ABORT(device->readTextureResource(
                resultTexture, ResourceState::CopySource, resultBlob.writeRef(), &rowPitch, &pixelSize));

            //writeImage("C:/Users/lucchen/Documents/test.hdr", resultBlob, width, height, rowPitch, pixelSize);

            auto buffer = removePadding(resultBlob, width, height, rowPitch, pixelSize);
            float expectedResult[16] = { 1, 1, 1, 1,
                                         0, 0, 1, 1,
                                         0, 1, 0, 1,
                                         1, 0, 0, 1 };
            auto actualData = (float*)buffer.getBuffer();
            SLANG_CHECK(memcmp(actualData, expectedResult, sizeof(expectedResult)) == 0)
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
