#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-debug-map.h"
#include "slang-hgvm/slang-hgvm-debug.h"
#include "slang-hgvm/slang-hgvm-events.h"

using namespace Slang;
using namespace Slang::HGVM;

namespace
{
struct ManagedValueEvents : IEventSink
{
    Dictionary<const void*, bool> heap;
    Dictionary<const void*, bool> payloads;
    bool valid = true;

    void onEvent(const TrackingEvent& event) noexcept override
    {
        switch (event.kind)
        {
        case TrackingEventKind::HeapAlloc:
            valid &= !heap.containsKey(event.heap.address);
            heap[event.heap.address] = true;
            break;
        case TrackingEventKind::HeapFree:
            valid &= heap.containsKey(event.heap.address);
            heap.remove(event.heap.address);
            break;
        case TrackingEventKind::PayloadAlloc:
            valid &= !payloads.containsKey(event.payload.address);
            payloads[event.payload.address] = true;
            break;
        case TrackingEventKind::PayloadFree:
            valid &= payloads.containsKey(event.payload.address);
            payloads.remove(event.payload.address);
            break;
        default:
            break;
        }
    }
};

// Independently compiled entry points share the same nominal type declarations, as separate
// modules importing one source module do. The recursive class is a chain, not an owning cycle.
RefPtr<Program> compileManagedInvocationProgram(slang::IGlobalSession* global, const char* body)
{
    String source = R"(
        class Counter
        {
            Counter next;
            int count;
            __init(int value) { count = value; }
        }
        struct Inner { Counter first; Counter second; String label; }
        struct State { Inner inner; RuntimeArray<Counter> counters; }
        __target_intrinsic(hgbc, "OpTimerType")
        struct Timer {}
        struct Timers { Timer first; Timer second; }
        Timer makeTimer()
        {
            return hgbc_asm { result : $$Timer = OpCreateTimer; };
        }
        Timers makeTimers()
        {
            let timer = makeTimer();
            return Timers(timer, timer);
        }
        int inspectTimer(Timer timer)
        {
            float elapsed = hgbc_asm { result : $$float = OpTimerElapsed $timer; };
            return elapsed >= 0.0 ? 1 : 0;
        }
        String format<each T>(String pattern, expand each T arguments)
        {
            return hgbc_asm {
                result : $$String = OpFormatString $pattern $expand each arguments;
            };
        }
        uint byteCount(String value)
        {
            return hgbc_asm { result : $$uint = OpStringGetByteCount $value; };
        }
        [noinline] State makeState(int seed)
        {
            Counter counter = new Counter(seed);
            counter.next = new Counter(seed + 1);
            RuntimeArray<Counter> counters = RuntimeArray<Counter>::alloc(2);
            counters[0] = counter;
            counters[1] = counter;
            Inner inner = {counter, counter, format("value: {}", seed)};
            return State(inner, counters);
        }
    )";
    source.append(body);
    return compileHGVMTestProgram(
        global,
        source.getBuffer(),
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_managed_invocation");
}
} // namespace

SLANG_UNIT_TEST(hgvmManagedValueInvocation)
{
    auto producer = compileManagedInvocationProgram(
        unitTestContext->slangGlobalSession,
        "[host] State main(int seed) { return makeState(seed); }");
    auto consumer = compileManagedInvocationProgram(unitTestContext->slangGlobalSession, R"(
        [host] int main(State value, int delta)
        {
            value.inner.first.count += delta;
            return value.inner.second.count + value.counters[0].count + value.counters[1].count +
                value.inner.first.next.count + int(byteCount(value.inner.label));
        }
    )");
    auto transform = compileManagedInvocationProgram(unitTestContext->slangGlobalSession, R"(
        [host] State main(State value, int divisor)
        {
            State temporary = makeState(24);
            value.inner.first.count += temporary.inner.first.count / divisor;
            return value;
        }
    )");

    RefPtr<ManagedValueEvents> events = new ManagedValueEvents;
    Desc desc;
    desc.enableEventTracking = true;
    desc.eventSink = events;
    Interpreter interpreter(desc);
    String error;
    RefPtr<BoundFunction> make, inspect, change;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter.bindFunction(producer, producer->findFunctionExport("main")->id, make, error)));
    Argument seed[] = {Argument::scalar(int32_t(7))};
    Value value;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)(makeArrayView(seed), value, error)));
    SLANG_CHECK(value.isValid());
    SLANG_CHECK(value.getType().kind == HostValueType::Kind::Aggregate);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    SLANG_CHECK(events->heap.getCount() == 4); // Two counters, an array, and a dynamic string.
