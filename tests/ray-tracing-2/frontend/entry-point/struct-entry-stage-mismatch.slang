//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHit -stage miss -no-codegen

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
/*CHECK:
       ^^^^^^^^^^ structural ray-tracing entry point stage mismatch
       ^^^^^^^^^^ struct 'ClosestHit' does not implement the selected 'miss' stage
*/
{
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}
