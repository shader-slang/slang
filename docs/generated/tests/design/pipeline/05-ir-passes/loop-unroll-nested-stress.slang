//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=352225772f21036f2bb726491a17e9298fba9124204787acc9180745246b4332
//META: purpose=Stress: two nested `[ForceUnroll]` loops (4x4 = 16 iterations) both unroll fully — neither outer nor inner `for` keyword survives in the GLSL emit.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Nested-loop stress for loop-unroll. Both loops carry
// `[ForceUnroll]`. The pass must unroll the outer loop's body N
// times, each containing the fully-unrolled inner loop.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        [ForceUnroll]
        for (int j = 0; j < 4; ++j)
        {
            s += i * 4 + j;
        }
    }
    buf[0] = s;
}

// GLSL: void main
// GLSL-NOT: for (
// GLSL-NOT: for(
