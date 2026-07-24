// Regression test for https://github.com/shader-slang/slang/issues/12197
//
// NRVO must fire even when the return value is chosen on more than one control-flow path, as long
// as every `return` names the *same* function-local. findCommonReturnedLocalVar recurses into the
// `if` arms and accepts because both `return q;` name `q`; the local is then aliased to the return
// destination. Compiled at -O0 so `initQuery` stays its own function and we can confirm NRVO fired
// across the branchy body: `OpRayQueryInitializeKHR` targets the return-destination parameter, with
// no callee-local ray-query. (`RayQuery` cannot run on the CPU, so this pins the SPIR-V shape.)

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry main -O0

RaytracingAccelerationStructure g_tlas;
RWStructuredBuffer<uint> g_out;

RayQuery<RAY_FLAG_NONE> initQuery(RayDesc ray, bool flip)
{
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(g_tlas, RAY_FLAG_NONE, 0xFF, ray);
    if (flip)
        return q;
    else
        return q;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = float3(0, 0, -2);
    ray.TMin = 0;
    ray.Direction = float3(0, 0, 1);
    ray.TMax = 100;

    RayQuery<RAY_FLAG_NONE> q = initQuery(ray, tid.x == 0);
    g_out[0] = q.Proceed() ? 1u : 0u;
}

// Capture the ray-query type and its pointer type first, so we match the return-destination
// parameter rather than the leading `RayDesc` parameter.
// CHECK: [[RQ:%[A-Za-z0-9_]+]] = OpTypeRayQueryKHR
// CHECK: [[RQPTR:%[A-Za-z0-9_]+]] = OpTypePointer Function [[RQ]]
// CHECK: %initQuery = OpFunction
// CHECK: [[DEST:%[A-Za-z0-9_]+]] = OpFunctionParameter [[RQPTR]]
// No callee-local ray-query variable and no whole-handle copy between the parameters and the
// initialization — NRVO fired across the branchy body and eliminated both.
// CHECK-NOT: OpVariable [[RQPTR]] Function
// CHECK-NOT: OpCopyMemory
// CHECK: OpRayQueryInitializeKHR [[DEST]]
