//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature -entry legacyMiss -stage miss -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/command-line-stage.slang -experimental-feature -o tests/ray-tracing-2/compatibility/mixed-api/command-line-stage-cache.slang-module
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SERIALIZED): -experimental-feature tests/ray-tracing-2/compatibility/mixed-api/command-line-stage-cache.slang-module -entry legacyMiss -stage miss -no-codegen

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
// The serialized module has no source locations, so its expectations have no carets.
//SERIALIZED: the same module also uses the structural API
{
    typealias Context = MissContext;
    static void invoke(in rt::MissInput<Context> input) {}
}

// This function has no shader attribute or legacy intrinsic call. Selecting it with -stage miss
// must still diagnose the conflict after the module's ordinary semantic checks have completed.
// Serializing it without an entry request succeeds; selecting it from that module still fails.
void legacyMiss() {}
/*CHECK:
     ^^^^^^^^^^ cannot mix legacy and structural ray-tracing APIs in one module
     ^^^^^^^^^^ this declaration uses the legacy API
*/
//SERIALIZED: cannot mix legacy and structural ray-tracing APIs in one module
//SERIALIZED: this declaration uses the legacy API
