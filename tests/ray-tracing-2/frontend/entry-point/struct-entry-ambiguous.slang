//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry MultiStage -no-codegen
//TEST:SIMPLE: -experimental-feature -entry MultiStage -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry MultiStage -stage anyhit -no-codegen

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

struct MultiStage : rt::IClosestHitShader<HitContext>, rt::IAnyHitShader<HitContext>
/*CHECK:
       ^^^^^^^^^^ ambiguous structural ray-tracing entry point stage
       ^^^^^^^^^^ struct 'MultiStage' implements more than one executable ray-tracing stage; use '-stage <name>' to select one
*/
{
    void invoke(rt::ClosestHitInput<HitContext> input) {}
    void invoke(rt::AnyHitInput<HitContext> input) {}
}
