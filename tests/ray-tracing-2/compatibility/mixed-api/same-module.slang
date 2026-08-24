//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry legacyMiss -stage miss -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry StructuralClosestHit -stage closesthit -no-codegen

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

struct StructuralClosestHit : rt::IClosestHitShader<HitContext>
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^ the same module also uses the structural API
*/
{
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

[shader("miss")]
void legacyMiss() {}
/*CHECK:
     ^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^ this declaration uses the legacy API
*/
