//TEST:SIMPLE: -experimental-feature -entry Stages.main -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry Stages.main -no-codegen
//TEST:SIMPLE(filecheck=IR): -experimental-feature -entry Stages.main -stage closesthit -dump-ir -no-codegen

// A selected struct remains the logical entry point in this frontend-only slice. The compiler
// records the checked stage contract on `invoke`. Using a qualified, target-unsafe source name here
// also ensures the metadata preserves the full logical name that later physical-name encoding uses.
// IR: [entryPoint({{.*}}"Stages.main"
// IR-NEXT: [structuralRayTracingEntryPointInfo(
// IR-SAME: "Stages.main"

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

    void invoke(rt::ClosestHitInput<::HitContext> input)
    {
        outputBuffer[0] = 0x12345678;
    }
}
}
