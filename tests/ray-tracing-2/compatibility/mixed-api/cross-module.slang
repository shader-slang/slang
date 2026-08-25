//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/legacy-stage-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_legacy_stage.slang-module
//TEST:SIMPLE: -experimental-feature -entry main -stage raygeneration -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -entry main -stage raygeneration -entry LegacyMiss -stage miss -no-codegen

import slang.raytracing;
import structural_rt_legacy_stage;

struct Payload
{
    uint value;
}

struct TraceContext : rt::ITraceContext
{
    typealias Payload = ::Payload;
    typealias AccelerationStructure = rt::AccelerationStructure;
    typealias Motion = rt::NoMotion;
}

struct ProgramLayout : rt::ITraceProgramLayout
{
    typealias TraceContext = ::TraceContext;
    typealias HitGroups = rt::NoHitGroups<::TraceContext>;
    typealias MissGroups = rt::NoMissGroups<::TraceContext>;
    typealias CallableGroups = rt::NoCallableGroups<::TraceContext>;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<ProgramLayout> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    Payload payload = {};
    rt::RayTracer<ProgramLayout> tracer;
    tracer.trace(desc, scene, program, payload);
}

// CHECK: cannot mix legacy and structural ray-tracing APIs in one linked program
// CHECK: this selected entry point uses the legacy API
// CHECK: another selected entry point uses the structural API
