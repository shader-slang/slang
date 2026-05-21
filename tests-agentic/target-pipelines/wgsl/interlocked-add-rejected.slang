//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=287321dc153823860396c030325e4ad4c0f6064005fa37b19c8ba53087dcd3e6
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
{
    InterlockedAdd(outBuf[tid.x], 1);
}
