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

} // namespace gfx_test