#endif

    // Later staging must preserve both the returned object's identity and its owning metadata.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(consumer, consumer->findFunctionExport("main")->id, inspect, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(transform, transform->findFunctionExport("main")->id, change, error)));
    auto read = [&](const Value& input, int32_t delta = 0)
    {
        Argument arguments[] = {Argument::fromValue(input), Argument::scalar(delta)};
        Value result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*inspect)(makeArrayView(arguments), result, error)));
        int32_t count = 0;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(result.getScalarData(ScalarType::Int32, &count, sizeof(count), error)));
        return count;
    };
    SLANG_CHECK(read(value) == 37);
    const Index liveHeap = events->heap.getCount();
    const Index livePayloads = events->payloads.getCount();
    {
        Value alias = value;
        SLANG_CHECK(read(alias, 5) == 52);
        SLANG_CHECK(read(value) == 52);

        // Replacing the external result also used as input preserves shared class identities.
        Argument arguments[] = {Argument::fromValue(value), Argument::scalar(int32_t(24))};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*change)(makeArrayView(arguments), value, error)));
        SLANG_CHECK(read(value) == 55);
        SLANG_CHECK(read(alias) == 55);
        arguments[0] = Argument::fromValue(value);
        arguments[1] = Argument::scalar(int32_t(0));
        SLANG_CHECK(SLANG_FAILED((*change)(makeArrayView(arguments), value, error)));
        SLANG_CHECK(read(value) == 55);
        SLANG_CHECK(events->heap.getCount() == liveHeap);
    }
    SLANG_CHECK(events->payloads.getCount() == livePayloads);

    // A correct nominal type still cannot carry references into a different VM.
    {
        Interpreter other;
        RefPtr<BoundFunction> foreignConsumer;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(other.bindFunction(
            consumer,
            consumer->findFunctionExport("main")->id,
            foreignConsumer,
            error)));
        Argument arguments[] = {Argument::fromValue(value), Argument::scalar(int32_t(0))};
        Value untouched;
        SLANG_CHECK(SLANG_FAILED((*foreignConsumer)(makeArrayView(arguments), untouched, error)));
        SLANG_CHECK(untouched.isVoid());
    }
    value = Value();
    SLANG_CHECK(events->heap.getCount() == 0);
    SLANG_CHECK(events->payloads.getCount() == 0);
    SLANG_CHECK(events->valid);

    // Close releases managed payloads immediately, even when external wrapper aliases survive.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)(makeArrayView(seed), value, error)));
    Value alias = value;
    String name = value.getType().name;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.close(error)));
    SLANG_CHECK(!value.isValid());
    SLANG_CHECK(!alias.isValid());
    SLANG_CHECK(alias.getType().name == name);
    SLANG_CHECK(events->heap.getCount() == 0);
    SLANG_CHECK(events->payloads.getCount() == 0);
    value = Value();
    alias = Value();
    SLANG_CHECK(events->valid);
}

SLANG_UNIT_TEST(hgvmManagedValueCancellation)
{
    struct Control : IStepControl
    {
        Index stops = 0;
        Index stopAt = -1;
        StepAction onStep(DebugInstructionAddress, bool) override
        {
            return stops++ == stopAt ? StepAction::Stop : StepAction::Step;
        }
    } control;
    auto program = compileManagedInvocationProgram(unitTestContext->slangGlobalSession, R"(
        [host] State main()
        {
            State first = makeState(7);
            State copy = first;
            return copy;
        }
    )");
    RefPtr<ManagedValueEvents> events = new ManagedValueEvents;
    Desc desc;
    desc.enableEventTracking = true;
    desc.eventSink = events;
    desc.stepControl = &control;
    Interpreter interpreter(desc);
    RefPtr<BoundFunction> function;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(program, program->findFunctionExport("main")->id, function, error)));
    Value original;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*function)({}, original, error)));
    Index totalStops = control.stops;
    SLANG_CHECK(totalStops > 0);
    Index liveHeap = events->heap.getCount();
    Index livePayloads = events->payloads.getCount();

    // Exercise every cancellation boundary, including after a managed return has populated the
    // root result but before frame cleanup completes. The previous external result must survive.
    for (Index stop = 0; stop < totalStops; ++stop)
    {
        control.stops = 0;
        control.stopAt = stop;
        Value output = original;
        SLANG_CHECK((*function)({}, output, error) == SLANG_E_ABORT);
        SLANG_CHECK(output.isValid());
        SLANG_CHECK(output.getType().name == original.getType().name);
        SLANG_CHECK(events->valid);
        SLANG_CHECK(events->heap.getCount() == liveHeap);
        SLANG_CHECK(events->payloads.getCount() == livePayloads);
    }
    original = Value();
    SLANG_CHECK(events->heap.getCount() == 0);
    SLANG_CHECK(events->payloads.getCount() == 0);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.close(error)));
    SLANG_CHECK(events->valid);
}

SLANG_UNIT_TEST(hgvmManagedValueKinds)
{
    struct TestCase
    {
        const char* type;
        const char* expression;
        const char* read;
        int32_t expected;
    };
    const TestCase cases[] = {
        {"Counter", "makeState(7).inner.first", "value.count + value.next.count", 15},
        {"RuntimeArray<Counter>", "makeState(7).counters", "value[0].count + value[1].count", 14},
        {"String", "makeState(7).inner.label", "int(byteCount(value))", 8},
        {"String", "\"literal\"", "int(byteCount(value))", 7},
        {"Timer", "makeTimer()", "inspectTimer(value)", 1},
        {"Timers", "makeTimers()", "inspectTimer(value.first) + inspectTimer(value.second)", 2},
        {"Optional<Timer>", "makeTimer()", "value.hasValue ? inspectTimer(value.value) : 0", 1},
        {"Optional<RWTexture2D<float4>>", "none", "value.hasValue ? 1 : 0", 0},
    };
    for (const auto& item : cases)
    {
        String producerSource =
            String("[host] ") + item.type + " main() { return " + item.expression + "; }";
        // These are VM arguments. Explicit uniform also keeps Optional<Texture> out of the
        // shader varying-parameter validation path used by this entry-point compile helper.
        String consumerSource = String("[host] int main(uniform ") + item.type +
                                " value) { return " + item.read + "; }";
        auto producer = compileManagedInvocationProgram(
            unitTestContext->slangGlobalSession,
            producerSource.getBuffer());
        auto consumer = compileManagedInvocationProgram(
            unitTestContext->slangGlobalSession,
            consumerSource.getBuffer());
        RefPtr<ManagedValueEvents> events = new ManagedValueEvents;
        Desc desc;
        desc.enableEventTracking = true;
        desc.eventSink = events;
        Interpreter interpreter(desc);
        RefPtr<BoundFunction> make, read;
        String error;
        auto bindResult = interpreter.bindFunction(
            producer,
            producer->findFunctionExport("main")->id,
            make,
            error);
        if (SLANG_FAILED(bindResult))
            fprintf(stderr, "%s: %s\n", item.type, error.getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(bindResult));
        Value value;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)({}, value, error)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            interpreter
                .bindFunction(consumer, consumer->findFunctionExport("main")->id, read, error)));
        {
            Argument arguments[] = {Argument::fromValue(value)};
            Value result;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*read)(makeArrayView(arguments), result, error)));
            int32_t actual = 0;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                result.getScalarData(ScalarType::Int32, &actual, sizeof(actual), error)));
            SLANG_CHECK(actual == item.expected);
        }
        value = Value();
        SLANG_CHECK(events->heap.getCount() == 0);
        SLANG_CHECK(events->payloads.getCount() == 0);
        // CLI callers discard managed results through the same native ownership path.
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.call(producer, "main", error)));
        SLANG_CHECK(events->heap.getCount() == 0);
        SLANG_CHECK(events->payloads.getCount() == 0);
        SLANG_CHECK(events->valid);
    }
}

