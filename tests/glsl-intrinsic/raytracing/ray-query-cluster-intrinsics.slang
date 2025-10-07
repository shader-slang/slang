//TEST:SIMPLE(filecheck=CHECK_GLSL): -entry computeMain -target glsl -profile cs_6_5
//TEST:SIMPLE(filecheck=CHECK_SPV): -entry computeMain -target spirv-assembly -profile cs_6_5

//CHECK_GLSL:rayQueryGetIntersectionClusterIdNV
//CHECK_SPV:OpRayQueryGetIntersectionClusterIdNV

uniform RaytracingAccelerationStructure accelStruct;

float GetRayT<let RAY_QUERY_FLAGS: uint>(uint rayInlineFlags)
{
    RayDesc ray;
    ray.Origin = float3(0.1f, 0.1f, 0.0f);
    ray.Direction = float3(0.0f, 0.0f, 1.0f);
    ray.TMin = 0.01f;
    ray.TMax = 1e4f;

    RayQuery<RAY_QUERY_FLAGS> rq;
    rq.TraceRayInline(accelStruct, rayInlineFlags, 0xff, ray);
    bool proceed = rq.Proceed();

    if( proceed )
    {
        switch( rq.CandidateType() )
        {
            case CANDIDATE_NON_OPAQUE_TRIANGLE:
                int candidateClusterId = rq.CandidateClusterID();
                if (candidateClusterId >= 0)
                {
                    rq.CommitNonOpaqueTriangleHit();
                }
                rq.Abort();
                return rq.CommittedRayT();

            case CANDIDATE_PROCEDURAL_PRIMITIVE:
                rq.CommitProceduralPrimitiveHit(0.5f);
                rq.Abort();
                return rq.CommittedRayT();

            default:
                rq.Abort();
                return 0.0f;
        }
    }
    else
    {
        if( rq.CommittedStatus() == COMMITTED_TRIANGLE_HIT )
        {
            rq.Abort();
            return rq.CommittedRayT();
        }
    }

    int committedClusterId = rq.CommittedClusterID();
    ray.TMin = 0.01f + committedClusterId;
    return 0.0f;
}

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int idx = dispatchThreadID.x;
    float val = 0.0f;

    const uint rayFlags[] = {
        RAY_FLAG_NONE,
    };
    const uint numRayFlags = sizeof(rayFlags) / sizeof(uint);

    // RAY_FLAG_FORCE_NON_OPAQUE, RAY_FLAG_CULL_OPAQUE, RAY_FLAG_SKIP_TRIANGLES returns 0 instead of 0.5
    for( uint i = 0; i < numRayFlags; ++i )
    {
        val += GetRayT<RAY_FLAG_NONE>(rayFlags[i]);
    }

    outputBuffer[idx] = val;
}
