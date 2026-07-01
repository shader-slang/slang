//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=2f4c4839739aa129e6a768311053d73d3691a989f5e00f77e04a70f426d5093f
//META: purpose=Boundary: a `[ForceUnroll]` loop with exactly one static iteration is unrolled — no `for` keyword appears in the GLSL emit.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// One-iter edge of `[ForceUnroll]`. The body is emitted exactly once
// with no surrounding `for`.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    [ForceUnroll]
    for (int i = 0; i < 1; ++i)
    {
        buf[i] = i;
    }
}

// GLSL: void main
// GLSL-NOT: for (
// GLSL-NOT: for(
