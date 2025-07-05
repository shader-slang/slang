#if 0
// Duplcated: This is ported to slang-rhi\tests\test-ray-tracing.cpp

#include "core/slang-basic.h"
#include "gfx-test-texture-util.h"
#include "gfx-test-util.h"
#include "platform/vector-math.h"
#include "unit-test/slang-unit-test.h"

#include <chrono>
#include <slang-rhi.h>
#include <slang-rhi/shader-cursor.h>

using namespace rhi;
using namespace Slang;

namespace gfx_test
{
struct Vertex
{
    float position[3];
};

static const int kVertexCount = 9;
static const Vertex kVertexData[kVertexCount] = {
    // Triangle 1
    {0, 0, 1},
    {4, 0, 1},
    {0, 4, 1},

    // Triangle 2
    {-4, 0, 1},
    {0, 0, 1},
    {0, 4, 1},

    // Triangle 3
    {0, 0, 1},
    {4, 0, 1},
    {0, -4, 1},
};
static const int kIndexCount = 9;
static const uint32_t kIndexData[kIndexCount] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
};

struct BaseRayTracingTest
{
    IDevice* device;
    UnitTestContext* context;

    ComPtr<ICommandQueue> queue;

    ComPtr<IRayTracingPipeline> renderPipelineState;
    ComPtr<IBuffer> vertexBuffer;
    ComPtr<IBuffer> indexBuffer;
    ComPtr<IBuffer> transformBuffer;
    ComPtr<IBuffer> instanceBuffer;
    ComPtr<IBuffer> BLASBuffer;
    ComPtr<IAccelerationStructure> BLAS;
    ComPtr<IBuffer> TLASBuffer;
    ComPtr<IAccelerationStructure> TLAS;
    ComPtr<ITexture> resultTexture;
    ComPtr<ITextureView> resultTextureUAV;
    ComPtr<IShaderTable> shaderTable;

    uint32_t width = 2;
    uint32_t height = 2;

    void init(IDevice* device, UnitTestContext* context)
    {
        if (!device->hasFeature("ray-tracing"))
        {
            SLANG_IGNORE_TEST;
        }

        this->device = device;
        this->context = context;
    }

