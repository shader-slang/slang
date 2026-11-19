//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_4 -capability rayquery_opacity_micromap -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=ORDINARY): -target spirv -profile spirv_1_4 -capability rayquery_opacity_micromap -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry enabledA -profile sm_6_9 -restrictive-capability-check

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

// The native query operands come from rayFlags and Features, independently of
// the enclosing helper's instanceMask argument.
// HLSL: RayQuery<2{{[uU]? *, *}}1{{[uU]? *}}>
// HLSL: .TraceRayInline(

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

typedef RayQueryFeatures<RayQueryOpacityMicroMaps> MicromapFeatures;

struct QueryHelper<let instanceMask : uint, Features : IRayQueryFeature>
{
    [noinline]
    static uint trace<let rayFlags : RAY_FLAG>()
    {
        RayDesc ray;
        ray.Origin = float3(0, 0, 0);
        ray.Direction = float3(0, 0, 1);
        ray.TMin = 0;
        ray.TMax = 100;
        RayQuery<rayFlags, Features> query;
        query.TraceRayInline(scene, RAY_FLAG_NONE, instanceMask, ray);
        return query.Proceed();
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void enabledA()
{
    output[0] = QueryHelper<0xff, MicromapFeatures>::trace<RAY_FLAG_FORCE_NON_OPAQUE>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void enabledB()
{
    output[1] = QueryHelper<0xff, MicromapFeatures>::trace<RAY_FLAG_FORCE_NON_OPAQUE>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void ordinary()
{
    output[2] = QueryHelper<0xff, RayQueryFeatures<>>::trace<RAY_FLAG_NONE>();
}
