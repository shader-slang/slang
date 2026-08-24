//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#instrumentation
//META: doc_section_digest=138b9939ba5bbb4e52a45b224e1ac3fdd87ee9defa3e1c0614c72903418b616e
//META: purpose=For Metal targets counting-mode counters are capped at 4 bytes because MSL has no 64-bit atomic fetch-add, and an explicitly requested 64-bit width is capped with a warning.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal-specific cap on the counting path. The 64-bit width
// is requested explicitly, which is the condition the row attaches the
// warning to, so the diagnostic must fire rather than the request being
// silently honoured or silently dropped. E45115 is pinned by code, and its
// text is matched loosely because the message is long.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal -stage compute -entry main -trace-coverage -trace-coverage-counter-width 64

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x);
}

//CHECK: E45115
