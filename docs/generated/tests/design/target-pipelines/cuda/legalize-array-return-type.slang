//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=60a878125219aee6cefce03caff6a6f1ca88b56b8f0f8f9f00a510f58c6ec5ad
//META: purpose=legalizeArrayReturnType rewrites T[N] foo() into a void function with an out FixedArray pointer parameter on CUDA.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Row 17 of the Phase C table: `legalizeArrayReturnType` is gated
// `!isMetalTarget && !isSPIRV` and CUDA is neither. Functions that
// return an array are rewritten to `void` returning the array via
// an `out` pointer parameter (a `FixedArray<T, N> *` in CUDA emit).

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

int[4] makeArr(int n)
{
    int r[4];
    r[0] = n;
    r[1] = n + 1;
    r[2] = n + 2;
    r[3] = n + 3;
    return r;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a[4] = makeArr(int(tid.x));
    outBuf[tid.x] = a[0] + a[3];
}

// CHECK: __device__ void makeArr{{[_0-9]*}}(int {{.*}}, FixedArray<int, 4>
