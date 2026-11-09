// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "core/slang-writer.h"
#include "hgvm-test-helpers.h"
#include "slangi/hgvm-stats-console.h"

using namespace Slang;
using namespace Slang::HGVM;
using namespace SlangITool;

SLANG_UNIT_TEST(hgvmPipelineShaderDisplay)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            [shader("compute")] [numthreads(1, 1, 1)] void work<T>() {}
            [host] void main() { dispatch(work<uint>, uint3(1, 1, 1))(); }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "shader_names");
    DebugMapBuilder builder(program);
    uint32_t offset = 0;
    for (const auto& record : program->getDecodedProgram().records)
    {
        if (record.opcode != HGBCOpcode::SpecializeSlangEntryPoint)
            continue;
        builder.setEntryPointName(offset, HGBC::getResultId(record));
        ++offset;
    }
    auto map = builder.getDebugMap();
    SLANG_CHECK_ABORT(offset == 1 && map->findEntryPointName(0));
    SLANG_CHECK(*map->findEntryPointName(0) == "shader_names::work<uint>");
    SLANG_CHECK(!map->findEntryPointName(1));
    SLANG_CHECK(!map->findSourceLocation(0));

    const struct
    {
        SlangStage stage;
        const char* name;
        bool hitGroup;
    } cases[] = {
        {SLANG_STAGE_COMPUTE, "compute", false},
        {SLANG_STAGE_VERTEX, "vertex", false},
        {SLANG_STAGE_FRAGMENT, "fragment", false},
        {SLANG_STAGE_RAY_GENERATION, "raygeneration", false},
        {SLANG_STAGE_MISS, "miss", false},
        {SLANG_STAGE_CALLABLE, "callable", false},
        {SLANG_STAGE_CLOSEST_HIT, "closesthit", true},
        {SLANG_STAGE_ANY_HIT, "anyhit", true},
        {SLANG_STAGE_INTERSECTION, "intersection", true},
    };
    for (bool full : {false, true})
        for (const auto& test : cases)
        {
            EventText text;
            HGVMEventDisplay display(full);
            TrackedPipelineShader shader{
                test.stage,
                {map->getProgramId(), 0},
                test.hitGroup ? 2u : UINT32_MAX};
            display.appendPipelineShader(text, shader, map);
            StringBuilder expected;
            if (test.hitGroup)
                expected << "hitgroup[2] ";
            expected << test.name << ' ';
            if (full)
                expected << "shader_names::";
            expected << "work<uint>";
            SLANG_CHECK(text.render(false) == expected.produceString());
            SLANG_CHECK(text.render(true).contains("\x1b[36m"));
        }
}

SLANG_UNIT_TEST(hgvmPipelineStatsIdentity)
{
    RefPtr<HGVMStatsSink> sink = new HGVMStatsSink;
    HGVMStatsSnapshot totals;
    uint64_t firstId;
    {
        EventTracker tracker(true, sink);
        int storage;
        auto handle = &storage;
        List<TrackedPipelineShader> shaders;
        shaders.add({SLANG_STAGE_RAY_GENERATION, {12, 34}});
        tracker.trackPipelineCreation(
            handle,
            TrackedPipelineKind::Raytracing,
            SLANG_OK,
            0,
            {},
            shaders);
        firstId = tracker.findPipelineDescription(handle)->id;
        TrackingEvent completion{};
        completion.tracker = &tracker;
        completion.kind = TrackingEventKind::PipelineExecutionComplete;
        completion.pipelineExecution = {1, 1, handle, TrackedPipelineKind::Raytracing, true, 100};
        sink->onEvent(completion);
        totals.merge(sink->takeSnapshot());
        sink->onEvent(completion);
        totals.merge(sink->takeSnapshot());
        // Reusing a native address must neither rename nor combine the old pipeline's totals.
        shaders.add({SLANG_STAGE_MISS, {12, 56}});
        tracker.trackPipelineCreation(
            handle,
            TrackedPipelineKind::Raytracing,
            SLANG_OK,
            0,
            {},
            shaders);
        SLANG_CHECK(tracker.findPipelineDescription(handle)->id != firstId);
        sink->onEvent(completion);
        totals.merge(sink->takeSnapshot());
        tracker.trackPipelineCreation(nullptr, TrackedPipelineKind::Raytracing, SLANG_FAIL, 0);
        SLANG_CHECK(!tracker.findPipelineDescription(nullptr));
    }
    SLANG_CHECK(totals.pipelines.getCount() == 2);
    SLANG_CHECK(totals.pipelineDescriptions.getCount() == 2);
    for (const auto& row : totals.pipelines)
    {
        auto description = totals.pipelineDescriptions[row.first];
        bool first = row.first.dynamicPipelineId == firstId;
        SLANG_CHECK(row.second.count == (first ? 2 : 1));
        SLANG_CHECK(row.second.durationNanoseconds == (first ? 200 : 100));
        SLANG_CHECK(description->shaders.getCount() == (first ? 1 : 2));
        SLANG_CHECK(description->shaders[0].entryPoint.recordWordOffset == 34);
    }
}