SLANG_UNIT_TEST(hgvmManagedResourceValueKinds)
{
    // Opaque null handles exercise binding and cross-program type identity without requiring
    // a device or host service. Live object ownership is covered by the timer and GPU tests.
    const char* types[] = {
        "RWTexture2D<float4>",
        "Texture2D<float4>",
        "RaytracingAccelerationStructure",
        "HWRaytracingContext<>.BottomLevelAccelerationStructure",
        "RaytracingPipeline",
        "DescriptorHandle<RWTexture2D<float4>>",
        "DescriptorHandle<RaytracingAccelerationStructure>",
        "Defer<uint>",
        "Stream",
        "Window",
        "Box",
    };
    for (const char* type : types)
    {
        String declarations = R"(
            import slang.pipeline_rt;
            __target_intrinsic(hgbc, "OpByteStreamType") struct Stream {}
            __target_intrinsic(hgbc, "OpWindowType") struct Window {}
            __target_intrinsic(hgbc, "OpBoxType") struct Box {}
        )";
        String producerSource = declarations + "[host] " + type +
                                " main() { return hgbc_asm { result : $$" + type +
                                " = OpNullLit; }; }";
        String consumerSource =
            declarations + "[host] " + type + " main(" + type + " value) { return value; }";
        auto producer =
            compileHGVMTestProgram(unitTestContext->slangGlobalSession, producerSource.getBuffer());
        auto consumer =
            compileHGVMTestProgram(unitTestContext->slangGlobalSession, consumerSource.getBuffer());
        Interpreter vm;
        RefPtr<BoundFunction> make, copy;
        String error;
        auto bindResult =
            vm.bindFunction(producer, producer->findFunctionExport("main")->id, make, error);
        if (SLANG_FAILED(bindResult))
            fprintf(stderr, "%s: %s\n", type, error.getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(bindResult));
        Value original;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)({}, original, error)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            vm.bindFunction(consumer, consumer->findFunctionExport("main")->id, copy, error)));
        Argument args[] = {Argument::fromValue(original)};
        Value result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*copy)(makeArrayView(args), result, error)));
        SLANG_CHECK(result.isValid());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(vm.close(error)));
        SLANG_CHECK(!original.isValid());
        SLANG_CHECK(!result.isValid());
    }
}

