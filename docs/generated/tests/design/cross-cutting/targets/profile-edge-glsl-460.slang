//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=e5b9757f49d35cc72a9566431514fc5b19482718219a34a0a4240001ec32891e
//META: purpose=The highest documented GLSL profile (`glsl_460` — top of the GLSL family in slang-profile-defs.h) compiles a vanilla compute shader; the emitted module declares `#version 460`.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Profile upper-edge probe for the GLSL family. `glsl_460` is the top
// of the GLSL PROFILE_VERSION list in slang-profile-defs.h. The
// Profiles section already exercises glsl_450 (mid family); this test
// pins the highest documented version, asserting the back-end emits
// the matching `#version 460` header.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_460 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// GLSL: #version 460
// GLSL: local_size_x = 1
