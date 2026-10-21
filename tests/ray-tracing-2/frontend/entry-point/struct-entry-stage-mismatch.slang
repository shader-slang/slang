//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHit -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=COMPUTE): -experimental-feature -entry ClosestHit -stage compute -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=RAYGEN): -experimental-feature -entry ClosestHit -stage raygeneration -no-codegen

import slang.raytracing;

struct TestPayload {}
struct TestRecord {}

struct TestTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = TestTraceContext;
    typealias Payload = TestPayload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = TestRecord;
}

struct ClosestHit : rt::IClosestHitShader
/*CHECK:
       ^^^^^^^^^^ structural ray-tracing entry point stage mismatch
       ^^^^^^^^^^ struct 'ClosestHit' does not implement the selected 'miss' stage
*/
/*COMPUTE:
       ^^^^^^^^^^ structural ray-tracing entry point stage mismatch
       ^^^^^^^^^^ struct 'ClosestHit' does not implement the selected 'compute' stage
*/
/*RAYGEN:
       ^^^^^^^^^^ structural ray-tracing entry point stage mismatch
       ^^^^^^^^^^ struct 'ClosestHit' does not implement the selected 'raygeneration' stage
*/
{
    typealias Context = HitContext;
    static void invoke(rt::ClosestHitInput<HitContext> input) {}
}
