//TEST:SIMPLE: -experimental-feature -entry Stages.main -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Stages.main -no-codegen
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry Stages.main -stage closesthit -dump-ir -no-codegen

// A selected struct remains the logical entry point in this frontend-only slice. The compiler
// retains its checked conformance on `invoke`; the ordinary entry-point decoration retains its
// qualified source name.
// IR: [entryPoint({{.*}}"Stages.main"
// IR-NEXT: [structuralRayTracingEntryPointInfo({{%[A-Za-z0-9_]+}})]
// The selected static method has exactly one parameter, input. No stage object is passed as this.
// IR: [nameHint("Stages.main.invoke")]
// IR-NEXT: func {{.*}} : Func(Void, specialize({{.*}}))
// IR-NEXT: {
// IR-NEXT: block {{.*}}(
// IR-NEXT: [nameHint("input")]
// IR-NEXT: param {{.*}} : specialize({{.*}})):
// IR-NEXT: let

import slang.raytracing;

RWStructuredBuffer<uint> outputBuffer;

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

namespace Stages
{
struct main : rt::IClosestHitShader
{
    typealias Context = ::HitContext;
    // This ordinary field does not add a receiver to the selected static function.
    uint state;

    static void invoke(rt::ClosestHitInput<::HitContext> input)
    {
        outputBuffer[0] = 0x12345678;
    }
}
}
