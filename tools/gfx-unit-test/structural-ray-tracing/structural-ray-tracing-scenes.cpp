#include "structural-ray-tracing-scenes.h"

#include "gfx-unit-test/gfx-test-util.h"

#include <cstring>
#include <slang-rhi/acceleration-structure-utils.h>
#include <vector>

using namespace rhi;

namespace gfx_test
{

namespace
{

struct Vertex
{
    float position[3];
};

static const Vertex kTriangleVertices[] = {
    {{0.0f, 0.0f, 1.0f}},
    {{1.0f, 0.0f, 1.0f}},
    {{0.0f, 1.0f, 1.0f}},
};

static const uint32_t kTriangleIndices[] = {0, 1, 2};

// Each array element becomes a separate BLAS geometry, not merely another primitive in the same
// geometry. Native SBT addressing therefore observes geometry indices zero and one.
static const Vertex kTwoGeometryTriangleVertices[][3] = {
    {
        {{0.0f, 0.0f, 1.0f}},
        {{1.0f, 0.0f, 1.0f}},
        {{0.0f, 1.0f, 1.0f}},
    },
    {
        {{2.0f, 0.0f, 1.0f}},
        {{3.0f, 0.0f, 1.0f}},
        {{2.0f, 1.0f, 1.0f}},
    },
};

static const AccelerationStructureAABB kProceduralAabb = {
    -0.5f,
    -0.5f,
    0.5f,
    0.5f,
    0.5f,
    2.0f,
};

ComPtr<IBuffer> createScratchBuffer(IDevice* device, Size size)
{
    BufferDesc desc = {};
    desc.size = size;
    desc.usage = BufferUsage::UnorderedAccess;
    desc.defaultState = ResourceState::UnorderedAccess;
    return device->createBuffer(desc);
}

void createSingleInstanceTopLevel(
    IDevice* device,
    ICommandQueue* queue,
    IAccelerationStructure* bottomLevel,
    AccelerationStructureInstanceFlags flags,
    uint32_t instanceID,
    uint32_t instanceContributionToHitGroupIndex,
    const float* transform,
    ComPtr<IBuffer>& outInstanceBuffer,
    ComPtr<IAccelerationStructure>& outTopLevel)
{
    AccelerationStructureInstanceDescGeneric genericInstance = {};
    static const float kIdentityTransform[12] = {
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
    };
    memcpy(
        genericInstance.transform,
        transform ? transform : kIdentityTransform,
        sizeof(kIdentityTransform));
    genericInstance.instanceID = instanceID;
    genericInstance.instanceMask = 0xff;
    genericInstance.instanceContributionToHitGroupIndex = instanceContributionToHitGroupIndex;
    genericInstance.flags = flags;
    genericInstance.accelerationStructure = bottomLevel->getHandle();

    auto instanceType = getAccelerationStructureInstanceDescType(device);
    auto instanceStride = getAccelerationStructureInstanceDescSize(instanceType);
    std::vector<uint8_t> nativeInstance(instanceStride);
    convertAccelerationStructureInstanceDescs(
        1,
        instanceType,
        nativeInstance.data(),
        instanceStride,
        &genericInstance,
        sizeof(genericInstance));

    BufferDesc instanceDesc = {};
    instanceDesc.size = nativeInstance.size();
    instanceDesc.usage = BufferUsage::ShaderResource | BufferUsage::AccelerationStructureBuildInput;
    instanceDesc.defaultState = ResourceState::ShaderResource;
    outInstanceBuffer = device->createBuffer(instanceDesc, nativeInstance.data());
    SLANG_CHECK_ABORT(outInstanceBuffer != nullptr);

    AccelerationStructureBuildInput instanceInput = {};
    instanceInput.type = AccelerationStructureBuildInputType::Instances;
    instanceInput.instances.instanceBuffer = outInstanceBuffer;
    instanceInput.instances.instanceStride = uint32_t(instanceStride);
    instanceInput.instances.instanceCount = 1;

    AccelerationStructureBuildDesc topBuild = {};
    topBuild.inputs = &instanceInput;
    topBuild.inputCount = 1;

    AccelerationStructureSizes topSizes = {};
    GFX_CHECK_CALL_ABORT(device->getAccelerationStructureSizes(topBuild, &topSizes));
    auto topScratch = createScratchBuffer(device, topSizes.scratchSize);
    SLANG_CHECK_ABORT(topScratch != nullptr);

    AccelerationStructureDesc topDesc = {};
    topDesc.kind = AccelerationStructureKind::TopLevel;
    topDesc.size = topSizes.accelerationStructureSize;
    GFX_CHECK_CALL_ABORT(device->createAccelerationStructure(topDesc, outTopLevel.writeRef()));

    auto commandEncoder = queue->createCommandEncoder();
    commandEncoder
        ->buildAccelerationStructure(topBuild, outTopLevel, nullptr, topScratch, 0, nullptr);
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());
}

} // namespace