SLANG_UNIT_TEST(hgvmAccelerationBuildStats)
{
    for (bool timed : {false, true})
    {
        RefPtr<HGVMStatsSink> sink = new HGVMStatsSink;
        EventTracker tracker(true, sink);
        StringBuilder outputText, statsText;
        ComPtr<ISlangWriter> output(new StringWriter(&outputText));
        ComPtr<ISlangWriter> error(new StringWriter(&statsText));
        EventOutputOptions options;
        options.color = EventOutputOptions::Color::Never;
        RefPtr<HGVMStatsConsole> console = new HGVMStatsConsole(sink, output, error, options);
        TrackingEvent event{};
        event.tracker = &tracker;
        event.accelerationStructureBuild =
            {1, 1, &tracker, rhi::AccelerationStructureKind::BottomLevel, false, 0};
        HGVMStatsSnapshot totals;
        for (uint32_t i = 0; i < 3; ++i)
        {
            event.kind = TrackingEventKind::AccelerationStructureBuild;
            event.accelerationStructureBuild.executionId = i + 1;
            event.accelerationStructureBuild.hasDuration = false;
            event.accelerationStructureBuild.durationNanoseconds = 0;
            sink->onEvent(event);
            // Recording and completion can arrive in different reporting intervals. One build
            // is abandoned, and one completion has no duration even on a timed device.
            totals.merge(sink->takeSnapshot());
            if (i == 2)
                continue;
            event.kind = TrackingEventKind::AccelerationStructureBuildComplete;
            event.accelerationStructureBuild.hasDuration = timed && i == 1;
            event.accelerationStructureBuild.durationNanoseconds = timed && i == 1 ? 1250000 : 0;
            sink->onEvent(event);
            totals.merge(sink->takeSnapshot());
        }
        auto completed = totals.get(TrackingEventKind::AccelerationStructureBuildComplete);
        SLANG_CHECK(totals.get(TrackingEventKind::AccelerationStructureBuild).count == 3);
        SLANG_CHECK(completed.count == 2 && completed.timedCount == (timed ? 1 : 0));
        SLANG_CHECK(completed.durationNanoseconds == (timed ? 1250000 : 0));
        SLANG_CHECK(totals.pipelines.getCount() == 0);
        // Also verify that the report distinguishes an unavailable duration from measured time.
        event.kind = TrackingEventKind::AccelerationStructureBuild;
        event.accelerationStructureBuild.hasDuration = false;
        sink->onEvent(event);
        event.kind = TrackingEventKind::AccelerationStructureBuildComplete;
        event.accelerationStructureBuild.hasDuration = timed;
        event.accelerationStructureBuild.durationNanoseconds = timed ? 1250000 : 0;
        sink->onEvent(event);
        console->finish();
        auto stats = statsText.produceString();
        SLANG_CHECK(stats.contains("Device Statistics"));
        SLANG_CHECK(stats.contains(
            timed
                ? "Acceleration structures: build 1; complete 1; timed 1 (1.250 GPU ms)"
                : "Acceleration structures: build 1; complete 1; timed 0 (GPU time unavailable)"));
    }
}

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
namespace
{
/// Exercises assembly, cache reuse, map retention, and final formatting through real dispatches.
void testRayPipelineStats(UnitTestContext* context, rhi::DeviceType api)
{
    auto flag = api == rhi::DeviceType::Vulkan ? RenderApiFlag::Vulkan : RenderApiFlag::D3D12;
    if (!(context->enabledApis & flag))
    {
        SLANG_IGNORE_TEST;
    }
    rhi::DeviceDesc deviceDesc{};
    deviceDesc.deviceType = api;
    deviceDesc.slang.slangGlobalSession = context->slangGlobalSession;
    deviceDesc.debugCallback = context->debugCallback;
    auto device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);
    if (!device->hasFeature(rhi::Feature::RayTracing) ||
        !device->hasFeature(rhi::Feature::TimestampQuery))
    {
        SLANG_IGNORE_TEST;
    }
    const char* source = R"(
        import slang.pipeline_rt;

