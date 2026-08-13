//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=Boundary: even with a wave intrinsic (WaveActiveSum) in the kernel, the prelude still emits the SLANG_HLSL_ENABLE_NVAPI guard — it does not switch to an unconditional include.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: NV-extension conditional emit — the wave-active
// path is a built-in DXC intrinsic and does not require the
// NVAPI extension include. The guard stays conditional.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint v = WaveActiveSum(tid.x);
    outBuf[tid.x] = v;
}

// CHECK: #ifdef SLANG_HLSL_ENABLE_NVAPI
// CHECK: #include "nvHLSLExtns.h"
// CHECK: #endif
