//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
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
