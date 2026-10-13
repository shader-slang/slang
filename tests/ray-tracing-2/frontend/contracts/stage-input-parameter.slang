//TEST:SIMPLE: -experimental-feature -no-codegen

import slang.raytracing;

struct TestPayload { uint value; }
struct TestRecord { uint value; }

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct TestHitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

[require(closesthit)]
void updatePayload(in rt::ClosestHitInput<TestHitContext> input)
{
    // Copying a view preserves access to the same payload and selected SBT record.
    let local = input;
    local.payload.value = local.record.value;
}

[require(closesthit)]
void closestHitHelper(in rt::ClosestHitInput<TestHitContext> input)
{
    rt::ClosestHitInput<TestHitContext> explicitCopy = input;
    let inferredCopy = input;
    rt::ClosestHitInput<TestHitContext> copyOfCopy = inferredCopy;

    updatePayload(explicitCopy);
    updatePayload(copyOfCopy);
    updatePayload(rt::ClosestHitInput<TestHitContext>(input));
}

[require(anyhit)]
void anyHitHelper(in rt::AnyHitInput<TestHitContext> input)
{
    let local = input;
    local.payload.value = local.record.value;
}

struct TestProceduralContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<float2>;
    typealias Record = TestRecord;
}

[require(structural_raytracing_intersection_payload)]
void intersectionHelper(in rt::IntersectionInput<TestProceduralContext> input)
{
    let local = input;
    local.payload.value = local.record.value;
}

struct TestCallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = TestPayload;
    typealias Record = TestRecord;
}

[require(callable)]
void callableHelper(in rt::CallableInput<TestCallableContext> input)
{
    let local = input;
    local.data.value = local.record.value;
}

[require(closesthit)]
void choose(in rt::ClosestHitInput<TestHitContext> input, float value) {}
void choose(int first, int second) {}

void selectOrdinaryOverload()
{
    // The int overload wins. Trying the other candidate must not diagnose an input construction.
    choose({}, 0);
}
