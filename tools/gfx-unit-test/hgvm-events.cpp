#include "core/slang-dictionary.h"
#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-debug-map.h"
#include "slang-hgvm/slang-hgvm-events.h"
#include "slang-hgvm/slang-hgvm-interpreter.h"
#include "slang-hgvm/slang-hgvm-staged-record-builder.h"
#include "slang-hgvm/slang-hgvm-types.h"
#include "slang-hgvm/slang-hgvm.h"
#include "unit-test/slang-unit-test.h"

#include <chrono>
#include <thread>

using namespace Slang;
using namespace Slang::HGVM;

namespace
{
struct RecordingEventSink : IEventSink
{
    List<TrackingEvent> events;
    Dictionary<uint64_t, RefPtr<DebugMap>> debugMaps;

    const SourceLocation* findSourceLocation(const EventOrigin& origin) const
    {
        auto map = debugMaps.tryGetValue(origin.programId);
        return map ? (*map)->findSourceLocation(origin.recordWordOffset) : nullptr;
    }
    Dictionary<const void*, uint64_t> heap;
    Dictionary<const void*, uint64_t> payloads;
    bool valid = true;
    bool* destroyed = nullptr;

    ~RecordingEventSink()
    {
        if (destroyed)
            *destroyed = true;
    }

    void onEvent(const TrackingEvent& event) noexcept override
    {
        // This receiver keeps maps explicitly because it verifies events after interpreter
        // teardown.
        if (auto interpreter = event.tracker->getInterpreter())
        {
            for (auto id : {event.origin.programId, event.allocationOrigin.programId})
                if (auto map = interpreter->getDebugMap(id))
                    debugMaps[id] = map;
        }
        valid &= event.sequence == uint64_t(events.getCount() + 1);
        if (events.getCount())
            valid &= event.timestampNanoseconds >= events.getLast().timestampNanoseconds;
        switch (event.kind)
        {
        case TrackingEventKind::HeapAlloc:
            valid &= !heap.containsKey(event.heap.address);
            heap[event.heap.address] = event.heap.byteSize;
            break;
        case TrackingEventKind::HeapFree:
            {
                auto* size = heap.tryGetValue(event.heap.address);
                valid &= size && *size == event.heap.byteSize;
                heap.remove(event.heap.address);
                break;
            }
        case TrackingEventKind::PayloadAlloc:
            valid &= !payloads.containsKey(event.payload.address);
            payloads[event.payload.address] = event.payload.byteSize;
            break;
        case TrackingEventKind::PayloadFree:
            valid &= payloads.containsKey(event.payload.address);
            payloads.remove(event.payload.address);
            break;
        default:
            break;
        }
        events.add(event);
        // Names are borrowed for this callback. This test records only module identity and status.
        if (event.kind == TrackingEventKind::ModuleLoad)
            events.getLast().module.name = nullptr;
    }

    Index count(TrackingEventKind kind) const
    {
        Index result = 0;
        for (const auto& event : events)
            if (event.kind == kind)
                ++result;
        return result;
    }
};

struct EventTestOutput : IHGVMOutputSink
{
    String text;
    void writeLine(std::string_view line) override
    {
        text.append(line.data(), Index(line.size()));
        text.append("\n");
    }
};

const char* kEventTestSource = R"(
        class Node { Node next; __init() {} }
        String formatValue(uint value)
        {
            String pattern = "{}";
            return hgbc_asm {
                result : $$String = OpFormatString $pattern $$uint $value;
            };
        }
        [shader("compute")] [numthreads(1,1,1)]
        void store(uniform RWStructuredBuffer<uint> dst, uniform uint value) { dst[0] = value; }
        RWTexture2D<float4> texture()
        {
            uint two = 2, one = 1;
            return hgbc_asm {
                result : $$RWTexture2D<float4> = OpAllocateResource
                    Texture Texture2D $__getHGBCNaturalTextureFormat<float4>()
                    DeviceLocal UnorderedAccess $$float4 $two $two $one 1 1 0
                    ShaderResource UnorderedAccess CopySource CopyDestination RenderTarget;
            };
        }
        [host] void main()
        {
            var root = new Node();
            root.next = new Node();
            var nodes = RuntimeArray<Node>::alloc(2);
            nodes[0] = root;
            var buffer = RWStructuredBuffer<uint>::alloc(4);
            dispatch(store, uint3(1,1,1))(buffer, 12u);
            dispatch(store, uint3(1,1,1))(buffer, 34u);
            let pending = asyncReadBack(buffer, 0);
            print(formatValue(pending.wait()));
            // Leave graphics work for call/destructor submission and cleanup.
            var source = texture();
            var destination = texture();
            uint zero = 0, one = 1;
            float4 value = float4(1, 0, 0, 1);
            hgbc_asm {
                OpClearTexture $$RWTexture2D<float4> $source $zero
                    $zero $one $zero $one $value;
                OpBlitTexture2D $$RWTexture2D<float4> $$RWTexture2D<float4>
                    $destination $source;
            };
        }
    )";

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
/// Exercises query-page boundaries and reuse independently of shader compilation and execution.
void testExecutionQueryPools(rhi::IDevice* device)
{
    SLANG_CHECK_ABORT(device->hasFeature(rhi::Feature::TimestampQuery));
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    EventTracker tracker(true, sink, device);
    auto queue = device->getQueue(rhi::QueueType::Graphics);
    Dictionary<const void*, bool> previousPools;
    uint64_t nextId = 1;
    for (uint64_t batchId = 1; batchId <= 3; ++batchId)
    {
        DeviceExecutionBatch batch;
        auto encoder = queue->createCommandEncoder();
        SLANG_CHECK_ABORT(encoder);
        const uint32_t executionCount = 140;
        uint64_t firstId = nextId;
        for (uint32_t i = 0; i < executionCount; ++i)
        {
            if (i % 2)
            {
                tracker.beginAccelerationStructureBuild(
                    batch,
                    encoder,
                    batchId,
                    &tracker,
                    rhi::AccelerationStructureKind::BottomLevel);
                SLANG_CHECK(
                    sink->events.getLast().accelerationStructureBuild.executionId == nextId++);
            }
            else
            {
                tracker.beginPipelineExecution(
                    batch,
                    encoder,
                    batchId,
                    &tracker,
                    TrackedPipelineKind::Compute);
                tracker.trackDispatch(batch, batchId, &tracker, 1, 1, 1);
                SLANG_CHECK(sink->events.getLast().dispatch.executionId == nextId++);
            }
            tracker.endDeviceExecution(batch, encoder);
        }
        SLANG_CHECK(batch.queryPools.getCount() > 1);
        for (const auto& pool : batch.queryPools)
        {
            SLANG_CHECK_ABORT(pool);
            if (batchId > 1)
                SLANG_CHECK(previousPools.containsKey(pool.get()));
            previousPools[pool.get()] = true;
        }
        auto commandBuffer = encoder->finish();
        SLANG_CHECK_ABORT(commandBuffer);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(queue->submit(commandBuffer)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(queue->waitOnHost()));
        Index firstCompletion = sink->events.getCount();
        tracker.completeDeviceExecutions(batch);
        SLANG_CHECK(sink->events.getCount() == firstCompletion + executionCount);
        for (uint32_t i = 0; i < executionCount; ++i)
        {
            const auto& event = sink->events[firstCompletion + i];
            if (i % 2)
            {
                SLANG_CHECK(event.kind == TrackingEventKind::AccelerationStructureBuildComplete);
                SLANG_CHECK(event.accelerationStructureBuild.executionId == firstId + i);
                SLANG_CHECK(event.accelerationStructureBuild.batchId == batchId);
                SLANG_CHECK(event.accelerationStructureBuild.accelerationStructure == &tracker);
                SLANG_CHECK(event.accelerationStructureBuild.hasDuration);
            }
            else
            {
                SLANG_CHECK(event.kind == TrackingEventKind::PipelineExecutionComplete);
                SLANG_CHECK(event.pipelineExecution.executionId == firstId + i);
                SLANG_CHECK(event.pipelineExecution.batchId == batchId);
                SLANG_CHECK(event.pipelineExecution.pipeline == &tracker);
                SLANG_CHECK(event.pipelineExecution.hasDuration);
            }
        }
        SLANG_CHECK(batch.executions.getCount() == 0 && batch.queryPools.getCount() == 0);
    }
    SLANG_CHECK(sink->valid);
}
#endif

