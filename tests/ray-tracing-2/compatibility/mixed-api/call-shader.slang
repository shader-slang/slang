//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry StructuralClosestHit -stage closesthit -no-codegen

import slang.raytracing;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct HitContext : rt::IHitContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = ::Payload;
    typealias Primitive = rt::TrianglePrimitive;
    typealias Record = void;
}

struct StructuralClosestHit : rt::IClosestHitShader
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^ the same module also uses the structural API
*/
{
    typealias Context = HitContext;
    void invoke(rt::ClosestHitInput<HitContext> input) {}
}

void legacyCallableDispatch(inout Payload payload)
/*CHECK:
     ^^^^^^^^^^^^^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^^^^^^^^^^^^^ this declaration uses the legacy API
*/
{
    CallShader(0, payload);
}
