// optix-ser.slang


//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGenerationMain -stage raygeneration

//TEST_INPUT: set scene = AccelerationStructure
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
        int clusterID = hit.GetClusterID();
        uint shaderTableIndex = hit.GetShaderTableIndex();
        // spriv and glsl lack these methods
        uint setShaderTableIndex = hit.SetShaderTableIndex(0);
        uint ialbedo = hit.LoadLocalRootTableConstant(0);
        SomeValues objSomeValues = hit.GetAttributes<SomeValues>();
        
        r += instanceIndex;
        r += instanceID;
        r += geometryIndex;
        r += primitiveIndex;
        r += objSomeValues.a;
        r += clusterID;
        r += shaderTableIndex;
        r += setShaderTableIndex;
        r += ialbedo;
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
    // CHECK: optixTraverse
    HitObject hit = HitObject::TraceRay(scene, 
        rayFlags, 
        instanceInclusionMask, 
        rayContributionToHitGroupIndex, 
        multiplierForGeometryContributionToHitGroupIndex, 
        missShaderIndex, 
        ray, 
        someValues);
    
    ReorderThread( hit );
    ReorderThread(hit, uint(idx & 3), 2);
    ReorderThread(uint(idx & 1), 1);

    outputBuffer[idx] = calcValue(hit);
    HitObject miss[2];
    miss[0] = HitObject::MakeMiss(0u, ray);
    miss[1] = HitObject::MakeMotionMiss(0u, ray, 1.f);

    uint hitGroupRecordIndex = 0;
    uint instanceIndex = 0xff;
    uint geometryIndex = 0;
    uint primitiveIndex = 0;
    uint hitKind = 0;
    BuiltInTriangleIntersectionAttributes attr = {0.01f, 0.2f};

    HitObject hitObj = HitObject::MakeHit(hitGroupRecordIndex, scene,
        instanceIndex,
        geometryIndex,
        primitiveIndex,
        hitKind,
        ray,
        attr);
    HitObject nopObj = HitObject::MakeNop();
    outputBuffer[idx] = uint(nopObj.IsNop());

    outputBuffer[idx] += calcValue(hit);
    outputBuffer[idx] += calcValue(miss[0]);
    outputBuffer[idx] += calcValue(miss[1]);
    outputBuffer[idx] += calcValue(hitObj);
    outputBuffer[idx] += calcValue(nopObj);

    // Change the payload
    SomeValues otherValues = { idx * -2, idx * 8.0f };

    HitObject::Invoke( scene, hit, otherValues );
    HitObject motionHitObj[2];
    motionHitObj[0] = HitObject::MakeMotionHit(
        scene,
        instanceIndex,
        geometryIndex,
        primitiveIndex,
        hitKind,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex,
        ray,
        0.f,
        attr);
    motionHitObj[1] = HitObject::MakeMotionHit(
        hitGroupRecordIndex,
        scene,
        instanceIndex,
        geometryIndex,
        primitiveIndex,
        hitKind,
        ray,
        0.f,
        attr);
    outputBuffer[idx] += calcValue(motionHitObj[0]);
    outputBuffer[idx] += calcValue(motionHitObj[1]);

    RayDesc rayD = hit.GetRayDesc();

    outputBuffer[idx] += uint(rayD.TMin > 0);
    outputBuffer[idx] += uint(rayD.TMax < ray.TMin);

}
