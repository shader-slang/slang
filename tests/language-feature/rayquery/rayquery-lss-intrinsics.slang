// Real-compilation coverage for the line-swept-spheres / sphere ray-query
// accessors backed by SPV_NV_linear_swept_spheres (SPIR-V), GL_NV_linear_swept_spheres
// (GLSL via glslang), and NVAPI (HLSL).

// Direct SPIR-V via slang's own backend (disassembled so the Intersection
// operand is checkable as text):
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly
// GLSL path actually compiled through glslang into SPIR-V:
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-via-glsl
// HLSL emission with NVAPI (NvRt* calls):
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile sm_6_5 -stage compute -entry computeMain -capability hlsl_nvapi

RaytracingAccelerationStructure scene;
RWStructuredBuffer<float> gOutput;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, 0.0);
    ray.TMin = 0.1;
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMax = 1000.0;

    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);

    float accum = 0.0;
    while (q.Proceed())
    {
        if (q.CandidateIsNonOpaqueSphereNV())
        {
            float4 s = q.CandidateSphereObjectPositionAndRadiusNV();
            accum += s.x + s.w;
            q.CommitNonOpaqueTriangleHit();
        }
        else if (q.CandidateIsNonOpaqueLssNV())
        {
            float2x4 lss = q.CandidateLssObjectPositionsAndRadiiNV();
            accum += lss[0].x + lss[1].w;
            accum += q.CandidateLssHitParameterNV();
            q.CommitNonOpaqueTriangleHit();
        }
    }

    // Committed-side accessors.
    if (q.CommittedIsSphereNV())
    {
        float4 cs = q.CommittedSphereObjectPositionAndRadiusNV();
        accum += cs.x + cs.w;
    }
    else if (q.CommittedIsLssNV())
    {
        float2x4 clss = q.CommittedLssObjectPositionsAndRadiiNV();
        accum += clss[0].x + clss[1].w;
        accum += q.CommittedLssHitParameterNV();
    }

    gOutput[0] = accum;
}

// Slang's direct SPIR-V backend must emit the raw NV ray-query ops + extension.
// Each op is checked in both its candidate form (Intersection operand %uint_0)
// and its committed form (Intersection operand %uint_1), so a swapped or
// duplicated selector at the meta.slang call site would fail the test rather
// than silently mis-reading the wrong intersection record.
// SPIRV: OpExtension "SPV_NV_linear_swept_spheres"
// SPIRV-DAG: OpRayQueryIsSphereHitNV {{.*}} %uint_0
// SPIRV-DAG: OpRayQueryIsSphereHitNV {{.*}} %uint_1
// SPIRV-DAG: OpRayQueryGetIntersectionSpherePositionNV {{.*}} %uint_0
// SPIRV-DAG: OpRayQueryGetIntersectionSpherePositionNV {{.*}} %uint_1
// SPIRV-DAG: OpRayQueryGetIntersectionSphereRadiusNV {{.*}} %uint_0
// SPIRV-DAG: OpRayQueryGetIntersectionSphereRadiusNV {{.*}} %uint_1
// SPIRV-DAG: OpRayQueryIsLSSHitNV {{.*}} %uint_0
// SPIRV-DAG: OpRayQueryIsLSSHitNV {{.*}} %uint_1
// SPIRV-DAG: OpRayQueryGetIntersectionLSSPositionsNV {{.*}} %uint_0
// SPIRV-DAG: OpRayQueryGetIntersectionLSSPositionsNV {{.*}} %uint_1
// SPIRV-DAG: OpRayQueryGetIntersectionLSSRadiiNV {{.*}} %uint_0
// SPIRV-DAG: OpRayQueryGetIntersectionLSSRadiiNV {{.*}} %uint_1
// SPIRV-DAG: OpRayQueryGetIntersectionLSSHitValueNV {{.*}} %uint_0
// SPIRV-DAG: OpRayQueryGetIntersectionLSSHitValueNV {{.*}} %uint_1

// The GLSL->glslang path must also produce the NV ops (proves the GLSL
// intrinsic strings + prelude helper actually compile through glslang).
// glslang lowers the candidate/committed boolean to signed-int constants
// (%int_0 / %int_1), so the operand binding is checked against those.
// GLSLSPV-DAG: OpRayQueryIsSphereHitNV {{.*}} %int_0
// GLSLSPV-DAG: OpRayQueryIsSphereHitNV {{.*}} %int_1
// GLSLSPV-DAG: OpRayQueryGetIntersectionSpherePositionNV {{.*}} %int_0
// GLSLSPV-DAG: OpRayQueryGetIntersectionSpherePositionNV {{.*}} %int_1
// GLSLSPV-DAG: OpRayQueryGetIntersectionLSSPositionsNV {{.*}} %int_0
// GLSLSPV-DAG: OpRayQueryGetIntersectionLSSPositionsNV {{.*}} %int_1
// GLSLSPV-DAG: OpRayQueryGetIntersectionLSSHitValueNV {{.*}} %int_0
// GLSLSPV-DAG: OpRayQueryGetIntersectionLSSHitValueNV {{.*}} %int_1
// GLSLSPV-DAG: OpRayQueryGetIntersectionSphereRadiusNV {{.*}} %int_0
// GLSLSPV-DAG: OpRayQueryGetIntersectionSphereRadiusNV {{.*}} %int_1
// GLSLSPV-DAG: OpRayQueryGetIntersectionLSSRadiiNV {{.*}} %int_0
// GLSLSPV-DAG: OpRayQueryGetIntersectionLSSRadiiNV {{.*}} %int_1
// GLSLSPV-DAG: OpRayQueryIsLSSHitNV {{.*}} %int_0
// GLSLSPV-DAG: OpRayQueryIsLSSHitNV {{.*}} %int_1

// HLSL emission uses the NVAPI ray-query calls.
// HLSL-DAG: NvRtCandidateIsNonOpaqueSphere
// HLSL-DAG: NvRtCandidateSphereObjectPositionAndRadius
// HLSL-DAG: NvRtCandidateLssObjectPositionsAndRadii
// HLSL-DAG: NvRtCandidateLssHitParameter
// HLSL-DAG: NvRtCommittedIsSphere
// HLSL-DAG: NvRtCommittedSphereObjectPositionAndRadius
// HLSL-DAG: NvRtCommittedLssObjectPositionsAndRadii
// HLSL-DAG: NvRtCandidateIsNonOpaqueLss
// HLSL-DAG: NvRtCommittedIsLss
// HLSL-DAG: NvRtCommittedLssHitParameter
