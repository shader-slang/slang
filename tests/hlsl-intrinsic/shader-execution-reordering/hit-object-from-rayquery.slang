// hit-object-from-rayquery.slang
// Test DXR 1.3 FromRayQuery methods

//TEST:SIMPLE(filecheck=CHECK): -target dxil -entry rayGenerationMain -stage raygeneration -profile sm_6_9

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0, 0, 0, 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:set accelStruct = ubuffer(data=[0], stride=4)
RaytracingAccelerationStructure accelStruct;

struct MyAttribs
{
    float2 barycentrics;
};

void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    // Create a RayQuery
    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(accelStruct, RAY_FLAG_NONE, 0xFF, ray);

    // Process the query to find a hit
    while (q.Proceed())
    {
        if (q.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE)
        {
            q.CommitProceduralPrimitiveHit(0.5f);
        }
    }

    uint result = 0;

    // DXR 1.3: FromRayQuery - create HitObject from committed RayQuery hit
    // CHECK: dx::HitObject::FromRayQuery
    if (q.CommittedStatus() != COMMITTED_NOTHING)
    {
        HitObject hitObj = HitObject::FromRayQuery(q);
        result = hitObj.GetInstanceIndex();
    }

    // DXR 1.3: FromRayQuery with custom attributes - for procedural hits
    // CHECK: dx::HitObject::FromRayQuery
    if (q.CommittedStatus() == COMMITTED_PROCEDURAL_PRIMITIVE_HIT)
    {
        MyAttribs attribs;
        attribs.barycentrics = float2(0.25f, 0.25f);
        HitObject hitObj2 = HitObject::FromRayQuery(q, 128, attribs);
        result += hitObj2.GetHitKind();
    }

    outputBuffer[idx] = result;
}
