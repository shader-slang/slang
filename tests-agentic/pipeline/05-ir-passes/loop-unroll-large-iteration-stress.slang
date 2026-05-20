//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=2f4c4839739aa129e6a768311053d73d3691a989f5e00f77e04a70f426d5093f
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