void testInterpreterEvents(UnitTestContext* context, rhi::DeviceType api)
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
    ComPtr<rhi::IDevice> device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    testExecutionQueryPools(device);
#endif
    auto program = compileHGVMTestProgram(
        context->slangGlobalSession,
        kEventTestSource,
        SLANG_DEBUG_INFO_LEVEL_MINIMAL);
    for (bool enabled : {false, true})
    {
        RefPtr<RecordingEventSink> sink = new RecordingEventSink;
        RefPtr<EventTestOutput> output = new EventTestOutput;
        Desc desc{};
        desc.device = device;
        desc.enableEventTracking = enabled;
        desc.eventSink = sink;
        desc.outputSink = output;
        {
            Interpreter interpreter(desc);
            String error;
            for (int i = 0; i < 2; ++i)
            {
                SlangResult result = interpreter.call(program, "main", error);
                if (SLANG_FAILED(result))
                    fprintf(stderr, "%s\n", error.getBuffer());
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            }
            SLANG_CHECK(interpreter.getBufferPoolStats().hits >= 1);
        }
        SLANG_CHECK(output->text == "34\n34\n");
        SLANG_CHECK(sink->valid);
        SLANG_CHECK(sink->heap.getCount() == 0 && sink->payloads.getCount() == 0);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        if (enabled)
        {
            SLANG_CHECK(sink->count(TrackingEventKind::HeapAlloc) > 0);
            // Only the two runtime arrays have separate storage; classes include their fields.
            SLANG_CHECK(sink->count(TrackingEventKind::PayloadAlloc) == 2);
            SLANG_CHECK(sink->count(TrackingEventKind::ModuleLoad) == 2);
            SLANG_CHECK(sink->count(TrackingEventKind::PipelineCreation) == 2);
            SLANG_CHECK(sink->count(TrackingEventKind::Dispatch) == 4);
            SLANG_CHECK(sink->count(TrackingEventKind::TextureCreation) == 4);
            SLANG_CHECK(sink->count(TrackingEventKind::Draw) == 2);
            SLANG_CHECK(sink->count(TrackingEventKind::Blit) == 2);
            SLANG_CHECK(sink->count(TrackingEventKind::Clear) == 2);
            SLANG_CHECK(sink->count(TrackingEventKind::BufferReuse) >= 1);
            SLANG_CHECK(sink->count(TrackingEventKind::BufferEviction) >= 1);
            Dictionary<uint64_t, bool> batches;
            Dictionary<uint64_t, const void*> executions;
            Dictionary<uint64_t, EventOrigin> origins;
            bool sawClassType = false;
            Index completionCount = 0;
            for (const auto& event : sink->events)
            {
                if (event.kind == TrackingEventKind::HeapAlloc ||
                    event.kind == TrackingEventKind::HeapFree)
                {
                    SLANG_CHECK_ABORT(
                        sink->debugMaps.containsKey(event.allocationOrigin.programId));
                    auto type = sink->debugMaps[event.allocationOrigin.programId]->findType(
                        event.heap.typeId);
                    SLANG_CHECK_ABORT(type);
                    switch (event.heap.objectKind)
                    {
                    case HeapObjectKind::Class:
                        SLANG_CHECK(type->opcode == HGBCOpcode::ClassType);
                        break;
                    case HeapObjectKind::RuntimeArray:
                        SLANG_CHECK(type->opcode == HGBCOpcode::RuntimeArrayType);
                        break;
                    case HeapObjectKind::StructuredBuffer:
                        SLANG_CHECK(type->opcode == HGBCOpcode::RWStructuredBufferType);
                        break;
                    case HeapObjectKind::Texture2D:
                        SLANG_CHECK(type->opcode == HGBCOpcode::Texture2DType);
                        break;
                    case HeapObjectKind::String:
                        SLANG_CHECK(type->opcode == HGBCOpcode::StringType);
                        break;
                    case HeapObjectKind::DeferredReadback:
                        SLANG_CHECK(type->opcode == HGBCOpcode::DeferType);
                        break;
                    default:
                        SLANG_CHECK(false);
                        break;
                    }
                }
                if ((event.kind == TrackingEventKind::HeapAlloc ||
                     event.kind == TrackingEventKind::HeapFree) &&
                    event.heap.objectKind == HeapObjectKind::Class)
                {
                    SLANG_CHECK_ABORT(
                        sink->debugMaps.containsKey(event.allocationOrigin.programId));
                    auto type = sink->debugMaps[event.allocationOrigin.programId]->findType(
                        event.heap.typeId);
                    SLANG_CHECK_ABORT(type);
                    SLANG_CHECK(type->qualifiedName == "Node");
                    SLANG_CHECK(type->moduleName == "hgvm_events");
                    SLANG_CHECK(event.allocationOrigin.recordWordOffset != UINT32_MAX);
                    sawClassType = true;
                }
                if (event.kind == TrackingEventKind::Draw ||
                    event.kind == TrackingEventKind::Dispatch)
                {
                    auto location = sink->findSourceLocation(event.origin);
                    if (event.kind == TrackingEventKind::Dispatch)
                        SLANG_CHECK_ABORT(location);
                    if (location)
                        SLANG_CHECK(location->line && location->path.endsWith("hgvm_events.slang"));
                    uint64_t id = event.kind == TrackingEventKind::Draw
                                      ? event.draw.executionId
                                      : event.dispatch.executionId;
                    origins.add(id, event.origin);
                }
                if (event.kind == TrackingEventKind::PipelineExecutionComplete)
                {
                    auto origin = origins.tryGetValue(event.pipelineExecution.executionId);
                    SLANG_CHECK_ABORT(origin);
                    SLANG_CHECK(origin->programId == event.origin.programId);
                    SLANG_CHECK(origin->recordWordOffset == event.origin.recordWordOffset);
                    SLANG_CHECK(
                        sink->findSourceLocation(*origin) ==
                        sink->findSourceLocation(event.origin));
                }
                if (event.kind == TrackingEventKind::CommandBatchBegin)
                    batches[event.batch.id] = false;
                else if (event.kind == TrackingEventKind::Dispatch)
                {
                    auto* submitted = batches.tryGetValue(event.dispatch.batchId);
                    SLANG_CHECK(submitted && !*submitted);
                    SLANG_CHECK(!executions.containsKey(event.dispatch.executionId));
                    executions.add(event.dispatch.executionId, event.dispatch.pipeline);
                }
                else if (event.kind == TrackingEventKind::Draw)
                {
                    auto* submitted = batches.tryGetValue(event.draw.batchId);
                    SLANG_CHECK(submitted && !*submitted);
                    SLANG_CHECK(event.draw.count == 3 && !event.draw.indexed);
                    SLANG_CHECK(!executions.containsKey(event.draw.executionId));
                    executions.add(event.draw.executionId, event.draw.pipeline);
                }
                else if (event.kind == TrackingEventKind::CommandBufferSubmit)
                {
                    SLANG_CHECK(batches.containsKey(event.batch.id));
                    SLANG_CHECK(SLANG_SUCCEEDED(event.batch.result));
                    batches[event.batch.id] = true;
                }
                else if (event.kind == TrackingEventKind::PipelineExecutionComplete)
                {
                    const auto& completion = event.pipelineExecution;
                    auto pipeline = executions.tryGetValue(completion.executionId);
                    SLANG_CHECK(pipeline && *pipeline == completion.pipeline);
                    auto submitted = batches.tryGetValue(completion.batchId);
                    SLANG_CHECK(submitted && *submitted);
                    SLANG_CHECK(completion.hasDuration);
                    executions.remove(completion.executionId);
                    ++completionCount;
                }
            }
            SLANG_CHECK(batches.getCount() > 0);
            for (const auto& batch : batches)
                SLANG_CHECK(batch.second);
            SLANG_CHECK(completionCount == 6 && executions.getCount() == 0);
            SLANG_CHECK(sawClassType);
        }
        else
#endif
            SLANG_CHECK(sink->events.getCount() == 0);
    }
}
} // namespace

