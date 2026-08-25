#pragma once

#include "gfx-unit-test/gfx-test-util.h"

#include <slang-rhi.h>

namespace gfx_test
{

struct StructuralRayTracingRuntimeResult
{
    uint32_t stage;
    uint32_t primitiveIndex;
    uint32_t dispatchWidth;
};

Slang::ComPtr<rhi::IDevice> createStructuralRayTracingTestDevice(
    UnitTestContext* context,
    rhi::DeviceType deviceType);

void runStructuralRayTracingTriangleHitMiss(rhi::IDevice* device);

} // namespace gfx_test
