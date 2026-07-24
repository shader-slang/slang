// Regression test for https://github.com/shader-slang/slang/issues/12197
//
// The two RayQuery positive tests run at -O3, where `initQuery` is inlined into `main`, so they
// prove the caller's query ends up connected but never confirm *why*: that NRVO makes the callee
// initialize the return-destination parameter directly. This test compiles at -O0 so `initQuery`
// stays its own function, and pins that premise — `OpRayQueryInitializeKHR` targets the callee's
// return-destination parameter (an OpFunctionParameter), with no callee-local ray-query variable
// and no whole-handle OpLoad/OpStore copy. This is the positive counterpart to the -O0 negative
// tests (defer / throws / aliasing bail), which rely on exactly this "NRVO => Initialize on a
// parameter" shape. (`RayQuery` cannot run on the CPU, so this pins the SPIR-V lowering shape.)

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry main -O0

RaytracingAccelerationStructure g_tlas;
RWStructuredBuffer<uint> g_out;

RayQuery<RAY_FLAG_NONE> initQuery(RayDesc ray)
{
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(g_tlas, RAY_FLAG_NONE, 0xFF, ray);
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

    RayQuery<RAY_FLAG_NONE> q = initQuery(ray);
    g_out[0] = q.Proceed() ? 1u : 0u;
}

// The callee initializes its return-destination parameter in place — NRVO gave the named local `q`
// no storage of its own, so the query operand of OpRayQueryInitializeKHR is the ray-query-typed
// parameter itself. Capture the ray-query type and its pointer type first, so we match the
// return-destination parameter rather than the leading `RayDesc` parameter.
// CHECK: [[RQ:%[A-Za-z0-9_]+]] = OpTypeRayQueryKHR
// CHECK: [[RQPTR:%[A-Za-z0-9_]+]] = OpTypePointer Function [[RQ]]
// CHECK: %initQuery = OpFunction
// CHECK: [[DEST:%[A-Za-z0-9_]+]] = OpFunctionParameter [[RQPTR]]
// Between the function's parameters and its ray-query initialization there must be no callee-local
// ray-query variable and no whole-handle copy of one — NRVO eliminated both.
// CHECK-NOT: OpVariable [[RQPTR]] Function
// CHECK-NOT: OpCopyMemory
// CHECK: OpRayQueryInitializeKHR [[DEST]]