StructuralRayTracingTriangleScene::StructuralRayTracingTriangleScene(
    IDevice* device,
    ICommandQueue* queue,
    AccelerationStructureInstanceFlags instanceFlags,
    uint32_t instanceID,
    const float* transform)
{
    BufferDesc vertexDesc = {};
    vertexDesc.size = sizeof(kTriangleVertices);
    vertexDesc.usage = BufferUsage::AccelerationStructureBuildInput;
    vertexDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
    vertexBuffer = device->createBuffer(vertexDesc, kTriangleVertices);
    SLANG_CHECK_ABORT(vertexBuffer != nullptr);

    BufferDesc indexDesc = {};
    indexDesc.size = sizeof(kTriangleIndices);
    indexDesc.usage = BufferUsage::AccelerationStructureBuildInput;
    indexDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
    indexBuffer = device->createBuffer(indexDesc, kTriangleIndices);
    SLANG_CHECK_ABORT(indexBuffer != nullptr);

    AccelerationStructureBuildInput triangleInput = {};
    triangleInput.type = AccelerationStructureBuildInputType::Triangles;
    triangleInput.triangles.vertexBuffers[0] = vertexBuffer;
    triangleInput.triangles.vertexBufferCount = 1;
    triangleInput.triangles.vertexFormat = Format::RGB32Float;
    triangleInput.triangles.vertexCount = SLANG_COUNT_OF(kTriangleVertices);
    triangleInput.triangles.vertexStride = sizeof(Vertex);
    triangleInput.triangles.indexBuffer = indexBuffer;
    triangleInput.triangles.indexFormat = IndexFormat::Uint32;
    triangleInput.triangles.indexCount = SLANG_COUNT_OF(kTriangleIndices);
    triangleInput.triangles.flags = AccelerationStructureGeometryFlags::Opaque;

    AccelerationStructureBuildDesc bottomBuild = {};
    bottomBuild.inputs = &triangleInput;
    bottomBuild.inputCount = 1;

    AccelerationStructureSizes bottomSizes = {};
    GFX_CHECK_CALL_ABORT(device->getAccelerationStructureSizes(bottomBuild, &bottomSizes));
    auto bottomScratch = createScratchBuffer(device, bottomSizes.scratchSize);
    SLANG_CHECK_ABORT(bottomScratch != nullptr);

    AccelerationStructureDesc bottomDesc = {};
    bottomDesc.kind = AccelerationStructureKind::BottomLevel;
    bottomDesc.size = bottomSizes.accelerationStructureSize;
    GFX_CHECK_CALL_ABORT(device->createAccelerationStructure(bottomDesc, bottomLevel.writeRef()));

    auto commandEncoder = queue->createCommandEncoder();
    commandEncoder
        ->buildAccelerationStructure(bottomBuild, bottomLevel, nullptr, bottomScratch, 0, nullptr);
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    createSingleInstanceTopLevel(
        device,
        queue,
        bottomLevel,
        instanceFlags,
        instanceID,
        0,
        transform,
        instanceBuffer,
        topLevel);
}

