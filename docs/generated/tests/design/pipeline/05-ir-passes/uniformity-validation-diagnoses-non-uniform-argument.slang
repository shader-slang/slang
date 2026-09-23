//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#loop-transformations
//META: doc_section_digest=5346083df587dd61328c503d8394df501ae34fed3523d17881e22225881336af
//META: purpose=The uniformity analysis runs only under -validate-uniformity and its user-visible output is a diagnostic against the non-uniform value.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the divergence analysis has a diagnostic as its surface.
// `expectUniform` declares its parameter `dynamic_uniform`, and the entry
// point passes it a per-thread value derived from `SV_DispatchThreadID`,
// which the analysis cannot prove uniform. The caret annotation pins the
// diagnostic to the argument position, so the test fails both if the pass
// stops running and if it starts reporting somewhere else.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main -validate-uniformity

RWStructuredBuffer<int> outBuf;

void expectUniform(dynamic_uniform int v)
{
    outBuf[0] = v;
}

[numthreads(1, 1, 1)]
void main(int tid: SV_DispatchThreadID)
{
    expectUniform(tid);
//CHECK:         ^ argument might not be dynamic uniform
//CHECK:         ^ argument for 'v' might not be a dynamic uniform, use `asDynamicUniform()` to silence this warning.
}
