//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHit -stage closesthit -target hlsl

import slang.raytracing;

struct Payload {}
struct Record {}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = ::Record;
}

struct ClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;

    void invoke(in rt::ClosestHitInput<HitContext> input)
//CHECK: ^^^^^^ structural ray-tracing entry point was not lowered
//CHECK: ^^^^^^ structural ray-tracing entry point 'ClosestHit.invoke' must be converted to a native adapter before target code generation
    {}
}
