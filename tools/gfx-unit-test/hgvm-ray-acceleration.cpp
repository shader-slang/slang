#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-events.h"
#include "slang-hgvm/slang-hgvm-interpreter.h"
#include "slangi/hgvm-stats.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace Slang::HGVM;

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
namespace
{
struct AccelerationBuildEventSink : SlangITool::HGVMStatsSink
{
    List<TrackingEvent> events;

    void onEvent(const TrackingEvent& event) noexcept override
    {
        HGVMStatsSink::onEvent(event);
        if (event.kind == TrackingEventKind::AccelerationStructureBuild ||
            event.kind == TrackingEventKind::AccelerationStructureBuildComplete ||
            event.kind == TrackingEventKind::CommandBufferSubmit ||
            event.kind == TrackingEventKind::CommandBatchDiscard)
            events.add(event);
    }
};

/// Checks native build counts, lazy TLAS reuse, and completion identity through real GPU work.
void testAccelerationBuildEvents(UnitTestContext* context, rhi::DeviceType api)
{
    auto apiFlag = api == rhi::DeviceType::Vulkan ? RenderApiFlag::Vulkan : RenderApiFlag::D3D12;
    if (!(context->enabledApis & apiFlag))
    {
        SLANG_IGNORE_TEST;
    }
    rhi::DeviceDesc deviceDesc{};
    deviceDesc.deviceType = api;
    deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
    deviceDesc.debugCallback = context->debugCallback;
    auto device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);
    if (!device->hasFeature(rhi::Feature::RayQuery) ||
        !device->hasFeature(rhi::Feature::RayTracing) ||
        !device->hasFeature(rhi::Feature::TimestampQuery))
    {
        SLANG_IGNORE_TEST;
    }
    const char* declarations = R"(
        import slang.pipeline_rt;

