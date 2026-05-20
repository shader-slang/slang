//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
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
