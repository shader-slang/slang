//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-c-cuda-legalization-lowering-phi-elimination
//META: doc_section_digest=4b70b0a669cc9b83124a252d2add0ed8fb3787692f5a88feaade1d3780fda053
//META: purpose=Verifies legalizeArrayReturnType produces a FixedArray<T,16> * out-parameter for N=16 on CUDA emit (upper-edge array-size sweep).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: array-return with a larger size (N=16). The
// rewrite shape is unchanged but the parameter type carries the
// concrete N.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

int[16] makeArr(int n)
{
    int r[16];
    [unroll] for (int i = 0; i < 16; i++) r[i] = n + i;
    return r;
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a[16] = makeArr(int(tid.x));
    outBuf[tid.x] = a[0] + a[15];
}

// CHECK: FixedArray<int, 16>
