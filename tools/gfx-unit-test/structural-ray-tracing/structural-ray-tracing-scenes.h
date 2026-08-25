#pragma once

#include <slang-rhi.h>

namespace gfx_test
{

struct StructuralRayTracingTriangleScene
{
    rhi::ComPtr<rhi::IBuffer> vertexBuffer;
    rhi::ComPtr<rhi::IBuffer> indexBuffer;
    rhi::ComPtr<rhi::IBuffer> instanceBuffer;
    rhi::ComPtr<rhi::IAccelerationStructure> bottomLevel;
    rhi::ComPtr<rhi::IAccelerationStructure> topLevel;

    StructuralRayTracingTriangleScene(
        rhi::IDevice* device,
        rhi::ICommandQueue* queue,
        rhi::AccelerationStructureInstanceFlags instanceFlags =
            rhi::AccelerationStructureInstanceFlags::TriangleFacingCullDisable);
};

struct StructuralRayTracingProceduralScene
{
    rhi::ComPtr<rhi::IBuffer> aabbBuffer;
    rhi::ComPtr<rhi::IBuffer> instanceBuffer;
    rhi::ComPtr<rhi::IAccelerationStructure> bottomLevel;
    rhi::ComPtr<rhi::IAccelerationStructure> topLevel;

    StructuralRayTracingProceduralScene(rhi::IDevice* device, rhi::ICommandQueue* queue);
};

} // namespace gfx_test
