//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute
// CPP: CandidateType
// CPP: CandidateProceduralPrimitiveNonOpaque
// CPP: CommitProceduralPrimitiveHit
//
// Slang's render-test scene currently contains only opaque triangles, so this test validates C++
// emission rather than procedural traversal at runtime. The paired slang-rhi PR provides executable
// CPU coverage for procedural and non-opaque commits, abort/skip behavior, and matrix accessors:
// https://github.com/shader-slang/slang-rhi/pull/803

RaytracingAccelerationStructure scene;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    RayDesc ray = {float3(0.0), 0.0, float3(0.0, 0.0, 1.0), 10.0};
    RayQuery<RAY_FLAG_NONE> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    while (query.Proceed())
    {
        if (query.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE &&
            query.CandidateProceduralPrimitiveNonOpaque())
            query.CommitProceduralPrimitiveHit(1.0);
    }
}