SLANG_UNIT_TEST(hgvmEventsClassStorage)
{
    const char* source = R"(
        class Empty { __init() {} }
        struct Fields { bool enabled; Empty owner; uint value; }
        class Holder
        {
            Fields first;
            Fields second;
            uint values[3];
            __init() {}
        }
        [noinline] Holder makeHolder()
        {
            var result = new Holder();
            var owner = new Empty();
            result.first.owner = owner;
            result.second.owner = owner;
            return result;
        }
        [host] void main()
        {
            var holder = makeHolder();
            print(holder.first.value);
            let alias = holder;
            alias.first.enabled = true;
            alias.first.value = 12345;
            alias.values[2] = 77;
            holder = new Holder();
            print(uint(alias.first.enabled));
            print(alias.first.value);
            print(alias.values[2]);
            print(holder.values[2]);
        }
    )";
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, source);
    for (bool enabled : {false, true})
    {
        RefPtr<RecordingEventSink> sink = new RecordingEventSink;
        RefPtr<EventTestOutput> output = new EventTestOutput;
        Desc desc{};
        desc.enableEventTracking = enabled;
        desc.eventSink = sink;
        desc.outputSink = output;
        {
            Interpreter interpreter(desc);
            for (int i = 0; i < 2; ++i)
            {
                String error;
                SlangResult result = interpreter.call(program, "main", error);
                if (SLANG_FAILED(result))
                    fprintf(stderr, "%s\n", error.getBuffer());
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            }
        }
        SLANG_CHECK(output->text == "0\n1\n12345\n77\n0\n0\n1\n12345\n77\n0\n");
        SLANG_CHECK(sink->valid && sink->heap.getCount() == 0 && sink->payloads.getCount() == 0);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        if (enabled)
        {
            // An empty class still has an object header. Each Holder adds two 24-byte Fields,
            // a 12-byte array, two four-byte cleanup offsets, and four bytes of tail padding.
            List<uint64_t> sizes;
            for (const auto& event : sink->events)
            {
                if (event.kind == TrackingEventKind::HeapAlloc)
                {
                    SLANG_CHECK(event.heap.objectKind == HeapObjectKind::Class);
                    SLANG_CHECK(uintptr_t(event.heap.address) % alignof(uint64_t) == 0);
                    sizes.add(event.heap.byteSize);
                }
            }
            SLANG_CHECK_ABORT(sizes.getCount() == 6);
            for (Index i = 0; i < sizes.getCount(); i += 3)
            {
                SLANG_CHECK(sizes[i + 1] >= sizeof(HeapObject));
                SLANG_CHECK(sizes[i] == sizes[i + 1] + 72);
                SLANG_CHECK(sizes[i + 2] == sizes[i]);
            }
            SLANG_CHECK(sink->count(TrackingEventKind::HeapFree) == sizes.getCount());
            SLANG_CHECK(sink->count(TrackingEventKind::PayloadAlloc) == 0);
            SLANG_CHECK(sink->count(TrackingEventKind::PayloadFree) == 0);
        }
        else
#endif
        {
            SLANG_CHECK(sink->events.getCount() == 0);
        }
    }
}

SLANG_UNIT_TEST(hgvmEventsClassFailureCleanup)
{
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        class Node { Node next; __init() {} }
        [host] void main()
        {
            var root = new Node();
            root.next = new Node();
            String message = "class cleanup after failure";
            hgbc_asm { OpFail $message; };
        }
    )");
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    Desc desc{};
    desc.enableEventTracking = true;
    desc.eventSink = sink;
    {
        Interpreter interpreter(desc);
        String error;
        SLANG_CHECK(SLANG_FAILED(interpreter.call(program, "main", error)));
        SLANG_CHECK(error.contains("class cleanup after failure"));
        SLANG_CHECK(sink->valid && sink->heap.getCount() == 0 && sink->payloads.getCount() == 0);
    }
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    SLANG_CHECK(sink->count(TrackingEventKind::HeapAlloc) == 2);
    SLANG_CHECK(sink->count(TrackingEventKind::HeapFree) == 2);
    SLANG_CHECK(sink->count(TrackingEventKind::PayloadAlloc) == 0);
#else
    SLANG_CHECK(sink->events.getCount() == 0);
#endif
}

SLANG_UNIT_TEST(hgvmRayRegistryOwnership)
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

    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
        import slang.pipeline_rt;

        [raypayload] struct Payload
        {
            uint value : read(caller, closesthit, miss) : write(caller, closesthit, miss);
        }
        [shader("miss")]
        void miss<F : IFunc<Payload, Payload>>(inout Payload payload, uniform F callback)
        {
            payload = callback(payload);
        }
        [shader("closesthit")]
        void hit<F : IFunc<Payload, Payload>>(
            inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes,
            uniform F callback)
        {
            payload = callback(payload);
        }
        [host] void registerShaders<F : IFunc<Payload, Payload>>(
            RaytracingPipeline tracker, uint geometryCount, F callback)
        {
            __setRayMiss(tracker, 0, miss<F>)(callback);
            for (uint geometry = 0; geometry < geometryCount; ++geometry)
                __setRayHitShader(tracker, geometry, 0, hit<F>)(callback);
        }
        [host] [noinline] void install(RaytracingPipeline tracker, uint geometryCount)
        {
            let buffer = RWStructuredBuffer<uint>::alloc(1);
            print("allocated");
            registerShaders(tracker, geometryCount, (Payload payload) =>
            {
                payload.value = buffer[0] + getBindingOffset(tracker);
                return payload;
            });
        }
        [host] [noinline] void replace(RaytracingPipeline tracker, uint geometryCount)
        {
            registerShaders(tracker, geometryCount, (Payload payload) =>
            {
                payload.value = 42 + getBindingOffset(tracker);
                return payload;
            });
        }
        [host] void main()
        {
            let tracker = makeRaytracingPipeline(1);
            replace(tracker, 3);
            print("empty");
            install(tracker, 3);
            print("retained");
            replace(tracker, 3);
            print("replaced");
        }
    )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_ray_registry_ownership");

    struct LifetimeOutput : IHGVMOutputSink
    {
        RecordingEventSink* sink = nullptr;
        uint32_t checkpoints = 0;
        uint32_t moduleLoadsAfterAllocation = 0;
        void writeLine(std::string_view line) override
        {
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
            SLANG_CHECK(
                sink->heap.getCount() == (line == "retained" || line == "allocated" ? 2 : 1));
            if (line == "empty")
            {
                SLANG_CHECK(sink->count(TrackingEventKind::ModuleLoad) == 0);
            }
            else if (line == "allocated")
            {
                moduleLoadsAfterAllocation = sink->count(TrackingEventKind::ModuleLoad);
            }
            else
            {
                SLANG_CHECK(
                    sink->count(TrackingEventKind::ModuleLoad) == moduleLoadsAfterAllocation);
            }
#endif
            ++checkpoints;
        }
    };
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    RefPtr<LifetimeOutput> output = new LifetimeOutput;
    output->sink = sink;
    Desc desc{};
    desc.device = device;
    desc.enableEventTracking = true;
    desc.eventSink = sink;
    desc.outputSink = output;
    {
        Interpreter interpreter(desc);
        String error;
        SlangResult result = interpreter.call(program, "main", error);
        if (SLANG_FAILED(result))
            fprintf(stderr, "%s\n", error.getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    }
    SLANG_CHECK(output->checkpoints == 4);
    SLANG_CHECK(sink->valid && sink->heap.getCount() == 0);
}

