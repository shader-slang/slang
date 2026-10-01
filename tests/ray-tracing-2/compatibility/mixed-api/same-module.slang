//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry legacyMiss -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry StructuralClosestHit -stage closesthit -no-codegen

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
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^ the same module also uses the structural API
*/
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

[shader("miss")]
void legacyMiss() {}
/*CHECK:
     ^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^ this declaration uses the legacy API
*/
