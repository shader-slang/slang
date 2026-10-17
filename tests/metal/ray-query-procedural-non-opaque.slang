//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain

// MTL: is_candidate_non_opaque_bounding_box()

// LIB: computeMain

uniform RaytracingAccelerationStructure accelStruct;
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    RayDesc ray;
    ray.Origin = float3(0.0f, 0.0f, -1.0f);
    ray.Direction = float3(0.0f, 0.0f, 1.0f);
    ray.TMin = 0.0f;
    ray.TMax = 1e4f;

    RayQuery<RAY_FLAG_NONE> rq;
    rq.TraceRayInline(accelStruct, RAY_FLAG_NONE, 0xff, ray);

    uint nonOpaqueCount = 0;
    while (rq.Proceed())
    {
        if (rq.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE && rq.CandidateProceduralPrimitiveNonOpaque())
            nonOpaqueCount += 1;
    }

    outputBuffer[0] = nonOpaqueCount;
}