SLANG_UNIT_TEST(hgvmRayRegistryInvalidArguments)
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

    const char* declarations = R"(
        import slang.pipeline_rt;

        [raypayload] struct Payload
        {
            uint value : read(caller, miss) : write(caller, miss);
        }
        [shader("miss")]
        void miss<F : IFunc<Payload, Payload>>(inout Payload payload, uniform F callback)
        {
            payload = callback(payload);
        }
        [host] void onMiss<F : IFunc<Payload, Payload>>(
            RaytracingPipeline tracker, uint slot, F callback)
        {
            __setRayMiss(tracker, slot, miss<F>)(callback);
        }
    )";
    struct TestCase
    {
        const char* body;
        const char* expectedError;
        uint32_t expectedAllocations;
    };
    const TestCase cases[] = {
        {R"(
            let tracker = makeRaytracingPipeline(0);
            onMiss(tracker, 0, (Payload payload) => { return payload; });
        )",
         "between one and fifteen ray slots",
         0},
        {R"(
            let tracker = makeRaytracingPipeline(16);
            onMiss(tracker, 0, (Payload payload) => { return payload; });
        )",
         "between one and fifteen ray slots",
         0},
        {R"(
            let tracker = hgbc_asm {
                result : $$RaytracingPipeline = OpNullLit;
            };
            onMiss(tracker, 0, (Payload payload) => { return payload; });
        )",
         "cannot register a shader on a null ray tracker",
         0},
        {R"(
            let tracker = makeRaytracingPipeline(1);
            onMiss(tracker, 1, (Payload payload) => { return payload; });
        )",
         "out-of-range ray slot",
         1},
        {R"(
            let tracker = makeRaytracingPipeline(1);
            let other = makeRaytracingPipeline(1);
            let buffer = RWStructuredBuffer<uint>::alloc(1);
            onMiss(tracker, 0, (Payload payload) =>
            {
                payload.value = buffer[0] + getBindingOffset(tracker);
                return payload;
            });
            onMiss(tracker, 0, (Payload payload) =>
            {
                payload.value = buffer[0] + getBindingOffset(other);
                return payload;
            });
        )",
         "a callback captured another tracker",
         3},
    };
    for (const auto& testCase : cases)
    {
        StringBuilder source;
        source << declarations << "[host] void main() {" << testCase.body << "}";
        auto program = compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_NONE,
            "hgvm_ray_registry_invalid_arguments");
        RefPtr<RecordingEventSink> sink = new RecordingEventSink;
        Desc desc{};
        desc.device = device;
        desc.enableEventTracking = true;
        desc.eventSink = sink;
        {
            Interpreter interpreter(desc);
            String error;
            SlangResult result = interpreter.call(program, "main", error);
            SLANG_CHECK(SLANG_FAILED(result));
            if (!error.contains(testCase.expectedError))
                fprintf(stderr, "%s\n", error.getBuffer());
            SLANG_CHECK(error.contains(testCase.expectedError));
        }
        // The foreign-tracker case first installs a valid registration. Failure while copying
        // its replacement must release the partially copied argument and the old registration
        // during normal invocation cleanup, including their shared buffer and self references.
        SLANG_CHECK(sink->valid && sink->heap.getCount() == 0 && sink->payloads.getCount() == 0);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
        SLANG_CHECK(sink->count(TrackingEventKind::HeapAlloc) == testCase.expectedAllocations);
        SLANG_CHECK(sink->count(TrackingEventKind::HeapFree) == testCase.expectedAllocations);
#endif
    }
}

SLANG_UNIT_TEST(hgvmRayDispatchInvalidArguments)
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

        [raypayload] struct Payload
        {
            uint value : read(caller, closesthit, miss) : write(caller, closesthit, miss);
        }
        [shader("miss")]
        void miss(inout Payload payload) { payload.value = 1; }
        [shader("raygeneration")]
        void noScene(uniform RaytracingPipeline tracker,
            uniform RWStructuredBuffer<uint>.Handle output)
        {
            output[0] = getBindingOffset(tracker);
        }
        [shader("raygeneration")]
        void twoTrackers(uniform RaytracingPipeline first, uniform RaytracingPipeline second,
            uniform RWStructuredBuffer<uint>.Handle output)
        {
            output[0] = getBindingOffset(first) + getBindingOffset(second);
        }
        [shader("raygeneration")]
        void rawResource(uniform RWStructuredBuffer<uint> output)
        {
            output[0] = 1;
        }
        [shader("raygeneration")]
        void withScene(uniform RaytracingPipeline tracker,
            uniform RaytracingAccelerationStructure.Handle scene,
            uniform RWStructuredBuffer<uint>.Handle output)
        {
            RayDesc ray;
            ray.Origin = float3(0, 0, -1);
            ray.Direction = float3(0, 0, 1);
            ray.TMin = 0.001;
            ray.TMax = 10;
            Payload payload = { 0 };
            TraceRay(*scene, 0, 255, getBindingOffset(tracker), 1,
                getBindingOffset(tracker), ray, payload);
            output[0] = payload.value;
        }
        [host]
        RaytracingAccelerationStructure makeScene()
        {
            let vertices = RuntimeArray<float3>::alloc(3);
            vertices[0] = float3(-1, -1, 0);
            vertices[1] = float3(1, -1, 0);
            vertices[2] = float3(0, 1, 0);
            let vertexBuffer = StructuredBuffer<float3>::alloc(3);
            copy(vertexBuffer, 0, vertices, 0, 3);
            let indices = RuntimeArray<uint>::alloc(3);
            indices[0] = 0; indices[1] = 1; indices[2] = 2;
            let indexBuffer = StructuredBuffer<uint>::alloc(3);
            copy(indexBuffer, 0, indices, 0, 3);
            HWRaytracingContext<>.TriangleGeometry geometry = HWRaytracingContext<>.TriangleGeometry(vertexBuffer, indexBuffer);
            geometry.vertexCount = geometry.indexCount = 3;
            let geometries = RuntimeArray<HWRaytracingContext<>.TriangleGeometry>::alloc(1);
            geometries[0] = geometry;
            let blas = HWRaytracingContext<>.BottomLevelAccelerationStructure::alloc();
            blas.build(geometries);
            HWRaytracingContext<>.Instance instance = HWRaytracingContext<>.Instance(blas);
            let instances = RuntimeArray<HWRaytracingContext<>.Instance>::alloc(1);
            instances[0] = instance;
            let scene = RaytracingAccelerationStructure::alloc();
            scene.build(instances);
            return scene;
        }
    )";
    struct TestCase
    {
        const char* body;
        const char* expectedError;
    };
    const TestCase cases[] = {
        {R"(
            let tracker = makeRaytracingPipeline(1);
            let output = RWStructuredBuffer<uint>::alloc(1);
            __dispatchRays(noScene, uint3(1, 1, 1), 1)(tracker, output.getDescriptorHandle());
        )",
         "missing a miss registration"},
        {R"(
            let first = makeRaytracingPipeline(1);
            let second = makeRaytracingPipeline(1);
            __setRayMiss(first, 0, miss)();
            __setRayMiss(second, 0, miss)();
            let output = RWStructuredBuffer<uint>::alloc(1);
            __dispatchRays(twoTrackers, uint3(1, 1, 1), 1)(
                first, second, output.getDescriptorHandle());
        )",
         "supports exactly one captured tracker"},
        {R"(
            let output = RWStructuredBuffer<uint>::alloc(1);
            __dispatchRays(rawResource, uint3(1, 1, 1), 1)(output);
        )",
         "requires resource slots; use getDescriptorHandle()"},
        {R"(
            let tracker = makeRaytracingPipeline(1);
            __setRayMiss(tracker, 0, miss)();
            let scene = makeScene();
            let output = RWStructuredBuffer<uint>::alloc(1);
            __dispatchRays(withScene, uint3(1, 1, 1), 1)(
                tracker, scene.getDescriptorHandle(), output.getDescriptorHandle());
        )",
         "missing a geometry/ray hit registration"},
    };
    for (const auto& testCase : cases)
    {
        StringBuilder source;
        source << declarations << "[host] void main() {" << testCase.body << "}";
        auto program = compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_NONE,
            "hgvm_ray_dispatch_invalid_arguments");
        RefPtr<RecordingEventSink> sink = new RecordingEventSink;
        Desc desc{};
        desc.device = device;
        desc.enableEventTracking = true;
        desc.eventSink = sink;
        {
            Interpreter interpreter(desc);
            String error;
            SlangResult result = interpreter.call(program, "main", error);
            SLANG_CHECK(SLANG_FAILED(result));
            if (!error.contains(testCase.expectedError))
                fprintf(stderr, "%s\n", error.getBuffer());
            SLANG_CHECK(error.contains(testCase.expectedError));
        }
        // Failed assembly can follow successful uploads/AS builds. Teardown must release both
        // the invocation's managed objects and any resources retained by its pending batch.
        SLANG_CHECK(sink->valid && sink->heap.getCount() == 0 && sink->payloads.getCount() == 0);
    }
}

