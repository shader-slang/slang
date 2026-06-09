//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=40b4f2fae77db035dad60bd27fe1b81fe055b218323e5afdfe04adf91d306631
//META: purpose=A Profile binds a `Family` + `Version` to a target. `-profile sm_6_0` parses and the resulting capability set lets a compute shader compile on HLSL; `-profile glsl_450` does the same on GLSL.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Profiles section says a `Profile` carries a Stage, a
// Family + Version, and a target hint that maps onto a capability
// set at the back-end input. We exercise two profile spellings —
// HLSL Shader Model 6.0 and GLSL 450 — and assert the same compute
// kernel compiles under each. The marker on each is the target-
// shaped compute entry point that downstream IR passes emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_450 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HLSL: numthreads(1, 1, 1)
// GLSL: #version 4{{[0-9]+}}
// GLSL: local_size_x = 1
