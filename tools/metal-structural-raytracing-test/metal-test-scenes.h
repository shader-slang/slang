#pragma once

#import <Metal/Metal.h>

struct MetalRayTracingScene
{
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> motionVertexBuffer;
    id<MTLBuffer> boundingBoxBuffer;
    id<MTLBuffer> radiusBuffer;
    id<MTLBuffer> indexBuffer;
    id<MTLBuffer> instanceDescriptorBuffer;
    id<MTLBuffer> innerInstanceDescriptorBuffer;
    id<MTLAccelerationStructure> primitiveAccelerationStructure;
    id<MTLAccelerationStructure> innerInstanceAccelerationStructure;
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

bool buildMetalCurveScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MetalRayTracingScene& outScene,
    NSString** outError);

bool buildMetalMultilevelScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MetalRayTracingScene& outScene,
    NSString** outError);

bool buildMetalPrimitiveMotionScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
    MetalRayTracingScene& outScene,
    NSString** outError);
