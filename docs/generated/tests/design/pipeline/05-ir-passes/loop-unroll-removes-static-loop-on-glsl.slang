//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=352225772f21036f2bb726491a17e9298fba9124204787acc9180745246b4332
//META: purpose=The loop-unroll pass unrolls a static `[ForceUnroll]` loop with a compile-time bound — no `for` loop remains in the emit on GLSL.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Loop unroll" row of the loop-transformations table
// (`slang-ir-loop-unroll.cpp`) honors `[ForceUnroll]`. On GLSL the
// resulting emit contains no `for` keyword for the unrolled loop
// (Slang itself unrolls; the downstream compiler does not have to).

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        buf[i] = i;
    }
}

// GLSL: void main
// GLSL-NOT: for (
// GLSL-NOT: for(
