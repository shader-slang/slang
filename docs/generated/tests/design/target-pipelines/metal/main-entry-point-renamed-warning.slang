//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#entry-point-shape
//META: doc_section_digest=6840c04e5449d96826b3b300b825e82111aff2cddc60c36c5c9944110ef35fdf
//META: purpose=An entry point named exactly `main` is renamed on Metal and the rename is reported as warning E40100 rather than applied silently.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the Metal rename of a `main` entry point is a *reported*
// rewrite, not a silent one: a caller who looks up the kernel by name in the
// compiled library has to learn the name changed. The compile emits exactly
// one diagnostic, so pinning the stable code E40100 is sufficient and no
// other diagnostic is left unannotated.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

//CHECK: E40100
