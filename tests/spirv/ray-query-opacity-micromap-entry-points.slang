//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_4 -capability rayquery_opacity_micromap -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=ORDINARY): -target spirv -profile spirv_1_4 -capability rayquery_opacity_micromap -fvk-use-entrypoint-name

// A shared, non-inlined helper opts in both callers, while the ordinary entry
// point in the same module must not acquire the execution mode.
// CHECK-DAG: OpEntryPoint GLCompute %enabledA "enabledA"
// CHECK-DAG: OpEntryPoint GLCompute %enabledB "enabledB"
// CHECK-DAG: OpEntryPoint GLCompute %ordinary "ordinary"
// CHECK-DAG: OpExecutionModeId %enabledA OpacityMicromapIdKHR %[[ENABLED:[A-Za-z0-9_]+]]
// CHECK-DAG: OpExecutionModeId %enabledB OpacityMicromapIdKHR %[[ENABLED]]
// CHECK: %[[ENABLED]] = OpConstantTrue %bool
// CHECK: OpFunctionCall

// Check the whole module independently of the order of the opted-in modes.
// ORDINARY-NOT: OpExecutionModeId %ordinary OpacityMicromapIdKHR

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

[noinline]
uint trace<let queryFlags : RAYQUERY_FLAG>()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 100;
    RayQuery<RAY_FLAG_NONE, queryFlags> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    return query.Proceed();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void enabledA()
{
    output[0] = trace<RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void enabledB()
{
    output[1] = trace<RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void ordinary()
{
    output[2] = trace<RAYQUERY_FLAG_NONE>();
}
