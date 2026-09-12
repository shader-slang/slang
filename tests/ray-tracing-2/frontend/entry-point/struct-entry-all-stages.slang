//TEST:SIMPLE: -experimental-feature -entry ClosestHit -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry AnyHit -stage anyhit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Intersection -stage intersection -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Miss -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Callable -stage callable -no-codegen
//TEST:SIMPLE(filecheck=METAL_HELPER): -experimental-feature -entry ClosestHit -stage closesthit -target metal
//TEST:SIMPLE(filecheck=METAL_HELPER): -experimental-feature -entry AnyHit -stage anyhit -target metal
//TEST:SIMPLE(filecheck=METAL_HELPER): -experimental-feature -entry Intersection -stage intersection -target metal
//TEST:SIMPLE(filecheck=METAL_HELPER): -experimental-feature -entry Miss -stage miss -target metal
//TEST:SIMPLE(filecheck=METAL_HELPER): -experimental-feature -entry Callable -stage callable -target metal

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}
struct TestAttributes {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::BoundingBoxPrimitive<TestAttributes>;
    typealias Record = TestRecord;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Record = TestRecord;
}

struct CallableContext : rt::ICallableContext
{
    typealias TraceContext = TestTraceContext;
    typealias CallableData = uint;
    typealias Record = TestRecord;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

struct AnyHit : rt::IAnyHitShader
{
    typealias Context = HitContext;
    void invoke(rt::AnyHitInput<HitContext> input) {}
}

struct Intersection : rt::IIntersectionShader
{
    typealias Context = HitContext;
    void invoke(rt::IntersectionInput<HitContext> input) {}
}

struct Miss : rt::IMissShader
{
    typealias Context = MissContext;
    void invoke(rt::MissInput<MissContext> input) {}
}

struct Callable : rt::ICallableShader
{
    typealias Context = CallableContext;
    void invoke(rt::CallableInput<CallableContext> input) {}
}

// METAL_HELPER: void {{.*}}_invoke_0()