    // Load and compile shader code from source.
    Result loadShaderProgram(IDevice* device, IShaderProgram** outProgram)
    {
        ComPtr<slang::ISession> slangSession;
        slangSession = device->getSlangSession();

        ComPtr<slang::IBlob> diagnosticsBlob;
        slang::IModule* module =
            slangSession->loadModule("ray-tracing-test-shaders", diagnosticsBlob.writeRef());
        if (!module)
            return SLANG_FAIL;

        Slang::List<slang::IComponentType*> componentTypes;
        componentTypes.add(module);
        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_RETURN_ON_FAIL(module->findEntryPointByName("rayGenShaderA", entryPoint.writeRef()));
        componentTypes.add(entryPoint);
        SLANG_RETURN_ON_FAIL(module->findEntryPointByName("rayGenShaderB", entryPoint.writeRef()));
        componentTypes.add(entryPoint);
        SLANG_RETURN_ON_FAIL(module->findEntryPointByName("missShaderA", entryPoint.writeRef()));
        componentTypes.add(entryPoint);
        SLANG_RETURN_ON_FAIL(module->findEntryPointByName("missShaderB", entryPoint.writeRef()));
        componentTypes.add(entryPoint);
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName("closestHitShaderA", entryPoint.writeRef()));
        componentTypes.add(entryPoint);
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName("closestHitShaderB", entryPoint.writeRef()));
        componentTypes.add(entryPoint);

        ComPtr<slang::IComponentType> linkedProgram;
        SlangResult result = slangSession->createCompositeComponentType(
            componentTypes.getBuffer(),
            componentTypes.getCount(),
            linkedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        SLANG_RETURN_ON_FAIL(result);

        ShaderProgramDesc programDesc = {};
        programDesc.slangGlobalScope = linkedProgram;
        SLANG_RETURN_ON_FAIL(device->createShaderProgram(programDesc, outProgram));

        return SLANG_OK;
    }

    void createResultTexture()
    {
        TextureDesc resultTextureDesc = {};
        resultTextureDesc.type = TextureType::Texture2D;
        resultTextureDesc.mipCount = 1;
        resultTextureDesc.size.width = width;
        resultTextureDesc.size.height = height;
        resultTextureDesc.size.depth = 1;
        resultTextureDesc.defaultState = ResourceState::UnorderedAccess;
        resultTextureDesc.format = Format::RGBA32Float;
        resultTextureDesc.usage = TextureUsage::UnorderedAccess | TextureUsage::CopySource;
        resultTexture = device->createTexture(resultTextureDesc);
        
        TextureViewDesc resultUAVDesc = {};
        resultUAVDesc.format = resultTextureDesc.format;
        resultTextureUAV = resultTexture->createView(resultUAVDesc);
    }

    void createRequiredResources()
    {
        GFX_CHECK_CALL_ABORT(device->getQueue(QueueType::Graphics, queue.writeRef()));

        BufferDesc vertexBufferDesc;
        vertexBufferDesc.size = kVertexCount * sizeof(Vertex);
        vertexBufferDesc.defaultState = ResourceState::ShaderResource;
        vertexBufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::AccelerationStructureBuildInput;
        vertexBuffer = device->createBuffer(vertexBufferDesc, &kVertexData[0]);
        SLANG_CHECK_ABORT(vertexBuffer != nullptr);

        BufferDesc indexBufferDesc;
        indexBufferDesc.size = kIndexCount * sizeof(int32_t);
        indexBufferDesc.defaultState = ResourceState::ShaderResource;
        indexBufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::AccelerationStructureBuildInput;
        indexBuffer = device->createBuffer(indexBufferDesc, &kIndexData[0]);
        SLANG_CHECK_ABORT(indexBuffer != nullptr);

        BufferDesc transformBufferDesc;
        transformBufferDesc.size = sizeof(float) * 12;
        transformBufferDesc.defaultState = ResourceState::ShaderResource;
        transformBufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::AccelerationStructureBuildInput;
        float transformData[12] =
            {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        transformBuffer = device->createBuffer(transformBufferDesc, &transformData);
        SLANG_CHECK_ABORT(transformBuffer != nullptr);

        createResultTexture();

        // Build bottom level acceleration structure.
        {
            AccelerationStructureBuildInput geomInput = {};
            geomInput.type = AccelerationStructureBuildInputType::Triangles;
            geomInput.triangles.flags = AccelerationStructureGeometryFlags::Opaque;
            geomInput.triangles.indexCount = kIndexCount;
            geomInput.triangles.indexBuffer = BufferOffsetPair(indexBuffer, 0);
            geomInput.triangles.indexFormat = IndexFormat::Uint32;
            geomInput.triangles.vertexCount = kVertexCount;
            geomInput.triangles.vertexBuffers[0] = BufferOffsetPair(vertexBuffer, 0);
            geomInput.triangles.vertexBufferCount = 1;
            geomInput.triangles.vertexFormat = Format::RGB32Float;
            geomInput.triangles.vertexStride = sizeof(Vertex);
            geomInput.triangles.preTransformBuffer = BufferOffsetPair(transformBuffer, 0);

            AccelerationStructureBuildDesc buildInputs = {};
            buildInputs.inputs = &geomInput;
            buildInputs.inputCount = 1;
            buildInputs.flags = AccelerationStructureBuildFlags::AllowCompaction;

            // Query buffer size for acceleration structure build.
            AccelerationStructureSizes sizes;
            GFX_CHECK_CALL_ABORT(device->getAccelerationStructureSizes(buildInputs, &sizes));
            
            // Allocate buffers for acceleration structure.
            BufferDesc asDraftBufferDesc;
            asDraftBufferDesc.defaultState = ResourceState::AccelerationStructure;
            asDraftBufferDesc.size = sizes.accelerationStructureSize;
            asDraftBufferDesc.usage = BufferUsage::AccelerationStructure;
            ComPtr<IBuffer> draftBuffer = device->createBuffer(asDraftBufferDesc);
            
            BufferDesc scratchBufferDesc;
            scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
            scratchBufferDesc.size = sizes.scratchSize;
            scratchBufferDesc.usage = BufferUsage::UnorderedAccess;
            ComPtr<IBuffer> scratchBuffer = device->createBuffer(scratchBufferDesc);

            // Build acceleration structure.
            ComPtr<IQueryPool> compactedSizeQuery;
            QueryPoolDesc queryPoolDesc;
            queryPoolDesc.count = 1;
            queryPoolDesc.type = QueryType::AccelerationStructureCompactedSize;
            GFX_CHECK_CALL_ABORT(
                device->createQueryPool(queryPoolDesc, compactedSizeQuery.writeRef()));

            ComPtr<IAccelerationStructure> draftAS;
            AccelerationStructureDesc draftCreateDesc;
            draftCreateDesc.size = sizes.accelerationStructureSize;
            GFX_CHECK_CALL_ABORT(
                device->createAccelerationStructure(draftCreateDesc, draftAS.writeRef()));

            compactedSizeQuery->reset();

            auto commandEncoder = queue->createCommandEncoder();
            AccelerationStructureQueryDesc compactedSizeQueryDesc = {};
            compactedSizeQueryDesc.queryPool = compactedSizeQuery;
            compactedSizeQueryDesc.queryType = QueryType::AccelerationStructureCompactedSize;
            commandEncoder->buildAccelerationStructure(buildInputs, draftAS, nullptr, BufferOffsetPair(scratchBuffer, 0), 1, &compactedSizeQueryDesc);
            auto commandBuffer = commandEncoder->finish();
            queue->submit(commandBuffer);
            queue->waitOnHost();

            uint64_t compactedSize = 0;
            compactedSizeQuery->getResult(0, 1, &compactedSize);
            
            BufferDesc asBufferDesc;
            asBufferDesc.defaultState = ResourceState::AccelerationStructure;
            asBufferDesc.size = (size_t)compactedSize;
            asBufferDesc.usage = BufferUsage::AccelerationStructure;
            BLASBuffer = device->createBuffer(asBufferDesc);
            
            AccelerationStructureDesc createDesc;
            createDesc.size = (size_t)compactedSize;
            device->createAccelerationStructure(createDesc, BLAS.writeRef());

            commandEncoder = queue->createCommandEncoder();
            commandEncoder->copyAccelerationStructure(
                BLAS,
                draftAS,
                AccelerationStructureCopyMode::Compact);
            commandBuffer = commandEncoder->finish();
            queue->submit(commandBuffer);
            queue->waitOnHost();
        }

        // Build top level acceleration structure.
        {
            List<AccelerationStructureInstanceDescGeneric> instanceDescs;
            instanceDescs.setCount(1);
            instanceDescs[0].accelerationStructure.value = BLAS->getDeviceAddress();
            instanceDescs[0].flags = AccelerationStructureInstanceFlags::TriangleFacingCullDisable;
            instanceDescs[0].instanceContributionToHitGroupIndex = 0;
            instanceDescs[0].instanceID = 0;
            instanceDescs[0].instanceMask = 0xFF;
            float transformMatrix[] =
                {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
            memcpy(&instanceDescs[0].transform[0][0], transformMatrix, sizeof(float) * 12);

            BufferDesc instanceBufferDesc;
            instanceBufferDesc.size = instanceDescs.getCount() * sizeof(AccelerationStructureInstanceDescGeneric);
            instanceBufferDesc.defaultState = ResourceState::ShaderResource;
            instanceBufferDesc.usage = BufferUsage::ShaderResource | BufferUsage::AccelerationStructureBuildInput;
            instanceBuffer = device->createBuffer(instanceBufferDesc, instanceDescs.getBuffer());
            SLANG_CHECK_ABORT(instanceBuffer != nullptr);

            AccelerationStructureBuildInput instanceInput = {};
            instanceInput.type = AccelerationStructureBuildInputType::Instances;
            instanceInput.instances.instanceBuffer = BufferOffsetPair(instanceBuffer, 0);
            instanceInput.instances.instanceStride = sizeof(AccelerationStructureInstanceDescGeneric);
            instanceInput.instances.instanceCount = instanceDescs.getCount();

            AccelerationStructureBuildDesc buildInputs = {};
            buildInputs.inputs = &instanceInput;
            buildInputs.inputCount = 1;

            // Query buffer size for acceleration structure build.
            AccelerationStructureSizes sizes;
            GFX_CHECK_CALL_ABORT(device->getAccelerationStructureSizes(buildInputs, &sizes));

            BufferDesc asBufferDesc;
            asBufferDesc.defaultState = ResourceState::AccelerationStructure;
            asBufferDesc.size = sizes.accelerationStructureSize;
            asBufferDesc.usage = BufferUsage::AccelerationStructure;
            TLASBuffer = device->createBuffer(asBufferDesc);

            BufferDesc scratchBufferDesc;
            scratchBufferDesc.defaultState = ResourceState::UnorderedAccess;
            scratchBufferDesc.size = sizes.scratchSize;
            scratchBufferDesc.usage = BufferUsage::UnorderedAccess;
            ComPtr<IBuffer> scratchBuffer = device->createBuffer(scratchBufferDesc);

            AccelerationStructureDesc createDesc;
            createDesc.size = sizes.accelerationStructureSize;
            GFX_CHECK_CALL_ABORT(device->createAccelerationStructure(createDesc, TLAS.writeRef()));

            auto commandEncoder = queue->createCommandEncoder();
            commandEncoder->buildAccelerationStructure(buildInputs, TLAS, nullptr, BufferOffsetPair(scratchBuffer, 0), 0, nullptr);
            auto commandBuffer = commandEncoder->finish();
            queue->submit(commandBuffer);
            queue->waitOnHost();
        }

        const char* hitgroupNames[] = {"hitgroupA", "hitgroupB"};

        ComPtr<IShaderProgram> rayTracingProgram;
        SLANG_CHECK_ABORT(loadShaderProgram(device, rayTracingProgram.writeRef()));
        RayTracingPipelineDesc rtpDesc = {};
        rtpDesc.program = rayTracingProgram;
        rtpDesc.hitGroupCount = 2;
        HitGroupDesc hitGroups[2];
        hitGroups[0].closestHitEntryPoint = "closestHitShaderA";
        hitGroups[0].hitGroupName = hitgroupNames[0];
        hitGroups[1].closestHitEntryPoint = "closestHitShaderB";
        hitGroups[1].hitGroupName = hitgroupNames[1];
        rtpDesc.hitGroups = hitGroups;
        rtpDesc.maxRayPayloadSize = 64;
        rtpDesc.maxRecursion = 2;
        GFX_CHECK_CALL_ABORT(
            device->createRayTracingPipeline(rtpDesc, renderPipelineState.writeRef()));
        SLANG_CHECK_ABORT(renderPipelineState != nullptr);

        const char* raygenNames[] = {"rayGenShaderA", "rayGenShaderB"};
        const char* missNames[] = {"missShaderA", "missShaderB"};

        ShaderTableDesc shaderTableDesc = {};
        shaderTableDesc.program = rayTracingProgram;
        shaderTableDesc.hitGroupCount = 2;
        shaderTableDesc.hitGroupNames = hitgroupNames;
        shaderTableDesc.rayGenShaderCount = 2;
        shaderTableDesc.rayGenShaderEntryPointNames = raygenNames;
        shaderTableDesc.missShaderCount = 2;
        shaderTableDesc.missShaderEntryPointNames = missNames;
        GFX_CHECK_CALL_ABORT(device->createShaderTable(shaderTableDesc, shaderTable.writeRef()));
    }

    void checkTestResults(float* expectedResult, uint32_t count)
    {
        ComPtr<ISlangBlob> resultBlob;
        auto commandEncoder = queue->createCommandEncoder();
        commandEncoder->setTextureState(resultTexture, ResourceState::CopySource);
        queue->submit(commandEncoder->finish());
        queue->waitOnHost();

        SubresourceLayout layout;
        GFX_CHECK_CALL_ABORT(device->readTexture(
            resultTexture,
            0, 0,
            resultBlob.writeRef(),
            &layout));
        size_t rowPitch = layout.rowPitch;
        size_t pixelSize = 4 ;

#if 0 // for debugging only
            writeImage("test.hdr", resultBlob, width, height, (uint32_t)rowPitch, (uint32_t)pixelSize);
#endif
        auto buffer = removePadding(resultBlob, width, height, rowPitch, pixelSize);
        auto actualData = (float*)buffer.data();
        SLANG_CHECK_ABORT(memcmp(actualData, expectedResult, count * sizeof(float)) == 0)
    }
};

struct RayTracingTestA : BaseRayTracingTest
{
    void renderFrame()
    {
        auto commandEncoder = queue->createCommandEncoder();
        auto renderEncoder = commandEncoder->beginRayTracingPass();
        auto rootObject = renderEncoder->bindPipeline(renderPipelineState, shaderTable);
        auto cursor = ShaderCursor(rootObject);
        cursor["resultTexture"].setBinding(Binding(resultTextureUAV));
        cursor["sceneBVH"].setBinding(Binding(TLAS));
        renderEncoder->dispatchRays(0, width, height, 1);
        renderEncoder->end();
        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    void run()
    {
        createRequiredResources();
        renderFrame();

        float expectedResult[16] = {1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1};
        checkTestResults(expectedResult, 16);
    }
};

struct RayTracingTestB : BaseRayTracingTest
{
    void renderFrame()
    {
        auto commandEncoder = queue->createCommandEncoder();
        auto renderEncoder = commandEncoder->beginRayTracingPass();
        auto rootObject = renderEncoder->bindPipeline(renderPipelineState, shaderTable);
        auto cursor = ShaderCursor(rootObject);
        cursor["resultTexture"].setBinding(Binding(resultTextureUAV));
        cursor["sceneBVH"].setBinding(Binding(TLAS));
        renderEncoder->dispatchRays(1, width, height, 1);
        renderEncoder->end();
        auto commandBuffer = commandEncoder->finish();
        queue->submit(commandBuffer);
        queue->waitOnHost();
    }

    void run()
    {
        createRequiredResources();
        renderFrame();

        float expectedResult[16] = {0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1};
        checkTestResults(expectedResult, 16);
    }
};

template<typename T>
void rayTracingTestImpl(IDevice* device, UnitTestContext* context)
{
    T test;
    test.init(device, context);
    test.run();
}

SLANG_UNIT_TEST(RayTracingTestAD3D12)
{
    runTestImpl(rayTracingTestImpl<RayTracingTestA>, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(RayTracingTestAVulkan)
{
    runTestImpl(rayTracingTestImpl<RayTracingTestA>, unitTestContext, DeviceType::Vulkan);
}

SLANG_UNIT_TEST(RayTracingTestBD3D12)
{
    runTestImpl(rayTracingTestImpl<RayTracingTestB>, unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(RayTracingTestBVulkan)
{
    runTestImpl(rayTracingTestImpl<RayTracingTestB>, unitTestContext, DeviceType::Vulkan);
}
} // namespace gfx_test
#endif