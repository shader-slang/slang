//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=e9f2e62d5a5d7efaa803abbf9f002d3308a6da2c7e2ea7013076a2b7c23eb4c4
//META: purpose=legalizeArrayReturnType rewrites array-returning functions into out-parameter form on WGSL
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL forbids array return values. A T[N] foo() becomes a
// function taking a ptr<function, array<T, N>> output parameter.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

int[4] makeArr()
{
    int arr[4] = {1, 2, 3, 4};
    return arr;
}

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a[4] = makeArr();
    outBuf[tid.x] = a[1];
}

// CHECK: fn makeArr
// CHECK-SAME: ptr<function, array<i32
