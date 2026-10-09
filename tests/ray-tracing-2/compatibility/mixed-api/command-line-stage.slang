//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry legacyMiss -stage miss -no-codegen

import slang.raytracing;

struct TraceContext : rt::ITraceContext
{
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct MissContext : rt::IPayloadContext
{
    typealias TraceContext = ::TraceContext;
    typealias Payload = uint;
    typealias Record = void;
}

struct StructuralMiss : rt::IMissShader
/*CHECK:
       ^^^^^^^^^^^^^^ the same module also uses the structural API
*/
{
    typealias Context = MissContext;
    void invoke(in rt::MissInput<Context> input) {}
}

// This function has no shader attribute or legacy intrinsic call. Selecting it with -stage miss
// must still diagnose the conflict after the module's ordinary semantic checks have completed.
void legacyMiss() {}
/*CHECK:
     ^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^ this declaration uses the legacy API
*/
