//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/identity/stage-name-collision-common.slang -o tests/ray-tracing-2/ir/identity/structural_stage_name_collision_common.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/identity/stage-name-collision-a.slang -o tests/ray-tracing-2/ir/identity/structural_stage_name_collision_a.slang-module
//TEST:COMPILE: -experimental-feature tests/ray-tracing-2/ir/identity/stage-name-collision-b.slang -o tests/ray-tracing-2/ir/identity/structural_stage_name_collision_b.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature -target hlsl -profile sm_6_5 -entry main -stage raygeneration

import slang.raytracing;
import structural_stage_name_collision_common;
import structural_stage_name_collision_a;
import structural_stage_name_collision_b;

struct Schema : rt::ITraceProgramSchema
{
    typealias TraceContext = SharedTraceContext;
    typealias HitGroups = rt::NoHitGroups;
    typealias MissShaders = rt::MissShaderList<CollisionStageA, CollisionStageB>;
    typealias CallableShaders = rt::NoCallableShaders;
}

rt::AccelerationStructure scene;
rt::TraceProgramDescriptor<Schema> program;

[shader("raygeneration")]
void main()
{
    rt::RayTraversalDesc desc = {};
    SharedPayload payload = {};
    rt::RayTracer<Schema> tracer;
    tracer.trace(desc, scene, program, payload);
    /*CHECK:
           ^ conflicting structural ray-tracing entry-point name
           ^ physical entry-point name 'Miss' is requested by distinct structural stages
    */
}
