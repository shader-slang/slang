//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#entry-point-shape
//META: doc_section_digest=6840c04e5449d96826b3b300b825e82111aff2cddc60c36c5c9944110ef35fdf
//META: purpose=An entry point whose name is not exactly `main` is left alone: it keeps its Slang spelling in the emitted Metal and no rename warning is reported.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out on the Metal rename rule -- it triggers on the
// exact name `main` and on nothing else. The counterpart test compiles the
// same kernel under the name `main` and gets both a renamed symbol and a
// warning; here the name survives verbatim and the compile is silent, which
// is what the CHECK-NOT on the rename code asserts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

//CHECK-NOT: E40100
