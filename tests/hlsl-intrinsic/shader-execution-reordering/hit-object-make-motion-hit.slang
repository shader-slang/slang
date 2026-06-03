// hit-object-make-motion-hit.slang

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderMotionNV

// SPIRV-DAG: OpExtension "SPV_NV_ray_tracing_motion_blur"
// SPIRV-DAG: OpCapability RayTracingMotionBlurNV
// SPIRV-DAG: OpExtension "SPV_NV_shader_invocation_reorder"
// SPIRV-DAG: OpCapability ShaderInvocationReorderNV

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
        SomeValues attrs = hit.GetAttributes<SomeValues>();
        r += attrs.a;
    }

    return r;
}

void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    SomeValues attrs = { idx, idx * 2.0f };

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    uint hitKind = 0;
    uint rayContributionToHitGroupIndex = 0;
    uint multiplierForGeometryContributionToHitGroupIndex = 4;
    float currentTime = idx / 4.0f;

    // SPIRV-DAG: OpHitObjectRecordHitMotionNV
    HitObject computedIndexHit = HitObject::MakeMotionHit(
        scene,
        idx,
        idx * 2,
        idx * 3,
        hitKind,
        rayContributionToHitGroupIndex,
        multiplierForGeometryContributionToHitGroupIndex,
        ray,
        currentTime,
        attrs);

    // SPIRV-DAG: OpHitObjectRecordHitWithIndexMotionNV
    HitObject explicitIndexHit = HitObject::MakeMotionHit(
        0,
        scene,
        idx,
        idx * 2,
        idx * 3,
        hitKind,
        ray,
        currentTime,
        attrs);

    outputBuffer[idx] = calcValue(computedIndexHit) + calcValue(explicitIndexHit);
}
