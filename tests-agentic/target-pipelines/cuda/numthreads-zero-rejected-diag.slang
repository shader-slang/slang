//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=db1bbab6f46f461cd7603b49543603decc6e6d9598a8009d4e83aac70becf0c4
//META: purpose=Negative: [numthreads(0,1,1)] is rejected at check time with an "expected a positive integer" diagnostic before CUDA emit can run.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: numthreads with a zero dimension. The dimension
// must be a positive integer for the attribute to be valid, and
// the check fires before any target-specific emit.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(0, 1, 1)]
/*CHECK:
 ^^^^^^^^^^ invalid 'numthreads' value
 ^^^^^^^^^^ expected a positive integer in 'numthreads' attribute, got '0'
*/
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}
