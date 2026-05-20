//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#legalizeentrypointvaryingparamsforcuda
//META: doc_section_digest=c65a336bb0116ae8b15edc71afa3dd2f75a932f4f071b1ec88bad87a44c9547b
//META: purpose=legalizeEntryPointVaryingParamsForCUDA rewrites SV_DispatchThreadID into the CUDA builtin expression blockIdx * blockDim + threadIdx.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The CUDA-only Phase-C pass `legalizeEntryPointVaryingParamsForCUDA`
// turns the `SV_DispatchThreadID` varying parameter into a derived
// expression over CUDA builtins. The semantic does not survive as
// a literal token.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: blockIdx
// CHECK-SAME: blockDim
// CHECK-SAME: threadIdx
