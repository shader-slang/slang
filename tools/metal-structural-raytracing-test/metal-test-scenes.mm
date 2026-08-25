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

bool buildInstanceAccelerationStructureLevel(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    const MTLAccelerationStructureUserIDInstanceDescriptor* instances,
    NSUInteger instanceCount,
    NSArray<id<MTLAccelerationStructure>>* children,
    id<MTLBuffer> __strong& outInstanceBuffer,
    id<MTLAccelerationStructure> __strong& outAccelerationStructure,
    NSString** outError)
{
    outInstanceBuffer = [device newBufferWithBytes:instances
                                            length:sizeof(*instances) * instanceCount
                                           options:MTLResourceStorageModeShared];

    auto instanceDescriptor = [MTLInstanceAccelerationStructureDescriptor descriptor];
    instanceDescriptor.instancedAccelerationStructures = children;
    instanceDescriptor.instanceCount = instanceCount;
    instanceDescriptor.instanceDescriptorBuffer = outInstanceBuffer;
    instanceDescriptor.instanceDescriptorStride = sizeof(*instances);
    instanceDescriptor.instanceDescriptorType =
        MTLAccelerationStructureInstanceDescriptorTypeUserID;
    outAccelerationStructure =
        buildAccelerationStructure(device, queue, instanceDescriptor, outError);
    return outAccelerationStructure != nil;
}

MTLPackedFloat4x3 makeTransform(float x, float y, float z)
{
    MTLPackedFloat4x3 result = {};
    result.columns[0] = {1.0f, 0.0f, 0.0f};
    result.columns[1] = {0.0f, 1.0f, 0.0f};
    result.columns[2] = {0.0f, 0.0f, 1.0f};
    result.columns[3] = {x, y, z};
    return result;
}

MTLAccelerationStructureUserIDInstanceDescriptor makeInstance(
    uint32_t accelerationStructureIndex,
    uint32_t userID,
    MTLAccelerationStructureInstanceOptions options,
    const MTLPackedFloat4x3& transform)
{
    MTLAccelerationStructureUserIDInstanceDescriptor result = {};
    result.transformationMatrix = transform;
    result.options = options;
    result.mask = 0xff;
    result.intersectionFunctionTableOffset = 0;
    result.accelerationStructureIndex = accelerationStructureIndex;
    result.userID = userID;
    return result;
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
    auto instance = makeInstance(
        0,
        userInstanceID,
        instanceOptions,
        transform ? *transform : makeTransform(0.0f, 0.0f, 0.0f));
    return buildInstanceAccelerationStructureLevel(
        device,
        queue,
        &instance,
        1,
        @[ scene.primitiveAccelerationStructure ],
        scene.instanceDescriptorBuffer,
        scene.instanceAccelerationStructure,
        outError);
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

bool buildMetalCurveScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MetalRayTracingScene& outScene,
    NSString** outError)
{
    static const simd_float3 kControlPoints[] = {
        {-0.5f, 0.0f, 1.0f},
        {0.5f, 0.0f, 1.0f},
    };
    static const float kRadii[] = {0.1f, 0.1f};
    static const uint16_t kIndices[] = {0, 1};
    outScene.vertexBuffer = [device newBufferWithBytes:kControlPoints
                                                length:sizeof(kControlPoints)
                                               options:MTLResourceStorageModeShared];
    outScene.radiusBuffer = [device newBufferWithBytes:kRadii
                                                length:sizeof(kRadii)
                                               options:MTLResourceStorageModeShared];
    outScene.indexBuffer = [device newBufferWithBytes:kIndices
                                               length:sizeof(kIndices)
                                              options:MTLResourceStorageModeShared];
    if (!outScene.vertexBuffer || !outScene.radiusBuffer || !outScene.indexBuffer)
    {
        if (outError)
            *outError = @"failed to allocate curve geometry buffers";
        return false;
    }

    auto geometry = [MTLAccelerationStructureCurveGeometryDescriptor descriptor];
    geometry.controlPointBuffer = outScene.vertexBuffer;
    geometry.controlPointCount = 2;
    geometry.controlPointStride = sizeof(simd_float3);
    geometry.controlPointFormat = MTLAttributeFormatFloat3;
    geometry.radiusBuffer = outScene.radiusBuffer;
    geometry.radiusStride = sizeof(float);
    geometry.radiusFormat = MTLAttributeFormatFloat;
    geometry.indexBuffer = outScene.indexBuffer;
    geometry.indexType = MTLIndexTypeUInt16;
    geometry.segmentCount = 1;
    geometry.segmentControlPointCount = 2;
    geometry.curveType = MTLCurveTypeRound;
    geometry.curveBasis = MTLCurveBasisLinear;
    geometry.curveEndCaps = MTLCurveEndCapsSphere;
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

bool buildMetalMultilevelScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
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

    MTLAccelerationStructureUserIDInstanceDescriptor innerInstances[] = {
        makeInstance(
            0,
            10,
            MTLAccelerationStructureInstanceOptionOpaque,
            makeTransform(5.0f, 0.0f, 0.0f)),
        makeInstance(
            0,
            11,
            MTLAccelerationStructureInstanceOptionOpaque,
            makeTransform(0.0f, 0.0f, 0.0f)),
    };
    if (!buildInstanceAccelerationStructureLevel(
            device,
            queue,
            innerInstances,
            2,
            @[ outScene.primitiveAccelerationStructure ],
            outScene.innerInstanceDescriptorBuffer,
            outScene.innerInstanceAccelerationStructure,
            outError))
        return false;

    auto outerInstance = makeInstance(
        0,
        20,
        MTLAccelerationStructureInstanceOptionOpaque,
        makeTransform(0.0f, 0.0f, 0.0f));
    return buildInstanceAccelerationStructureLevel(
        device,
        queue,
        &outerInstance,
        1,
        @[ outScene.innerInstanceAccelerationStructure ],
        outScene.instanceDescriptorBuffer,
        outScene.instanceAccelerationStructure,
        outError);
}

bool buildMetalPrimitiveMotionScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MetalRayTracingScene& outScene,
    NSString** outError)
{
    static const simd_float3 kVerticesAtStart[] = {
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f},
    };
    static const simd_float3 kVerticesAtEnd[] = {
        {2.0f, 0.0f, 1.0f},
        {3.0f, 0.0f, 1.0f},
        {2.0f, 1.0f, 1.0f},
    };
    outScene.vertexBuffer = [device newBufferWithBytes:kVerticesAtStart
                                                length:sizeof(kVerticesAtStart)
                                               options:MTLResourceStorageModeShared];
    outScene.motionVertexBuffer = [device newBufferWithBytes:kVerticesAtEnd
                                                      length:sizeof(kVerticesAtEnd)
                                                     options:MTLResourceStorageModeShared];
    if (!outScene.vertexBuffer || !outScene.motionVertexBuffer)
    {
        if (outError)
            *outError = @"failed to allocate motion-triangle vertex buffers";
        return false;
    }

    auto startKeyframe = [MTLMotionKeyframeData data];
    startKeyframe.buffer = outScene.vertexBuffer;
    auto endKeyframe = [MTLMotionKeyframeData data];
    endKeyframe.buffer = outScene.motionVertexBuffer;

    auto geometry = [MTLAccelerationStructureMotionTriangleGeometryDescriptor descriptor];
    geometry.vertexBuffers = @[ startKeyframe, endKeyframe ];
    geometry.vertexStride = sizeof(simd_float3);
    geometry.vertexFormat = MTLAttributeFormatFloat3;
    geometry.triangleCount = 1;
    geometry.opaque = YES;

    auto primitiveDescriptor = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
    primitiveDescriptor.geometryDescriptors = @[ geometry ];
    primitiveDescriptor.motionStartTime = 0.0f;
    primitiveDescriptor.motionEndTime = 1.0f;
    primitiveDescriptor.motionKeyframeCount = 2;
    outScene.primitiveAccelerationStructure =
        buildAccelerationStructure(device, queue, primitiveDescriptor, outError);
    if (!outScene.primitiveAccelerationStructure)
        return false;

    return buildInstanceAccelerationStructure(
        device,
        queue,
        MTLAccelerationStructureInstanceOptionOpaque,
        0,
        nullptr,
        outScene,
        outError);
}
