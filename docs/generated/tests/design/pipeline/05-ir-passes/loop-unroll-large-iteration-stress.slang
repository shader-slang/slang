//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=39b16ddaf934d7d1113295e5d32175dd04362dd53d6fdc0bf63ddf66a80ed159
//META: purpose=Stress: a `[ForceUnroll]` loop with 32 static iterations is fully unrolled — no `for` keyword survives in the GLSL emit.
//META: notes=The default unroll budget is large enough to handle 32 iterations on a single accumulator.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// High-N edge for `[ForceUnroll]`. Thirty-two iterations on a single
// accumulator fits within the default unroll budget and produces no
// residual `for` in the GLSL emit.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [ForceUnroll]
    for (int i = 0; i < 32; ++i)
    {
        s += i;
    }
    buf[0] = s;
}

// GLSL: void main
// GLSL-NOT: for (
// GLSL-NOT: for(