SLANG_UNIT_TEST(hgvmManagedGPUValueInvocation)
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

    auto compile = [&](const char* body)
    {
        String source = R"(
            struct GpuElement { uint number; float weight; }
            struct GpuState { RWStructuredBuffer<GpuElement> values; uint count; }
            [shader("compute")] [numthreads(1, 1, 1)]
            void initialize(RWStructuredBuffer<GpuElement> values, uint seed)
            {
                values[0] = GpuElement(seed, 1.0);
                values[1] = GpuElement(seed + 1, 2.0);
            }
            [shader("compute")] [numthreads(1, 1, 1)]
            void increment(RWStructuredBuffer<GpuElement> values, uint amount)
            {
                values[0].number += amount;
                values[1].number += amount;
            }
        )";
        source.append(body);
        return compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_STANDARD,
            "hgvm_gpu_values");
    };
    auto producer = compile(R"(
        [host] GpuState main(uint seed)
        {
            let values = RWStructuredBuffer<GpuElement>::alloc(2);
            dispatch(initialize, uint3(1, 1, 1))(values, seed);
            return GpuState(values, 2);
        }
    )");
    auto consumer = compile(R"(
        [host] uint main(GpuState state)
        {
            return state.values.readBack(0).number + state.values.readBack(1).number;
        }
    )");
    auto copier = compile("[host] GpuState main(GpuState state) { return state; }");
    auto updater = compile(R"(
        [host] void main(GpuState state, uint amount)
        {
            dispatch(increment, uint3(1, 1, 1))(state.values, amount);
        }
    )");
    auto failure = compile(R"(
        [host] GpuState main(uint divisor)
        {
            let values = RWStructuredBuffer<GpuElement>::alloc(2);
            return GpuState(values, 8 / divisor);
        }
    )");
    auto readOnly = compile(R"(
        [host] StructuredBuffer<GpuElement> main()
        {
            let data = RuntimeArray<GpuElement>::alloc(1);
            data[0] = GpuElement(23, 1.0);
            let values = StructuredBuffer<GpuElement>::alloc(1);
            copy(values, 0, data, 0, 1);
            return values;
        }
    )");
    auto readOnlyConsumer = compile(R"(
        [host] uint main(StructuredBuffer<GpuElement> values)
        {
            return values.readBack(0).number;
        }
    )");

    struct Events : ManagedValueEvents
    {
        Address buffer;
        void onEvent(const TrackingEvent& event) noexcept override
        {
            ManagedValueEvents::onEvent(event);
            if (event.kind == TrackingEventKind::HeapAlloc &&
                event.heap.objectKind == HeapObjectKind::StructuredBuffer)
                buffer.value = uint64_t(reinterpret_cast<uintptr_t>(event.heap.address));
        }
    };
    struct Control : IStepControl, IDebugHandler
    {
        Interpreter* vm = nullptr;
        Events* events = nullptr;
        DebugCallId call;
        bool inspect = false;
        bool inspected = false;
        void onFunctionEnter(DebugInstructionAddress, DebugCallId id) override { call = id; }
        void onFunctionExit() override {}
        void onScope(uint32_t, uint32_t) override {}
        StepAction onStep(DebugInstructionAddress address, bool) override
        {
            if (inspect && !inspected)
            {
                DebugBufferInfo info;
                if (vm->getDebugBufferInfo(call, events->buffer, info))
                {
                    auto type = vm->getDebugMap(address.programId)->findType(info.elementType);
                    SLANG_CHECK_ABORT(type);
                    SLANG_CHECK(type->qualifiedName.contains("GpuElement"));
                    SLANG_CHECK(info.elementCount == 2 && info.hostStride == 8);
                    inspected = true;
                }
            }
            return StepAction::Step;
        }
    } control;
    RefPtr<Events> events = new Events;
    Desc desc;
    desc.device = device;
    desc.gpuExecution = GPUExecutionMode::Buffered;
    desc.enableEventTracking = true;
    desc.eventSink = events;
    desc.stepControl = &control;
    desc.debugHandler = &control;
    Interpreter vm(desc);
    control.vm = &vm;
    control.events = events;
    String error;
    auto bind = [&](Program* program)
    {
        RefPtr<BoundFunction> function;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            vm.bindFunction(program, program->findFunctionExport("main")->id, function, error)));
        return function;
    };
    auto make = bind(producer);
    Value original;
    Argument seed[] = {Argument::scalar(uint32_t(7))};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)(makeArrayView(seed), original, error)));
    auto read = bind(consumer);
    auto copy = bind(copier);
    auto update = bind(updater);
    auto fail = bind(failure);
    auto makeReadOnly = bind(readOnly);
    auto readFirst = bind(readOnlyConsumer);
    auto invokeValue = [&](BoundFunction* function, const Value& value)
    {
        Argument args[] = {Argument::fromValue(value)};
        Value result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*function)(makeArrayView(args), result, error)));
        return result;
    };
    auto readNumber = [&](BoundFunction* function, const Value& value)
    {
        Value result = invokeValue(function, value);
        uint32_t number = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            result.getScalarData(ScalarType::UInt32, &number, sizeof(number), error)));
        return number;
    };
    control.inspect = true;
    SLANG_CHECK(readNumber(read, original) == 15);
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    SLANG_CHECK(control.inspected);
#endif
    control.inspect = false;
    Value alias = invokeValue(copy, original);
    {
        Argument args[] = {Argument::fromValue(alias), Argument::scalar(uint32_t(3))};
        Value result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*update)(makeArrayView(args), result, error)));
    }
    SLANG_CHECK(readNumber(read, original) == 21);
    original = Value();
    SLANG_CHECK(vm.getBufferPoolStats().leasedBytes == 16);
    SLANG_CHECK(readNumber(read, alias) == 21);
    alias = Value();
    SLANG_CHECK(vm.getBufferPoolStats().leasedBytes == 0);
    Value readonlyValue;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*makeReadOnly)({}, readonlyValue, error)));
    SLANG_CHECK(readNumber(readFirst, readonlyValue) == 23);
    readonlyValue = Value();
    SLANG_CHECK(vm.getBufferPoolStats().leasedBytes == 0);

    // Allocation followed by a host failure must return its lease to the existing pool.
    Argument zero[] = {Argument::scalar(uint32_t(0))};
    Value untouched;
    SLANG_CHECK(SLANG_FAILED((*fail)(makeArrayView(zero), untouched, error)));
    SLANG_CHECK(untouched.isVoid());
    SLANG_CHECK(vm.getBufferPoolStats().leasedBytes == 0);

    // Close must drain submitted work while a returned GPU value still exists.
    Value survivor;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)(makeArrayView(seed), survivor, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(vm.close(error)));
    SLANG_CHECK(!survivor.isValid());
    SLANG_CHECK(survivor.getType().name.contains("GpuState"));
    survivor = Value();
    SLANG_CHECK(events->heap.getCount() == 0 && events->payloads.getCount() == 0);
    SLANG_CHECK(events->valid);
}

SLANG_UNIT_TEST(hgvmManagedTextureValueInvocation)
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

    auto compile = [&](const char* body)
    {
        String source = R"(
        import slang.pipeline_rt;

            struct Images { RWTexture2D<float4> first; RWTexture2D<float4> second; }
            struct State { Images images; RaytracingAccelerationStructure scene; }
            RWTexture2D<float4> allocateImage()
            {
                uint size = 2;
                uint one = 1;
                return hgbc_asm {
                    result : $$RWTexture2D<float4> = OpAllocateResource
                        Texture Texture2D RGBA32Float DeviceLocal UnorderedAccess $$float4
                        $size $size $one 1 1 0
                        ShaderResource UnorderedAccess CopySource CopyDestination RenderTarget;
                };
            }
            [shader("compute")] [numthreads(1, 1, 1)]
            void writeImage(RWTexture2D<float4> image)
            {
                image[uint2(0, 0)] = float4(23, 0, 0, 1);
            }
            [shader("compute")] [numthreads(1, 1, 1)]
            void readImage(RWTexture2D<float4> image, RWStructuredBuffer<uint> output)
            {
                output[0] = uint(image[uint2(0, 0)].x);
            }
        )";
        source.append(body);
        return compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_NONE,
            "hgvm_texture_values");
    };
    auto producer = compile(R"(
        [host] State main()
        {
            let image = allocateImage();
            dispatch(writeImage, uint3(1, 1, 1))(image);
            return State(Images(image, image), RaytracingAccelerationStructure::alloc());
        }
    )");
    auto consumer = compile(R"(
        [host] uint main(State state)
        {
            let output = RWStructuredBuffer<uint>::alloc(1);
            dispatch(readImage, uint3(1, 1, 1))(state.images.second, output);
            return output.readBack(0);
        }
    )");
    auto copier = compile("[host] State main(State state) { return state; }");
    RefPtr<ManagedValueEvents> events = new ManagedValueEvents;
    Desc desc;
    desc.device = device;
    desc.enableEventTracking = true;
    desc.eventSink = events;
    Interpreter vm(desc);
    String error;
    RefPtr<BoundFunction> make, read, copy;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        vm.bindFunction(producer, producer->findFunctionExport("main")->id, make, error)));
    Value original;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)({}, original, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        vm.bindFunction(consumer, consumer->findFunctionExport("main")->id, read, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        vm.bindFunction(copier, copier->findFunctionExport("main")->id, copy, error)));
    Value alias;
    {
        Argument arguments[] = {Argument::fromValue(original)};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*copy)(makeArrayView(arguments), alias, error)));
    }
    original = Value();
    {
        Argument arguments[] = {Argument::fromValue(alias)};
        Value result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*read)(makeArrayView(arguments), result, error)));
        uint32_t pixel = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            result.getScalarData(ScalarType::UInt32, &pixel, sizeof(pixel), error)));
        SLANG_CHECK(pixel == 23);
    }
