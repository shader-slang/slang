#pragma once

#import <Metal/Metal.h>

struct MetalRayTracingScene
{
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> secondVertexBuffer;
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

/// Builds one primitive acceleration structure with two spatially separated triangle geometries.
/// Their geometry IDs are zero and one, matching the two-geometry portable runtime scene.
bool buildMetalTwoGeometryTriangleScene(
    id<MTLDevice> device,
    id<MTLCommandQueue> queue,
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