SLANG_UNIT_TEST(hgvmRayDispatchPipelineReuse)
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

    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
        import slang.pipeline_rt;

        [raypayload] struct Payload
        {
            uint value : read(caller, miss) : write(caller, miss);
        }
        [shader("miss")]
        void miss(inout Payload payload, uniform uint value) { payload.value = value; }
        [shader("raygeneration")]
        void generate(uniform RaytracingPipeline tracker,
            uniform RWStructuredBuffer<uint>.Handle output, uniform uint value)
        {
            output[DispatchRaysIndex().x] = getBindingOffset(tracker) + value;
        }
        [host] void main()
        {
            let tracker = makeRaytracingPipeline(1);
            let first = RWStructuredBuffer<uint>::alloc(1);
            let second = RWStructuredBuffer<uint>::alloc(2);
            __setRayMiss(tracker, 0, miss)(5);
            __dispatchRays(generate, uint3(1, 1, 1), 1)(tracker, first.getDescriptorHandle(), 7);
            __setRayMiss(tracker, 0, miss)(17);
            __dispatchRays(generate, uint3(2, 1, 1), 1)(tracker, second.getDescriptorHandle(), 23);
            print(first.readBack(0));
            print(second.readBack(0));
            print(second.readBack(1));
        }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_ray_dispatch_pipeline_reuse");
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    RefPtr<EventTestOutput> output = new EventTestOutput;
    Desc desc{};
    desc.device = device;
    desc.enableEventTracking = true;
    desc.eventSink = sink;
    desc.outputSink = output;
    {
        Interpreter interpreter(desc);
        String error;
        SlangResult result = interpreter.call(program, "main", error);
        if (SLANG_FAILED(result))
            fprintf(stderr, "%s\n", error.getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    }
    SLANG_CHECK(output->text == "7\n23\n23\n");
    SLANG_CHECK(sink->valid && sink->heap.getCount() == 0 && sink->payloads.getCount() == 0);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    uint32_t rayPipelines = 0;
    for (const auto& event : sink->events)
    {
        if (event.kind == TrackingEventKind::PipelineCreation &&
            event.pipeline.kind == TrackedPipelineKind::Raytracing)
            ++rayPipelines;
    }
    // Captured record values, descriptor identities, and launch extent are deliberately absent
    // from the native code key. The second dispatch must reuse the first native pipeline.
    SLANG_CHECK(rayPipelines == 1);
#endif
}

SLANG_UNIT_TEST(hgvmEventsCompileOut)
{
    int evaluated = 0;
    EventTracker* tracker = nullptr;
    SLANG_HGVM_TRACK((++evaluated, tracker), trackPayloadFree((++evaluated, nullptr)));
    SLANG_HGVM_BEGIN_TIMING((++evaluated, tracker), start);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    SLANG_CHECK(evaluated == 2 && start == 0);
#else
    SLANG_CHECK(evaluated == 0);
#endif
}

SLANG_UNIT_TEST(hgvmEventsDisabled)
{
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    EventTracker disabled(false, sink);
    DeviceExecutionBatch executions;
    disabled.beginPipelineExecution(executions, nullptr, 0, nullptr, TrackedPipelineKind::Compute);
    disabled.beginAccelerationStructureBuild(
        executions,
        nullptr,
        0,
        nullptr,
        rhi::AccelerationStructureKind::BottomLevel);
    disabled.endDeviceExecution(executions, nullptr);
    disabled.completeDeviceExecutions(executions);
    SLANG_CHECK(executions.executions.getCount() == 0 && executions.queryPools.getCount() == 0);
    uint64_t start = 123;
    disabled.beginOperation(start);
    SLANG_CHECK(start == 123);
    disabled.trackHeapAlloc(nullptr, 8);
    disabled.trackHeapFree(nullptr, 8);
    disabled.trackPayloadAlloc(nullptr, 8, 8);
    disabled.trackPayloadFree(nullptr);
    disabled.trackModuleLoad(nullptr, nullptr, 0, TrackedModuleFormat::IR, SLANG_FAIL, start);
    disabled.trackPipelineCreation(nullptr, TrackedPipelineKind::Compute, SLANG_FAIL, start);
    disabled.trackBufferCreation(nullptr, {}, TrackedBufferPurpose::Program, SLANG_FAIL, start);
    disabled.trackTextureCreation(nullptr, {}, SLANG_FAIL);
    disabled.trackBufferReuse(nullptr, 8);
    disabled.trackBufferRecycle(nullptr, 8);
    disabled.trackBufferEviction(nullptr, 8);
    uint64_t id = 0;
    disabled.trackCommandBatchBegin(id);
    disabled.trackCommandBatchDiscard(id, 0);
    disabled.trackCommandBufferSubmit(id, nullptr, 0, 0, SLANG_FAIL);
    disabled.trackDraw(executions, id, nullptr, 3, 1, false);
    disabled.trackDispatch(executions, id, nullptr, 1, 1, 1);
    disabled.trackBlit(id, nullptr, nullptr);
    disabled.trackClear(id, nullptr);
    SLANG_CHECK(id == 0 && sink->events.getCount() == 0);
    EventTracker noSink(true, nullptr);
    noSink.beginOperation(start);
    SLANG_CHECK(start == 123);
    noSink.trackHeapAlloc(nullptr, 0);
}

SLANG_UNIT_TEST(hgvmEventsDirectCall)
{
    // The event implementation is available even when interpreter instrumentation is compiled out.
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    EventTracker tracker(true, sink);
    uint64_t start = 0;
    tracker.beginOperation(start);
    // A small known delay proves durations cover the operation, including failed attempts.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    tracker.trackPipelineCreation(nullptr, TrackedPipelineKind::Compute, SLANG_FAIL, start);
    SLANG_CHECK_ABORT(sink->events.getCount() == 1);
    SLANG_CHECK(sink->events[0].kind == TrackingEventKind::PipelineCreation);
    SLANG_CHECK(sink->events[0].pipeline.result == SLANG_FAIL);
    SLANG_CHECK(!sink->events[0].pipeline.pipeline);
    tracker
        .trackModuleLoad(nullptr, "failed-module", 0, TrackedModuleFormat::IR, SLANG_FAIL, start);
    tracker.trackBufferCreation(nullptr, {}, TrackedBufferPurpose::Program, SLANG_FAIL, start);
    SLANG_CHECK_ABORT(sink->events.getCount() == 3);
    SLANG_CHECK(sink->events[0].pipeline.durationNanoseconds >= 1000000);
    SLANG_CHECK(sink->events[1].module.durationNanoseconds >= 1000000);
    SLANG_CHECK(sink->events[2].buffer.durationNanoseconds >= 1000000);
    SLANG_CHECK(
        sink->events[0].pipeline.durationNanoseconds <=
        sink->events[0].timestampNanoseconds - start);
    SLANG_CHECK(
        sink->events[1].module.durationNanoseconds <= sink->events[1].timestampNanoseconds - start);
    SLANG_CHECK(
        sink->events[2].buffer.durationNanoseconds <= sink->events[2].timestampNanoseconds - start);
}

