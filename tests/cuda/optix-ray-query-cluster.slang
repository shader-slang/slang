//TEST:SIMPLE(filecheck=CHECK): -target cuda
//CHECK: __global__ void kernel

[shader("compute")]
[numthreads(1, 1, 1)]
void kernel(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    RayQuery<RAY_FLAG_NONE> rayQuery;

    // Initialize ray
    RaytracingAccelerationStructure accel;
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.0f;
    ray.TMax = 1000.0f;

    rayQuery.TraceRayInline(accel, RAY_FLAG_NONE, 0xFF, ray);

    while (rayQuery.Proceed())
    {
        if (rayQuery.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)
        {
            // Test candidate cluster ID
            int candidateClusterId = rayQuery.CandidateClusterID();
            if (candidateClusterId >= 0)
            {
                rayQuery.CommitNonOpaqueTriangleHit();
            }
        }
    }

    // Test committed cluster ID
    if (rayQuery.CommittedStatus() != COMMITTED_NOTHING)
    {
        int committedClusterId = rayQuery.CommittedClusterID();
        // Do something with the committed cluster ID
    }
}
