//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=ff5496c69314ad29906a734c4d1b9e25bf4607f4a43b1f56af9601044c1031b4
//META: purpose=Negative: a self-recursive function is rejected before CUDA emit with the "recursion not allowed" diagnostic (CUDA target does not allow recursion).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: recursive function. The CUDA target does not allow
// recursion (the checkForRecursive* Phase-B validation rejects
// before emit). The diagnostic text is copied verbatim.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int fact(int n)
{
    if (n <= 1) return 1;
    return n * fact(n - 1);
//CHECK: recursion detected in call to 'fact', but the current code generation target does not allow recursion.
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = fact(int(tid.x));
}
