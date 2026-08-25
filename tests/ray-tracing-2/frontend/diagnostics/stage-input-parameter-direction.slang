//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHitStage -stage closesthit -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHitStage -stage closesthit -target metal

import slang.raytracing;

struct PayloadData {}
struct Record {}

struct MyTraceContext : rt::ITraceContext
{
    typealias Payload = PayloadData;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MyTraceContext;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = Record;
}

[require(closesthit)]
void outputInput(out rt::ClosestHitInput<HitContext> input) { input = {}; }
//CHECK:                                             ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                             ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

[require(closesthit)]
void updateInput(inout rt::ClosestHitInput<HitContext> input) {}
//CHECK:                                               ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                               ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

[require(closesthit)]
void referenceInput(__ref rt::ClosestHitInput<HitContext> input) {}
//CHECK:                                                  ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                                  ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

[require(closesthit)]
void readInput(__constref rt::ClosestHitInput<HitContext> input) {}
//CHECK:                                                  ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                                  ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' is compiler-provided and may only be used as a value parameter

struct ClosestHitStage : rt::IClosestHitShader<HitContext>
{
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}
