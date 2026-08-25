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

    StructuralRayTracingTriangleScene(rhi::IDevice* device, rhi::ICommandQueue* queue);
};

} // namespace gfx_test
