//TEST:SIMPLE: -experimental-feature -entry legacyMiss -stage miss -no-codegen
//TEST:SIMPLE: -experimental-feature -entry StructuralMiss -stage miss -no-codegen
//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/command-line-stage.slang -experimental-feature -o tests/ray-tracing-2/compatibility/mixed-api/command-line-stage-cache.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/compatibility/mixed-api/command-line-stage-cache.slang-module -entry legacyMiss -stage miss -no-codegen
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/compatibility/mixed-api/command-line-stage-cache.slang-module -entry StructuralMiss -stage miss -no-codegen

import slang.raytracing.portable;

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
{
    typealias Context = MissContext;
    static void invoke(in rt::MissInput<Context> input) {}
}

// This function has no shader attribute or legacy intrinsic call. Selecting it with -stage miss
// selects an ordinary native entry with its own signature. The neighboring structural stage
// remains selectable too, whether the module was loaded from source or a saved module.
void legacyMiss() {}
