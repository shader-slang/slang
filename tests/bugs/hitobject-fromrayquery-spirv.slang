//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry main -stage raygeneration
// CHECK: OpEntryPoint

// Regression test for https://github.com/shader-slang/slang/issues/10307
// HitObject.FromRayQuery fails on SPIR-V with error 41009 in hlsl.meta.slang:23722.
// Other HitObject methods (MakeNop, Invoke, IsNop) work correctly on SPIR-V.

RaytracingAccelerationStructure accelStruct;

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.0;
    ray.TMax = 1000.0;

    RayQuery<RAY_FLAG_FORCE_OPAQUE> rq;
    rq.TraceRayInline(accelStruct, RAY_FLAG_FORCE_OPAQUE, 0xFF, ray);
    rq.Proceed();

    HitObject hitObj = HitObject.FromRayQuery(rq);
}
