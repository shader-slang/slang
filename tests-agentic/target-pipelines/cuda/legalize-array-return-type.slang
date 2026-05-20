//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=2314d32203f54be9e61ab85654673e194c467242384d7feb583ce0587536f9da
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
