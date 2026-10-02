//TEST:SIMPLE(filecheck=DXIL):-target dxil-asm -profile sm_6_6 -O3 -entry computeMain -conformance Near:IGeometry=3 -conformance Far:IGeometry=7
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -profile sm_6_6 -O3 -entry computeMain -conformance Near:IGeometry=3 -conformance Far:IGeometry=7

// Both query-state reads and acceptance must remain inside the selected helper.
// CommittedRayT changes after acceptance, so repeated iterations also exercise
// the boundary that prevents treating query getters as freely movable pure values.
interface IGeometry
{
    static float distance(float3 origin, float limit);
}

struct Near : IGeometry
{
    static float distance(float3 origin, float limit)
    {
        return min(abs(origin.x), limit);
    }
}

struct Far : IGeometry
{
    static float distance(float3 origin, float limit)
    {
        return min(length(origin), limit);
    }
}

[__specializePerConformance]
void intersectCase<G : IGeometry>(G geometry, inout RayQuery<0> query)
{
    float distance = G.distance(query.CandidateObjectRayOrigin(), query.CommittedRayT());
    if (distance > query.RayTMin())
        query.CommitProceduralPrimitiveHit(distance);
}

RaytracingAccelerationStructure scene;
StructuredBuffer<uint> geometryTypes;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    RayQuery<0> query;
    RayDesc ray = {float3(tid.x, 0, 0), 0.01, float3(0, 0, 1), 1000};
    query.TraceRayInline(scene, 0, 0xff, ray);
    while (query.Proceed())
    {
        if (query.CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE)
        {
            uint id = geometryTypes[query.CandidatePrimitiveIndex()];
            intersectCase(createDynamicObject<IGeometry>(id, 0u), query);
        }
    }
    outputBuffer[tid.x] = query.CommittedRayT();
}

// DXIL: call void @dx.op.rayQuery_CommitProceduralPrimitiveHit
// DXIL: call void @dx.op.rayQuery_CommitProceduralPrimitiveHit
// DXIL-NOT: call void @dx.op.rayQuery_CommitProceduralPrimitiveHit
// SPIRV: OpRayQueryGetIntersectionObjectRayOriginKHR
// SPIRV: OpRayQueryGenerateIntersectionKHR
// SPIRV: OpRayQueryGetIntersectionObjectRayOriginKHR
// SPIRV: OpRayQueryGenerateIntersectionKHR
// SPIRV-NOT: OpRayQueryGenerateIntersectionKHR
