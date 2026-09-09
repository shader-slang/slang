//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=7d3641e4955ae039b13fce50516f0ed39854eeed3ca279f81a665ace48b92daa
//META: purpose=HLSL-style InterlockedAdd is rejected for the WGSL compute target with E36107 (use Atomic<T> instead)
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// README.md 'Out of scope' documents that HLSL-style
// InterlockedAdd / InterlockedExchange are rejected for WGSL; the
// idiomatic path is Atomic<T> (covered positively by
// atomic-add-buffer.slang). This test asserts the diagnostic
// fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'wgsl' compilation target.
*/
// CHECK: E36107
{
    InterlockedAdd(outBuf[tid.x], 1);
}
