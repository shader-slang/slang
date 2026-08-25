#include "metal-test-scenes.h"

#include <simd/simd.h>

namespace
{

id<MTLAccelerationStructure> buildAccelerationStructure(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MTLAccelerationStructureDescriptor* descriptor,
    NSString** outError)
{
    MTLAccelerationStructureSizes sizes =
        [device accelerationStructureSizesWithDescriptor:descriptor];
    id<MTLAccelerationStructure> accelerationStructure =
        [device newAccelerationStructureWithSize:sizes.accelerationStructureSize];
    id<MTLBuffer> scratch = [device newBufferWithLength:sizes.buildScratchBufferSize
                                                options:MTLResourceStorageModePrivate];
    if (!accelerationStructure || !scratch)
    {
        if (outError)
            *outError = @"failed to allocate acceleration-structure storage";
        return nil;
    }

    id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
    id<MTLAccelerationStructureCommandEncoder> encoder =
        [commandBuffer accelerationStructureCommandEncoder];
    [encoder buildAccelerationStructure:accelerationStructure
                             descriptor:descriptor
                          scratchBuffer:scratch
                    scratchBufferOffset:0];
    [encoder endEncoding];
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
    if (commandBuffer.status == MTLCommandBufferStatusError)
    {
        if (outError)
            *outError = commandBuffer.error.localizedDescription;
        return nil;
    }
    return accelerationStructure;
}

bool buildInstanceAccelerationStructure(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MTLAccelerationStructureInstanceOptions instanceOptions,
    uint32_t userInstanceID,
    const MTLPackedFloat4x3* transform,
    MetalRayTracingScene& scene,
    NSString** outError)
{
    MTLAccelerationStructureUserIDInstanceDescriptor instance = {};
    if (transform)
    {
        instance.transformationMatrix = *transform;
    }
    else
    {
        instance.transformationMatrix.columns[0] = {1.0f, 0.0f, 0.0f};
        instance.transformationMatrix.columns[1] = {0.0f, 1.0f, 0.0f};
        instance.transformationMatrix.columns[2] = {0.0f, 0.0f, 1.0f};
        instance.transformationMatrix.columns[3] = {0.0f, 0.0f, 0.0f};
    }
    instance.options = instanceOptions;
    instance.mask = 0xff;
    instance.intersectionFunctionTableOffset = 0;
    instance.accelerationStructureIndex = 0;
    instance.userID = userInstanceID;
    scene.instanceDescriptorBuffer = [device newBufferWithBytes:&instance
                                                         length:sizeof(instance)
                                                        options:MTLResourceStorageModeShared];

    auto instanceDescriptor = [MTLInstanceAccelerationStructureDescriptor descriptor];
    instanceDescriptor.instancedAccelerationStructures = @[ scene.primitiveAccelerationStructure ];
    instanceDescriptor.instanceCount = 1;
    instanceDescriptor.instanceDescriptorBuffer = scene.instanceDescriptorBuffer;
    instanceDescriptor.instanceDescriptorStride = sizeof(instance);
    instanceDescriptor.instanceDescriptorType =
        MTLAccelerationStructureInstanceDescriptorTypeUserID;
    scene.instanceAccelerationStructure =
        buildAccelerationStructure(device, queue, instanceDescriptor, outError);
    return scene.instanceAccelerationStructure != nil;
}

} // namespace

bool buildMetalTriangleScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MTLAccelerationStructureInstanceOptions instanceOptions,
    uint32_t userInstanceID,
    const MTLPackedFloat4x3* transform,
    MetalRayTracingScene& outScene,
    NSString** outError)
{
    static const simd_float3 kVertices[] = {
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f},
    };
    outScene.vertexBuffer = [device newBufferWithBytes:kVertices
                                                length:sizeof(kVertices)
                                               options:MTLResourceStorageModeShared];
    if (!outScene.vertexBuffer)
    {
        if (outError)
            *outError = @"failed to allocate the triangle vertex buffer";
        return false;
    }

    auto geometry = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
    geometry.vertexBuffer = outScene.vertexBuffer;
    geometry.vertexStride = sizeof(simd_float3);
    geometry.triangleCount = 1;
    geometry.opaque = YES;

    auto primitiveDescriptor = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
    primitiveDescriptor.geometryDescriptors = @[ geometry ];
    outScene.primitiveAccelerationStructure =
        buildAccelerationStructure(device, queue, primitiveDescriptor, outError);
    if (!outScene.primitiveAccelerationStructure)
        return false;

    return buildInstanceAccelerationStructure(
        device,
        queue,
        instanceOptions,
        userInstanceID,
        transform,
        outScene,
        outError);
}

bool buildMetalBoundingBoxScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MetalRayTracingScene& outScene,
    NSString** outError)
{
    MTLAxisAlignedBoundingBox boundingBox = {};
    boundingBox.min = {-0.5f, -0.5f, 0.5f};
    boundingBox.max = {0.5f, 0.5f, 2.0f};
    outScene.boundingBoxBuffer = [device newBufferWithBytes:&boundingBox
                                                     length:sizeof(boundingBox)
                                                    options:MTLResourceStorageModeShared];
    if (!outScene.boundingBoxBuffer)
    {
        if (outError)
            *outError = @"failed to allocate the bounding-box buffer";
        return false;
    }

    auto geometry = [MTLAccelerationStructureBoundingBoxGeometryDescriptor descriptor];
    geometry.boundingBoxBuffer = outScene.boundingBoxBuffer;
    geometry.boundingBoxStride = sizeof(MTLAxisAlignedBoundingBox);
    geometry.boundingBoxCount = 1;
    geometry.opaque = NO;

    auto primitiveDescriptor = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
    primitiveDescriptor.geometryDescriptors = @[ geometry ];
    outScene.primitiveAccelerationStructure =
        buildAccelerationStructure(device, queue, primitiveDescriptor, outError);
    if (!outScene.primitiveAccelerationStructure)
        return false;

    return buildInstanceAccelerationStructure(
        device,
        queue,
        MTLAccelerationStructureInstanceOptionNone,
        0,
        nullptr,
        outScene,
        outError);
}