        [raypayload] struct Payload
        { uint value : read(caller, closesthit, miss) : write(caller, closesthit, miss); }
        [shader("miss")] void miss<let bias : uint>(inout Payload payload)
        { payload.value = bias; }
        [shader("closesthit")]
        void hit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
        { payload.value = 9; }
        [shader("raygeneration")]
        void generate(uniform RaytracingPipeline pipeline,
                      uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
        { output[0] = 42 + getBindingOffset(pipeline); }
        [host] void main()
        {
            let output = RWStructuredBuffer<uint>::alloc(1);
            let handle = output.getDescriptorHandle();
            let pipeline = makeRaytracingPipeline(1);
            __setRayMiss(pipeline, 0, miss<1>)();
            __setRayHitShader(pipeline, 0, 0, hit)();
            __dispatchRays(generate, uint3(1, 1, 1), 1)(pipeline, handle);
            __setRayMiss(pipeline, 0, miss<2>)();
            __dispatchRays(generate, uint3(1, 1, 1), 1)(pipeline, handle);
            __dispatchRays(generate, uint3(1, 1, 1), 1)(pipeline, handle);
            print(readBack(output, 0));
        }
    )";
    for (bool full : {false, true})
    {
        RefPtr<HGVMStatsSink> sink = new HGVMStatsSink;
        StringBuilder outputText, statsText;
        ComPtr<ISlangWriter> output(new StringWriter(&outputText));
        ComPtr<ISlangWriter> error(new StringWriter(&statsText));
        EventOutputOptions options;
        options.fullNames = full;
        options.color = EventOutputOptions::Color::Never;
        options.statsTop = 2;
        RefPtr<HGVMStatsConsole> console = new HGVMStatsConsole(sink, output, error, options);
        {
            auto program = compileHGVMTestProgram(
                context->slangGlobalSession,
                source,
                SLANG_DEBUG_INFO_LEVEL_NONE,
                "ray_stats");
            Desc desc{};
            desc.device = device;
            desc.enableEventTracking = true;
            desc.eventSink = sink;
            desc.outputSink = console;
            Interpreter interpreter(desc);
            String message;
            auto result = interpreter.call(program, "main", message);
            if (SLANG_FAILED(result))
                fprintf(stderr, "%s\n", message.getBuffer());
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        }
        // Both the program and interpreter are gone before the final report resolves any name.
        console->finish();
        String stats = statsText.produceString();
        SLANG_CHECK(outputText.produceString() == "42\n");
        SLANG_CHECK(!stats.contains("<unknown>"));
        SLANG_CHECK(!stats.contains("hgvm_ray_"));
        SLANG_CHECK(!stats.contains("\x1b"));
        SLANG_CHECK(stats.contains("ray-tracing\n            raygeneration "));
        SLANG_CHECK(stats.contains("\n            hitgroup[0] closesthit "));
        SLANG_CHECK(stats.contains(full ? "miss ray_stats::miss<1>" : "miss miss<1>"));
        SLANG_CHECK(stats.contains(full ? "miss ray_stats::miss<2>" : "miss miss<2>"));
        SLANG_CHECK(
            stats.contains(full ? "raygeneration ray_stats::generate" : "raygeneration generate"));
        SLANG_CHECK(stats.contains("ray_stats::") == full);
    }
}
} // namespace

SLANG_UNIT_TEST(hgvmPipelineStatsRayVulkan)
{
    testRayPipelineStats(unitTestContext, rhi::DeviceType::Vulkan);
}

SLANG_UNIT_TEST(hgvmPipelineStatsRayD3D12)
{
    testRayPipelineStats(unitTestContext, rhi::DeviceType::D3D12);
}
#endif