StructuralRayTracingTwoGeometryTriangleScene::StructuralRayTracingTwoGeometryTriangleScene(
    IDevice* device,
    ICommandQueue* queue,
    uint32_t instanceContributionToHitGroupIndex)
{
    BufferDesc indexDesc = {};
    indexDesc.size = sizeof(kTriangleIndices);
    indexDesc.usage = BufferUsage::AccelerationStructureBuildInput;
    indexDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
    indexBuffer = device->createBuffer(indexDesc, kTriangleIndices);
    SLANG_CHECK_ABORT(indexBuffer != nullptr);

    AccelerationStructureBuildInput triangleInputs[2] = {};
    for (uint32_t geometryIndex = 0; geometryIndex < SLANG_COUNT_OF(triangleInputs);
         ++geometryIndex)
    {
        BufferDesc vertexDesc = {};
        vertexDesc.size = sizeof(kTwoGeometryTriangleVertices[geometryIndex]);
        vertexDesc.usage = BufferUsage::AccelerationStructureBuildInput;
        vertexDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
        vertexBuffers[geometryIndex] =
            device->createBuffer(vertexDesc, kTwoGeometryTriangleVertices[geometryIndex]);
        SLANG_CHECK_ABORT(vertexBuffers[geometryIndex] != nullptr);

        // One build input denotes one native geometry. Sharing the index data is harmless; keeping
        // the inputs separate is what makes the traversal result report distinct geometry indices.
        auto& triangleInput = triangleInputs[geometryIndex];
        triangleInput.type = AccelerationStructureBuildInputType::Triangles;
        triangleInput.triangles.vertexBuffers[0] = vertexBuffers[geometryIndex];
        triangleInput.triangles.vertexBufferCount = 1;
        triangleInput.triangles.vertexFormat = Format::RGB32Float;
        triangleInput.triangles.vertexCount = 3;
        triangleInput.triangles.vertexStride = sizeof(Vertex);
        triangleInput.triangles.indexBuffer = indexBuffer;
        triangleInput.triangles.indexFormat = IndexFormat::Uint32;
        triangleInput.triangles.indexCount = SLANG_COUNT_OF(kTriangleIndices);
        triangleInput.triangles.flags = AccelerationStructureGeometryFlags::Opaque;
    }

    AccelerationStructureBuildDesc bottomBuild = {};
    bottomBuild.inputs = triangleInputs;
    bottomBuild.inputCount = SLANG_COUNT_OF(triangleInputs);

    AccelerationStructureSizes bottomSizes = {};
    GFX_CHECK_CALL_ABORT(device->getAccelerationStructureSizes(bottomBuild, &bottomSizes));
    auto bottomScratch = createScratchBuffer(device, bottomSizes.scratchSize);
    SLANG_CHECK_ABORT(bottomScratch != nullptr);

    AccelerationStructureDesc bottomDesc = {};
    bottomDesc.kind = AccelerationStructureKind::BottomLevel;
    bottomDesc.size = bottomSizes.accelerationStructureSize;
    GFX_CHECK_CALL_ABORT(device->createAccelerationStructure(bottomDesc, bottomLevel.writeRef()));

    auto commandEncoder = queue->createCommandEncoder();
    commandEncoder
        ->buildAccelerationStructure(bottomBuild, bottomLevel, nullptr, bottomScratch, 0, nullptr);
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    createSingleInstanceTopLevel(
        device,
        queue,
        bottomLevel,
        AccelerationStructureInstanceFlags::TriangleFacingCullDisable,
        0,
        instanceContributionToHitGroupIndex,
        nullptr,
        instanceBuffer,
        topLevel);
}

StructuralRayTracingProceduralScene::StructuralRayTracingProceduralScene(
    IDevice* device,
    ICommandQueue* queue)
{
    BufferDesc aabbDesc = {};
    aabbDesc.size = sizeof(kProceduralAabb);
    aabbDesc.usage = BufferUsage::AccelerationStructureBuildInput;
    aabbDesc.defaultState = ResourceState::AccelerationStructureBuildInput;
    aabbBuffer = device->createBuffer(aabbDesc, &kProceduralAabb);
    SLANG_CHECK_ABORT(aabbBuffer != nullptr);

    AccelerationStructureBuildInput proceduralInput = {};
    proceduralInput.type = AccelerationStructureBuildInputType::ProceduralPrimitives;
    proceduralInput.proceduralPrimitives.aabbBuffers[0] = aabbBuffer;
    proceduralInput.proceduralPrimitives.aabbBufferCount = 1;
    proceduralInput.proceduralPrimitives.aabbStride = sizeof(AccelerationStructureAABB);
    proceduralInput.proceduralPrimitives.primitiveCount = 1;
    proceduralInput.proceduralPrimitives.flags = AccelerationStructureGeometryFlags::None;

    AccelerationStructureBuildDesc bottomBuild = {};
    bottomBuild.inputs = &proceduralInput;
    bottomBuild.inputCount = 1;

    AccelerationStructureSizes bottomSizes = {};
    GFX_CHECK_CALL_ABORT(device->getAccelerationStructureSizes(bottomBuild, &bottomSizes));
    auto bottomScratch = createScratchBuffer(device, bottomSizes.scratchSize);
    SLANG_CHECK_ABORT(bottomScratch != nullptr);

    AccelerationStructureDesc bottomDesc = {};
    bottomDesc.kind = AccelerationStructureKind::BottomLevel;
    bottomDesc.size = bottomSizes.accelerationStructureSize;
    GFX_CHECK_CALL_ABORT(device->createAccelerationStructure(bottomDesc, bottomLevel.writeRef()));

    auto commandEncoder = queue->createCommandEncoder();
    commandEncoder
        ->buildAccelerationStructure(bottomBuild, bottomLevel, nullptr, bottomScratch, 0, nullptr);
    GFX_CHECK_CALL_ABORT(queue->submit(commandEncoder->finish()));
    GFX_CHECK_CALL_ABORT(queue->waitOnHost());

    createSingleInstanceTopLevel(
        device,
        queue,
        bottomLevel,
        AccelerationStructureInstanceFlags::None,
        0,
        0,
        nullptr,
        instanceBuffer,
        topLevel);
}

} // namespace gfx_test
