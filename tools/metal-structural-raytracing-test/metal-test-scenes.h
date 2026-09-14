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
    id<MTLBuffer> siblingInnerInstanceDescriptorBuffer;
    id<MTLAccelerationStructure> primitiveAccelerationStructure;
    id<MTLAccelerationStructure> innerInstanceAccelerationStructure;
    id<MTLAccelerationStructure> siblingInnerInstanceAccelerationStructure;
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

/// Builds two sibling inner IAS branches whose leaf instances both have local index zero.
/// Rays can therefore distinguish correct full-path record selection from a leaf-only lookup.
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
