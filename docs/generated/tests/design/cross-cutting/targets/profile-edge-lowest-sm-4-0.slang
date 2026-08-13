//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
//META: purpose=The lowest documented HLSL profile (`sm_4_0` — the floor of the DX_4_0 family in slang-profile-defs.h) compiles a vanilla compute shader; the profile-driven capability set is satisfied by the trivial kernel.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Profile lower-edge probe. The doc's Profiles section says a Profile
// carries Family + Version. `sm_4_0` is the lowest Family=DX Version
// supported by Slang's profile table. The compute kernel uses only
// features available on every Shader Model, so the capability join
// should succeed at this floor.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_4_0 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HLSL: numthreads(1, 1, 1)
