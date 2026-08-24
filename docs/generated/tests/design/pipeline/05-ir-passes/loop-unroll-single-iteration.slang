//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
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