#if SLANG_ENABLE_HGVM_EVENT_TRACKING
    SLANG_CHECK(events->heap.getCount() == 2); // One aliased texture and one logical TLAS.
#endif
    alias = Value();
    SLANG_CHECK(events->heap.getCount() == 0);
    SLANG_CHECK(events->payloads.getCount() == 0);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*make)({}, original, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(vm.close(error)));
    SLANG_CHECK(!original.isValid());
    SLANG_CHECK(events->heap.getCount() == 0);
    SLANG_CHECK(events->payloads.getCount() == 0);
    SLANG_CHECK(events->valid);
}

SLANG_UNIT_TEST(hgvmManagedValueRejectsBorrowedFields)
{
    const char* types[] = {"Borrowed", "RuntimeArray<Borrowed>", "Wrapper"};
    for (const char* type : types)
    {
        String source = R"(
            class Borrowed { Borrowed next; Ptr<int> pointer; }
            struct Wrapper { RuntimeArray<Borrowed> values; }
        )";
        source.append("[host] int main(");
        source.append(type);
        source.append(" value) { return 1; }");
        auto program = compileHGVMTestProgram(
            unitTestContext->slangGlobalSession,
            source.getBuffer(),
            SLANG_DEBUG_INFO_LEVEL_NONE,
            "hgvm_borrowed_invocation");
        Interpreter interpreter;
        RefPtr<BoundFunction> function;
        String error;
        SLANG_CHECK(SLANG_FAILED(
            interpreter
                .bindFunction(program, program->findFunctionExport("main")->id, function, error)));
        SLANG_CHECK(!function);
    }
}

SLANG_UNIT_TEST(hgvmHostWindowInvocation)
{
    struct Host : IHGVMHostServices
    {
        int calls = 0;
        bool failNext = false;
        SlangResult processWindowEvents(HGVMHostObject* window, bool& open, String& error) override
        {
            SLANG_CHECK(window->getOwner() == this);
            ++calls;
            if (failNext)
            {
                error = "Cancelled browser frame wait";
                return SLANG_E_ABORT;
            }
            open = true;
            return SLANG_OK;
        }
    };
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        __target_intrinsic(hgbc, "OpWindowType")
        public struct Window {}
        [host] int main(Window window, int divisor)
        {
            bool open = hgbc_asm {
                result : $$bool = OpProcessWindowEvents $window;
            };
            return open ? 24 / divisor : 0;
        }
    )");
    RefPtr<Host> host = new Host;
    Desc desc;
    desc.hostServices = host;
    Interpreter vm(desc);
    RefPtr<BoundFunction> function;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        vm.bindFunction(program, program->findFunctionExport("main")->id, function, error)));
    SLANG_CHECK(function->getParameterTypes()[0].kind == HostValueType::Kind::HostObject);
    RefPtr<HGVMHostObject> window = new HGVMHostObject(HGVMHostObjectKind::Window, host);
    Argument args[] = {Argument::fromHostObject(window), Argument::scalar(int32_t(3))};
    auto references = window->debugGetReferenceCount();
    Value value;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*function)(makeArrayView(args), value, error)));
    int32_t result = 0;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(value.getScalarData(ScalarType::Int32, &result, sizeof(result), error)));
    SLANG_CHECK(result == 8);
    SLANG_CHECK(window->debugGetReferenceCount() == references);

    // Both a host failure and a later instruction failure release the root's Window reference.
    host->failNext = true;
    SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(args), value, error)));
    SLANG_CHECK(window->debugGetReferenceCount() == references);
    host->failNext = false;
    args[1] = Argument::scalar(int32_t(0));
    SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(args), value, error)));
    SLANG_CHECK(window->debugGetReferenceCount() == references);
    int calls = host->calls;
    args[1] = Argument::scalar(int32_t(3));
    RefPtr<Host> otherHost = new Host;
    RefPtr<HGVMHostObject> foreign = new HGVMHostObject(HGVMHostObjectKind::Window, otherHost);
    args[0] = Argument::fromHostObject(foreign);
    SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(args), value, error)));
    RefPtr<HGVMHostObject> surface = new HGVMHostObject(HGVMHostObjectKind::Surface, host);
    args[0] = Argument::fromHostObject(surface);
    SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(args), value, error)));
    args[0] = Argument::scalar(int32_t(1));
    SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(args), value, error)));
    SLANG_CHECK(host->calls == calls);
    args[0] = Argument::fromHostObject(window);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(vm.close(error)));
    SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(args), value, error)));
}

