//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
//META: purpose=With -validate-uniformity off the uniformity pass does not run at all, so the same shader that draws E41201 under the flag compiles clean without it.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "with the flag off the pass does not run at all" half of
// the uniformity row, which is what makes the analysis opt-in rather than
// a permanent cost. The shader is the same shape that draws the
// dynamic-uniform diagnostic under the flag; here it is compiled without
// it. The CHECKs assert the entry point is emitted normally and that no
// uniformity diagnostic text reaches the output.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

RWStructuredBuffer<int> outBuf;

void expectUniform(dynamic_uniform int v)
{
    outBuf[0] = v;
}

[numthreads(1, 1, 1)]
void main(int tid: SV_DispatchThreadID)
{
    expectUniform(tid);
}

// HLSL: void main(
// HLSL-NOT: dynamic uniform
