//TEST:SIMPLE: -experimental-feature -entry legacyMiss -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -no-codegen
//TEST:SIMPLE: -experimental-feature -entry StructuralClosestHit -stage closesthit -no-codegen
//TEST:SIMPLE: -experimental-feature -entry legacyMiss -stage miss -entry StructuralClosestHit -stage closesthit -no-codegen

// StructuralClosestHit implements the structural stage interface, while legacyMiss declares a
// native [shader("miss")] entry point in the same module. Each has its own valid signature.
// Explicit native selection, automatic discovery, and structural selection must all succeed.

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

struct StructuralClosestHit : rt::IClosestHitShader
{
    typealias Context = HitContext;
    static void invoke(rt::ClosestHitInput<HitContext> input) {}
}

[shader("miss")]
void legacyMiss() {}
