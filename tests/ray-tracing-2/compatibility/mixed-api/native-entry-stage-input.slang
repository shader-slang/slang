//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry nativeClosestHit -stage closesthit -no-codegen

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
// signature therefore mixes the two pipeline models in one module.
[shader("closesthit")]
void nativeClosestHit(in rt::ClosestHitInput<HitContext> input) {}
/*CHECK:
     ^^^^^^^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^^^^^^^ this declaration uses the structural API
     ^^^^^^^^^^^^^^^^ the same module also uses the legacy API
*/