SLANG_UNIT_TEST(hgvmEventsCompletionWithoutTimestamps)
{
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    // A tracker without a timestamp device exercises completion independently of query support.
    EventTracker tracker(true, sink);
    DeviceExecutionBatch batch;
    const void* pipeline = &tracker;
    for (uint64_t batchId = 1; batchId <= 2; ++batchId)
    {
        tracker.beginPipelineExecution(
            batch,
            nullptr,
            batchId,
            pipeline,
            TrackedPipelineKind::Compute);
        tracker.trackDispatch(batch, batchId, pipeline, 1, 1, 1);
        uint64_t id = sink->events.getLast().dispatch.executionId;
        SLANG_CHECK(id == batchId);
        tracker.endDeviceExecution(batch, nullptr);
        SLANG_CHECK(batch.queryPools.getCount() == 0);
        tracker.completeDeviceExecutions(batch);
        const auto& event = sink->events.getLast();
        SLANG_CHECK(event.kind == TrackingEventKind::PipelineExecutionComplete);
        SLANG_CHECK(event.pipelineExecution.executionId == id);
        SLANG_CHECK(event.pipelineExecution.pipeline == pipeline);
        SLANG_CHECK(event.pipelineExecution.batchId == batchId);
        SLANG_CHECK(!event.pipelineExecution.hasDuration);
        SLANG_CHECK(batch.executions.getCount() == 0);
    }
    Index count = sink->events.getCount();
    tracker.completeDeviceExecutions(batch);
    SLANG_CHECK(sink->events.getCount() == count);
    tracker.beginPipelineExecution(batch, nullptr, 3, pipeline, TrackedPipelineKind::Compute);
    tracker.trackDispatch(batch, 3, pipeline, 1, 1, 1);
    tracker.endDeviceExecution(batch, nullptr);
    batch.clear(); // Abandoned commands must not acquire a successful completion event.
    tracker.completeDeviceExecutions(batch);
    SLANG_CHECK(sink->events.getCount() == count + 1);
}

SLANG_UNIT_TEST(hgvmEventsAccelerationBuildWithoutTimestamps)
{
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    EventTracker tracker(true, sink);
    DeviceExecutionBatch batch;
    for (auto kind :
         {rhi::AccelerationStructureKind::BottomLevel, rhi::AccelerationStructureKind::TopLevel})
    {
        tracker.beginAccelerationStructureBuild(batch, nullptr, 1, &tracker, kind);
        TrackingEvent recorded = sink->events.getLast();
        SLANG_CHECK(recorded.kind == TrackingEventKind::AccelerationStructureBuild);
        SLANG_CHECK(!recorded.accelerationStructureBuild.hasDuration);
        tracker.endDeviceExecution(batch, nullptr);
        SLANG_CHECK(batch.queryPools.getCount() == 0);
        tracker.completeDeviceExecutions(batch);
        const auto& completed = sink->events.getLast();
        SLANG_CHECK(completed.kind == TrackingEventKind::AccelerationStructureBuildComplete);
        const auto& build = completed.accelerationStructureBuild;
        SLANG_CHECK(build.executionId == recorded.accelerationStructureBuild.executionId);
        SLANG_CHECK(build.batchId == 1 && build.accelerationStructure == &tracker);
        SLANG_CHECK(build.kind == kind && !build.hasDuration && build.durationNanoseconds == 0);
    }
    Index count = sink->events.getCount();
    tracker.beginAccelerationStructureBuild(
        batch,
        nullptr,
        2,
        &tracker,
        rhi::AccelerationStructureKind::BottomLevel);
    tracker.endDeviceExecution(batch, nullptr);
    batch.clear();
    tracker.completeDeviceExecutions(batch);
    SLANG_CHECK(sink->events.getCount() == count + 1);
    SLANG_CHECK(sink->events.getLast().kind == TrackingEventKind::AccelerationStructureBuild);
}

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
SLANG_UNIT_TEST(hgvmEventsInterpreterDebugMaps)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            class Node { __init() {} }
            [host] void main() { let node = new Node(); }
        )",
        SLANG_DEBUG_INFO_LEVEL_MINIMAL);
    auto function = program->findFunctionExport("main");
    SLANG_CHECK_ABORT(function);
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    RefPtr<EventTracker> tracker;
    {
        InterpreterCore::Desc desc;
        desc.enableEventTracking = true;
        desc.eventSink = sink;
        InterpreterCore interpreter(desc);
        SLANG_CHECK(!interpreter.getDebugMap(0));
        String error;
        RefPtr<BoundFunction> bound;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(interpreter.bindFunction(program, function->id, bound, error)));
        Value result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(bound, {}, result, error)));
        SLANG_CHECK_ABORT(sink->events.getCount());
        auto origin = sink->events[0].origin;
        auto map = interpreter.getDebugMap(origin.programId);
        SLANG_CHECK_ABORT(map);
        SLANG_CHECK(map->findRecordType(origin.recordWordOffset));
        tracker = const_cast<EventTracker*>(sink->events[0].tracker);
        SLANG_CHECK(tracker->getInterpreter() == &interpreter);

        // Programs remain staged until close, so a release request preserves old event maps
        // and existing function bindings rather than creating a second staging identity.
        interpreter.releaseProgram(program);
        SLANG_CHECK(interpreter.getDebugMap(origin.programId) == map);
        Index nextEvent = sink->events.getCount();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(bound, {}, result, error)));
        SLANG_CHECK_ABORT(sink->events.getCount() > nextEvent);
        auto nextOrigin = sink->events[nextEvent].origin;
        SLANG_CHECK(nextOrigin.programId == origin.programId);
        SLANG_CHECK(interpreter.getDebugMap(nextOrigin.programId) == map);
        SLANG_CHECK(interpreter.getDebugMap(origin.programId) == map);
    }
    SLANG_CHECK(!tracker->getInterpreter());
    // A late free still reaches the receiver without borrowing the destroyed interpreter.
    RefPtr<HeapObject> object = new HeapObject(HeapObjectKind::Timer);
    tracker->trackHeapAlloc(object, sizeof(HeapObject));
    object = nullptr;
    SLANG_CHECK(sink->events.getLast().kind == TrackingEventKind::HeapFree);
    SLANG_CHECK(sink->valid && sink->heap.getCount() == 0);
}

SLANG_UNIT_TEST(hgvmEventsLifetimeAndFailure)
{
    bool destroyed = false;
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    sink->destroyed = &destroyed;
    RefPtr<EventTracker> tracker = new EventTracker(true, sink);
    RefPtr<HeapObject> object = new HeapObject(HeapObjectKind::Timer);
    tracker->trackHeapAlloc(object, sizeof(HeapObject));
    uint64_t start = 0;
    tracker->beginOperation(start);
    tracker->trackPipelineCreation(nullptr, TrackedPipelineKind::Compute, SLANG_FAIL, start);
    SLANG_CHECK(sink->events.getLast().pipeline.result == SLANG_FAIL);
    SLANG_CHECK(!sink->events.getLast().pipeline.pipeline);
    tracker = nullptr;
    SLANG_CHECK(!destroyed);
    object = nullptr;
    SLANG_CHECK(sink->valid && sink->heap.getCount() == 0);
    // A second registration is the sole owner of the tracker and sink at destruction time.
    tracker = new EventTracker(true, sink);
    sink->events.clear();
    object = new HeapObject(HeapObjectKind::Timer);
    tracker->trackHeapAlloc(object, sizeof(HeapObject));
    tracker = nullptr;
    sink = nullptr;
    SLANG_CHECK(!destroyed);
    object = nullptr;
    SLANG_CHECK(destroyed);
}
#endif

