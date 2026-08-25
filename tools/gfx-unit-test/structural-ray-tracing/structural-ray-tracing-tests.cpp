#include "structural-ray-tracing-test-util.h"
#include "unit-test/slang-unit-test.h"

using namespace rhi;

namespace gfx_test
{

static void runTriangleHitMiss(UnitTestContext* context, DeviceType deviceType)
{
    auto device = createStructuralRayTracingTestDevice(context, deviceType);
    runStructuralRayTracingTriangleHitMiss(device);
}

SLANG_UNIT_TEST(structuralRayTracingTriangleHitMissD3D12)
{
    runTriangleHitMiss(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structuralRayTracingTriangleHitMissVulkan)
{
    runTriangleHitMiss(unitTestContext, DeviceType::Vulkan);
}

static void runProceduralHitFilter(UnitTestContext* context, DeviceType deviceType)
{
    auto device = createStructuralRayTracingTestDevice(context, deviceType);
    runStructuralRayTracingProceduralHitFilter(device);
}

SLANG_UNIT_TEST(structuralRayTracingProceduralHitFilterD3D12)
{
    runProceduralHitFilter(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structuralRayTracingProceduralHitFilterVulkan)
{
    runProceduralHitFilter(unitTestContext, DeviceType::Vulkan);
}

static void runCallableRecord(UnitTestContext* context, DeviceType deviceType)
{
    auto device = createStructuralRayTracingTestDevice(context, deviceType);
    runStructuralRayTracingCallableRecord(device);
}

SLANG_UNIT_TEST(structuralRayTracingCallableRecordD3D12)
{
    runCallableRecord(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structuralRayTracingCallableRecordVulkan)
{
    runCallableRecord(unitTestContext, DeviceType::Vulkan);
}

static void runRecursiveTrace(UnitTestContext* context, DeviceType deviceType)
{
    auto device = createStructuralRayTracingTestDevice(context, deviceType);
    runStructuralRayTracingRecursiveTrace(device);
}

SLANG_UNIT_TEST(structuralRayTracingRecursiveTraceD3D12)
{
    runRecursiveTrace(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structuralRayTracingRecursiveTraceVulkan)
{
    runRecursiveTrace(unitTestContext, DeviceType::Vulkan);
}

static void runMultipleSlots(UnitTestContext* context, DeviceType deviceType)
{
    auto device = createStructuralRayTracingTestDevice(context, deviceType);
    runStructuralRayTracingMultipleSlots(device);
}

SLANG_UNIT_TEST(structuralRayTracingMultipleSlotsD3D12)
{
    runMultipleSlots(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structuralRayTracingMultipleSlotsVulkan)
{
    runMultipleSlots(unitTestContext, DeviceType::Vulkan);
}

static void runTriangleAttributesFlags(UnitTestContext* context, DeviceType deviceType)
{
    auto device = createStructuralRayTracingTestDevice(context, deviceType);
    runStructuralRayTracingTriangleAttributesFlags(device);
}

SLANG_UNIT_TEST(structuralRayTracingTriangleAttributesFlagsD3D12)
{
    runTriangleAttributesFlags(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structuralRayTracingTriangleAttributesFlagsVulkan)
{
    runTriangleAttributesFlags(unitTestContext, DeviceType::Vulkan);
}

static void runStageInputState(UnitTestContext* context, DeviceType deviceType)
{
    auto device = createStructuralRayTracingTestDevice(context, deviceType);
    runStructuralRayTracingStageInputState(device);
}

SLANG_UNIT_TEST(structuralRayTracingStageInputStateD3D12)
{
    runStageInputState(unitTestContext, DeviceType::D3D12);
}

SLANG_UNIT_TEST(structuralRayTracingStageInputStateVulkan)
{
    runStageInputState(unitTestContext, DeviceType::Vulkan);
}

} // namespace gfx_test
