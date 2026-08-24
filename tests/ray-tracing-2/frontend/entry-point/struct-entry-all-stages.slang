//TEST:SIMPLE: -experimental-feature -entry ClosestHit -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry AnyHit -stage anyhit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Intersection -stage intersection -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Miss -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Callable -stage callable -no-codegen

import slang.raytracing;

struct Payload {}
struct Record {}
struct Attributes {}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TraceContext;
    typealias Primitive = rt::BoundingBoxPrimitive<Attributes>;
    typealias Record = Record;
}

struct MissContext : rt::IMissGroupContext
{
    typealias TraceContext = TraceContext;
    typealias Record = Record;
}

struct CallableContext : rt::ICallableGroupContext
{
    typealias TraceContext = TraceContext;
    typealias CallableData = uint;
    typealias Record = Record;
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

struct AnyHit : rt::IAnyHitShader<HitContext>
{
    void invoke(rt::AnyHitInput<HitContext> input) {}
}

struct Intersection : rt::IIntersectionShader<HitContext>
{
    void invoke(rt::IntersectionInput<HitContext> input) {}
}

struct Miss : rt::IMissShader<MissContext>
{
    void invoke(rt::MissInput<MissContext> input) {}
}

struct Callable : rt::ICallableShader<CallableContext>
{
    void invoke(rt::CallableInput<CallableContext> input) {}
}
