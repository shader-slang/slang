"""Fixed ray-tracing stability shaders; scene construction lives in the runner."""

CASES = ("rayquery_visibility", "rayquery_closest", "raytrace_pipeline")


def generate(case):
    if case not in CASES:
        raise ValueError(case)
    common = """// Scene v1: 32x32 cells, opaque triangles at z=2 then z=1.
RaytracingAccelerationStructure scene;
StructuredBuffer<float4> rayOrigins;
RWStructuredBuffer<float4> outputData;
uniform uint count;
uniform uint dispatchWidth;

RayDesc makeRay(uint i)
{
    RayDesc ray;
    ray.Origin = rayOrigins[i].xyz;
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = rayOrigins[i].w;
    return ray;
}
"""
    if case == "raytrace_pipeline":
        return common + """
struct Payload { float4 result; };
[shader("raygeneration")]
void rayGenMain()
{
    uint3 tid = DispatchRaysIndex();
    uint i = tid.x + tid.y * dispatchWidth;
    if (i >= count) return;
    Payload payload;
    payload.result = float4(-99);
    TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, 255, 0, 1, 0, makeRay(i), payload);
    outputData[i] = payload.result;
}
[shader("closesthit")]
void closestHitMain(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    payload.result = float4(RayTCurrent(), float(PrimitiveIndex()), attributes.barycentrics);
}
[shader("miss")]
void missMain(inout Payload payload)
{
    payload.result = float4(-1);
}
"""
    flags = ("RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH"
             if case == "rayquery_visibility" else "RAY_FLAG_FORCE_OPAQUE")
    result = ("float4(query.CommittedStatus() == COMMITTED_TRIANGLE_HIT ? 1.0 : 0.0, 0, 0, 0)"
              if case == "rayquery_visibility" else
              "query.CommittedStatus() == COMMITTED_TRIANGLE_HIT ? "
              "float4(query.CommittedRayT(), float(query.CommittedPrimitiveIndex()), query.CommittedTriangleBarycentrics()) : float4(-1)")
    return common + f"""
[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{{
    uint i = tid.x + tid.y * dispatchWidth;
    if (i >= count) return;
    RayQuery<{flags}> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 255, makeRay(i));
    while (query.Proceed()) {{}}
    outputData[i] = {result};
}}
"""
