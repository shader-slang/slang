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

struct StructuralRayTracingTriangleAttributesFlagsResult
{
    uint32_t stage;
    uint32_t anyHitCount;
    uint32_t barycentricX;
    uint32_t barycentricY;
    uint32_t frontFacing;
    uint32_t flagsMatch;
    uint32_t dispatchWidth;
};

struct StructuralRayTracingStageInputStateResult
{
    uint32_t stage;
    uint32_t minDistance;
    uint32_t distance;
    uint32_t worldOriginX;
    uint32_t worldDirectionZ;
    uint32_t objectOriginX;
    uint32_t objectDirectionZ;
    uint32_t objectToWorldXX;
    uint32_t worldToObjectXX;
    uint32_t primitiveIndex;
    uint32_t geometryIndex;
    uint32_t instanceIndex;
    uint32_t instanceID;
    uint32_t flagsMatch;
    uint32_t dispatchIndex;
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
void runStructuralRayTracingTriangleAttributesFlags(rhi::IDevice* device);
void runStructuralRayTracingStageInputState(rhi::IDevice* device);

} // namespace gfx_test
