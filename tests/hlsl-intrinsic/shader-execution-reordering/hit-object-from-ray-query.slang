// Test HitObject::FromRayQuery() - DXR 1.3 only

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_9 -capability ser_dxr

// CHECK: dx::HitObject::FromRayQuery

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    uint r = 0;

    RayQuery<RAY_FLAG_NONE> rq;
    rq.TraceRayInline(scene, RAY_FLAG_NONE, 0xFF, ray);

    while (rq.Proceed())
    {
        if (rq.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE)
        {
            rq.CommitNonOpaqueTriangleHit();
        }
    }

    HitObject hitFromQuery = HitObject::FromRayQuery(rq);

    // CHECK: .IsHit
    if (hitFromQuery.IsHit())
    {
        r += 1;
        // CHECK: .GetPrimitiveIndex
        r += hitFromQuery.GetPrimitiveIndex();
    }
    // CHECK: .IsMiss
    else if (hitFromQuery.IsMiss())
    {
        r += 2;
    }
    // CHECK: .IsNop
    else if (hitFromQuery.IsNop())
    {
        r += 4;
    }

    outputBuffer[idx] = r;
}
