//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry nativeClosestHit -stage closesthit -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry nativeClosestHit -stage closesthit -no-codegen -DUNANNOTATED
//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/native-entry-stage-input.slang -experimental-feature -DUNANNOTATED -o tests/ray-tracing-2/compatibility/mixed-api/native-entry-stage-input-cache.slang-module
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SERIALIZED): -experimental-feature tests/ray-tracing-2/compatibility/mixed-api/native-entry-stage-input-cache.slang-module -entry nativeClosestHit -stage closesthit -no-codegen

import slang.raytracing.portable;

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
// parameter. Reject the input in this selected native signature, with or without [shader],
// rather than rejecting unrelated native and structural entries in the same module.
#ifndef UNANNOTATED
[shader("closesthit")]
#endif
void nativeClosestHit(in rt::ClosestHitInput<HitContext> input) {}
/*CHECK:
                             ^^^^^^^^^^^^^^^ invalid native entry point signature
                             ^^^^^^^^^^^^^^^ cannot be used in a native entry point signature
*/
// The saved module has no source locations; its checks still belong beside this parameter.
//SERIALIZED: invalid native entry point signature
//SERIALIZED: cannot be used in a native entry point signature
