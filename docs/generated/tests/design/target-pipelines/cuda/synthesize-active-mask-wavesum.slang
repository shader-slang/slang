//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#synthesizeactivemask
//META: doc_section_digest=5c60c1e74c4dc31170e8b898550f9d31292c5de0929d4a245fca93fe5df51dfe
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