SLANG_UNIT_TEST(hgvmEventsPipelineNames)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            [shader("compute")] [numthreads(1, 1, 1)]
            void work<T, let count : int>() {}
            [host] void main()
            {
                dispatch(work<uint, 4>, uint3(1, 1, 1))();
                dispatch(work<float, 8>, uint3(1, 1, 1))();
            }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "pipeline_names");
    DebugMapBuilder builder(program);
    uint32_t offset = 0;
    bool sawUInt = false, sawFloat = false;
    for (const auto& record : program->getDecodedProgram().records)
    {
        if (record.opcode != HGBCOpcode::MakeDispatcher)
            continue;
        builder.setPipelineName(offset, record);
        auto name = builder.getDebugMap()->findPipelineName(offset);
        SLANG_CHECK_ABORT(name);
        sawUInt |= *name == "pipeline_names::work<uint, 4>";
        sawFloat |= *name == "pipeline_names::work<float, 8>";
        SLANG_CHECK(!builder.getDebugMap()->findSourceLocation(offset));
        ++offset;
    }
    SLANG_CHECK(sawUInt && sawFloat && offset == 2);
    SLANG_CHECK(!builder.getDebugMap()->findPipelineName(offset));

    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    EventTracker disabled(false, sink);
    int pipelineStorage = 0;
    const void* pipeline = &pipelineStorage;
    EventOrigin origin{builder.getDebugMap()->getProgramId(), 0};
    disabled.trackPipelineCreation(pipeline, TrackedPipelineKind::Compute, SLANG_OK, 0, origin);
    SLANG_CHECK(!disabled.findPipelineOrigin(pipeline));
    SLANG_CHECK(!sink->events.getCount());

    EventTracker tracker(true, sink);
    tracker.trackPipelineCreation(pipeline, TrackedPipelineKind::Compute, SLANG_OK, 0, origin);
    auto recorded = tracker.findPipelineOrigin(pipeline);
    SLANG_CHECK_ABORT(recorded);
    SLANG_CHECK(recorded->programId == origin.programId && recorded->recordWordOffset == 0);
    // A later resource can reuse the handle for a different specialization or an internal blit.
    origin.recordWordOffset = 1;
    tracker.trackPipelineCreation(pipeline, TrackedPipelineKind::Compute, SLANG_OK, 0, origin);
    SLANG_CHECK(tracker.findPipelineOrigin(pipeline)->recordWordOffset == 1);
    tracker.trackPipelineCreation(pipeline, TrackedPipelineKind::Blit, SLANG_OK, 0);
    SLANG_CHECK(!tracker.findPipelineOrigin(pipeline));
    tracker.trackPipelineCreation(nullptr, TrackedPipelineKind::Compute, SLANG_FAIL, 0, origin);
    SLANG_CHECK(!tracker.findPipelineOrigin(nullptr));
}

SLANG_UNIT_TEST(hgvmEventsDebugMap)
{
    RefPtr<DebugMap> debugMap;
    uint32_t firstLocation = UINT32_MAX;
    HGBCId classType = 0;
    {
        auto program = compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            R"(
            class Box<T> { T value; __init(T v) { value = v; } }
            [host] void main()
            {
                let a = new Box<int>(7);
                let b = new Box<float>(2.5);
                print(a.value);
                print(b.value);
            }
        )",
            SLANG_DEBUG_INFO_LEVEL_MINIMAL);
        DebugMapBuilder builder(program);
        debugMap = builder.getDebugMap();
        DebugMapBuilder second(program);
        SLANG_CHECK(debugMap->getProgramId() != second.getDebugMap()->getProgramId());
        bool sawInt = false, sawFloat = false;
        for (const auto& record : program->getDecodedProgram().records)
        {
            if (record.opcode != HGBCOpcode::ClassType)
                continue;
            auto type = debugMap->findType(HGBC::getResultId(record));
            SLANG_CHECK_ABORT(type);
            SLANG_CHECK(type->moduleName == "hgvm_events");
            sawInt |= type->qualifiedName == "Box<int>";
            sawFloat |= type->qualifiedName == "Box<float>";
            classType = type->id;
        }
        SLANG_CHECK(sawInt && sawFloat);
        StagedRecordStorage storage;
        StagedRecordBuilder recordBuilder(storage);
        HGBC::Record debugLine;
        for (const auto& function : program->getDecodedProgram().functions)
        {
            builder.beginFunction(function.id, recordBuilder.getWordOffset());
            for (const auto& block : function.blocks)
                for (auto recordIndex : block.recordIndices)
                {
                    const auto& record = program->getDecodedProgram().records[recordIndex];
                    if (hasHGBCOpcodeFlag(record.opcode, HGBCOpcodeFlags::Debug))
                    {
                        builder.processDebugRecord(record, recordBuilder.getWordOffset());
                        if (record.opcode == HGBCOpcode::DebugLine)
                            debugLine = record;
                        continue;
                    }
                    uint32_t offset = uint32_t(recordBuilder.emitReturn().getUnsignedPayload());
                    uint32_t secondOffset =
                        uint32_t(recordBuilder.emitReturn().getUnsignedPayload());
                    auto location = debugMap->findSourceLocation(offset);
                    SLANG_CHECK(location == debugMap->findSourceLocation(secondOffset));
                    SLANG_CHECK(location == debugMap->findSourceLocation(offset + 1));
                    SLANG_CHECK(debugMap->findRecordType(offset) == 0);
                    if (location)
                    {
                        firstLocation = offset;
                        SLANG_CHECK(location->line && location->column);
                    }
                }
            builder.clearSourceLocation(recordBuilder.getWordOffset());
            uint32_t offset = uint32_t(recordBuilder.emitFinishFrameExit().getUnsignedPayload());
            SLANG_CHECK(!debugMap->findSourceLocation(offset));
        }
        // Multiple directives at the same address replace the range, and a range covers all
        // emitted records up to the next directive. Type entries remain sparse and exact.
        SLANG_CHECK_ABORT(debugLine.opcode == HGBCOpcode::DebugLine);
        uint32_t rangeStart = recordBuilder.getWordOffset();
        builder.processDebugRecord(debugLine, rangeStart);
        debugLine.operands[1] = debugLine.operands[2] = 200;
        builder.processDebugRecord(debugLine, rangeStart);
        uint32_t typedRecord = uint32_t(recordBuilder.emitReturn().getUnsignedPayload());
        uint32_t untypedRecord = uint32_t(recordBuilder.emitReturn().getUnsignedPayload());
        builder.setRecordType(typedRecord, classType);
        SLANG_CHECK(!debugMap->findSourceLocation(rangeStart - 1));
        SLANG_CHECK(debugMap->findSourceLocation(typedRecord)->line == 200);
        SLANG_CHECK(debugMap->findSourceLocation(untypedRecord)->line == 200);
        SLANG_CHECK(debugMap->findRecordType(typedRecord) == classType);
        SLANG_CHECK(!debugMap->findRecordType(typedRecord + 1));
        SLANG_CHECK(!debugMap->findRecordType(untypedRecord));
        HGBC::Record noScope;
        noScope.opcode = HGBCOpcode::DebugNoScope;
        uint32_t rangeEnd = recordBuilder.getWordOffset();
        builder.processDebugRecord(noScope, rangeEnd);
        recordBuilder.emitReturn();
        SLANG_CHECK(debugMap->findSourceLocation(rangeEnd - 1)->line == 200);
        SLANG_CHECK(!debugMap->findSourceLocation(rangeEnd));
        SLANG_CHECK(!debugMap->findSourceLocation(UINT32_MAX));

        // Optional debug records can be incomplete without invalidating execution metadata.
        builder.processDebugRecord(debugLine, recordBuilder.getWordOffset());
        recordBuilder.emitReturn();
        HGBC::Record malformed;
        malformed.opcode = HGBCOpcode::DebugLine;
        builder.processDebugRecord(malformed, recordBuilder.getWordOffset());
        uint32_t offset = uint32_t(recordBuilder.emitReturn().getUnsignedPayload());
        SLANG_CHECK(!debugMap->findSourceLocation(offset));
        recordBuilder.finalize();
    }
    // Both the decoded program and builder are gone; receivers can still decode saved events.
    SLANG_CHECK_ABORT(firstLocation != UINT32_MAX);
    SLANG_CHECK(debugMap->findSourceLocation(firstLocation)->path.endsWith("hgvm_events.slang"));
    SLANG_CHECK(debugMap->findType(classType)->moduleName == "hgvm_events");
    int captures = 0;
    auto capture = [](void* count) -> EventOrigin
    {
        ++*static_cast<int*>(count);
        return {};
    };
    RefPtr<RecordingEventSink> sink = new RecordingEventSink;
    EventTracker disabled(false, sink);
    EventTracker::ContextScope scope(&disabled, capture, &captures);
    disabled.trackClear(1, nullptr);
    SLANG_CHECK(captures == 0 && sink->events.getCount() == 0);

    const char* sameName = R"(
        class Same { __init() {} }
        [host] void main() { let value = new Same(); }
    )";
    auto a = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        sameName,
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "module_a");
    auto b = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        sameName,
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "module_b");
    DebugMapBuilder aBuilder(a), bBuilder(b);
    for (const auto& record : a->getDecodedProgram().records)
    {
        if (record.opcode != HGBCOpcode::ClassType)
            continue;
        HGBCId id = HGBC::getResultId(record);
        auto aType = aBuilder.getDebugMap()->findType(id);
        auto bType = bBuilder.getDebugMap()->findType(id);
        SLANG_CHECK_ABORT(aType && bType);
        SLANG_CHECK(aType->qualifiedName == bType->qualifiedName);
        SLANG_CHECK(aType->moduleName == "module_a" && bType->moduleName == "module_b");
        SLANG_CHECK(
            aBuilder.getDebugMap()->getProgramId() != bBuilder.getDebugMap()->getProgramId());
    }
}

