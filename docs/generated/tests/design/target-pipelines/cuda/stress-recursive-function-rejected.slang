//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=21c9155c4ad2f346c0598d3001ce01acb069faa9bdcbd07e447c0eb95df61d51
//META: purpose=Negative: a self-recursive function is rejected before CUDA emit with the "recursion not allowed" diagnostic (CUDA target does not allow recursion).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: recursive function. The CUDA target does not allow
// recursion (the checkForRecursive* Phase-B validation rejects
// before emit). The diagnostic text is copied verbatim.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int fact(int n)
{
    if (n <= 1) return 1;
    return n * fact(n - 1);
//CHECK: recursion detected in call to 'fact', but the current code generation target does not allow recursion.
//CHECK: E55201
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = fact(int(tid.x));
}
