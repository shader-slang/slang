//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#legalizeentrypointvaryingparamsforcuda
//META: doc_section_digest=d141689276dbc4a39e279f1adba85a6899409f5555430cdbc264fa99d8cea1a1
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
