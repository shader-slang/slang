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
            rhi::AccelerationStructureInstanceFlags::TriangleFacingCullDisable,
        uint32_t instanceID = 0,
        const float* transform = nullptr);
};

/// Builds one BLAS containing two independently indexed triangle geometries and one TLAS instance.
///
/// The triangles are spatially separated so a test ray can select geometry index zero or one. The
/// caller controls the TLAS instance contribution independently, which lets runtime tests exercise
/// every term of the native hit-record addressing formula.
struct StructuralRayTracingTwoGeometryTriangleScene
{
    rhi::ComPtr<rhi::IBuffer> vertexBuffers[2];
    rhi::ComPtr<rhi::IBuffer> indexBuffer;
    rhi::ComPtr<rhi::IBuffer> instanceBuffer;
    rhi::ComPtr<rhi::IAccelerationStructure> bottomLevel;
    rhi::ComPtr<rhi::IAccelerationStructure> topLevel;

    StructuralRayTracingTwoGeometryTriangleScene(
        rhi::IDevice* device,
        rhi::ICommandQueue* queue,
        uint32_t instanceContributionToHitGroupIndex);
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
