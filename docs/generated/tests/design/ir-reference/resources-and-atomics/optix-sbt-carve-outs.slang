//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#raytracing-payload
//META: doc_section_digest=67617f4eb68ef6a0c6b50b0fba002690b874dbf4cf412014b8b61a11403f1548
//META: purpose=Two shapes near the SBT worked example do not produce `getOptiXSbtDataPointer`: a module-scope `uniform` global becomes an ordinary `global_param` read out of the CUDA global parameter block, and a `compute` entry point is skipped by the pass so its `uniform` parameter becomes a kernel launch argument.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two explicit carve-outs the raytracing section names, which
// are what make the SBT rule "where the uniform is declared" rather than
// "an entry point has a uniform".
//
// One file carries both shapes and two directives select between them by
// entry point. The closesthit entry reads a module-scope `uniform`: the
// CHECKs assert the CUDA reaches it through the global parameter block and
// assert the *absence* of the SBT accessor, which is the whole point of
// the carve-out. The compute entry takes the uniform as a parameter: the
// CHECKs assert it became a kernel launch argument on the `__global__`
// signature and, again, that no SBT lookup appears. `optixGetSbtDataPointer`
// is a distinctive enough token that a CHECK-NOT on it cannot collide with
// an unrelated substring.

//TEST:SIMPLE(filecheck=GLOBALUNIFORM):-target cuda -stage closesthit -entry ch
//TEST:SIMPLE(filecheck=COMPUTEUNIFORM):-target cuda -stage compute -entry cs

struct Payload
{
    float3 color;
};

uniform float gScale;

RWStructuredBuffer<float> outBuf;

[shader("closesthit")]
void ch(inout Payload p)
{
    p.color = float3(gScale, gScale, gScale);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cs(uint3 tid : SV_DispatchThreadID, uniform float scale)
{
    outBuf[tid.x] = scale;
}

// GLOBALUNIFORM: globalParams_{{[0-9]+}}->gScale_{{[0-9]+}}
// GLOBALUNIFORM-NOT: optixGetSbtDataPointer

// COMPUTEUNIFORM: __global__ void cs{{[A-Za-z0-9_]*}}(float scale_{{[0-9]+}})
// COMPUTEUNIFORM-NOT: optixGetSbtDataPointer