SLANG_UNIT_TEST(hgvmSurfaceWaitInvocation)
{
    struct Host : IHGVMHostServices
    {
        SlangResult waitResult = SLANG_OK;
        int waits = 0;
        int queries = 0;

        SlangResult createSurface(
            HGVMHostObject* window,
            rhi::Format,
            uint32_t,
            RefPtr<HGVMHostObject>& surface,
            String&) override
        {
            SLANG_CHECK(window->getOwner() == this);
            surface = new HGVMHostObject(HGVMHostObjectKind::Surface, this);
            return SLANG_OK;
        }

        SlangResult getSurfaceDimensions(
            HGVMHostObject* surface,
            uint32_t& width,
            uint32_t& height,
            String&) override
        {
            SLANG_CHECK(surface->getOwner() == this);
            ++queries;
            width = 320;
            height = 240;
            return SLANG_OK;
        }

        SlangResult waitForSurface(HGVMHostObject* surface, String& error) override
        {
            SLANG_CHECK(surface->getOwner() == this);
            ++waits;
            if (waitResult == SLANG_E_ABORT)
                error = "Surface wait cancelled";
            return waitResult;
        }
    };
    // A fake surface keeps this test independent of native windows and presentation APIs.
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        __target_intrinsic(hgbc, "OpWindowType")
        public struct Window {}
        [host] uint main(Window window)
        {
            Surface<43> surface = hgbc_asm {
                result : $$Surface<43> = OpCreateSurface $window $2u;
            };
            uint2 dimensions = hgbc_asm {
                result : $$uint2 = OpGetSurfaceDimensions $surface;
            };
            bool ready = hgbc_asm { result : $$bool = OpWaitForSurface $surface; };
            if (!ready) return 0;
            ready = hgbc_asm { result : $$bool = OpWaitForSurface $surface; };
            return ready ? dimensions.x + dimensions.y : 1;
        }
    )");
    RefPtr<Host> host = new Host;
    Desc desc;
    desc.hostServices = host;
    Interpreter vm(desc);
    RefPtr<BoundFunction> function;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        vm.bindFunction(program, program->findFunctionExport("main")->id, function, error)));
    RefPtr<HGVMHostObject> window = new HGVMHostObject(HGVMHostObjectKind::Window, host);
    Argument arguments[] = {Argument::fromHostObject(window)};
    auto references = window->debugGetReferenceCount();
    Value value;
    uint32_t result = 0;
    SlangResult invocationResult = (*function)(makeArrayView(arguments), value, error);
    if (SLANG_FAILED(invocationResult))
        fprintf(stderr, "%s\n", error.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(invocationResult));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(value.getScalarData(ScalarType::UInt32, &result, sizeof(result), error)));
    SLANG_CHECK(result == 560 && host->waits == 2 && host->queries == 1);

    host->waitResult = SLANG_E_NOT_AVAILABLE;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*function)(makeArrayView(arguments), value, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(value.getScalarData(ScalarType::UInt32, &result, sizeof(result), error)));
    SLANG_CHECK(result == 0 && host->waits == 3 && host->queries == 2);

    host->waitResult = SLANG_E_ABORT;
    SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(arguments), value, error)));
    SLANG_CHECK(error.contains("Surface wait cancelled"));
    SLANG_CHECK(window->debugGetReferenceCount() == references);
}

SLANG_UNIT_TEST(hgvmScalarInvocation)
{
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        [noinline] int nested(int a)
        {
            // A large callee frame forces stack growth after the root result slot is allocated.
            int values[256];
            for (int i = 0; i < 256; ++i) values[i] = a + i;
            return values[255] - 255 + a + a;
        }
        [host] int main(int a, uint b, float c) { return nested(a) + int(b) + int(c); }
    )");
    auto function = program->findFunctionExport("main");
    SLANG_CHECK_ABORT(function);
    FunctionSignature signature;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(program->getFunctionSignature(function->id, signature, error)));
    SLANG_CHECK_ABORT(signature.parameterTypes.getCount() == 3);
    int32_t a = -7;
    uint32_t b = 9;
    float c = 2.5f;
    int32_t result = 100;
    ValueView arguments[] = {
        {signature.parameterTypes[0], &a, sizeof(a)},
        {signature.parameterTypes[1], &b, sizeof(b)},
        {signature.parameterTypes[2], &c, sizeof(c)}};
    MutableValueView output{signature.resultType, &result, sizeof(result)};
    Interpreter interpreter;
    for (int i = 0; i < 20; ++i)
    {
        SLANG_CHECK(SLANG_SUCCEEDED(
            interpreter.invoke(program, function->id, makeArrayView(arguments), output, error)));
        SLANG_CHECK(result == -10);
    }
    // Invalid host input must fail before execution and leave caller-owned result storage intact.
    result = 100;
    arguments[0].size = 1;
    SLANG_CHECK(SLANG_FAILED(
        interpreter.invoke(program, function->id, makeArrayView(arguments), output, error)));
    SLANG_CHECK(result == 100);
    arguments[0].size = sizeof(a);
    arguments[0].typeId = signature.parameterTypes[1];
    SLANG_CHECK(SLANG_FAILED(
        interpreter.invoke(program, function->id, makeArrayView(arguments), output, error)));
    SLANG_CHECK(result == 100);
    arguments[0].typeId = signature.parameterTypes[0];
    SLANG_CHECK(SLANG_FAILED(interpreter.invoke(program, function->id, {}, output, error)));
    output.size = 1;
    SLANG_CHECK(SLANG_FAILED(
        interpreter.invoke(program, function->id, makeArrayView(arguments), output, error)));
    SLANG_CHECK(result == 100);
    // Input/output aliasing is safe because all arguments enter the VM before the host result copy.
    output = {signature.resultType, &a, sizeof(a)};
    SLANG_CHECK(SLANG_SUCCEEDED(
        interpreter.invoke(program, function->id, makeArrayView(arguments), output, error)));
    SLANG_CHECK(a == -10);
}

