#pragma once

#import <Metal/Metal.h>

struct MetalRayTracingScene
{
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> boundingBoxBuffer;
    id<MTLBuffer> instanceDescriptorBuffer;
    id<MTLAccelerationStructure> primitiveAccelerationStructure;
    id<MTLAccelerationStructure> instanceAccelerationStructure;
};

bool buildMetalTriangleScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MTLAccelerationStructureInstanceOptions instanceOptions,
    uint32_t userInstanceID,
    const MTLPackedFloat4x3* transform,
    MetalRayTracingScene& outScene,
    NSString** outError);

bool buildMetalBoundingBoxScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MetalRayTracingScene& outScene,
    NSString** outError);
