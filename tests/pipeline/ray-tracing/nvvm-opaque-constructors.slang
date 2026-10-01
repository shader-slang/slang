// Explicit constructor expressions exercise the canonical void(destination) intrinsic.
//TEST:SIMPLE: -target hlsl -profile lib_6_9 -capability hlsl_nvapi -capability ser_hlsl_native -entry hitMain -stage raygeneration -o opaque-hit.hlsl.actual
//TEST:SIMPLE: -target cuda -entry hitMain -stage raygeneration -capability cuda_sm_8_0 -o opaque-hit.cu.actual
//TEST:SIMPLE: -target spirv -entry hitMain -stage raygeneration -capability spvShaderInvocationReorderNV -o opaque-hit.spv.actual
//TEST:SIMPLE: -target ptx -emit-cuda-via-nvvm -entry hitMain -stage raygeneration -capability cuda_sm_8_0 -o opaque-hit.ptx.actual
//TEST:SIMPLE: -target hlsl -profile cs_6_5 -entry queryMain -o opaque-query.hlsl.actual
//TEST:SIMPLE: -target spirv -entry queryMain -stage compute -o opaque-query.spv.actual

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void hitMain()
{
    HitObject object = HitObject();
    output[0] = object.IsNop();
}

[shader("compute")]
[numthreads(1,1,1)]
void queryMain()
{
    RayQuery<RAY_FLAG_NONE> query = RayQuery<RAY_FLAG_NONE>();
    RayDesc ray = { float3(0, 0, 1), 0, float3(0, 0, -1), 10 };
    query.TraceRayInline(scene, RAY_FLAG_NONE, 255, ray);
    while (query.Proceed()) {}
    output[0] = query.CommittedStatus();
}