        [host] HWRaytracingContext<>.BottomLevelAccelerationStructure makeBlas()
        {
            let vertices = StructuredBuffer<float3>::alloc(3);
            let indices = StructuredBuffer<uint>::alloc(3);
            let positions = RuntimeArray<float3>::alloc(3);
            positions[0] = float3(-1, -1, 1);
            positions[1] = float3(1, -1, 1);
            positions[2] = float3(0, 1, 1);
            let triangle = RuntimeArray<uint>::alloc(3);
            triangle[0] = 0;
            triangle[1] = 1;
            triangle[2] = 2;
            copy(vertices, 0, positions, 0, 3);
            copy(indices, 0, triangle, 0, 3);
            let geometries = RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(1);
            geometries[0] = HWRaytracingContext<>.TriangleGeometry(vertices, indices);
            geometries[0].vertexCount = 3;
            geometries[0].indexCount = 3;
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(geometries);
            return blas;
        }
        [shader("compute")] [numthreads(1, 1, 1)]
        void queryScene(uniform DescriptorHandle<RaytracingAccelerationStructure> scene,
                        uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
        {
            RayDesc ray = {float3(0, 0, 0), 0.001, float3(0, 0, 1), 10.0};
            RayQuery<RAY_FLAG_FORCE_OPAQUE> query;
            query.TraceRayInline(*scene, RAY_FLAG_NONE, 255, ray);
            while (query.Proceed()) {}
            output[0] = query.CommittedStatus() == COMMITTED_TRIANGLE_HIT ? 1 : 0;
        }
    )";
    for (bool abandon : {false, true})
    {
        StringBuilder source;
        source << declarations << "[host] void main() { let blas = makeBlas();";
        source
            << (abandon ? R"(
            blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(0));
        )"
                        : R"(
            let instances = RuntimeArray<HWRaytracingContext<>.Instance>::alloc(1);
            instances[0] = HWRaytracingContext<>.Instance(blas);
            let scene = RaytracingAccelerationStructure::alloc();
            scene.build(instances);
            scene.build(instances); // Neither snapshot records a native TLAS yet.
            let output = RWStructuredBuffer<uint>::alloc(1);
            let sceneHandle = scene.getDescriptorHandle();
            let outputHandle = output.getDescriptorHandle();
            dispatch(queryScene, uint3(1, 1, 1))(sceneHandle, outputHandle);
            dispatch(queryScene, uint3(1, 1, 1))(sceneHandle, outputHandle); // Reuses the TLAS.
            scene.build(instances);
            dispatch(queryScene, uint3(1, 1, 1))(sceneHandle, outputHandle); // Builds a new TLAS.
            if (readBack(output, 0) != 1)
                blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(0));
        )");
        source << '}';
        auto program = compileHGVMTestProgram(
            context->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_MINIMAL,
            abandon ? "abandoned_build_events" : "completed_build_events");
        for (bool enabled : {false, true})
        {
            RefPtr<AccelerationBuildEventSink> sink = new AccelerationBuildEventSink;
            Desc desc{};
            desc.device = device;
            desc.eventSink = sink;
            desc.enableEventTracking = enabled;
            {
                Interpreter interpreter(desc);
                String error;
                auto result = interpreter.call(program, "main", error);
                if (SLANG_FAILED(result) != abandon)
                    fprintf(
                        stderr,
                        "abandon=%d, tracking=%d: %s\n",
                        abandon,
                        enabled,
                        error.getBuffer());
                SLANG_CHECK_ABORT(SLANG_FAILED(result) == abandon);
            }
            auto stats = sink->takeSnapshot();
            if (!enabled)
            {
                SLANG_CHECK(sink->events.getCount() == 0 && stats.events.getCount() == 0);
                continue;
            }
            Dictionary<uint64_t, TrackingEvent> builds;
            Dictionary<uint64_t, bool> submitted;
            uint32_t bottomLevels = 0, topLevels = 0;
            for (const auto& event : sink->events)
            {
                if (event.kind == TrackingEventKind::CommandBufferSubmit)
                {
                    submitted[event.batch.id] = SLANG_SUCCEEDED(event.batch.result);
                    continue;
                }
                if (event.kind == TrackingEventKind::CommandBatchDiscard)
                    continue;
                const auto& build = event.accelerationStructureBuild;
                if (event.kind == TrackingEventKind::AccelerationStructureBuild)
                {
                    SLANG_CHECK(!builds.containsKey(build.executionId));
                    SLANG_CHECK(!submitted.containsKey(build.batchId));
                    SLANG_CHECK(build.accelerationStructure && !build.hasDuration);
                    SLANG_CHECK(
                        event.origin.programId && event.origin.recordWordOffset != UINT32_MAX);
                    builds.add(build.executionId, event);
                    if (build.kind == rhi::AccelerationStructureKind::BottomLevel)
                        ++bottomLevels;
                    else
                        ++topLevels;
                }
                else
                {
                    auto recorded = builds.tryGetValue(build.executionId);
                    SLANG_CHECK_ABORT(recorded);
                    const auto& before = recorded->accelerationStructureBuild;
                    SLANG_CHECK(before.batchId == build.batchId);
                    SLANG_CHECK(before.accelerationStructure == build.accelerationStructure);
                    SLANG_CHECK(before.kind == build.kind && build.hasDuration);
                    SLANG_CHECK(recorded->origin.programId == event.origin.programId);
                    SLANG_CHECK(recorded->origin.recordWordOffset == event.origin.recordWordOffset);
                    auto submission = submitted.tryGetValue(build.batchId);
                    SLANG_CHECK(submission && *submission);
                    builds.remove(build.executionId);
                }
            }
            auto completed = stats.get(TrackingEventKind::AccelerationStructureBuildComplete);
            SLANG_CHECK(bottomLevels == 1 && topLevels == (abandon ? 0 : 2));
            SLANG_CHECK(completed.count == (abandon ? 0 : 3));
            SLANG_CHECK(completed.timedCount == completed.count);
            SLANG_CHECK(builds.getCount() == (abandon ? 1 : 0));
            SLANG_CHECK(
                stats.get(TrackingEventKind::CommandBatchDiscard).count == (abandon ? 1 : 0));
            SLANG_CHECK(stats.get(TrackingEventKind::Dispatch).count == (abandon ? 0 : 3));
            SLANG_CHECK(
                stats.get(TrackingEventKind::PipelineExecutionComplete).count == (abandon ? 0 : 3));
            SLANG_CHECK(stats.pipelines.getCount() == (abandon ? 0 : 1));
        }
    }
}
} // namespace

SLANG_UNIT_TEST(hgvmAccelerationBuildEventsVulkan)
{
    testAccelerationBuildEvents(unitTestContext, rhi::DeviceType::Vulkan);
}

SLANG_UNIT_TEST(hgvmAccelerationBuildEventsD3D12)
{
    testAccelerationBuildEvents(unitTestContext, rhi::DeviceType::D3D12);
}
#endif