SLANG_UNIT_TEST(hgvmEventsLambdaLocations)
{
    const char* source =
        "[shader(\"compute\")] [numthreads(1,1,1)] void probe<T>() {}\n"
        "[host] void forward<T>(T value) { dispatch(probe<T>, uint3(1,1,1))(); }\n"
        "[host] void make<T : __BuiltinArithmeticType>() { forward(() => T(1)); }\n"
        "[host] void main() { forward(() => 2u); forward(() => 3u); "
        "make<uint>(); make<float>(); }\n";
    for (auto level : {SLANG_DEBUG_INFO_LEVEL_NONE, SLANG_DEBUG_INFO_LEVEL_MINIMAL})
    {
        auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, source, level);
        DebugMapBuilder builder(program);
        auto map = builder.getDebugMap();
        int lambdaArguments = 0;
        HashSet<uint32_t> columns;
        for (const auto& record : program->getDecodedProgram().records)
        {
            // All closure values are dead. Only their reflection strings reach HGBC.
            SLANG_CHECK(record.opcode != HGBCOpcode::StructType);
            if (record.opcode != HGBCOpcode::SpecializeSlangEntryPoint)
                continue;
            auto name = program->findString(record.operands[3]);
            SLANG_CHECK_ABORT(name);
            SLANG_CHECK(name->find("_slang_Lambda_") != std::string::npos);
            ++lambdaArguments;
            auto location = map->findLambdaLocation(name->c_str(), "hgvm_events");
            if (level == SLANG_DEBUG_INFO_LEVEL_NONE)
            {
                SLANG_CHECK(!location);
                continue;
            }
            SLANG_CHECK_ABORT(location);
            SLANG_CHECK(location->path.endsWith("hgvm_events.slang"));
            SLANG_CHECK(location->line == 3 || location->line == 4);
            if (location->line == 4)
                columns.add(location->column);
            SLANG_CHECK(!map->findLambdaLocation(name->c_str(), "another_module"));
        }
        SLANG_CHECK(lambdaArguments == 4);
        if (level != SLANG_DEBUG_INFO_LEVEL_NONE)
        {
            SLANG_CHECK(columns.getCount() == 2);
        }
        else
        {
            SLANG_CHECK(map->getLambdaLocations().getCount() == 0);
        }
    }
}

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
static void testSurfaceWaitSubmissions(UnitTestContext* context, rhi::DeviceType api)
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
    ComPtr<rhi::IDevice> device = rhi::getRHI()->createDevice(deviceDesc);
    SLANG_CHECK_ABORT(device);

    struct Host : IHGVMHostServices
    {
        RefPtr<RecordingEventSink> events;
        ComPtr<rhi::ITexture> image;
        int waits = 0;
        int acquisitions = 0;
        int presentations = 0;

        SlangResult createSurface(
            HGVMHostObject*,
            rhi::Format,
            uint32_t,
            RefPtr<HGVMHostObject>& surface,
            String&) override
        {
            surface = new HGVMHostObject(HGVMHostObjectKind::Surface, this);
            return SLANG_OK;
        }

        SlangResult getSurfaceDimensions(
            HGVMHostObject*,
            uint32_t& width,
            uint32_t& height,
            String&) override
        {
            // Metadata must not turn into an implicit submission or image acquisition.
            SLANG_CHECK(events->count(TrackingEventKind::CommandBufferSubmit) == 0);
            width = height = 2;
            return SLANG_OK;
        }
        SlangResult waitForSurface(HGVMHostObject*, String&) override
        {
            SLANG_CHECK(events->count(TrackingEventKind::CommandBufferSubmit) == 1);
            ++waits;
            return SLANG_OK;
        }
        SlangResult acquireSurfaceTexture(HGVMHostObject*, ComPtr<rhi::ITexture>& texture, String&)
            override
        {
            // Work between wait and acquire must be submitted before presentation sync starts.
            SLANG_CHECK(events->count(TrackingEventKind::CommandBufferSubmit) == 2);
            texture = image;
            ++acquisitions;
            return SLANG_OK;
        }
        SlangResult presentSurface(HGVMHostObject*, String&) override
        {
            SLANG_CHECK(events->count(TrackingEventKind::CommandBufferSubmit) == 3);
            ++presentations;
            return SLANG_OK;
        }
    };
    RefPtr<Host> host = new Host;
    host->events = new RecordingEventSink;
    rhi::TextureDesc textureDesc{};
    textureDesc.type = rhi::TextureType::Texture2D;
    textureDesc.size = {2, 2, 1};
    textureDesc.format = rhi::Format::RGBA8UnormSrgb;
    textureDesc.usage = rhi::TextureUsage::RenderTarget;
    textureDesc.defaultState = rhi::ResourceState::RenderTarget;
    host->image = device->createTexture(textureDesc);
    SLANG_CHECK_ABORT(host->image);
    auto program = compileHGVMTestProgram(context->slangGlobalSession, R"(
        __target_intrinsic(hgbc, "OpWindowType")
        public struct Window {}
        [shader("compute")] [numthreads(1,1,1)]
        void store(uniform RWStructuredBuffer<uint> buffer, uniform uint value)
        {
            buffer[0] = value;
        }
        [host] void main(Window window)
        {
            Surface<43> surface = hgbc_asm {
                result : $$Surface<43> = OpCreateSurface $window $2u;
            };
            let buffer = RWStructuredBuffer<uint>::alloc(1);
            dispatch(store, uint3(1,1,1))(buffer, 12u);
            uint2 size = hgbc_asm { result : $$uint2 = OpGetSurfaceDimensions $surface; };
            bool ready = hgbc_asm { result : $$bool = OpWaitForSurface $surface; };
            if (!ready) return;
            dispatch(store, uint3(1,1,1))(buffer, size.x);
            RenderTarget2D<float4, 43> image = hgbc_asm {
                result : $$RenderTarget2D<float4, 43> = OpAcquireSurfaceTexture $surface;
            };
            dispatch(store, uint3(1,1,1))(buffer, 56u);
            hgbc_asm { OpPresentSurface $surface; };
        }
    )");
    Desc desc{};
    desc.device = device;
    desc.hostServices = host;
    desc.gpuExecution = GPUExecutionMode::Buffered;
    desc.enableEventTracking = true;
    desc.eventSink = host->events;
    Interpreter interpreter(desc);
    RefPtr<BoundFunction> function;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(program, program->findFunctionExport("main")->id, function, error)));
    RefPtr<HGVMHostObject> window = new HGVMHostObject(HGVMHostObjectKind::Window, host);
    Argument arguments[] = {Argument::fromHostObject(window)};
    Value value;
    SlangResult result = (*function)(makeArrayView(arguments), value, error);
    if (SLANG_FAILED(result))
        fprintf(stderr, "%s\n", error.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    SLANG_CHECK(host->waits == 1 && host->acquisitions == 1 && host->presentations == 1);
}

SLANG_UNIT_TEST(hgvmSurfaceWaitSubmissionsVulkan)
{
    testSurfaceWaitSubmissions(unitTestContext, rhi::DeviceType::Vulkan);
}
SLANG_UNIT_TEST(hgvmSurfaceWaitSubmissionsD3D12)
{
    testSurfaceWaitSubmissions(unitTestContext, rhi::DeviceType::D3D12);
}
#endif

SLANG_UNIT_TEST(hgvmEventsVulkan)
{
    testInterpreterEvents(unitTestContext, rhi::DeviceType::Vulkan);
}
SLANG_UNIT_TEST(hgvmEventsD3D12)
{
    testInterpreterEvents(unitTestContext, rhi::DeviceType::D3D12);
}
