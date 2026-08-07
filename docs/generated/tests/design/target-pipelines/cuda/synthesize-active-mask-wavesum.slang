//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#synthesizeactivemask
//META: doc_section_digest=d5d74eb55ef0b953f5cabff60e64c0b4128f9850b1cc103fada075896a2c9bea
//META: purpose=synthesizeActiveMask threads an explicit warp-mask through a wave-active sum so the CUDA emit calls _waveSum with a synthesized mask operand instead of leaving the implicit IR-level active-mask reference.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `synthesizeActiveMask` (Phase C row 3) is CUDA-only and converts
// IR-level "active mask" references into an explicit warp-mask
// parameter that the emitter forwards into the warp-sync prelude
// helpers. `WaveActiveSum` on CUDA lowers to `_waveSum(<mask>.x,
// value)` per `hlsl.meta.slang` line 18378 — the `_waveSum` call
// shape is the user-visible fingerprint that the mask was
// synthesized and threaded through.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -capability cuda_sm_7_0 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int laneVal = int(tid.x);
    int sum = WaveActiveSum(laneVal);
    outBuf[tid.x] = sum;
}

// CHECK: _waveSum
