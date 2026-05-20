//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=940b62a9312b41f261f039cda6fd50e2e307dcb924128aa8ff8c28474a8c6945
//META: purpose=AppendStructuredBuffer.Append is rejected for the WGSL compute target with E36107 (unavailable features)
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The 'lowerAppendConsumeStructuredBuffers' pass that exists for
// WGSL is unreachable from .slang source because Slang rejects
// AppendStructuredBuffer.Append for the WGSL target with E36107
// before the pass can run. This is documented in BUNDLE.md's
// 'Out of scope' list and is asserted here.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;
AppendStructuredBuffer<uint> appBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'wgsl' compilation target.
*/
{
    appBuf.Append(tid.x);
    outBuf[tid.x] = tid.x;
}