SLANG_UNIT_TEST(hgvmInvocationFailureRecovery)
{
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        [host] int main(int divisor) { return 24 / divisor; }
    )");
    auto function = program->findFunctionExport("main");
    FunctionSignature signature;
    String error;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(program->getFunctionSignature(function->id, signature, error)));
    int32_t divisor = 0;
    int32_t result = 77;
    ValueView argument{signature.parameterTypes[0], &divisor, sizeof(divisor)};
    MutableValueView output{signature.resultType, &result, sizeof(result)};
    Interpreter interpreter;
    SLANG_CHECK(SLANG_FAILED(
        interpreter.invoke(program, function->id, makeArrayView(&argument, 1), output, error)));
    SLANG_CHECK(result == 77);
    divisor = 3;
    SLANG_CHECK(SLANG_SUCCEEDED(
        interpreter.invoke(program, function->id, makeArrayView(&argument, 1), output, error)));
    SLANG_CHECK(result == 8);
}

SLANG_UNIT_TEST(hgvmInvocationRejectsReentry)
{
    struct Sink : IHGVMOutputSink
    {
        Interpreter* interpreter = nullptr;
        Program* program = nullptr;
        RefPtr<BoundFunction> function;
        bool rejected = false;
        void writeLine(std::string_view) override
        {
            String error;
            rejected = SLANG_FAILED(interpreter->call(program, "main", error));
            Value result;
            rejected &= SLANG_FAILED((*function)({}, result, error));
        }
    };
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        [host] int main() { print("reenter"); return 42; }
    )");
    RefPtr<Sink> sink = new Sink;
    Desc desc;
    desc.outputSink = sink;
    Interpreter interpreter(desc);
    sink->interpreter = &interpreter;
    sink->program = program;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.bindFunction(
        program,
        program->findFunctionExport("main")->id,
        sink->function,
        error)));
    SLANG_CHECK(SLANG_SUCCEEDED(interpreter.call(program, "main", error)));
    SLANG_CHECK(sink->rejected);
}

SLANG_UNIT_TEST(hgvmBoundFunctionCallable)
{
    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        [host] int main(int divisor) { return 24 / divisor; }
    )");
    for (bool closeExplicitly : {false, true})
    {
        RefPtr<BoundFunction> function;
        Value result;
        String error;
        Argument arguments[] = {Argument::scalar(int32_t(3))};
        {
            Interpreter interpreter;
            uint64_t programId = 0;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(interpreter.prepareProgram(program, programId, error)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.bindFunction(
                program,
                program->findFunctionExport("main")->id,
                function,
                error)));

            const BoundFunction& divide = *function;
            error = "Previous error";
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(divide(makeArrayView(arguments), result, error)));
            SLANG_CHECK(error.getLength() == 0);
            int32_t number = 0;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                result.getScalarData(ScalarType::Int32, &number, sizeof(number), error)));
            SLANG_CHECK(number == 8);

            // Callable failures use the existing validation/unwind path and preserve the result.
            arguments[0] = Argument::scalar(int32_t(0));
            SLANG_CHECK(SLANG_FAILED(divide(makeArrayView(arguments), result, error)));
            SLANG_CHECK(SLANG_FAILED(divide({}, result, error)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                result.getScalarData(ScalarType::Int32, &number, sizeof(number), error)));
            SLANG_CHECK(number == 8);
            arguments[0] = Argument::scalar(int32_t(2));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(divide(makeArrayView(arguments), result, error)));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                result.getScalarData(ScalarType::Int32, &number, sizeof(number), error)));
            SLANG_CHECK(number == 12);

            if (closeExplicitly)
            {
                SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.close(error)));
                SLANG_CHECK(SLANG_FAILED(divide(makeArrayView(arguments), result, error)));
            }
        }
        // A callable must neither retain a destroyed VM nor dereference its old core pointer.
        SLANG_CHECK(!function->isValid());
        SLANG_CHECK(!result.isValid());
        SLANG_CHECK(function->getName() == "main");
        Value untouched;
        SLANG_CHECK(SLANG_FAILED((*function)(makeArrayView(arguments), untouched, error)));
        SLANG_CHECK(error.getLength() != 0);
        SLANG_CHECK(untouched.isVoid());
    }
}

