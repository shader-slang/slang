//DIAGNOSTIC_TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -profile spirv_1_4 -capability rayquery -restrictive-capability-check
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -profile sm_6_5 -restrictive-capability-check
//DIAGNOSTIC_TEST:SIMPLE(filecheck=METAL): -target metal -entry main
//TEST:SIMPLE(filecheck=METAL_OK): -target metal -entry main -DQUERY_FLAGS=0

// The requirement follows the numeric query flags after specialization, so using
// a literal instead of the named flag cannot bypass capability checking.
// SPIRV: error{{.*}}entry point uses capabilities not in specified profile
// SPIRV: specified profile 'spirv_1_4'. Missing capabilities are:
// HLSL: error{{.*}}entry point uses capabilities not in specified profile
// HLSL: Missing capabilities are: 'sm_6_9'
// METAL: error{{.*}}static assertion failed
// METAL: Metal does not support opacity micromaps in ray queries
// METAL_OK: ->reset(

#ifndef QUERY_FLAGS
#define QUERY_FLAGS 1
#endif

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 100;
    RayQuery<RAY_FLAG_NONE, QUERY_FLAGS> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    output[0] = query.Proceed();
}
