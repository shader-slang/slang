//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
//META: purpose=The highest documented HLSL Shader Model profile (`sm_6_9` per DX_6_9 in slang-profile-defs.h) compiles a vanilla compute shader; the upper-edge Profile value parses and selects a valid capability set.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Profile upper-edge probe. `sm_6_9` is the highest concrete Shader
// Model value in slang-profile-defs.h. The Profiles section claims the
// Profile carries Family + Version; this test asserts the maximum
// Version still parses and produces a valid Profile for the back-end.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_9 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HLSL: numthreads(1, 1, 1)
