//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
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
