//TEST:COMPILE: tests/ray-tracing-2/compatibility/mixed-api/legacy-helper-module.slang -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_open_legacy_helper.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/compatibility/mixed-api/open-reachable-stage-plugin.slang -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_open_legacy_helper.slang-module -o tests/ray-tracing-2/compatibility/mixed-api/structural_rt_mixed_open_stage.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_open_legacy_helper.slang-module -r tests/ray-tracing-2/compatibility/mixed-api/structural_rt_mixed_open_stage.slang-module -entry main -stage raygeneration -target hlsl

import slang.raytracing;
import structural_rt_legacy_helper;
import structural_rt_mixed_open_stage;

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = OpenTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::OpenMissShaders<IOpenMissTag>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    LegacyPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
}

// CHECK: cannot mix legacy and structural ray-tracing APIs in one selected entry point
// CHECK: legacy ray-tracing API is called here
// CHECK: the same selected entry point reaches the structural API
