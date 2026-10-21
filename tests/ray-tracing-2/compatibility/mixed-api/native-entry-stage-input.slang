//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry nativeClosestHit -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry nativeClosestHit -stage closesthit -no-codegen -DUNANNOTATED

import slang.raytracing;

struct Payload {}

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

// A matching native stage does not make a structural stage-input view into a native ABI
// parameter. Naming this function as a legacy entry point and using ClosestHitInput in its
// signature therefore mixes the two pipeline models in one module, with or without [shader].
#ifndef UNANNOTATED
[shader("closesthit")]
#endif
void nativeClosestHit(in rt::ClosestHitInput<HitContext> input) {}
/*CHECK:
     ^^^^^^^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^^^^^^^ this declaration uses the legacy API
     ^^^^^^^^^^^^^^^^ the same module also uses the structural API
*/
