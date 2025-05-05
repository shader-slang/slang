//TEST:SIMPLE(filecheck=CHECK): -target cuda
//CHECK: __global__ void __closesthit__closestHitShaderA

uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0, 0, 0, 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct SomeValues
{
    int a;
    float b;
};

uint calcValue(HitObject hit)
{
    uint r = 0;

    if (hit.IsHit())
    {
        uint instanceIndex = hit.GetInstanceIndex();
        uint instanceID = hit.GetInstanceID();
        uint geometryIndex = hit.GetGeometryIndex();
        uint primitiveIndex = hit.GetPrimitiveIndex();
        int clusterIndex = hit.GetClusterID();

        SomeValues objSomeValues = hit.GetAttributes<SomeValues>();

        r += instanceIndex;
        r += instanceID;
        r += geometryIndex;
        r += primitiveIndex;
        r += objSomeValues.a;
        r += clusterIndex;

    }

    return r;
}

void rayGenerationMain()
{
    int2 launchID = int2(DispatchRaysIndex().xy);
    int2 launchSize = int2(DispatchRaysDimensions().xy);

    int idx = launchID.x;

    SomeValues someValues = { idx, idx * 2.0f };

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    RAY_FLAG rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    uint instanceInclusionMask = 0xff;
    uint rayContributionToHitGroupIndex = 0;
    uint multiplierForGeometryContributionToHitGroupIndex = 4;
    uint missShaderIndex = 0;
    // SPIRV: OpHitObjectTraceRayNV
    HitObject hit = HitObject::TraceRay(scene,
        rayFlags,
        instanceInclusionMask,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex,
        missShaderIndex,
        ray,
        someValues);
}
