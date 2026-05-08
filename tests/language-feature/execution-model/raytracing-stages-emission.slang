// Ray-tracing entry point emission across HLSL DXIL and SPIRV.
//
// Covers the four common RT stages: raygeneration (drives the trace),
// closesthit (returns hit data + may continue ray), miss (returns
// background data), and anyhit (filter intersection candidates with
// IgnoreHit / AcceptHitAndEndSearch).
//
// Each stage has stage-specific intrinsics: TraceRay / ReportHit /
// IgnoreHit / AcceptHitAndEndSearch and the `RayDesc`, `RayPayload`,
// `BuiltInTriangleIntersectionAttributes` types.

//TEST:SIMPLE(filecheck=CHECK_RG_HLSL): -target hlsl -stage raygeneration -entry rayGenMain
//TEST:SIMPLE(filecheck=CHECK_CH_HLSL): -target hlsl -stage closesthit -entry closestHitMain
//TEST:SIMPLE(filecheck=CHECK_MISS_HLSL): -target hlsl -stage miss -entry missMain
//TEST:SIMPLE(filecheck=CHECK_AH_HLSL): -target hlsl -stage anyhit -entry anyHitMain
//TEST:SIMPLE(filecheck=CHECK_RG_SPV): -target spirv-asm -stage raygeneration -entry rayGenMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_CH_SPV): -target spirv-asm -stage closesthit -entry closestHitMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MISS_SPV): -target spirv-asm -stage miss -entry missMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_AH_SPV): -target spirv-asm -stage anyhit -entry anyHitMain -emit-spirv-directly

struct Payload
{
    float4 color;
};

struct Attribs
{
    float2 bary;
};

RaytracingAccelerationStructure scene;
RWTexture2D<float4> output;

[shader("raygeneration")]
void rayGenMain()
{
    uint2 launchId = DispatchRaysIndex().xy;

    RayDesc ray;
    ray.Origin = float3(float(launchId.x), float(launchId.y), 0.0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    Payload payload;
    payload.color = float4(0, 0, 0, 0);

    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

    output[launchId] = payload.color;
}

[shader("closesthit")]
void closestHitMain(inout Payload payload, in Attribs attr)
{
    payload.color = float4(attr.bary.x, attr.bary.y, 1.0, 1.0);
}

[shader("miss")]
void missMain(inout Payload payload)
{
    payload.color = float4(0.1, 0.1, 0.1, 1.0);
}

[shader("anyhit")]
void anyHitMain(inout Payload payload, in Attribs attr)
{
    if (attr.bary.x < 0.05)
    {
        IgnoreHit();
    }
    else if (attr.bary.x > 0.95)
    {
        AcceptHitAndEndSearch();
    }
}

// Raygeneration HLSL.
// CHECK_RG_HLSL-DAG: TraceRay
// CHECK_RG_HLSL-DAG: DispatchRaysIndex

// ClosestHit HLSL — the inout payload + Attribs survive.
// CHECK_CH_HLSL-DAG: closestHitMain
// CHECK_CH_HLSL-DAG: Payload
// CHECK_CH_HLSL-DAG: Attribs

// Miss HLSL.
// CHECK_MISS_HLSL-DAG: missMain
// CHECK_MISS_HLSL-DAG: Payload

// AnyHit HLSL — IgnoreHit / AcceptHitAndEndSearch survive.
// CHECK_AH_HLSL-DAG: IgnoreHit
// CHECK_AH_HLSL-DAG: AcceptHitAndEndSearch

// SPIRV emits dedicated ray-tracing opcodes (KHR ray-tracing extension).
// CHECK_RG_SPV-DAG: RayGenerationKHR
// CHECK_RG_SPV-DAG: OpTraceRayKHR
// CHECK_CH_SPV-DAG: ClosestHitKHR
// CHECK_MISS_SPV-DAG: MissKHR
// CHECK_AH_SPV-DAG: AnyHitKHR
// CHECK_AH_SPV-DAG: OpIgnoreIntersectionKHR
// CHECK_AH_SPV-DAG: OpTerminateRayKHR
