//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHit -stage miss -no-codegen

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias Payload = TestPayload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct ClosestHit : rt::IClosestHitShader<HitContext>
/*CHECK:
       ^^^^^^^^^^ structural ray-tracing entry point stage mismatch
       ^^^^^^^^^^ struct 'ClosestHit' does not implement the selected 'miss' stage
*/
{
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}
