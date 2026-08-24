//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHit -stage closesthit -no-codegen

import slang.raytracing;

struct Payload {}
struct Record {}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
{
    uint state;
//CHECK: ^^^^^ stateful structural ray-tracing stage
//CHECK: ^^^^^ instance field 'state' is not allowed because structural ray-tracing stage implementations are compiler-created

    void invoke(rt::ClosestHitInput<HitContext> input) {}
}
