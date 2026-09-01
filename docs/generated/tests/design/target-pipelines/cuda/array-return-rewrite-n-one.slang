//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=4b70b0a669cc9b83124a252d2add0ed8fb3787692f5a88feaade1d3780fda053
//META: purpose=Verifies legalizeArrayReturnType produces a FixedArray<T,1> * out-parameter for the lower-edge N=1 case on CUDA emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: array-return with the smallest non-empty size
// (N=1). The pass still rewrites the function to a `void`
// signature with a `FixedArray<T, 1> *` out-parameter.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

int[1] makeArr(int n) { int r[1]; r[0] = n; return r; }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a[1] = makeArr(int(tid.x));
    outBuf[tid.x] = a[0];
}

// CHECK: __device__ void makeArr{{[_0-9]*}}(int {{.*}}, FixedArray<int, 1>
