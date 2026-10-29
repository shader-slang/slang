//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHitStage -stage closesthit -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry ClosestHitStage -stage closesthit -target metal -no-codegen

import slang.raytracing.portable;

struct PayloadData {}
struct MaterialRecord {}

struct MyTraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = MyTraceContext;
    typealias Payload = PayloadData;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = MaterialRecord;
}

[require(closesthit)]
void outputInput(out rt::ClosestHitInput<HitContext> input) { input = {}; }
//CHECK:                                             ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                             ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' may only be used as an 'in' parameter or an initialized local copy
//CHECK:                                                              ^ construction of a structural ray-tracing type
//CHECK:                                                              ^ structural ray-tracing type 'rt.ClosestHitInput<HitContext>' cannot be constructed by user code

[require(closesthit)]
void updateInput(inout rt::ClosestHitInput<HitContext> input) {}
//CHECK:                                               ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                               ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' may only be used as an 'in' parameter or an initialized local copy

[require(closesthit)]
void referenceInput(__ref rt::ClosestHitInput<HitContext> input) {}
//CHECK:                                                  ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                                  ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' may only be used as an 'in' parameter or an initialized local copy

[require(closesthit)]
void readInput(__constref rt::ClosestHitInput<HitContext> input) {}
//CHECK:                                                  ^^^^^ storage of a structural ray-tracing stage input
//CHECK:                                                  ^^^^^ ray-tracing stage input type 'rt.ClosestHitInput<HitContext>' may only be used as an 'in' parameter or an initialized local copy

struct ClosestHitStage : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static void invoke(rt::ClosestHitInput<HitContext> input) {}
}
