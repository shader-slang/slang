// Companion to 13705-cpu-acceleration-structure-placeholder.slang: the CPU prelude defines
// RaytracingAccelerationStructure only so that a shader can declare or pass one. Tracing a ray
// against it is unsupported on the CPU targets and must be rejected by Slang, not left to the
// downstream C++ compiler.
//
// The `-target cpp` row is temporary: open PR #12282 (CPU RayQuery) makes this TraceRayInline
// compile on the C++ source path, and deletes or inverts that row when it lands. The
// `-emit-cpu-via-llvm` row uses the separate `llvm` capability atom, which stays unsupported.
// `non-exhaustive` because E36107 is followed by notes pointing into hlsl.meta.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target host-callable -emit-cpu-via-llvm -entry computeMain -stage compute

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
/*CHECK:
          ^^^^^^^^^^^^^^ see using of 'TraceRayInline'
*/
    outputBuffer[0] = query.Proceed() ? 1.0 : 0.0;
}