SLANG_UNIT_TEST(hgvmRayAccelerationInvalidArguments)
{
    if (!(unitTestContext->enabledApis & RenderApiFlag::Vulkan))
    {
        SLANG_IGNORE_TEST;
    }
    rhi::DeviceDesc deviceDesc{};
    deviceDesc.deviceType = rhi::DeviceType::Vulkan;
    deviceDesc.slang.slangGlobalSession = unitTestContext->slangGlobalSession;
    deviceDesc.debugCallback = unitTestContext->debugCallback;
    ComPtr<rhi::IDevice> device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);
    if (!device->hasFeature(rhi::Feature::RayTracing))
    {
        SLANG_IGNORE_TEST;
    }

    const char* declarations = R"(
        import slang.pipeline_rt;

        [host] RuntimeArray<HWRaytracingContext<>.TriangleGeometry> makeGeometry()
        {
            let vertices = StructuredBuffer<float3>::alloc(3);
            let indices = StructuredBuffer<uint>::alloc(3);
            let positions = RuntimeArray<float3>::alloc(3);
            positions[0] = float3(0, 0, 1);
            positions[1] = float3(1, 0, 1);
            positions[2] = float3(0, 1, 1);
            let triangle = RuntimeArray<uint>::alloc(3);
            triangle[0] = 0;
            triangle[1] = 1;
            triangle[2] = 2;
            copy(vertices, 0, positions, 0, 3);
            copy(indices, 0, triangle, 0, 3);
            let geometries = RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(1);
            geometries[0].vertices = vertices;
            geometries[0].indices = indices;
            geometries[0].vertexCount = 3;
            geometries[0].indexCount = 3;
            geometries[0].flags = 1;
            return geometries;
        }
        [host] RuntimeArray<HWRaytracingContext<>.Instance> makeInstances()
        {
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(makeGeometry());
            let instances = RuntimeArray<HWRaytracingContext<>.Instance>::alloc(1);
            instances[0].blas = blas;
            instances[0].transform = float3x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0);
            instances[0].mask = 255;
            return instances;
        }
    )";
    struct TestCase
    {
        const char* body;
        const char* expectedError;
    };
    const TestCase cases[] = {
        {R"(
            let blas = hgbc_asm {
                result : $$HWRaytracingContext<>.BottomLevelAccelerationStructure = OpNullLit;
            };
            blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(0));
        )",
         "cannot build a null ray acceleration structure"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(0));
        )",
         "require at least one input"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(0), 4, 1);
        )",
         "updates are not supported"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(0), 32);
        )",
         "flags are unsupported or conflicting"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(0), 12);
        )",
         "flags are unsupported or conflicting"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(1));
        )",
         "requires vertex and index buffers"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            let geometries = makeGeometry();
            geometries[0].vertexOffset = 1;
            blas.build(geometries);
        )",
         "buffer range is out of bounds"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            let geometries = makeGeometry();
            geometries[0].indexOffset = 0xffffffff;
            blas.build(geometries);
        )",
         "buffer range is out of bounds"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            let geometries = makeGeometry();
            geometries[0].indexCount = 2;
            blas.build(geometries);
        )",
         "nonempty vertex/index ranges and valid geometry flags"},
        {R"(
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            let geometries = makeGeometry();
            geometries[0].flags = 4;
            blas.build(geometries);
        )",
         "nonempty vertex/index ranges and valid geometry flags"},
        {R"(
            let scene = RaytracingAccelerationStructure::alloc();
            let instances = RuntimeArray<HWRaytracingContext<>.Instance>::alloc(1);
            instances[0].blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            scene.build(instances);
        )",
         "require a built bottom-level acceleration structure"},
        {R"(
            let scene = RaytracingAccelerationStructure::alloc();
            let instances = makeInstances();
            instances[0].instanceID = 0x1000000;
            scene.build(instances);
        )",
         "instance ID, visibility mask, or flags are out of range"},
        {R"(
            let scene = RaytracingAccelerationStructure::alloc();
            let instances = makeInstances();
            instances[0].mask = 256;
            scene.build(instances);
        )",
         "instance ID, visibility mask, or flags are out of range"},
        {R"(
            let scene = RaytracingAccelerationStructure::alloc();
            let instances = makeInstances();
            instances[0].flags = 12;
            scene.build(instances);
        )",
         "instance ID, visibility mask, or flags are out of range"},
        {R"(
            let scene = RaytracingAccelerationStructure::alloc();
            let instances = makeInstances();
            float zero = 0.0;
            instances[0].transform[0][0] = 1.0 / zero;
            scene.build(instances);
        )",
         "transforms must contain finite values"},
    };
    for (const auto& testCase : cases)
    {
        StringBuilder source;
        source << declarations << "[host] void main() {" << testCase.body << "}";
        auto program = compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_NONE,
            "hgvm_ray_acceleration_invalid_arguments");
        Desc desc{};
        desc.device = device;
        Interpreter interpreter(desc);
        String error;
        SlangResult result = interpreter.call(program, "main", error);
        SLANG_CHECK(SLANG_FAILED(result));
        if (!error.contains(testCase.expectedError))
            fprintf(stderr, "%s\n", error.getBuffer());
        SLANG_CHECK(error.contains(testCase.expectedError));
    }
}
