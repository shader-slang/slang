// A function that can exit via a propagating error must be excluded from the return-in-place
// (NRVO) optimization: on the throwing path the `return` never runs, but any writes the body made
// to a local aliased to the return destination would already have landed in the caller's storage,
// so the caller's `try`/`catch` would observe a value the failed call never produced. `makeQuery`
// must therefore keep its own local ray query. This test verifies only that a throwing function is
// excluded from NRVO (the callee owns its query); a successful un-elided opaque return remains
// unsupported. -O0 keeps the callee out of line so its own local is observable. (`RayQuery` cannot
// be executed on the CPU, so this pins the SPIR-V lowering shape.)

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -stage compute -entry main -O0

enum E
{
    Fail
}

RaytracingAccelerationStructure g_tlas;
RWStructuredBuffer<uint> g_out;

RayQuery<RAY_FLAG_NONE> makeQuery(RayDesc ray, bool fail) throws E
{
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(g_tlas, RAY_FLAG_NONE, 0xff, ray);
    if (fail)
        throw E.Fail;
    return q;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = 0;
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 1;
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(g_tlas, RAY_FLAG_NONE, 0xff, ray);
    do
    {
        q = try makeQuery(ray, true);
    }
    catch (E e)
    {
    }
    g_out[0] = q.Proceed() ? 1u : 0u;
}

// `makeQuery` initializes its own local ray query, not the caller-provided return destination.
// CHECK: %makeQuery = OpFunction
// CHECK: [[LOCAL:%[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} Function
// CHECK: OpRayQueryInitializeKHR [[LOCAL]]
