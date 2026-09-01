//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
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
