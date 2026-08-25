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

struct StructuralRayTracingProceduralResult
{
    uint32_t stage;
    uint32_t candidate;
    uint32_t anyHitCount;
    uint32_t dispatchWidth;
};

struct StructuralRayTracingCallableResult
{
    uint32_t value;
    uint32_t dispatchWidth;
};

struct StructuralRayTracingRecursiveResult
{
    uint32_t stage;
    uint32_t depth;
    uint32_t dispatchWidth;
};

struct StructuralRayTracingMultipleSlotsResult
{
    uint32_t stage;
    uint32_t recordValue;
    uint32_t dispatchWidth;
};

Slang::ComPtr<rhi::IDevice> createStructuralRayTracingTestDevice(
    UnitTestContext* context,
    rhi::DeviceType deviceType);

void runStructuralRayTracingTriangleHitMiss(rhi::IDevice* device);
void runStructuralRayTracingProceduralHitFilter(rhi::IDevice* device);
void runStructuralRayTracingCallableRecord(rhi::IDevice* device);
void runStructuralRayTracingRecursiveTrace(rhi::IDevice* device);
void runStructuralRayTracingMultipleSlots(rhi::IDevice* device);

} // namespace gfx_test
