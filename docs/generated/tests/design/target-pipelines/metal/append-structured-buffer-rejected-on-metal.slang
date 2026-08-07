//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=917bf080a9394af34b7de447539625595bb7bb0f0256de58d2502cec931c41aa
//META: purpose=Negative (doc-gap pair): `AppendStructuredBuffer<T>` on the Metal compute stage is rejected by `checkEntryPointDecorations` with diagnostic E36107, so `lowerAppendConsumeStructuredBuffers` (which the doc says fires for Metal) never produces observable text in compute emit.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims `lowerAppendConsumeStructuredBuffers` fires for
// Metal (`target != HLSL`), but Phase A's
// `checkEntryPointDecorations` rejects `AppendStructuredBuffer<T>`
// on the `compute` stage with E36107 before the pass can run.
// This pins the documented diagnostic verbatim and corroborates
// the doc-gap noted in README.md.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target metal -entry main -stage compute

AppendStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
//CHECK: E36107
//CHECK: uses features that are not available in 'compute' stage for 'metal' compilation target
{
    outBuf.Append(int(tid.x));
}