SLANG_UNIT_TEST(hgvmOwnedValueInvocation)
{
    auto producer = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            struct Point { float x; float y; }
            [host] Point main(float x, float y) { return Point(x, y); }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_owned_point");
    auto consumer = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            struct Point { float x; float y; }
            [host] float main(Point p) { return p.x * p.x + p.y * p.y; }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_owned_point");
    auto transform = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            struct Point { float x; float y; }
            [host] Point main(Point p, int divisor)
            {
                int delta = 24 / divisor;
                return Point(p.x + delta, p.y);
            }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_owned_point");

    Interpreter interpreter;
    String error;
    RefPtr<BoundFunction> makePoint, lengthSquared, translate;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(producer, producer->findFunctionExport("main")->id, makePoint, error)));
    Argument scalarArguments[] = {Argument::scalar(3.0f), Argument::scalar(4.0f)};
    Value point;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter.invoke(makePoint, makeArrayView(scalarArguments), point, error)));
    SLANG_CHECK(point.isValid());
    SLANG_CHECK(point.getType().kind == HostValueType::Kind::Aggregate);
    SLANG_CHECK(point.getType().byteSize == 8);

    // Staging more code after creating a value must preserve the value's canonical identity.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.bindFunction(
        consumer,
        consumer->findFunctionExport("main")->id,
        lengthSquared,
        error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(transform, transform->findFunctionExport("main")->id, translate, error)));
    auto inspect = [&](const Value& value)
    {
        Argument arguments[] = {Argument::fromValue(value)};
        Value result;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED((*lengthSquared)(makeArrayView(arguments), result, error)));
        float number = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            result.getScalarData(ScalarType::Float32, &number, sizeof(number), error)));
        return number;
    };
    SLANG_CHECK(inspect(point) == 25.0f);

    Value original = point;
    Argument change[] = {Argument::fromValue(point), Argument::scalar(int32_t(24))};
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED((*translate)(makeArrayView(change), point, error)));
    SLANG_CHECK(inspect(point) == 32.0f);
    SLANG_CHECK(inspect(original) == 25.0f);

    // Both ordinary runtime failure and pre-execution validation preserve an aliased output.
    change[0] = Argument::fromValue(point);
    change[1] = Argument::scalar(int32_t(0));
    SLANG_CHECK(SLANG_FAILED((*translate)(makeArrayView(change), point, error)));
    SLANG_CHECK(inspect(point) == 32.0f);
    Argument wrong[] = {Argument::scalar(1.0f)};
    SLANG_CHECK(
        SLANG_FAILED(interpreter.invoke(lengthSquared, makeArrayView(wrong), point, error)));
    SLANG_CHECK(inspect(point) == 32.0f);

    Interpreter other;
    RefPtr<BoundFunction> otherConsumer;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(other.bindFunction(
        consumer,
        consumer->findFunctionExport("main")->id,
        otherConsumer,
        error)));
    Argument foreign[] = {Argument::fromValue(point)};
    Value unused;
    SLANG_CHECK(SLANG_FAILED((*otherConsumer)(makeArrayView(foreign), unused, error)));

    auto name = point.getType().name;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.close(error)));
    SLANG_CHECK(SLANG_SUCCEEDED(interpreter.close(error)));
    SLANG_CHECK(!point.isValid());
    SLANG_CHECK(!original.isValid());
    SLANG_CHECK(!makePoint->isValid());
    SLANG_CHECK(point.getType().name == name);
    SLANG_CHECK(
        SLANG_FAILED(interpreter.invoke(lengthSquared, makeArrayView(foreign), unused, error)));
}

SLANG_UNIT_TEST(hgvmOwnedLargeAndEmptyValues)
{
    auto large = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        struct Large { bool enabled; float values[33]; float3 direction; }
        [noinline] Large makeLarge(float seed)
        {
            Large value;
            value.enabled = true;
            for (int i = 0; i < 33; ++i) value.values[i] = seed + i;
            value.direction = float3(1, 2, 3);
            return value;
        }
        [host] Large main(float seed) { return makeLarge(seed); }
    )");
    Interpreter interpreter;
    String error;
    RefPtr<BoundFunction> bound;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter.bindFunction(large, large->findFunctionExport("main")->id, bound, error)));
    Argument arguments[] = {Argument::scalar(3.0f)};
    Value result;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(interpreter.invoke(bound, makeArrayView(arguments), result, error)));
    SLANG_CHECK(result.getType().byteSize > 64);

    auto empty = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        struct Empty { }
        [host] Empty main() { return Empty(); }
    )");
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter.bindFunction(empty, empty->findFunctionExport("main")->id, bound, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(bound, {}, result, error)));
    SLANG_CHECK(!result.isVoid());
    SLANG_CHECK(result.getType().byteSize == 1);
    // The CLI convenience path must discard an aggregate through normal result ownership.
    SLANG_CHECK(SLANG_SUCCEEDED(interpreter.call(empty, "main", error)));
}

#if SLANG_ENABLE_HGVM_EVENT_TRACKING
SLANG_UNIT_TEST(hgvmOwnedValueRejectsEventReentry)
{
    struct Sink : IEventSink
    {
        Interpreter* interpreter = nullptr;
        Program* program = nullptr;
        BoundFunction* function = nullptr;
        HGBCId functionId = 0;
        int allocations = 0;
        int releases = 0;
        bool handling = false;
        bool rejected = true;

        void onEvent(const TrackingEvent& event) noexcept override
        {
            if (!function || handling ||
                (event.kind != TrackingEventKind::PayloadAlloc &&
                 event.kind != TrackingEventKind::PayloadFree))
                return;
            handling = true;
            if (event.kind == TrackingEventKind::PayloadAlloc)
                ++allocations;
            else
                ++releases;

            String error;
            Value result;
            RefPtr<BoundFunction> bound;
            uint64_t programId = 0;
            rejected &= SLANG_FAILED(interpreter->invoke(function, {}, result, error));
            rejected &= SLANG_FAILED(interpreter->prepareProgram(program, programId, error));
            rejected &= SLANG_FAILED(interpreter->bindFunction(program, functionId, bound, error));
            rejected &= SLANG_FAILED(interpreter->close(error));
            handling = false;
        }
    };

    auto program = compileHGVMTestProgram(unitTestContext->slangGlobalSession, R"(
        [host] float main() { return 42.0; }
    )");
    RefPtr<Sink> sink = new Sink;
    Desc desc;
    desc.enableEventTracking = true;
    desc.eventSink = sink;
    Interpreter interpreter(desc);
    RefPtr<BoundFunction> function;
    String error;
    auto functionId = program->findFunctionExport("main")->id;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(interpreter.bindFunction(program, functionId, function, error)));
    sink->interpreter = &interpreter;
    sink->program = program;
    sink->function = function;
    sink->functionId = functionId;

    Value value;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(function, {}, value, error)));
    float number = 0;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(value.getScalarData(ScalarType::Float32, &number, sizeof(number), error)));
    SLANG_CHECK(number == 42.0f);
    SLANG_CHECK(sink->allocations == 1);
    // Releasing a result between invocations also emits a callback and needs the same guard.
    value = Value();
    SLANG_CHECK(sink->releases == 1);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(function, {}, value, error)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.close(error)));
    SLANG_CHECK(sink->allocations == 2);
    SLANG_CHECK(sink->releases == 2);
    SLANG_CHECK(sink->rejected);
    SLANG_CHECK(!value.isValid());
}
#endif
